#include <drivers/pwm.h>
#include <drivers/gpio.h>

#include <logging/log.h>

TIM_HandleTypeDef LOCAL_tim_left;
TIM_HandleTypeDef LOCAL_tim_right;



extern void Error_Handler();

/* TIM3 init function */
void MX_TIM3_Init(void)
{
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  
  __HAL_RCC_TIM3_CLK_ENABLE();

  LOCAL_tim_left.Instance = TIM3;
  LOCAL_tim_left.Init.Prescaler = 0;
  LOCAL_tim_left.Init.CounterMode = TIM_COUNTERMODE_UP;
  LOCAL_tim_left.Init.Period = 0xFFFF;
  LOCAL_tim_left.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  LOCAL_tim_left.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&LOCAL_tim_left) != HAL_OK)
  {
    Error_Handler();
  }
  
  
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&LOCAL_tim_left, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&LOCAL_tim_left, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  
  if ( HAL_TIM_Base_Start(&LOCAL_tim_left) != HAL_OK)
  {
    Error_Handler();
  }   

}

/* TIM4 init function */
void MX_TIM4_Init(void)
{
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  
  __HAL_RCC_TIM4_CLK_ENABLE();
     

  LOCAL_tim_right.Instance = TIM4;
  LOCAL_tim_right.Init.Prescaler = 0;
  LOCAL_tim_right.Init.CounterMode = TIM_COUNTERMODE_UP;
  LOCAL_tim_right.Init.Period = 0xFFFF;
  LOCAL_tim_right.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  LOCAL_tim_right.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&LOCAL_tim_right) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ETRF;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
  sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&LOCAL_tim_right, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&LOCAL_tim_right, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  
  if (HAL_TIM_Base_Start(&LOCAL_tim_right ) != HAL_OK)
  {
    Error_Handler();
  }   

}




void motor_pos_init()
{
    MX_TIM3_Init();
    MX_TIM4_Init();
}


void motor_pos_get( u32_t* left, u32_t* right )
{
    (*left)  = __HAL_TIM_GET_COUNTER( &LOCAL_tim_left );
    (*right) = __HAL_TIM_GET_COUNTER( &LOCAL_tim_right );
    
}


