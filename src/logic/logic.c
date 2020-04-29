
#include <zephyr.h>
#include <logging/log.h>


#define SUPA_MODULE "loc"
LOG_MODULE_REGISTER(logic);

#include "../main.h"
#include "../motor/motors.h"
#include "../ui/ui.h"

typedef enum
{
    LOGIC_CMD_INVALID = 0,
    LOGIC_CMD_START,
    LOGIC_CMD_STOP,
    LOGIC_EVENT_MOTOR,
} Logic_event_type;

typedef struct
{
  Logic_event_type opcode; 
  uint32_t         params[2];
} Logic_event;



#define LOCAL_queue_size 8
static Logic_event __aligned(4) LOCAL_queue_buffer[ LOCAL_queue_size ];
static struct k_msgq LOCAL_queue;


#define LOGIC_DISTANCE_MAX_CM        1000    // 10m should be enough for everyone..
#define LOGIC_DISTANCE_BACKUP_CM     20
#define LOGIC_DISTANCE_MAX_MAKE_SURE 40
#define LOGIC_DISTANCE_NEXT_LANE_CM  10
#define LOGIC_ROTATE_ANGLE           90



static void motorsdone_callback ( Motor_cmd_type reason, int32_t* param, uint32_t param_n )
{

   
   Logic_event cmd;
   
   cmd.opcode = LOGIC_EVENT_MOTOR;
   cmd.params[0] = reason;
   
   if ( reason == MOTOR_CMD_EV_BUMBER || reason == MOTOR_CMD_EV_DONE || reason == MOTOR_CMD_EV_CANCELLED )
   {
       ASSERT( param_n == 2);
       cmd.params[1] = (param[0] + param[1])/2;
   }
   
   // send cmd
   ASSERT( k_msgq_put( &LOCAL_queue, &cmd, K_NO_WAIT ) == 0 );
}

void logic_activate( bool is_active )
{
     Logic_event cmd;
     
     if ( is_active == false )
     {
         cmd.opcode = LOGIC_CMD_STOP;
         
     }
     else
     {
         cmd.opcode = LOGIC_CMD_START;
         motors_set_callback( motorsdone_callback );
     }
    ASSERT( k_msgq_put( &LOCAL_queue, &cmd, K_NO_WAIT ) == 0 );
}

static void logic_init()
{
    k_msgq_init( &LOCAL_queue, (char*)LOCAL_queue_buffer, sizeof(Logic_event), LOCAL_queue_size );
    
}


