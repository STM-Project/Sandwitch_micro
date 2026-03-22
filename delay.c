/*
 * delay.c
 *
 *  Created on: 23 lut 2017
 *      Author: VA
 */

#include "stm32f1xx_hal.h"

TIM_HandleTypeDef htim6;

static void MX_TIM6_Init_moj(void)
{

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 63;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 60000;
  htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_Base_Start(&htim6) != HAL_OK)
  {
  	        Error_Handler();
  }

}

void init_timer_delay()
{
	MX_TIM6_Init_moj();
}

void czekaj_us(int us)
{
	__HAL_TIM_SET_COUNTER(&htim6,0);
    while(__HAL_TIM_GET_COUNTER(&htim6)<us);
}

