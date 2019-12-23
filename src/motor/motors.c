#include <zephyr.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>

#include <logging/log.h>

#include "motors.h"
#include "motor_ramp.h"

#include "../main.h"


LOG_MODULE_REGISTER(motors);


Motor_ramp LOCAL_target[2];

struct k_fifo LOCAL_fifo;


static void motor_cmd_drive( Motor_cmd* cmd )
{
    LOG_INF("Drive %d %d", cmd->params[0], cmd->params[1] );

    motor_ramp_init( &LOCAL_target[0], MOTOR_MAX_ACC_CM_S, MOTOR_MAX_SPEED_CM_SS, cmd->params[0] );
    motor_ramp_init( &LOCAL_target[1], MOTOR_MAX_ACC_CM_S, MOTOR_MAX_SPEED_CM_SS, cmd->params[1] );
                     
}

static void motor_cmd_stop( Motor_cmd* cmd )
{
    (void)(cmd);
}

struct k_fifo GLOBAL_motor_fifo;


void motors_main()
{
    k_fifo_init(&GLOBAL_motor_fifo);
    
    motor_timers_init();
    
    LOG_INF("Motor app started!");
    
    
    while( true )
    {
        
        Motor_cmd* cmd = k_fifo_get(&GLOBAL_motor_fifo, K_FOREVER);
        LOG_INF("Motor execute %d", cmd->opcode );
        
        switch( cmd->opcode )
        {
            
            case MOTOR_CMD_DRIVE:
                motor_cmd_drive( cmd );
                break;
                
            case MOTOR_CMD_STOP:
                motor_cmd_stop( cmd );
                break;
            
            default:
                FATAL_ERROR("Invalid cmd: %d", cmd->opcode );
                break;
        }
        k_free( cmd );
    }
}



K_THREAD_DEFINE( motor_thread, OS_DEFAULT_STACKSIZE, motors_main, NULL, NULL, NULL,
                 OS_DEFAULT_PRIORITY, 0, K_NO_WAIT);




