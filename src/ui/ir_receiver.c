#include <zephyr.h>


#include <drivers/gpio.h>

#define SUPA_MODULE "irc"

#include "ir_receiver.h"

LOG_MODULE_REGISTER(ir);




static struct gpio_callback LOCAL_ir_callback;




static struct device*  LOCAL_pin_dev;
static struct k_msgq* LOCAL_ir_queue = NULL;



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
        value = UI_QUEUE_IR_1;
    }
    else
    {
        value = UI_QUEUE_IR_1;
    }

    k_msgq_put( LOCAL_ir_queue, &value, K_NO_WAIT );
    
}




void ir_pins_init( struct k_msgq* msg_queue )
{
    LOCAL_ir_queue = msg_queue;
    
    DEV_GET_CHECK( LOCAL_pin_dev, DT_GPIO_KEYS_IR_INPUT_GPIOS_CONTROLLER );
    RET_CHECK( gpio_pin_configure( LOCAL_pin_dev, DT_GPIO_KEYS_IR_INPUT_GPIOS_PIN, GPIO_INPUT | DT_GPIO_KEYS_IR_INPUT_GPIOS_FLAGS  ) );

    gpio_init_callback( &LOCAL_ir_callback, ir_signal_isr, BIT( DT_GPIO_KEYS_IR_INPUT_GPIOS_PIN ) );

    RET_CHECK( gpio_add_callback( LOCAL_pin_dev, &LOCAL_ir_callback) );
    RET_CHECK( gpio_pin_interrupt_configure( LOCAL_pin_dev, DT_GPIO_KEYS_IR_INPUT_GPIOS_PIN, GPIO_INT_EDGE_TO_ACTIVE) );

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
    
    memset( buffer + buffer_n, 0x00, MAX_IR_CODE_SIZE - buffer_n );
    static uint16_t last_keycode = 0x00;
    
    if ( buffer_n == 5 )
    {
        static const u8_t LG_MAGIC_0  = 0xB3;
        static const u8_t LG_MAGIC_1  = 0xB0;
        
       if ( buffer[0] == LG_MAGIC_0 && buffer[1] == LG_MAGIC_1 )
       {
            u16_t keycode = buffer[2] | (buffer[3] << 8);
            last_keycode = keycode;
            ui_received_keycode( last_keycode, true, false );
            return;
       }
    }    
    else if ( buffer_n == 1 )
    {
        if ( ( buffer[0] == 0x7 ) && ( last_keycode != 0x00 ) )
        {

            ui_received_keycode( last_keycode, true, true  );
            return;
        }
    }
    
    LOG_WRN("Unknown keycode of size %d", buffer_n );
    LOG_HEXDUMP_WRN( buffer, buffer_n, "Code:");
}


          
void ir_receiver_code( uint8_t code_new )
{
   static u8_t code_buffer[ MAX_IR_CODE_SIZE ];
   static uint32_t code_n = 0;
   static const uint32_t MIN_CODE_SIZE = 3;
    
   if (code_new != 0x00 )
   {
        code_buffer[ code_n ] = ( code_new == UI_QUEUE_IR_1 );
        code_n += 1 ;
        
        if ( code_n >= MAX_IR_CODE_SIZE )
        {
           handle_received_code( code_buffer, code_n );
           code_n = 0;
        }
   }
   else
   {
        if ( code_n < MIN_CODE_SIZE )
        {
            code_n = 0;
            return; 
        }

        handle_received_code( code_buffer, code_n );
        code_n = 0;
   }
   
}
