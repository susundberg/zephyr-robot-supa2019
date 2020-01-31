#pragma once

#include "../main.h"


#define MOTOR_MAX_ACC_CM_SS   50.0   // cm per sec^2
#define MOTOR_MAX_SPEED_CM_S  10.0  // cm per sec

#define MOTOR_MAX_PARAMS 3


void motor_init();
void motor_abort();
void motors_send_cmd( uint32_t opcode, uint32_t* params, uint32_t nparams );
void motor_bumber_init();
void motor_control_function( bool running );

typedef enum
{
    MOTOR_CMD_INVALID = 0,
    MOTOR_CMD_DRIVE,
    MOTOR_CMD_STOP,
    MOTOR_CMD_TEST,
    MOTOR_CMD_EV_BUMBER,
} Motor_cmd_type; 

#define MOTOR_EVENT_BUMBER_LEFT  0x01
#define MOTOR_EVENT_BUMBER_RIGHT 0x02
