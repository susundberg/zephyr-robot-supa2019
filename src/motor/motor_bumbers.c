#include <drivers/gpio.h>

#define SUPA_MODULE "mob"
#include "../main.h"
#include "./motors.h"

LOG_MODULE_REGISTER(motor_bum);


#define BUMBER_INPUT_FLAGS ( GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW )

static const int   LOCAL_pin_pins[2] = { DT_GPIO_KEYS_BUMBER_LEFT_GPIOS_PIN, DT_GPIO_KEYS_BUMBER_RIGHT_GPIOS_PIN };
static struct device*     LOCAL_pin_dev[2];
static const char* LOCAL_pin_names[] = { DT_GPIO_KEYS_BUMBER_LEFT_GPIOS_CONTROLLER, DT_GPIO_KEYS_BUMBER_RIGHT_GPIOS_CONTROLLER };

static struct gpio_callback LOCAL_callback;


static void ISR_bumber_hit(struct device* gpio, struct gpio_callback* cb, u32_t pins)
{
    ARG_UNUSED(gpio);
    ARG_UNUSED(cb);
    uint32_t value = 0x00;
    
    if ((pins & BIT( LOCAL_pin_pins[0] )) != 0 )
    {
       value = MOTOR_EVENT_BUMBER_LEFT;
       motors_send_cmd( MOTOR_CMD_EV_BUMBER, &value, 1 );
    }
    
    if ((pins & BIT( LOCAL_pin_pins[1] )) != 0 )
    {
       value = MOTOR_EVENT_BUMBER_RIGHT;
       motors_send_cmd( MOTOR_CMD_EV_BUMBER, &value, 1 );
    }
    
    ASSERT_ISR( value != 0x00 );
}

    
void motor_bumber_init()
{
    gpio_init_callback( &LOCAL_callback, ISR_bumber_hit, BIT( LOCAL_pin_pins[0] ) | BIT( LOCAL_pin_pins[1] )  );
    
    for (int loop = 0; loop < 2; loop ++ )
    {
       LOCAL_pin_dev[loop] = device_get_binding( LOCAL_pin_names[loop] );
       
       if ( LOCAL_pin_dev[loop] == NULL )
       {
           FATAL_ERROR("Cannot find device: %s", LOCAL_pin_names[loop] );
           return;
       }
       
       RET_CHECK( gpio_pin_configure( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop], BUMBER_INPUT_FLAGS  ) );
       RET_CHECK( gpio_add_callback( LOCAL_pin_dev[loop], &LOCAL_callback) );
       RET_CHECK( gpio_pin_enable_callback( LOCAL_pin_dev[loop], LOCAL_pin_pins[loop]) );       
    }
}
