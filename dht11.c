/*
 * dht11.c
 *
 *  Created on: 12 sie 2017
 *      Author: Elektronika RM
 */

#include "stm32f1xx_hal.h"
#include <modem_uart.h>
#include "dht11.h"



int DHT11_read(uint8_t *temparature, uint8_t *humidity, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	//Initialisation
		uint8_t i, j, temp;
	uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	//Generate START condition
	TM_GPIO_SetPinAsOutput(GPIOx, GPIO_Pin);

	//Put LOW for at least 18ms
	TM_GPIO_SetPinLow(GPIOx, GPIO_Pin);

	//wait 18ms
	HAL_Delay(18);

		//Put HIGH for 20-40us
	TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin);

		//wait 40us
	   czekaj_us(40);
		//End start condition

		//io();
		//Input mode to receive data
	   TM_GPIO_SetPinAsInput(GPIOx, GPIO_Pin);

		//DHT11 ACK
		//should be LOW for at least 80us
		    __HAL_TIM_SET_COUNTER(&htim6,0);
		    while(!TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) {
			    if(__HAL_TIM_GET_COUNTER(&htim6)>100){
				    return 0;   }
		    }

		//should be HIGH for at least 80us
		    __HAL_TIM_SET_COUNTER(&htim6,0);
		    while(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) {
			    if(__HAL_TIM_GET_COUNTER(&htim6)>100){
				    return 0;   }
		    }

		//Read 40 bits (8*5)
		for(j = 0; j < 5; ++j) {
			for(i = 0; i < 8; ++i) {

				//LOW for 50us
			       __HAL_TIM_SET_COUNTER(&htim6,0);
			       while(!TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) {
				       if(__HAL_TIM_GET_COUNTER(&htim6)>60){
					       return 0;   }
			       }

				//Start counter
			     __HAL_TIM_SET_COUNTER(&htim6,0);

				//HIGH for 26-28us = 0 / 70us = 1
			       while(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) {
				       if(__HAL_TIM_GET_COUNTER(&htim6)>100){
					       return 0;   }
			       }


				//Calc amount of time passed
				temp = __HAL_TIM_GET_COUNTER(&htim6);

				//shift 0
				data[j] = data[j] << 1;

				//if > 30us it's 1
				if(temp > 40)
					data[j] = data[j]+1;
			}
		}

		//verify the Checksum
		if(data[4] != (data[0] + data[2])){
			 return 0;    }

		//set data
		*temparature = data[2];
		*humidity    = data[0];

	 return 1;

}

uint8_t read_dht11_dat(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  char i = 0;
  char result=0;

  for(i=0; i< 8; i++){
	  while(!TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin));  // wait for 50us
    czekaj_us(30);

    if(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin))
      result |=(1<<(7-i));
    while(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin));  // wait '1' finish
  }

  return result;
}

int DHT22_read(uint8_t *temparature, uint8_t *humidity, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  uint8_t dataIndex = 0;   int i;  int f;
  uint8_t counter = 7;
  uint8_t currByte = 0;
  uint8_t index = 0;
  uint32_t startTime = 0;

  /* reset data holder */
  for(index=0; index < 6; index++){
    data[index] = 0x00;
  }

  /* mcu sends start signal to sensor */
  TM_GPIO_SetPinAsOutput(GPIOx, GPIO_Pin);
  TM_GPIO_SetPinLow(GPIOx, GPIO_Pin);
  HAL_Delay(10);

  TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin);

  /* wait for at least 20 mSecs */
  czekaj_us(40);

  /* switch to input and wait for sensor response */
  TM_GPIO_SetPinAsInput(GPIOx, GPIO_Pin);

  __HAL_TIM_SET_COUNTER(&htim6,0);
  while(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) {
	    if(__HAL_TIM_GET_COUNTER(&htim6)>200){
		   /*dbg3("1");*/ return 0;   }
  }

  czekaj_us(80);

  /* DHT22 sends response signal */
  __HAL_TIM_SET_COUNTER(&htim6,0);
  while(!TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) {
	    if(__HAL_TIM_GET_COUNTER(&htim6)>200){
		  /* dbg3("2");*/return 0;    }
  }

  czekaj_us(80);

  /* DHT22 sends, finally, data */
  for (i=0; i<5; i++){
    data[i] = read_dht11_dat(GPIOx,GPIO_Pin);
  }


  if ((data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
   // dbg3("\r\nOK  ");

	f = data[2] & 0x7F;   //przelicz Temp
	      f *= 256;
	      f += data[3];
	     // f /= 10;
	         //if (data[2] & 0x80) f *= -1;
	      if (data[2] & 0x80) *(temparature+0) = '-';
	      else                *(temparature+0) = '+';
	  *(temparature+1) = ((f/10)/10)|0x30;
	  *(temparature+2) = ((f/10) - 10*((f/10)/10))|0x30;
	  *(temparature+3) = '.';
	  *(temparature+4) = (f-10*(f/10))|0x30;
	  *(temparature+5) = 0;


	      f = data[0];   //przelicz RHU
	      f *= 256;
	      f += data[1];
	      f /= 10;
	  *humidity    = f;

	  return 1;
 }
 else return 0;

}

