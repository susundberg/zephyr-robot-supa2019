#include <zephyr.h>
#include <string.h>
#include <stdio.h>

#include "pid.h"


void pid_control_setup( PidController* pid, float p, float i, float d, float dt )
{
    memset(pid, 0x00, sizeof(pid));
    pid->coeff_p = p;
    pid->coeff_i = i;
    pid->coeff_d = d;
    pid->dt = dt;
    pid->dt_p1 = 1.0f/dt;
    
}
void pid_control_clear( PidController* pid )
{
    pid->error_last = 0;
    pid->error_int  = 0;
}


float pid_control_step( PidController* pid, float target, float measured , bool debug_print )
{
   float error = target - measured;
   float error_d = ( error - pid->error_last ) * pid->dt_p1;
   pid->error_int += error * pid->dt;
   pid->error_last = error;
 
   float output = error * pid->coeff_p + pid->error_int * pid->coeff_i + error_d * pid->coeff_d; 
   
   if (debug_print)
      printf("PID %0.1f   %0.1f %0.1f    %0.1f %0.1f %0.1f\n", (double)output, (double)target, (double)measured, (double)error, (double)error_d, (double)pid->error_int);

   return output;
}


    


