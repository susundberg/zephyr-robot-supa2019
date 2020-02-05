
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>


#define SUPA_MODULE "mot"

#include "motors.h"


static TIM_HandleTypeDef LOCAL_tim_pwm;
static TIM_HandleTypeDef LOCAL_tim_left;
static TIM_HandleTypeDef LOCAL_tim_right;

#define PWM_TIM_PERIOD_CYCLES 1000



LOG_MODULE_REGISTER( motor_tim );


static const uint32_t LOCAL_pwm_channels[2] = { TIM_CHANNEL_1, TIM_CHANNEL_2 };

/* TIM2 init function */
static void tim_pwm_init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  __HAL_RCC_TIM3_CLK_ENABLE();
  
  LOCAL_tim_pwm.Instance = TIM3; 
  LOCAL_tim_pwm.Init.Prescaler     = (CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 200000) - 1; // 500 -> 200 khz
  LOCAL_tim_pwm.Init.CounterMode   = TIM_COUNTERMODE_UP;
  LOCAL_tim_pwm.Init.Period        = PWM_TIM_PERIOD_CYCLES;
  LOCAL_tim_pwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  LOCAL_tim_pwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_CHECK (HAL_TIM_Base_Init(&LOCAL_tim_pwm) );

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_CHECK (HAL_TIM_ConfigClockSource(&LOCAL_tim_pwm, &sClockSourceConfig) );
  HAL_CHECK (HAL_TIM_PWM_Init(&LOCAL_tim_pwm) );
 
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  
  for ( int loop = 0; loop < 2; loop ++ )
  {
     HAL_CHECK( HAL_TIM_PWM_ConfigChannel(&LOCAL_tim_pwm, &sConfigOC, LOCAL_pwm_channels[ loop ] ) );
     HAL_CHECK( HAL_TIM_PWM_Start( &LOCAL_tim_pwm, LOCAL_pwm_channels[ loop ] ) );   
  }
}



/* TIM3 init function */
static void tim_counter_init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};

  
  __HAL_RCC_TIM1_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();
  
  
  LOCAL_tim_left.Instance = TIM1;
  LOCAL_tim_left.Init.Prescaler = 0;
  LOCAL_tim_left.Init.CounterMode = TIM_COUNTERMODE_UP;
  LOCAL_tim_left.Init.Period = 0xFFFF;
  LOCAL_tim_left.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  LOCAL_tim_left.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  LOCAL_tim_right.Instance = TIM2;
  LOCAL_tim_right.Init = LOCAL_tim_left.Init;
  
  HAL_CHECK( HAL_TIM_Base_Init(&LOCAL_tim_left) );
  HAL_CHECK( HAL_TIM_Base_Init(&LOCAL_tim_right) );
  
  
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 0;
  
  HAL_CHECK (HAL_TIM_ConfigClockSource(&LOCAL_tim_left, &sClockSourceConfig));
  HAL_CHECK (HAL_TIM_ConfigClockSource(&LOCAL_tim_right, &sClockSourceConfig));
  

  HAL_CHECK( HAL_TIM_Base_Start(&LOCAL_tim_left) );
  HAL_CHECK( HAL_TIM_Base_Start(&LOCAL_tim_right) );
}


static void pos_get( uint32_t* pos )
{
    pos[0] = __HAL_TIM_GET_COUNTER( &LOCAL_tim_left );
    pos[1] = __HAL_TIM_GET_COUNTER( &LOCAL_tim_right );
    
}

uint32_t LOCAL_offset[2];


void motor_timers_abort()
{
  HAL_CHECK( HAL_TIM_Base_Stop(&LOCAL_tim_pwm) ); 
}


static const float MOTOR_MIN_TRAVEL_SPEED = 0.5f;

uint32_t motor_timers_set_speed( uint32_t motor, float speed_cm_per_sec )
{
   uint32_t pwm_target = 0;
   
   if ( speed_cm_per_sec <= MOTOR_MIN_TRAVEL_SPEED )
   {
       pwm_target = 0;
   }
   else
   {
        float pwm_target_f  = speed_cm_per_sec*MOTOR_CM_PER_SEC_TO_PWM + MOTOR_PWM_OFFSET + 0.5f;

        if UNLIKELY( pwm_target_f >= PWM_TIM_PERIOD_CYCLES ) 
        {
            pwm_target = PWM_TIM_PERIOD_CYCLES;
        }
        else
        {
            pwm_target = (uint32_t)pwm_target_f;
        }  
   }

    ASSERT( motor < 2 );
   __HAL_TIM_SET_COMPARE( &LOCAL_tim_pwm, LOCAL_pwm_channels[ motor ], pwm_target );
   return pwm_target;
}


void motor_timers_get_location( float* pos )
{
     uint32_t tim_now[2];
     
     pos_get( tim_now );
     
     for ( int loop = 0; loop < 2; loop ++ )
     {
        uint32_t count_add;

        
        if ( tim_now[loop] < LOCAL_offset[loop] )
        {
            count_add = tim_now[loop] + (0xFFFF - LOCAL_offset[loop]);
        }
        else // tim_now >= local_offset
        {
            count_add = tim_now[loop] - LOCAL_offset[loop];
        }
        
        LOCAL_offset[loop] = tim_now[loop];    
        pos[loop] += (count_add * MOTOR_TICKS_TO_CM);
     }
}

void motor_timers_set_location_zero( float* pos )
{
    pos_get( LOCAL_offset );
    pos[0] = 0.0f;
    pos[1] = 0.0f;
}



void motor_timers_init()
{
    tim_pwm_init();
    tim_counter_init();
    LOG_INF("Motor timers init done");
}

