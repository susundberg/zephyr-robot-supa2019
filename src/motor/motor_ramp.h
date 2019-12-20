#pragma once

#include <stdint.h>

typedef struct
{
    float const_M;
    float const_T;
    float const_A;
    float const_B;
    float const_D;
} MotorRampAcc;


typedef struct
{
    MotorRampAcc acc_ramp;
    float acc_ramp_len;
    float acc_ramp_time;
    float const_ramp_dist;
    float const_ramp_speed;
    float const_ramp_time;
    
} MotorRamp;


void motor_ramp_init( MotorRamp* ramp, float max_acc, float max_speed, float distance );
float motor_ramp_location( MotorRamp* ramp, float t );
float motor_ramp_fulltime( MotorRamp* ramp );

