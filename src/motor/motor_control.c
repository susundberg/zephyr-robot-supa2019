
#include "../main.h"
#include "motor_timers.h"


LOG_MODULE_REGISTER(motor_con);


void motor_control_init();

void motor_control_enable( int motor, bool reverse )
{
    LOG_INF("Motor DIR  %d=%d", motor, reverse );
    
}

void motor_control_disable( int motor )
{
    (void)motor;
}



