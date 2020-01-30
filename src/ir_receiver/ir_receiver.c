#include <zephyr.h>


#include <drivers/gpio.h>

#define SUPA_MODULE "irc"

#include "ir_receiver.h"

LOG_MODULE_REGISTER(ir);

static const int   LOCAL_pin_pins[2] = { DT_GPIO_KEYS_IR_INPUT_GPIOS_PIN, DT_GPIO_LEDS_IR_OUTPUT_GPIOS_PIN };
struct device*     LOCAL_pin_dev[2];
static const char* LOCAL_pin_names[] = { DT_GPIO_KEYS_IR_INPUT_GPIOS_CONTROLLER, DT_GPIO_LEDS_IR_OUTPUT_GPIOS_CONTROLLER };


#define LOCAL_isr_queue_n 64
static char __aligned(4) LOCAL_ir_queue_buffer[ LOCAL_isr_queue_n ];
static struct k_msgq LOCAL_ir_queue;
static struct gpio_callback LOCAL_ir_callback;

#define LOCAL_ir_registery_size 8
static IR_keycode LOCAL_ir_registry_key[ LOCAL_ir_registery_size ];
static IRCmd_callback LOCAL_ir_registry_fun[ LOCAL_ir_registery_size ];
static int LOCAL_ir_registry_n = 0;


static void ir_signal_isr(struct device* gpiob, struct gpio_callback* cb, u32_t pins)
{
    (void)gpiob;
    (void)pins;
    (void)cb;
    static u32_t start_time  = 0;
    static u32_t stop_time   = 0;
    
    static const u32_t threshold_for_binary = 1500000;
    
    stop_time = k_cycle_get_32();
    
    u64_t cycles_spent = 0;
    
    if ( stop_time > start_time )
        cycles_spent = stop_time - start_time;
    else
        cycles_spent = start_time + (0xFFFFFFFF - stop_time);
    
    start_time = stop_time;
    
    u32_t dtime = (u32_t)k_cyc_to_ns_floor64(cycles_spent);
    u8_t value = 0;
    if ( dtime > threshold_for_binary )  
    {
        value = 1;
    }
    else
    {
        value = 0;
    }

    k_msgq_put( &LOCAL_ir_queue, &value, K_NO_WAIT );
    
}



// #define IR_LED_INPUT_FLAGS ( GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE |  | GPIO_INT_DEBOUNCE | GPIO_INT_ACTIVE_HIGH )
#define IR_LED_INPUT_FLAGS ( GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE | GPIO_INT_ACTIVE_HIGH )

static void ir_pins_init()
{
    for (int loop = 0; loop < 2; loop ++ )
    {
       LOCAL_pin_dev[loop] = device_get_binding( LOCAL_pin_names[loop] );
       
       if ( LOCAL_pin_dev[loop] == NULL )
       {
           FATAL_ERROR("Cannot find device: %s", LOCAL_pin_names[loop] );
           return;
       }
    }
    
    int loop = 0;
    RET_CHECK( gpio_pin_configure( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop], IR_LED_INPUT_FLAGS  ) );
    
    gpio_init_callback( &LOCAL_ir_callback, ir_signal_isr, BIT( LOCAL_pin_pins[loop] ) );

    RET_CHECK( gpio_add_callback( LOCAL_pin_dev[loop], &LOCAL_ir_callback) );
    RET_CHECK( gpio_pin_enable_callback( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop]) );

    
       
    loop = 1;
    RET_CHECK ( gpio_pin_configure( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop], GPIO_DIR_OUT ) );
    RET_CHECK ( gpio_pin_write( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop], 0 )  );    
    
    
    
   
}


void ir_receiver_register( IR_keycode code, IRCmd_callback callback )
{
    
    ASSERT( LOCAL_ir_registry_n < LOCAL_ir_registery_size );
    
    LOCAL_ir_registry_fun[ LOCAL_ir_registry_n ] = callback;
    LOCAL_ir_registry_key[ LOCAL_ir_registry_n ] = code;
    LOCAL_ir_registry_n += 1 ;
}


static void handle_received_keycode( u16_t keycode, bool repeated )
{
   for ( int loop = 0; loop < LOCAL_ir_registry_n; loop ++ )
   {
       if ( LOCAL_ir_registry_key[ loop] != keycode )
           continue;
       
       LOCAL_ir_registry_fun[ loop ]( keycode, repeated );
       return;
   }
   LOG_INF("Key 0x%04X not registered", keycode );
}


static void handle_received_code( u8_t* buffer, u32_t buffer_n )
{
    u32_t byte_loop = 0;
    for ( int loop = 0; loop < buffer_n; loop ++ )
    {
        byte_loop       = (loop & 0xF8 ) >> 3;
        u32_t bit_loop  =  loop & 0x07;
        
        u32_t value = buffer[ loop ] << bit_loop;
    
        if ( bit_loop == 0 )
           buffer[ byte_loop ] = value;
        else
           buffer[ byte_loop ] |= value;
    }
    buffer_n = byte_loop + 1;
    
    
    memset( buffer + buffer_n, 0x00, LOCAL_isr_queue_n - buffer_n );
    
    
    static uint16_t last_keycode = 0x00;
    
    if ( buffer_n == 5 )
    {
        static const u8_t LG_MAGIC_0  = 0xB3;
        static const u8_t LG_MAGIC_1  = 0xB0;
        
       if ( buffer[0] == LG_MAGIC_0 && buffer[1] == LG_MAGIC_1 )
       {
            u16_t keycode = buffer[2] | (buffer[3] << 8);
            last_keycode = keycode;
            handle_received_keycode( last_keycode, false );
            return;
       }

    }    
    else if ( buffer_n == 1 )
    {
        if ( ( buffer[0] == 0x7 ) && ( last_keycode != 0x00 ) )
        {

            handle_received_keycode( last_keycode, true  );
            return;
        }
    }
    
    LOG_WRN("Unknown keycode of size %d", buffer_n );
    LOG_HEXDUMP_WRN( buffer, buffer_n, "Code:");

}


static void ir_receiver_main()
{
   
   k_msgq_init( &LOCAL_ir_queue, LOCAL_ir_queue_buffer, 1, LOCAL_isr_queue_n );

   ir_pins_init();
    
   LOG_INF("IR thread started!");

   uint32_t loop = 0;
   u8_t code_buffer[ LOCAL_isr_queue_n ];
   uint32_t code_n = 0;
   static const int MIN_CODE_SIZE = 3;
   while(1)
   {
      u8_t code_new;
      if ( k_msgq_get(&LOCAL_ir_queue, &code_new, 100 ) == 0  )
      {
          code_buffer[ code_n ] = code_new;
          code_n += 1 ;
          if ( code_n >= LOCAL_isr_queue_n )
          {
              handle_received_code( code_buffer, code_n );
              code_n = 0;
          }
      }
      else
      {
          if ( code_n < MIN_CODE_SIZE )
              continue;
          
          handle_received_code( code_buffer, code_n );
          code_n = 0;
      }
          
      gpio_pin_write( LOCAL_pin_dev[1], LOCAL_pin_pins[1], loop % 2);
      loop += 1;
      
   }
}



K_THREAD_DEFINE( ir_thread, OS_DEFAULT_STACKSIZE*2, ir_receiver_main, NULL, NULL, NULL,
                 OS_DEFAULT_PRIORITY, 0, K_NO_WAIT);