static const Logic_event* logic_queue_get( k_timeout_t wait_time )
{
   static Logic_event cmd;
   
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

static void set_ui_termination_reason(uint32_t event )
{
    switch( event )
    {
        case MOTOR_CMD_EV_BUMBER:
            ui_signal_state( UI_STATE_PROGRAM_ERROR_1 );
            return;
        case MOTOR_CMD_EV_DONE:
            ui_signal_state( UI_STATE_PROGRAM_ERROR_2 );
            return;
        case MOTOR_CMD_EV_STUCK:
            ui_signal_state( UI_STATE_PROGRAM_ERROR_3 );
            return;
        case MOTOR_CMD_EV_CANCELLED:
            ui_signal_state( UI_STATE_PROGRAM_ERROR_4 );
            return;            
    }

}



static bool wait_for_motorstermination( uint32_t wait_event, int32_t* params, int32_t params_n )
{
    const Logic_event* event = logic_queue_get( K_FOREVER  );
    
    if ( event->opcode == LOGIC_EVENT_MOTOR )
    {
        LOG_INF("Logic received motop %d / %d", event->opcode, event->params[0] );
        if ( event->params[0] == wait_event )
        {
            if ( params_n > 0 )
                memcpy( params, &(event->params[1]), params_n*sizeof(uint32_t));
            
            return true;
        }
        else
        {
            LOG_INF("Was waiting for opcode %d, bailing out!", wait_event );
            set_ui_termination_reason( event->params[0] );
        }
    }
    else
    {
         LOG_INF("Logic received opcode %d", event->opcode );
         ui_signal_state( UI_STATE_PROGRAM_ERROR_4 );
    }

    LOG_INF("Invalid opcode or event, exit!");
    return false;
    
}

static void motors_send_drive_cmd( uint32_t opcode, float distance )
{
     float params[3];
     params[0] = distance;
     params[1] = distance;
     params[2] = MOTOR_MAX_SPEED_CM_S;
     
     motors_send_cmd( opcode, params, 3 );
}




static bool motors_send_wait_drive_cmd_custom( float distance, float ignore_bumber )
{
    uint32_t opcode = MOTOR_CMD_DRIVE;
    
    if ( ignore_bumber )
        opcode = MOTOR_CMD_DRIVE_IGN_BUMBER;
    
    motors_send_drive_cmd( opcode, distance );
    
    if ( wait_for_motorstermination( MOTOR_CMD_EV_DONE, NULL, 0  ) == false )
        return false;
    
    return true;
}

static bool motors_send_wait_drive_cmd( float distance )
{
    return motors_send_wait_drive_cmd_custom( distance, false );
}

static bool motors_send_wait_drive_nob_cmd( float distance )
{
    return motors_send_wait_drive_cmd_custom( distance, true );
}



static bool motors_send_wait_rotate_cmd(  float angle )
{
     motors_send_cmd( MOTOR_CMD_ROTATE, &angle, 1 );
     
    if ( wait_for_motorstermination( MOTOR_CMD_EV_DONE, NULL, 0  ) == false )
        return false;
    
    return true;
}

// #define LOG_DEBUG_PROGRESS { LOG_INF("Logic reached line %d", __LINE__ ); };
#define LOG_DEBUG_PROGRESS {};

static bool logic_run_loop()
{
    int32_t distance_driven;
    
    // Ok, first we start going as far as possible
    motors_send_drive_cmd( MOTOR_CMD_DRIVE, LOGIC_DISTANCE_MAX_CM );
   
    if ( wait_for_motorstermination( MOTOR_CMD_EV_BUMBER, &distance_driven, 1  ) == false )
        return false;
    
    LOG_INF("Distance for this round seems to be %d", distance_driven );

    if( distance_driven < LOGIC_DISTANCE_BACKUP_CM )
    {
        LOG_INF("Distance to drive is too short, bailing out!");
        ui_signal_state( UI_STATE_PROGRAM_ERROR_1 );
        return false;
    }
    
    // ok, bumber hit, then back up
    if ( motors_send_wait_drive_nob_cmd(  -LOGIC_DISTANCE_BACKUP_CM ) == false )
        return false;
    
    LOG_DEBUG_PROGRESS;
    
    // ok backup done, rotate 90 deg
    if ( motors_send_wait_rotate_cmd(  -LOGIC_ROTATE_ANGLE ) == false )
        return false;
    
    LOG_DEBUG_PROGRESS;
    
    // drive little bit forward
    if ( motors_send_wait_drive_cmd(  LOGIC_DISTANCE_NEXT_LANE_CM ) == false )
        return false;
    
    LOG_DEBUG_PROGRESS;
    
    // rotate another deg
    if ( motors_send_wait_rotate_cmd(  LOGIC_ROTATE_ANGLE ) == false )
        return false;
    
    LOG_DEBUG_PROGRESS;
    
    // Make sure the grass is cut at the end
    motors_send_drive_cmd( MOTOR_CMD_DRIVE, LOGIC_DISTANCE_MAX_MAKE_SURE);
    
    if ( wait_for_motorstermination( MOTOR_CMD_EV_BUMBER, NULL, 0 ) == false )
        return false;
    
    LOG_DEBUG_PROGRESS;
    
    // ok, bumber hit, then back up
    if ( motors_send_wait_drive_nob_cmd(  -LOGIC_DISTANCE_BACKUP_CM ) == false )
        return false;

    LOG_DEBUG_PROGRESS;
    
    // Rotate 180
    if ( motors_send_wait_rotate_cmd(  -2*LOGIC_ROTATE_ANGLE ) == false )
        return false; 
    
    LOG_DEBUG_PROGRESS;
    
    // drive to start position 
    if ( motors_send_wait_drive_cmd(  distance_driven - LOGIC_DISTANCE_BACKUP_CM ) == false )
        return false;
    
    LOG_DEBUG_PROGRESS;
    
    // ok backup done, rotate 90 deg
    if ( motors_send_wait_rotate_cmd(  LOGIC_ROTATE_ANGLE ) == false )
        return false;
    
    LOG_DEBUG_PROGRESS;
    
    // drive little bit forward
    if ( motors_send_wait_drive_cmd(  LOGIC_DISTANCE_NEXT_LANE_CM ) == false )
        return false;

    LOG_DEBUG_PROGRESS;
    
    // ok backup done, rotate 90 deg
    if ( motors_send_wait_rotate_cmd(  LOGIC_ROTATE_ANGLE ) == false )
        return false;
    
    return true;
    
}

static void logic_run()
{
    ui_signal_state( UI_STATE_PROGRAM_RUN );
    for( ;; )
    {
        bool round_ok = logic_run_loop();
        
        if (round_ok == true)
        {
            LOG_INF("Round success, start again.");
            continue;
        }
        else
        {
            LOG_INF("Error while doing, sending stop to motors!");
            motors_send_cmd( MOTOR_CMD_STOP, NULL, 0 );
            motors_control_function( false );             
        }
        break;
        
    }
    LOG_INF("Rounds done, bailing out!");
    motors_set_callback( NULL );
}


static void logic_main()
{
    // Wait for action!
    logic_init();
    LOG_INF("Logic app started!");
    ui_signal_state( UI_STATE_PROGRAM_IDLE );
    while( true )
    {

        const Logic_event* cmd = logic_queue_get( K_FOREVER );
        LOG_INF("Logic execute %d", cmd->opcode );
        
        switch( cmd->opcode )
        {
            
            case LOGIC_CMD_START:
                logic_run();
                break;
                
            case LOGIC_CMD_STOP:
                LOG_INF("Ignore stop as not running.");
                break;
                
            default:
                FATAL_ERROR("Invalid cmd: %d", cmd->opcode );
                break;
        }
    }
}



K_THREAD_DEFINE( logic_thread, OS_DEFAULT_STACKSIZE, logic_main, NULL, NULL, NULL,
                 OS_DEFAULT_PRIORITY, 0, 0);
