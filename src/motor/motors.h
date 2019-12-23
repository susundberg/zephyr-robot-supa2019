#pragma once

#include "../main.h"



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
   int32_t        params[2];
   Motor_cmd_type opcode; 
} Motor_cmd;
