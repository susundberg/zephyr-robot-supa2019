#pragma once

#include <stdint.h>

typedef struct
{
    float const_M;
    float const_T;
    float const_A;
    float const_B;
    float const_D;
} Motor_ramp_acc;


typedef struct
{
    Motor_ramp_acc acc_ramp;
    float acc_ramp_len;
    float acc_ramp_time;
    float const_ramp_dist;
    float const_ramp_speed;
    float const_ramp_time;
    
} Motor_ramp;


void motor_ramp_init( Motor_ramp* ramp, float max_acc, float max_speed, float distance );
float motor_ramp_location( Motor_ramp* ramp, float t );
float motor_ramp_fulltime( Motor_ramp* ramp );

