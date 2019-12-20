#include <zephyr.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>

#include <logging/log.h>

#include "motors.h"
#include "../main.h"

struct k_fifo LOCAL_fifo;


void motors_main()
{
    k_fifo_init(&LOCAL_fifo);
    
    
    
}





void motor_pos_get( u32_t* left, u32_t* right )
{
    (*left)  = __HAL_TIM_GET_COUNTER( &LOCAL_tim_left );
    (*right) = __HAL_TIM_GET_COUNTER( &LOCAL_tim_right );
    
}


