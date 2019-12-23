#pragma once

#include "../main.h"


#define MOTOR_MAX_ACC_CM_SS   10.0   // cm per sec^2
#define MOTOR_MAX_SPEED_CM_S  100.0  // cm per sec

void motor_init();

extern struct k_fifo GLOBAL_motor_fifo;

typedef enum
{
    MOTOR_CMD_INVALID = 0,
    MOTOR_CMD_DRIVE,
    MOTOR_CMD_STOP,
} Motor_cmd_type; 

typedef struct
{
   uint32_t       _fifo_reserved;
   int32_t        params[3];
   Motor_cmd_type opcode; 
} Motor_cmd;
