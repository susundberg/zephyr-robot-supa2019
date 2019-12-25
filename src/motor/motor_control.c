#include <drivers/gpio.h>

#include "../main.h"
#include "motor_timers.h"


LOG_MODULE_REGISTER(motor_con);


struct device* LOCAL_dev[4];

void motor_control_init()
{
    const char* devices[] = { DT_GPIO_LEDS_MOTOR_1A_GPIOS_CONTROLLER, DT_GPIO_LEDS_MOTOR_1B_GPIOS_CONTROLLER,
                              DT_GPIO_LEDS_MOTOR_2A_GPIOS_CONTROLLER, DT_GPIO_LEDS_MOTOR_2B_GPIOS_CONTROLLER };
                              
    const int pins[] = { DT_GPIO_LEDS_MOTOR_1A_GPIOS_PIN, DT_GPIO_LEDS_MOTOR_1B_GPIOS_PIN, 
                         DT_GPIO_LEDS_MOTOR_2A_GPIOS_PIN, DT_GPIO_LEDS_MOTOR_2B_GPIOS_PIN };
                         
    for (int loop = 0; loop < 4; loop ++ )
    {
       LOCAL_dev[loop] = device_get_binding( devices[loop] );
       
       if ( LOCAL_dev[loop] == NULL )
       {
           FATAL_ERROR("Cannot find device: %s", devices[loop] );
           return;
       }
       
       if ( gpio_pin_configure( LOCAL_dev[loop], pins[loop], GPIO_DIR_OUT) != 0 )
       {
           FATAL_ERROR("Cannot configure device: %s-%d", devices[loop], pins[loop] );
           return;
       }
       
       ASSERT( gpio_pin_write( LOCAL_dev[loop], pins[loop], 0 ) == 0 );
    }
}

void motor_control_enable( int motor, bool reverse )
{
    LOG_INF("Motor ENABLE %d=%d", motor, reverse );
    
}

void motor_control_disable_all()
{
    motor_control_disable( 0 );
    motor_control_disable( 1 );
}

void motor_control_disable( int motor )
{
    LOG_INF("Motor DISABLE %d", motor );
    motor_timers_set_speed( motor, 0 );
}



