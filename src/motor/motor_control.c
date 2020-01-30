#include <drivers/gpio.h>

#define SUPA_MODULE "moc"
#include "../main.h"
#include "motor_timers.h"


LOG_MODULE_REGISTER(motor_con);


static struct device* LOCAL_dev[4];
static const char* LOCAL_names[] = { DT_GPIO_LEDS_MOTOR_1A_GPIOS_CONTROLLER, DT_GPIO_LEDS_MOTOR_1B_GPIOS_CONTROLLER,
                              DT_GPIO_LEDS_MOTOR_2A_GPIOS_CONTROLLER, DT_GPIO_LEDS_MOTOR_2B_GPIOS_CONTROLLER };
                              
static const int LOCAL_pins[] = { DT_GPIO_LEDS_MOTOR_1A_GPIOS_PIN, DT_GPIO_LEDS_MOTOR_1B_GPIOS_PIN, 
                         DT_GPIO_LEDS_MOTOR_2A_GPIOS_PIN, DT_GPIO_LEDS_MOTOR_2B_GPIOS_PIN };

                         
void motor_control_init()
{
                         
    for (int loop = 0; loop < 4; loop ++ )
    {
       LOCAL_dev[loop] = device_get_binding( LOCAL_names[loop] );
       
       if ( LOCAL_dev[loop] == NULL )
       {
           FATAL_ERROR("NODEV: %s", LOCAL_names[loop] );
           return;
       }
       
       if ( gpio_pin_configure( LOCAL_dev[loop], LOCAL_pins[loop], GPIO_DIR_OUT) != 0 )
       {
           FATAL_ERROR("NOCONF: %s-%d", LOCAL_names[loop], LOCAL_pins[loop] );
           return;
       }
       
       ASSERT( gpio_pin_write( LOCAL_dev[loop], LOCAL_pins[loop], 0 ) == 0 );
    }
}

static void set_motor_control( int motor, bool mot_a, bool mot_b )
{
    ASSERT( gpio_pin_write( LOCAL_dev[2*motor  ], LOCAL_pins[2*motor], mot_a ) == 0 );
    ASSERT( gpio_pin_write( LOCAL_dev[2*motor+1], LOCAL_pins[2*motor + 1], mot_b ) == 0 ); 
}

void motor_control_enable( int motor, bool reverse )
{
    LOG_INF("Motor ENABLE %d=%d", motor, reverse );
    if ( reverse == false )
    {
        set_motor_control( motor, true, false );
    }
    else
    {
        set_motor_control( motor, false, true );
    }
    
}

void motor_control_disable_all()
{
    motor_control_disable( 0 );
    motor_control_disable( 1 );
}

void motor_control_disable( int motor )
{
    LOG_INF("Motor DISABLE %d", motor );
    set_motor_control( motor, false, false );
    motor_timers_set_speed( motor, 0 );
}