int GetTemperature(void)
{
	int f;
	f = data[2] & 0x7F;
	      f *= 256;
	      f += data[3];
	     // f /= 10;
	      if (data[2] & 0x80) f *= -1;
	return f;
}
int GetHumidity(void)
{
	int f;
    f = data[0];
    f *= 256;
    f += data[1];
    f /= 10;
	return f;
}

//float convertCtoF(float cTemperature){
 // return cTemperature * 9 / 5 + 32;
//}

/*
uint32_t DHT22_GetReadings(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	uint32_t wait;
	uint8_t i;

	TM_GPIO_SetPinAsOutput(GPIOx, GPIO_Pin);

	// Generate start impulse for sensor
	TM_GPIO_SetPinLow(GPIOx, GPIO_Pin); // Pull down SDA (Bit_SET)
	HAL_Delay(2); // Host start signal at least 800us
	TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin); // Release SDA (Bit_RESET)

	// Switch pin to input with Pull-Up
	 TM_GPIO_SetPinAsInput(GPIOx, GPIO_Pin);

	// Wait for AM2302 to start communicate
	wait = 0;
	while ((TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) && (wait++ < 200)) czekaj_us(1);
	if (wait > 50){ dbg3("1"); return 0; }

	// Check ACK strobe from sensor
	wait = 0;
	while (!(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) && (wait++ < 200)) czekaj_us(1);
	if ((wait < 8) || (wait > 150)){ dbg3("2"); return 0; }

	wait = 0;
	while ((TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) && (wait++ < 200)) czekaj_us(1);
	if ((wait < 8) || (wait > 150)){ dbg3("3"); return 0; }

	// ACK strobe received --> receive 40 bits
	i = 0;
	while (i < 40) {
		// Measure bit start impulse (T_low = 50us)
		wait = 0;
		while (!(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) && (wait++ < 200)) czekaj_us(1);
		if (wait > 180) {
			// invalid bit start impulse length
			bits[i] = 0xffff;  dbg3("KURWA");
			while ((TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) && (wait++ < 20)) czekaj_us(1);
		} else {
			// Measure bit impulse length (T_h0 = 25us, T_h1 = 70us)
			wait = 0;
			while ((TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)) && (wait++ < 200)) czekaj_us(1);
			bits[i] = (wait < 40) ? wait : 0xffff;
		}

		i++;
	}

	for (i = 0; i < 40; i++) if (bits[i] == 0xffff){ dbg3("4"); return 0; }

	dbg3(" OK "); return 1;
}

uint16_t DHT22_DecodeReadings(void)
{
	uint8_t parity;
	uint8_t  i = 0;

	hMSB = 0;
	for (; i < 8; i++) {
		hMSB <<= 1;
		if (bits[i] > 7) hMSB |= 1;
	}
	hLSB = 0;
	for (; i < 16; i++) {
		hLSB <<= 1;
		if (bits[i] > 7) hLSB |= 1;
	}
	tMSB = 0;
	for (; i < 24; i++) {
		tMSB <<= 1;
		if (bits[i] > 7) tMSB |= 1;
	}
	tLSB = 0;
	for (; i < 32; i++) {
		tLSB <<= 1;
		if (bits[i] > 7) tLSB |= 1;
	}
	for (; i < 40; i++) {
		parity_rcv <<= 1;
		if (bits[i] > 7) parity_rcv |= 1;
	}

	parity  = hMSB + hLSB + tMSB + tLSB;

	return (parity_rcv << 8) | parity;
}

uint16_t DHT22_GetHumidity(void)
{
	return (hMSB << 8) + hLSB;
}

uint16_t DHT22_GetTemperature(void)
{
	return (tMSB << 8) + tLSB;
}
*/
