#include <zephyr.h>


#include <drivers/gpio.h>

#define SUPA_MODULE "uim"

#include "ir_receiver.h"

LOG_MODULE_REGISTER(ui);

#define LOCAL_ui_queue_n 64
static char __aligned(4) LOCAL_ui_queue_buffer[ LOCAL_ui_queue_n ];

static struct device*  LOCAL_led_out;
static struct k_msgq   LOCAL_ui_queue;

static struct gpio_callback LOCAL_button_cb[2];
static struct device*       LOCAL_button_dev[2];



static int LOCAL_ui_registry_n = 0;
static UI_keycode LOCAL_ui_registry_key[ MAX_IR_REGISTRY_SIZE ];
static UICmd_callback LOCAL_ui_registry_fun[ MAX_IR_REGISTRY_SIZE ];


void ui_receiver_register( UI_keycode code, UICmd_callback callback )
{
    ASSERT( LOCAL_ui_registry_n < MAX_IR_REGISTRY_SIZE );
    LOCAL_ui_registry_fun[ LOCAL_ui_registry_n ] = callback;
    LOCAL_ui_registry_key[ LOCAL_ui_registry_n ] = code;
    LOCAL_ui_registry_n += 1 ;
}

static u32_t ui_received_keycode_find( u16_t keycode )
{
   for ( int loop = 0; loop < LOCAL_ui_registry_n; loop ++ )
   {
       if ( LOCAL_ui_registry_key[ loop] != keycode )
           continue;
       return loop;
   }
   return LOCAL_ui_registry_n + 1;
}

void ui_received_keycode( u16_t keycode, bool is_ir, bool extra )
{
       static u32_t LOCAL_last_ir_key       = 0x00;
           
       if ( keycode == 0x00 )
       { // means that no IR key is active
           
          if ( LOCAL_last_ir_key != 0x00 )
          {
              u32_t old_index = ui_received_keycode_find( LOCAL_last_ir_key );
              ASSERT( LOCAL_last_ir_key < LOCAL_ui_registry_n );
              
              LOCAL_ui_registry_fun[ old_index ]( LOCAL_last_ir_key, false );
              LOCAL_last_ir_key = 0x00;
              
          }
          return;
       }    
       
       // this is continuation button press
       if ( is_ir && extra == true )
           return;
           
       u32_t index = ui_received_keycode_find( keycode );
       if ( index >= LOCAL_ui_registry_n )
       {
            LOG_INF("Key 0x%04X not registered", keycode );
            return;
       }
       
       bool pressed = false;
       if ( is_ir )
       {
           LOCAL_last_ir_key = keycode;
           pressed = true;
       }
       else
       {   // on button we have direct value
           pressed = extra;
       }
       
       LOCAL_ui_registry_fun[ index ]( keycode, pressed );
       return;
   
}


static void ui_button_signal_isr(struct device* dev, struct gpio_callback* cb, u32_t pins)
{
    
    (void)pins;
    uint32_t event = 0x00;
    uint32_t pin;
    if ( pins & BIT(DT_GPIO_KEYS_BUTTON_SW0_GPIOS_PIN) )
    {
        event = UI_QUEUE_BUTTON_0;
        pin   = DT_GPIO_KEYS_BUTTON_SW0_GPIOS_PIN;
    }
    else
    {
        event = UI_QUEUE_BUTTON_1;
        pin = DT_GPIO_KEYS_BUTTON_SW1_GPIOS_PIN;
    }
    
    uint32_t pin_value = gpio_pin_get( dev, pin );
    uint32_t queue_value = event | (UI_QUEUE_BUTTON_ACT*pin_value);
    k_msgq_put( &LOCAL_ui_queue, &queue_value, K_NO_WAIT );
}

static void ui_pin_init_button( struct device** dev, struct gpio_callback* callback, const char* controller, uint32_t pin, uint32_t flags )
{
    DEV_GET_CHECK( (*dev), controller );
    
    RET_CHECK( gpio_pin_configure( (*dev), pin, GPIO_INPUT | flags ) );
    gpio_init_callback( callback, ui_button_signal_isr, BIT( pin ) );
    RET_CHECK( gpio_add_callback( (*dev), callback) );
    RET_CHECK( gpio_pin_interrupt_configure( (*dev), pin, GPIO_INT_EDGE_BOTH) );
}

