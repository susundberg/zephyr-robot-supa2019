#pragma once



typedef struct
{
    float coeff_p;
    float coeff_i;
    float coeff_d;
    float dt;
    float dt_p1;
    float error_last;
    float error_int;
} PidController;

void pid_control_clear();
void pid_control_setup( PidController* pid, float p, float i, float d, float dt );
float pid_control_step( PidController* pid, float target, float measured, bool debug_print );


