

#include <math.h>
#include "motor_ramp.h"


static float motor_ramp_acc_solve_max_speed( float distance, float max_acc )
{
    const float scalar = (2.0f/3.0f);
    return sqrtf( scalar * distance * max_acc );
}

static float motor_ramp_acc_solve_distance( float max_acc, float speed_diff)
{
    const float scalar = (3.0f/4.0f);
    return (scalar*speed_diff*speed_diff) / max_acc;
}

static float motor_ramp_acc_solve_time(  float max_acc, float speed_diff )
{
    return ( ( 3.0f * speed_diff ) / (2.0f * max_acc) );
}


static float motor_ramp_acc_speed_i( MotorRampAcc* acc, float t)
{
    return (1.0f/3.0f) * acc->const_A*(t*t*t) + acc->const_M * t;
}

static float motor_ramp_acc_location_i( MotorRampAcc* acc, float t)
{
   float t2 = t*t; 
   return (1.0f/12.0f)*acc->const_A*(t2*t2) + 0.5f*acc->const_M*(t2) - acc->const_B*t;
}

static float motor_ramp_acc_location( MotorRampAcc* acc, float t )
{
    t = t - acc->const_T;
    return motor_ramp_acc_location_i(acc,t) - acc->const_D;
}

// static float motor_ramp_acc_speed( MotorRampAcc* acc, float t )
// {
//     t = t - acc->const_T;
//     return motor_ramp_acc_speed_i( acc, t ) - acc->const_B;
// }

static void motor_ramp_acc_init( MotorRampAcc* acc, float max_acc, float speed_diff )
{
    acc->const_M = max_acc;
    acc->const_T = ( ( 3.0f * speed_diff ) / (4.0f * max_acc) ) ;
    acc->const_A = ( -acc->const_M / (acc->const_T*acc->const_T));
    acc->const_B = motor_ramp_acc_speed_i( acc, -acc->const_T );
    acc->const_D = motor_ramp_acc_location_i( acc, -acc->const_T );
}


void motor_ramp_init( MotorRamp* ramp, float max_acc, float max_speed, float distance )
{
    
    float dist_max_speed = motor_ramp_acc_solve_max_speed( distance, max_acc );
    float dist_const_speed; 
    
    if ( dist_max_speed < max_speed )
    {
        LOG_INFO("Cannot go full speed, limiting max speed to %d", (int)(dist_max_speed*100.0f));
        max_speed = dist_max_speed ;
        dist_const_speed = 0;
    }
    else
    {
        float ramp_dist = motor_ramp_acc_solve_distance( max_acc, max_speed );
        dist_const_speed = distance - 2*ramp_dist;
        LOG_INFO("Full speed available it will take %d -> const dist %d", (int)(ramp_dist*100.0f), (int)(dist_const_speed*100.0f));
        ASSERT( dist_const_speed >= 0.0f );
    }
    
    motor_ramp_acc_init( &ramp->acc_ramp, max_acc, max_speed );
    
    ramp->acc_ramp_len     = motor_ramp_acc_solve_distance( max_acc, max_speed );
    ramp->acc_ramp_time    = motor_ramp_acc_solve_time( max_acc, max_speed );
    ramp->const_ramp_dist  = dist_const_speed;
    ramp->const_ramp_speed = max_speed;
    ramp->const_ramp_time  = dist_const_speed/max_speed;
        
}

float motor_ramp_fulltime( MotorRamp* ramp )
{
    return 2.0f*ramp->acc_ramp_time + ramp->const_ramp_time;
}


float motor_ramp_location( MotorRamp* ramp, float t )
{
    
    float loc = 0.0f;
    if ( t < ramp->acc_ramp_time )
        return motor_ramp_acc_location( &ramp->acc_ramp, t );
    
    t   -= ramp->acc_ramp_time;
    loc += ramp->acc_ramp_len;
    
    if ( t < ramp->const_ramp_time )
        return loc + ramp->const_ramp_speed*t;
    
    t   -= ramp->const_ramp_time;
    loc += ramp->const_ramp_dist;
    
    if ( t < ramp->acc_ramp_time )
    {
        float offset = (ramp->const_ramp_speed*t) - motor_ramp_acc_location( &ramp->acc_ramp, t );
        return loc + offset;
    }
    
    return ramp->const_ramp_dist + (2.0f*ramp->acc_ramp_len);
}