static void ui_pin_init()
{
    DEV_GET_CHECK( LOCAL_led_out, DT_GPIO_LEDS_LED_GREEN_GPIOS_CONTROLLER );

    RET_CHECK ( gpio_pin_configure( LOCAL_led_out, DT_GPIO_LEDS_LED_GREEN_GPIOS_PIN, GPIO_OUTPUT | DT_GPIO_LEDS_LED_GREEN_GPIOS_FLAGS ) );
    RET_CHECK ( gpio_pin_set( LOCAL_led_out, DT_GPIO_LEDS_LED_GREEN_GPIOS_PIN, 0 )  );   
    
    ui_pin_init_button( &LOCAL_button_dev[0], &LOCAL_button_cb[0], 
                        DT_GPIO_KEYS_BUTTON_SW0_GPIOS_CONTROLLER, DT_GPIO_KEYS_BUTTON_SW0_GPIOS_PIN, DT_GPIO_KEYS_BUTTON_SW0_GPIOS_FLAGS );
    ui_pin_init_button( &LOCAL_button_dev[1], &LOCAL_button_cb[1], 
                        DT_GPIO_KEYS_BUTTON_SW1_GPIOS_CONTROLLER, DT_GPIO_KEYS_BUTTON_SW1_GPIOS_PIN, DT_GPIO_KEYS_BUTTON_SW1_GPIOS_FLAGS );
}   
    
static void ui_button_receive( uint8_t code_raw )
{
    LOG_INF("UI Button pressed: 0x%02X", code_raw );
    u16_t code_keycode = 0x00;
    
    if ( code_raw & UI_QUEUE_BUTTON_0 )
    {
        code_keycode = UI_SW_0;
    }
    else if (code_raw & UI_QUEUE_BUTTON_1 )
    {
        code_keycode = UI_SW_1;
    }
    else
    {
        ASSERT(0);
    }
    
    ui_received_keycode( code_keycode, false, code_raw & UI_QUEUE_BUTTON_ACT );
}

static u8_t LOCAL_current_state = 0;
static u8_t LOCAL_loop_led = 0;
 
static void ui_state_loop()
{
  
   
   
   uint8_t value = 0;
    
   if ( ( LOCAL_current_state & 0x10 ) ) 
   {
       
       if ( LOCAL_loop_led & 0x10 ) // delay period (~3sec) for not blinkin
       {
           value = 0;
       }
       else
       {  // blink the code.
           uint8_t count        = (LOCAL_loop_led & 0x0F) >> 1;
           uint8_t count_wanted = LOCAL_current_state & 0x0F;
           value = ( count < count_wanted ) ? 1 : 0 ;
           value = value & ( LOCAL_loop_led & 0x01 );
       }
   }
   
   else
   { // Normal states, 
       if ( LOCAL_current_state == UI_STATE_PROGRAM_RUN ) 
       {
           value = LOCAL_loop_led & 0x01; // blink fast
       }
       else
       {
           value = LOCAL_loop_led & 0x04; // blink slooow
       }
   }
  
   
   gpio_pin_set( LOCAL_led_out, DT_GPIO_LEDS_LED_GREEN_GPIOS_PIN, value );
   LOCAL_loop_led += 1;
}


void ui_signal_state( UI_status state )
{
    LOG_INF("UI set led state: 0x%02X", state );
    LOCAL_current_state = state;
    LOCAL_loop_led = 0;
}


static void ui_main()
{
   
   k_msgq_init( &LOCAL_ui_queue, LOCAL_ui_queue_buffer, 1, LOCAL_ui_queue_n );

   ui_pin_init();
   ir_pins_init( &LOCAL_ui_queue );
    
   LOG_INF("UI thread started!");


  
   while(1)
   {
      u8_t code_new;
      if ( k_msgq_get(&LOCAL_ui_queue, &code_new, K_MSEC( 200 ) ) == 0  )
      {
          if ( code_new & UI_QUEUE_IR_MASK)
          {
             ir_receiver_code( code_new );
          }
          else if ( code_new & UI_QUEUE_BUTTON_MASK)
          {
              ui_button_receive( code_new );
          }
      }
      else
      {
          ir_receiver_code( 0x00 );
          ui_received_keycode( 0x00, true, false );
      }
      
      ui_state_loop();
          
      
   }
}



K_THREAD_DEFINE( ui_thread, OS_DEFAULT_STACKSIZE, ui_main, NULL, NULL, NULL,
                 OS_DEFAULT_PRIORITY, 0, 0);


