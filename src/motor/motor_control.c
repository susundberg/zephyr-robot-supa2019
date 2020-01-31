#include <drivers/gpio.h>

#define SUPA_MODULE "moc"
#include "../main.h"
#include "motor_timers.h"


LOG_MODULE_REGISTER(motor_con);

#define FUNCTION_PIN_INDEX 4 
#define MOTOR_PIN_COUNT 5

static struct device* LOCAL_dev[MOTOR_PIN_COUNT];

static const char* LOCAL_names[] = { DT_GPIO_LEDS_MOTOR_1A_GPIOS_CONTROLLER, DT_GPIO_LEDS_MOTOR_1B_GPIOS_CONTROLLER,
                              DT_GPIO_LEDS_MOTOR_2A_GPIOS_CONTROLLER, DT_GPIO_LEDS_MOTOR_2B_GPIOS_CONTROLLER,
                              DT_GPIO_LEDS_MOTOR_FUN_GPIOS_CONTROLLER };
                              
static const int LOCAL_pins[] = { DT_GPIO_LEDS_MOTOR_1A_GPIOS_PIN, DT_GPIO_LEDS_MOTOR_1B_GPIOS_PIN, 
                         DT_GPIO_LEDS_MOTOR_2A_GPIOS_PIN, DT_GPIO_LEDS_MOTOR_2B_GPIOS_PIN,
                         DT_GPIO_LEDS_MOTOR_FUN_GPIOS_PIN };
                         
static const char LOCAL_init_value[] = { 0, 0, 0, 0, 1};
               
void motor_control_function( bool value )
{
    LOG_INF("Setting motor FUN %d", value );
    RET_CHECK( gpio_pin_write( LOCAL_dev[ FUNCTION_PIN_INDEX ], LOCAL_pins[ FUNCTION_PIN_INDEX ], value ) );
    
}

void motor_control_init()
{
                         
    for (int loop = 0; loop < MOTOR_PIN_COUNT; loop ++ )
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
       
       ASSERT( gpio_pin_write( LOCAL_dev[loop], LOCAL_pins[loop], LOCAL_init_value[loop] ) == 0 );
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



