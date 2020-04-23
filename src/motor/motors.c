#include <zephyr.h>
#include <math.h>

#include <drivers/gpio.h>
#include <logging/log.h>

#define SUPA_MODULE "mom"

#include "../main.h"
#include "motors.h"
#include "motor_ramp.h"
#include "motor_timers.h"
#include "../pid/pid.h"

LOG_MODULE_REGISTER(motor);


typedef struct
{
  int32_t         params[3];
  Motor_cmd_type  opcode; 
} Motor_cmd;

typedef enum
{
    MOTOR_STATUS_IDLE,
    MOTOR_STATUS_DRIVE,
} Motor_state;


#define LOCAL_queue_size 8
static Motor_cmd __aligned(4) LOCAL_queue_buffer[ LOCAL_queue_size ];
static struct k_msgq LOCAL_queue;

static Motor_cmd_done_callback LOCAL_motor_callback = NULL;

#define MOTOR_CONTROL_LOOP_MS 10
static const float MOTOR_CONTROL_LOOP_DT_S = MOTOR_CONTROL_LOOP_MS / 1000.0f;
#define MOTOR_PWM_SANITY_CHECK_LIMIT 60000

static PidController LOCAL_pid[2];


void motors_pid_init()
{
    // Measured by trial and error
   static const float PID_COEFFS[3] = { 30.0f, 60.0f, 0.4f };
   
   for ( int loop = 0; loop < 2; loop ++ )
   {
       pid_control_setup( &LOCAL_pid[loop], PID_COEFFS[0], PID_COEFFS[1], PID_COEFFS[2], MOTOR_CONTROL_LOOP_DT_S );
   }
}

void motors_set_pid( int motor, const float* coeff )
{
    LOCAL_pid[motor].coeff_p = coeff[0];
    LOCAL_pid[motor].coeff_i = coeff[1];
    LOCAL_pid[motor].coeff_d = coeff[2];
    
}

void motors_set_callback( Motor_cmd_done_callback callback )
{
    if ( callback != NULL )
    {
       ASSERT( LOCAL_motor_callback == NULL );
    }
    LOCAL_motor_callback = callback ;
}

void motors_send_cmd( uint32_t opcode, void* params, uint32_t nparams )
{

    Motor_cmd cmd; 
    ASSERT_ISR( sizeof(float) == sizeof(uint32_t) );
    ASSERT_ISR( nparams <= MOTOR_MAX_PARAMS );
    
    memset( &cmd, 0x00, sizeof(cmd));

    cmd.opcode = opcode;
    memcpy( cmd.params, params, sizeof(uint32_t)*nparams );
    
    ASSERT_ISR( k_msgq_put( &LOCAL_queue, &cmd, K_NO_WAIT ) == 0 );
}

static const Motor_cmd* motor_queue_get( uint32_t wait_time )
{
   static Motor_cmd cmd;
   
   while (1)
   {
       int ret = k_msgq_get(&LOCAL_queue, &cmd, wait_time );
      if (  ret == 0 )
          return &cmd;
      
      if ( ret == -ENOMSG )
          continue;
      
      return NULL;
   }   
}


void motors_abort()
{
    motor_timers_abort();
}

static void motor_cmd_stop( const Motor_cmd* cmd )
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

static void motor_cmd_test( const Motor_cmd* cmd )
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
        
        uint32_t motor_pwm[2];
        
        for ( int loop = 0; loop < 2; loop ++ )
        {
           motor_pwm[loop] = motor_timers_set_speed( loop, speed_target*25.0f );
        }

        k_sleep( time_diff_sec * 1000 );
        
        
        motor_timers_get_location( position_cm );
        
        for ( int loop = 0; loop < 2; loop ++ )
           speed_obs[loop] = ( position_cm[loop]  - pos_old[loop] ) / time_diff_sec;
        
        memcpy( pos_old, position_cm, sizeof(float)*2 );
        
        printf("T=%0.1f - pos: %0.1f %0.1f -- speed: %0.1f %0.1f - target: %0.1f - pwm %d %d\n", 
               (double)time_sec, (double)position_cm[0], (double)position_cm[1], (double)speed_obs[0], (double)speed_obs[1], 
               (double)speed_target, motor_pwm[0], motor_pwm[1] ); 
    }
    
    motor_cmd_stop(NULL);
    motor_timers_get_location( position_cm );
    LOG_INF("Final position: %d %d", ROUND_INT( position_cm[0] ), ROUND_INT( position_cm[1])  ); 
     
}


