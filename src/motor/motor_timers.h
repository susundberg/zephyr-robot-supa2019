#pragma once

#include "motors.h"

void motor_timers_init();
// void motor_timers_pos_get( uint32_t* left, uint32_t* right );

uint32_t motor_timers_set_speed( uint32_t motor, float speed_cm_per_sec );
void motor_timers_set_location_zero( float* loc );
void motor_timers_get_location( float* loc );

void motor_control_init();
void motor_control_enable( int motor, bool reverse );
void motor_control_disable( int motor );
void motor_control_disable_all();
void motor_timers_abort();
