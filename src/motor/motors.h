#pragma once

#include "../main.h"


#define MOTOR_MAX_ACC_CM_SS   20.0f   // cm per sec^2
#define MOTOR_MAX_SPEED_CM_S  15.0f   // cm per sec
#define MOTOR_MAX_PARAMS      3

// Measured
#define MOTOR_TICKS_TO_CM        0.0137257575f
#define MOTOR_CM_PER_SEC_TO_PWM  25.0f
#define MOTOR_PWM_OFFSET         150.0f
#define DRIVE_CM_PER_ANGLE       0.27080755f
#define MOTOR_THRESHOLD_STUCK_CM 5.0f

void motors_init();
void motors_abort();
void motors_send_cmd( uint32_t opcode, void* params, uint32_t nparams );
void motors_bumber_init();
void motors_control_function( bool running );
void motors_set_pid( int motor,  const float* coeffs );

typedef enum
{
    MOTOR_CMD_INVALID = 0,
    MOTOR_CMD_DRIVE,             // param 0 = distance left, param 1 = distance right
    MOTOR_CMD_DRIVE_IGN_BUMBER,  // as above 
    MOTOR_CMD_ROTATE,           // param 0 = angle in deg
    MOTOR_CMD_STOP,
    MOTOR_CMD_TEST,
    MOTOR_CMD_EV_BUMBER = 100,     // Drive was terminated due bumber hit
    MOTOR_CMD_EV_DONE,       // Drive was terminated due distance end
    MOTOR_CMD_EV_STUCK,      // Drive was terminated due getting stuck. 
    MOTOR_CMD_EV_CANCELLED   // Drive was terminated due another command
} Motor_cmd_type; 

typedef void (*Motor_cmd_done_callback)(Motor_cmd_type reason, int32_t* param, uint32_t param_n );

void motors_set_callback( Motor_cmd_done_callback callback );

#define MOTOR_EVENT_BUMBER_LEFT  0x01
#define MOTOR_EVENT_BUMBER_RIGHT 0x02