static void motor_cmd_drive( float* distances, float max_speed, bool use_bumbers )
{
    static Motor_ramp LOCAL_target[2];
    s64_t drive_start_ticks;
    float position_cm[2];
    
    Motor_cmd_type end_event; 
    
    
    LOG_INF("Drive %d %d mm", ROUND_INT(distances[0]*10.0f), ROUND_INT(distances[1]*10.0f) );

    for ( int loop = 0; loop < 2; loop ++ )
    {
        pid_control_clear( &LOCAL_pid[loop] );
        
        bool reverse = false;    
        if ( distances[loop] < 0 )
        { 
            reverse = true;
            distances[loop] = distances[loop]*-1;
        }
        
        motor_ramp_init( &LOCAL_target[loop], MOTOR_MAX_ACC_CM_SS, max_speed, distances[loop] );
        motor_control_enable( loop, reverse );
    }    
            
    
    
    motor_timers_set_location_zero( position_cm );
    
    drive_time_start( &drive_start_ticks );
    
    float drive_stop_sec  = MAX( motor_ramp_fulltime( &LOCAL_target[0] ) , motor_ramp_fulltime( &LOCAL_target[1] ) );
    LOG_INF("Expected drive time: %d s", ROUND_INT(drive_stop_sec));
    
    uint32_t max_pwm[2] = { 0, 0};
    while(1)
    {
        
        // Get new location
        float time_sec = drive_time_get( &drive_start_ticks );
        
        if ( time_sec > drive_stop_sec )
        {
            LOG_INF("Motor ramp fulltime (%d) reached, stop!", (int)drive_stop_sec  );
            end_event = MOTOR_CMD_EV_DONE;
            break;
        }
        
        motor_timers_get_location( position_cm );
        int mot_pwm[2];
        for ( int loop = 0; loop < 2; loop ++ )
        {
            float position_target_cm   = motor_ramp_location( &LOCAL_target[loop], time_sec );
            
            float pwm_out_flt = pid_control_step( &LOCAL_pid[loop], position_target_cm, position_cm[loop], false );
            
            int pwm_out = ROUND_INT( pwm_out_flt*100 );
            
            if ( ABS(pwm_out) > MOTOR_PWM_SANITY_CHECK_LIMIT ) 
            {
                FATAL_ERROR("Motor %d pwm invalid %d!", loop, (int)pwm_out );
                break;                
            }
 
            mot_pwm[ loop ] = motor_timers_set_speed( loop, pwm_out );
            max_pwm[ loop ] = MAX( max_pwm[loop], mot_pwm[loop] );
            
            // printf("T=%0.1f - pos: %0.1f -- speed: %0.1f pwm %d\n", time_sec, pos_cm, speed_cm_per_sec,  motor_pwm ); 
        }
        
        // printk("PWM %d %d\n", mot_pwm[0], mot_pwm[1] );
        
        const Motor_cmd* cmd = motor_queue_get( MOTOR_CONTROL_LOOP_MS );
        
        if ( cmd != NULL )
        {
            if ( (use_bumbers == false) && ( cmd->opcode == MOTOR_CMD_EV_BUMBER ) )
            {
                LOG_INF("Bumber hit ignored, due drive without bumbers!");
                
            }
            else
            {
                LOG_INF("Command %d received while driving, stopping!", cmd->opcode );
                if ( cmd->opcode != MOTOR_CMD_EV_BUMBER )
                {
                   end_event = MOTOR_CMD_EV_CANCELLED; 
                }
                else
                {
                    end_event = MOTOR_CMD_EV_BUMBER;
                }
                break;
            }
        }
    }
    
    // make sure speed is zero.
    motor_cmd_stop( NULL );
    motor_timers_get_location( position_cm );
    int pos_diff[2] =  { ROUND_INT( (distances[0] - position_cm[0])*100.0f ), 
                         ROUND_INT( (distances[1] - position_cm[1])*100.0f ) };
        
    LOG_INF("Final position %d %d (mm) - D %d %d (1/10 mm) - max pwm: %d %d", ROUND_INT(position_cm[0]*10.0f), ROUND_INT(position_cm[1]*10.0f),
                                                  pos_diff[0], pos_diff[1], max_pwm[0], max_pwm[1] ); 

    if ( LOCAL_motor_callback != NULL )
    {
        int32_t position_cm_int[2] = { ROUND_INT( position_cm[0] ), ROUND_INT( position_cm[1] ) };
        LOCAL_motor_callback ( end_event, position_cm_int, 2 );
    }
}


static void motor_cmd_rotate( float angle )
{
    float to_drive[2] = {0,0} ;
    float rotating_wheel = DRIVE_CM_PER_ANGLE*angle;
    
    // rotate clockwise, right wheel not moe
    to_drive[0] =   rotating_wheel;
    to_drive[1] =  -rotating_wheel;
    
    motor_cmd_drive( to_drive, MOTOR_MAX_SPEED_CM_S*0.75f, true );
}

void motors_main()
{
    k_msgq_init( &LOCAL_queue, (char*)LOCAL_queue_buffer, sizeof(Motor_cmd), LOCAL_queue_size );

    motor_timers_init();
    motor_control_init();
    motors_bumber_init();
    motors_pid_init();
    
    LOG_INF("Motor app started!");
    
    
    float param_help[ MOTOR_MAX_PARAMS ];
    
    while( true )
    {

        const Motor_cmd* cmd = motor_queue_get( K_FOREVER );
        LOG_INF("Motor execute %d", cmd->opcode );
        
        switch( cmd->opcode )
        {
            
            case MOTOR_CMD_DRIVE:
                memcpy( param_help, cmd->params, sizeof(float)*MOTOR_MAX_PARAMS);
                motor_cmd_drive( param_help, param_help[2], true );
                break;
            
            case MOTOR_CMD_DRIVE_IGN_BUMBER:
                memcpy( param_help, cmd->params, sizeof(float)*MOTOR_MAX_PARAMS);
                motor_cmd_drive( param_help, param_help[2], false );
                break;
                
            case MOTOR_CMD_ROTATE:
                memcpy( param_help, cmd->params, sizeof(float)*1);
                motor_cmd_rotate( param_help[0] );
                break;
                
            case MOTOR_CMD_TEST:
                motor_cmd_test( cmd );
                break;

            case MOTOR_CMD_STOP:
                motor_cmd_stop( cmd );
                break;
                
            case MOTOR_CMD_EV_BUMBER:
                LOG_INF("Ignore bumber hit: 0x%X", cmd->params[0] );
                break;
                
            default:
                FATAL_ERROR("Invalid cmd: %d", cmd->opcode );
                break;
        }

    }
}



K_THREAD_DEFINE( motor_thread, OS_DEFAULT_STACKSIZE, motors_main, NULL, NULL, NULL,
                 OS_DEFAULT_PRIORITY, 0, K_NO_WAIT);




