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
    
static void ui_button_receive( uint8_t code )
{
    LOG_INF("UI Button pressed: 0x%02X", code );
}
    
static void ui_main()
{
   
   k_msgq_init( &LOCAL_ui_queue, LOCAL_ui_queue_buffer, 1, LOCAL_ui_queue_n );

   ui_pin_init();
   ir_pins_init( &LOCAL_ui_queue );
    
   LOG_INF("UI thread started!");

   uint8_t loop_led = 0;
  
   while(1)
   {
      u8_t code_new;
      if ( k_msgq_get(&LOCAL_ui_queue, &code_new, 100 ) == 0  )
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
      }
          
      gpio_pin_set( LOCAL_led_out, DT_GPIO_LEDS_LED_GREEN_GPIOS_PIN, loop_led % 2);
      loop_led += 1;
      
   }
}



K_THREAD_DEFINE( ui_thread, OS_DEFAULT_STACKSIZE*2, ui_main, NULL, NULL, NULL,
                 OS_DEFAULT_PRIORITY, 0, K_NO_WAIT);


