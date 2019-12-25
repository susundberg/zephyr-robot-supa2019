#include <zephyr.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>
#include <logging/log.h>

#include "motors.h"
#include "motor_ramp.h"
#include "motor_timers.h"

#include "../main.h"


LOG_MODULE_REGISTER(motor);



typedef enum
{
    MOTOR_STATUS_IDLE,
    MOTOR_STATUS_DRIVE,
} Motor_state ;



#define MOTOR_CONTROL_LOOP_MS 10
static const float MOTOR_CONTROL_LOOP_MS_P1 = 1000.0f / MOTOR_CONTROL_LOOP_MS;
#define MOTOR_MAX_CONTROL_SPEED_CM_PER_SEC 1000.0f



Motor_state GLOBAL_motor_state;


static void motor_cmd_stop( Motor_cmd* cmd )
{
    (void)(cmd);
}


static void motor_cmd_drive( Motor_cmd* cmd )
{
    static Motor_ramp LOCAL_target[2];
    s64_t LOCAL_drive_start;
    float LOCAL_pos_cm[2];
    

    uint32_t flags = (uint32_t) cmd->params[2];
    
    LOG_INF("Drive %d %d flags 0x%X", cmd->params[0], cmd->params[1], flags );

    for ( int loop = 0; loop < 2; loop ++ )
    {
        int32_t pos = cmd->params[0];
        bool reverse = false;    
        if ( pos < 0 )
        { 
            reverse = true;
            pos = pos*-1;
        }
        
        motor_ramp_init( &LOCAL_target[0], MOTOR_MAX_ACC_CM_SS, MOTOR_MAX_SPEED_CM_S, pos );
        motor_control_enable( loop, reverse );
    }    
            
    
    
    motor_timers_set_location_zero( LOCAL_pos_cm );
    
    LOCAL_drive_start = k_uptime_get();
    
    float drive_stop_sec  = MAX( motor_ramp_fulltime( &LOCAL_target[0] ) , motor_ramp_fulltime( &LOCAL_target[1] ) );
    
    while(1)
    {
        
        // Get new location
        s64_t milliseconds_spent = k_uptime_delta(&LOCAL_drive_start); 
        float time_sec = milliseconds_spent / 1000.0f;
        
        if ( time_sec > drive_stop_sec )
        {
            LOG_INF("Motor ramp fulltime (%d) reached, stop!", (int)drive_stop_sec  );
            break;
        }
        
        motor_timers_get_location( LOCAL_pos_cm );
        
        for ( int loop = 0; loop < 2; loop ++ )
        {
            float pos_cm   = motor_ramp_location( &LOCAL_target[loop], time_sec );
            float speed_cm_per_sec = (pos_cm - LOCAL_pos_cm[loop])*MOTOR_CONTROL_LOOP_MS_P1;

            if ( speed_cm_per_sec > MOTOR_MAX_CONTROL_SPEED_CM_PER_SEC || speed_cm_per_sec < -MOTOR_MAX_CONTROL_SPEED_CM_PER_SEC )
            {
                FATAL_ERROR("Motor %d speed invalid %d!", loop, (int)speed_cm_per_sec );
                break;
            }
            motor_timers_set_speed( loop, speed_cm_per_sec );
        }
        
        Motor_cmd* cmd = k_fifo_get(&GLOBAL_motor_fifo, MOTOR_CONTROL_LOOP_MS ); // Wait for commands POLL INTERVAL
        
        if ( cmd != NULL )
        {
            LOG_INF("Command %d received while driving, stopping!", cmd->opcode );
            k_free( cmd );
            break;
        }
    }
    
    // make sure speed is zero.
    motor_cmd_stop( NULL );
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
                GLOBAL_motor_state = MOTOR_STATUS_DRIVE;
                motor_cmd_drive( cmd );
                GLOBAL_motor_state = MOTOR_STATUS_IDLE;
                break;
                
            case MOTOR_CMD_STOP:
                motor_cmd_stop( cmd );
                GLOBAL_motor_state = MOTOR_STATUS_IDLE;
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




