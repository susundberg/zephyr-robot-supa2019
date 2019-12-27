#include <zephyr.h>
#include <math.h>

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

struct k_fifo GLOBAL_motor_fifo;
Motor_state GLOBAL_motor_state;


static void motor_cmd_stop( Motor_cmd* cmd )
{
    (void)(cmd);
    motor_control_disable_all();
}


static void drive_time_start( s64_t* timer )
{
    *timer = k_uptime_get();
}

static float drive_time_get( const s64_t* timer )
{
    s64_t tmp = *timer;
    s64_t milliseconds_spent = k_uptime_delta(&tmp);
    return milliseconds_spent/1000.0f;
}


#include <stdio.h>

static void motor_cmd_test( Motor_cmd* cmd )
{
    s64_t drive_start_ticks; 
    
    float position_cm[2];
    float pos_old[2] = { 0.0f, 0.0f };
    motor_timers_set_location_zero( position_cm );
    
    
    drive_time_start( &drive_start_ticks );
    
    float const_start = cmd->params[0];
    float const_stop  = const_start + cmd->params[1];
    float all_done    = const_stop + cmd->params[0];
    
    
    float const_speed = cmd->params[2];
    float acc_ramp = cmd->params[2] / cmd->params[0];
    
    LOG_INF("Acceleration ramp %d s  const %d s  -> const speed: %d cm/sec acc: %d cm/sec^2", 
            ROUND_INT(const_start), ROUND_INT(const_stop - const_start),  ROUND_INT(const_speed), ROUND_INT(acc_ramp) );

    const float time_diff_sec = 0.1;
    
    for ( int loop = 0; loop < 2; loop ++ )
       motor_control_enable( loop, false );
    
    while(1)
    {
        
        // Get new location
        float time_sec = drive_time_get( &drive_start_ticks ); 
        float speed_target;
        float speed_obs[2];
        
        if ( time_sec < const_start )
        {
            speed_target = time_sec * acc_ramp;
        }
        else if ( time_sec >= const_start && time_sec < const_stop )
        {
            speed_target = const_speed;
        }
        else if ( time_sec >= const_stop && time_sec < all_done )
        {
            float ramp_time = time_sec  - const_stop;
            speed_target = const_speed - ramp_time*acc_ramp;
        }
        else
        {
            LOG_INF("Motor test ramp done, bail out!");
            break;
        }
        
        for ( int loop = 0; loop < 2; loop ++ )
           motor_timers_set_speed( loop, speed_target );

        k_sleep( time_diff_sec * 1000 );
        
        
        motor_timers_get_location( position_cm );
        
        for ( int loop = 0; loop < 2; loop ++ )
           speed_obs[loop] = ( position_cm[loop]  - pos_old[loop] ) / time_diff_sec;
        
        memcpy( pos_old, position_cm, sizeof(float)*2 );
        
        printf("T=%0.1f - pos: %0.1f %0.1f -- speed: %0.1f %0.1f - target: %0.1f\n", time_sec, position_cm[0], position_cm[1], speed_obs[0], speed_obs[1], speed_target ); 
    }
    
    motor_cmd_stop(NULL);
    motor_timers_get_location( position_cm );
    LOG_INF("Final position: %d %d", ROUND_INT( position_cm[0] ), ROUND_INT( position_cm[1])  ); 
     
}


static void motor_cmd_drive( Motor_cmd* cmd )
{
    static Motor_ramp LOCAL_target[2];
    s64_t drive_start_ticks;
    float position_cm[2];
    

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
            
    
    
    motor_timers_set_location_zero( position_cm );
    
    drive_time_start( &drive_start_ticks );
    
    float drive_stop_sec  = MAX( motor_ramp_fulltime( &LOCAL_target[0] ) , motor_ramp_fulltime( &LOCAL_target[1] ) );
    
    while(1)
    {
        
        // Get new location
        float time_sec = drive_time_get( &drive_start_ticks );
        
        if ( time_sec > drive_stop_sec )
        {
            LOG_INF("Motor ramp fulltime (%d) reached, stop!", (int)drive_stop_sec  );
            break;
        }
        
        motor_timers_get_location( position_cm );
        
        for ( int loop = 0; loop < 2; loop ++ )
        {
            float pos_cm   = motor_ramp_location( &LOCAL_target[loop], time_sec );
            float speed_cm_per_sec = (pos_cm - position_cm[loop])*MOTOR_CONTROL_LOOP_MS_P1;

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



void motors_main()
{
    k_fifo_init(&GLOBAL_motor_fifo);
    
    motor_timers_init();
    motor_control_init();
    
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
                
            case MOTOR_CMD_TEST:
                GLOBAL_motor_state = MOTOR_STATUS_DRIVE;
                motor_cmd_test( cmd );
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




