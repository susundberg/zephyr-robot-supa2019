
#include "../main.h"
#include "motor_timers.h"


LOG_MODULE_REGISTER(motor_con);


void motor_control_init()
{
    // FIXME: GPIO enable
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



