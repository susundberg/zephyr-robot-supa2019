#pragma once

#include "../main.h"


#define MOTOR_MAX_ACC_CM_SS   20.0f   // cm per sec^2
#define MOTOR_MAX_SPEED_CM_S  7.5f  // cm per sec
#define MOTOR_MAX_PARAMS      3


// motor test 1 10 5 -- yielded around 10506 ticks -> 5 rounds - each is 22.62 cm long -> 5*(22.62) / 10506 = 0.0107653 CM / tick
#define MOTOR_TICKS_TO_CM       0.0107653f
#define MOTOR_CM_PER_SEC_TO_PWM 25.0f
#define MOTOR_PWM_OFFSET        150.0f


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
