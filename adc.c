/*
 * adc.c
 *
 *  Created on: 9 maj 2017
 *      Author: VA
 */

#include "stm32f1xx_hal.h"

extern ADC_HandleTypeDef hadc1;

void ADC_kanal(unsigned long chnl)
{
	  ADC_ChannelConfTypeDef sConfig;

	    /**Common config
	    */

	  hadc1.Instance = ADC1;
	  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	  hadc1.Init.ContinuousConvMode = DISABLE;
	  hadc1.Init.DiscontinuousConvMode = DISABLE;
	  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc1.Init.NbrOfConversion = 1;
	  if (HAL_ADC_Init(&hadc1) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	    /**Configure Regular Channel
	    */
	  sConfig.Channel = chnl;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }
}
