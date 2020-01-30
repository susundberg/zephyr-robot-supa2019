#pragma once

#include "../main.h"


#define MOTOR_MAX_ACC_CM_SS   50.0   // cm per sec^2
#define MOTOR_MAX_SPEED_CM_S  10.0  // cm per sec


void motor_init();
void motor_abort();
void motors_send_cmd( uint32_t opcode, uint32_t* params, uint32_t nparams );
void motor_bumber_init();

typedef enum
{
    MOTOR_CMD_INVALID = 0,
    MOTOR_CMD_DRIVE,
    MOTOR_CMD_STOP,
    MOTOR_CMD_TEST,
    
    MOTOR_CMD_EV_BUMBER,
} Motor_cmd_type; 

typedef struct
{
   uint32_t       _fifo_reserved;
   int32_t        params[3];
   Motor_cmd_type opcode; 
} Motor_cmd;
