

#include "motors.h"


TIM_HandleTypeDef LOCAL_tim_pwm;
TIM_HandleTypeDef LOCAL_tim_left;
TIM_HandleTypeDef LOCAL_tim_right;

#define HAL_CHECK(x) if ( (x) != HAL_OK) { FATAL_ERROR("HAL call failed!"); };
#define PWM_TIM_PERIOD_CYCLES 1000

LOG_MODULE_REGISTER( mot_tim );




/* TIM2 init function */
static void tim_pwm_init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  __HAL_RCC_TIM2_CLK_ENABLE();
  
  LOCAL_tim_pwm.Instance = TIM2;
  LOCAL_tim_pwm.Init.Prescaler     = 250; // Clock is 50 000 Khz / 250 -> 200 khz
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
  
  HAL_CHECK(HAL_TIM_PWM_ConfigChannel(&LOCAL_tim_pwm, &sConfigOC, TIM_CHANNEL_1) );
  HAL_CHECK(HAL_TIM_PWM_ConfigChannel(&LOCAL_tim_pwm, &sConfigOC, TIM_CHANNEL_2) );
  
  HAL_CHECK( HAL_TIM_PWM_Start( &LOCAL_tim_pwm, TIM_CHANNEL_1 ) );
  HAL_CHECK( HAL_TIM_PWM_Start( &LOCAL_tim_pwm, TIM_CHANNEL_2 ) );
}



/* TIM3 init function */
static void tim_counter_init(void)
{
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};

  
  __HAL_RCC_TIM3_CLK_ENABLE();
  __HAL_RCC_TIM4_CLK_ENABLE();
  
  
  LOCAL_tim_left.Instance = TIM3;
  LOCAL_tim_left.Init.Prescaler = 0;
  LOCAL_tim_left.Init.CounterMode = TIM_COUNTERMODE_UP;
  LOCAL_tim_left.Init.Period = 0xFFFF;
  LOCAL_tim_left.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  LOCAL_tim_left.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  LOCAL_tim_right.Instance = TIM4;
  LOCAL_tim_right.Init = LOCAL_tim_left.Init;
  
  HAL_CHECK( HAL_TIM_Base_Init(&LOCAL_tim_left) );
  HAL_CHECK( HAL_TIM_Base_Init(&LOCAL_tim_right) );
  
  
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  
  HAL_CHECK( HAL_TIM_SlaveConfigSynchro(&LOCAL_tim_left, &sSlaveConfig) );
  HAL_CHECK( HAL_TIM_SlaveConfigSynchro(&LOCAL_tim_right, &sSlaveConfig) );

  HAL_CHECK( HAL_TIM_Base_Start(&LOCAL_tim_left) );
  HAL_CHECK( HAL_TIM_Base_Start(&LOCAL_tim_right) );
}



void motor_timers_pos_get( uint32_t* left, uint32_t* right )
{
    (*left)  = __HAL_TIM_GET_COUNTER( &LOCAL_tim_left );
    (*right) = __HAL_TIM_GET_COUNTER( &LOCAL_tim_right );
    
}


void motor_timers_init()
{
    tim_pwm_init();
    tim_counter_init();
    LOG_INF("Motor timers init done");
}



