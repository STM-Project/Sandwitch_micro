/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */

#include "nrf24l01.h"
#include "flash_in.h"
#include <modem_uart.h>
#include "define.h"
#include "lcd.h"
#include "flash_in.h"
#include "eth.h"
#include "enc28j60.h"
#include "SX1278.h"
#include "zdarzenia.h"
#include "agh.h"
#include "gpio.h"
#include "irda.h"
#include "st7735/st7735.h"
#include "st7735/fonts.h"
#include "ili9341/core.h"


/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

WWDG_HandleTypeDef hwwdg;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
SX1278_hw_t SX1278_hw;
SX1278_t SX1278;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM6_Init(void);
static void MX_IWDG_Init(void);
static void MX_ADC1_Init(void);
static void MX_WWDG_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

static void MX_IWDG_Init_moj(void);

#define TM_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))
#define TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)

void TM_GPIO_SetPinAsInput__() {

	//GPIOB->MODER &= ~(0x03 << (2 * 14));

	  GPIO_InitTypeDef GPIO_InitStruct;

   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
   HAL_Delay(300);
	  GPIO_InitStruct.Pin = GPIO_PIN_12;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);




}

void TM_GPIO_SetPinAsOutput__() {

 // GPIOB->MODER = (GPIOB->MODER & ~(0x03 << (2 * 14))) | (0x01 << (2 * 14));


	 GPIO_InitTypeDef GPIO_InitStruct;


		  GPIO_InitStruct.Pin = GPIO_PIN_12;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

}

void SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {

	 GPIO_InitTypeDef GPIO_InitStruct;
		  GPIO_InitStruct.Pin = GPIO_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
void SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {

	  GPIO_InitTypeDef GPIO_InitStruct;
	  GPIO_InitStruct.Pin = GPIO_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

int timer_main;

int NRF_wyslij(char *buf)
{
	 int sts;
	   TM_NRF24L01_Transmit(buf);
	   do {
		   sts = TM_NRF24L01_GetTransmissionStatus();
	      } while (sts == 255);
	   TM_NRF24L01_PowerUpRx();
	   return sts;
}
void NRF_odbieranie()
{
	TM_NRF24L01_PowerUpRx();
}

int NRF_odbierz(char *buf, int ms)
{
    timer_main=0;
    while (!TM_NRF24L01_DataReady() && (timer_main < ms));
    if(timer_main < ms)
    {
		 TM_NRF24L01_GetData(buf);
		 return 1;
    }
    else
    {
   	     return 0;
    }
}
void NRF_Start()
{
	//TM_GPIO_SetPinAsInput(GPIOB, GPIO_PIN_14);
}

void NRF_Stop()
{
	   //TM_GPIO_SetPinHigh(GPIOB, GPIO_PIN_3);   //CS
	  // TM_GPIO_SetPinAsOutput(GPIOB, GPIO_PIN_14);
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void init() {
    ST7735_Init();

}

void loop() {
    // Check border
  //

	//DrawImageStart();  HAL_Delay(10000);  HAL_IWDG_Refresh(&hiwdg);





    ST7735_FillScreen(ST7735_BLACK);
    DrawWindowTempBig(0,15,320,90,"+16.8",ST7735_WHITE,"Pomieszczenie dddddddddddddd tttttt ");
    DrawWindowTempBig(0,135,320,90,"+34.5",ST7735_WHITE,"Pomieszczenie dddddddddddddd tttttt ");


/*
    ST7735_FillScreen(ST7735_BLACK);
    DrawWindowTemp(0,0,155+49,50,"+16.8",ST7735_WHITE,"Pomieszczenie dddddddddddddd tttttt ");   DrawWindowTemp(165+49,0,155-49,50,"+99.0",ST7735_RED,"Pomieszczeniepiwnica ttttttttttt");
    DrawWindowTemp(0,60,320,50,"+16.8 'C",ST7735_WHITE,"Pomieszczenie gospodarcze domku letniskowego  ");
    DrawWindowWE(0,120,155,50,"ZWARCIE",ST7735_RED,"Nazwa wejscia 1");   DrawWindowWE(165,120,155,50,"Rozwarcie",ST7735_WHITE,"Nazwa wejscia 2");
    DrawWindowPK(0,180,155,50,"Stan ON",ST7735_WHITE,"Zawor hyrauliczny");   DrawWindowPK(165,180,155,50,"Stan OFF",ST7735_BLACK,"pompa hydroforu");

    HAL_Delay(10000);  HAL_IWDG_Refresh(&hiwdg);

	ST7735_FillScreen(ST7735_BLACK);
	DrawWindowTempSmall(0,0*35,157,30,"+16.8",ST7735_WHITE,"Pomieszczenie dddddddddddddd ");   DrawWindowTempSmall(163,0*35,157,30,"+99.0",ST7735_RED,"Pomieszczenie piwnicaiiiiiiiiiiiiiiiiiiiiiii");
    DrawWindowTempSmall(0,1*35,320,30,"+16.8 'C",ST7735_WHITE,"Pomieszczenie dddddddddddddd Rafal Markielowski kkkkkkkkkkkkkkkk ");
    DrawWindowTempSmall(0,2*35,157,30,"+16.8",ST7735_WHITE,"Pomieszczenie dddddddddddddd ");   DrawWindowTempSmall(163,2*35,157,30,"+99.0",ST7735_RED,"Pomieszczenie piwnicaiiiiiiiiiiiiiiiiiiiiiii");
    DrawWindowTempSmall(0,3*35,157,30,"+16.8",ST7735_WHITE,"Pomieszczenie dddddddddddddd ");   DrawWindowTempSmall(163,3*35,157,30,"+99.0",ST7735_RED,"Pomieszczenie piwnicaiiiiiiiiiiiiiiiiiiiiiii");
    DrawWindowTempSmall(0,4*35,157,30,"+16.8",ST7735_WHITE,"Pomieszczenie dddddddddddddd ");   DrawWindowTempSmall(163,4*35,157,30,"+99.0",ST7735_RED,"Pomieszczenie piwnicaiiiiiiiiiiiiiiiiiiiiiii");
    DrawWindowPKSmall(0,5*35,157,30,"Stan ON iiiiiiiiiiiiiiiiiiiiiii",ST7735_WHITE,"Pomieszczenie dddddddddddddd ");   DrawWindowPKSmall(163,5*35,157,30,"Aa xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",ST7735_BLACK,"Pomieszczenie piwnicaiiiiiiiiiiiiiiiiiiiiiii");
    DrawWindowWESmall(0,6*35,157,30,"ZWARCIEiiiiiiiiiiiiiiiiiiiiiii",ST7735_RED,"Pomieszczenieiiiiiiiiiiiiiiiiiiiiiii  ");   DrawWindowWESmall(163,6*35,157,30,"Rozwarcieiiiiiiiiiiiiiiiiiiiiiii",ST7735_WHITE,"Pomieszczenie piwnicaiiiiiiiiiiiiiiiiiiiiiii");


*/
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(10000);


  //  ST7735_WriteString(0, 0, "Temperatura", Font_7x10, ST7735_WHITE, ST7735_BLACK);



//#ifdef ST7735_IS_128X128
    // Display test image 128x128
  //  ST7735_DrawImage(0, 0, ST7735_WIDTH, ST7735_HEIGHT, (uint16_t*)test_img_128x128);







//#endif // ST7735_IS_128X128

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


#ifndef _BootLoader
	  volatile uint32_t *VectorTable = (volatile uint32_t *)0x20000000;
	      uint32_t ui32_VectorIndex = 0;

	      for(ui32_VectorIndex = 0; ui32_VectorIndex < 48; ui32_VectorIndex++)
	      {
	          VectorTable[ui32_VectorIndex] = *(__IO uint32_t*)((uint32_t)_JumpToProgr + (ui32_VectorIndex << 2));
	      }

	     // __HAL_RCC_AHB_FORCE_RESET();

	      //  Enable SYSCFG peripheral clock
	     // __HAL_RCC_SYSCFG_CLK_ENABLE();

	     // __HAL_RCC_AHB_RELEASE_RESET();

	      // Remap RAM into 0x0000 0000
	     // __HAL_SYSCFG_REMAPMEMORY_SRAM();
	      SCB->VTOR = FLASH_BASE | (_JumpToProgr & 0x00FFFFFF);   //Vector Table Relocation in Internal SRAM.
#endif



  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  MX_IWDG_Init();
  MX_ADC1_Init();
  //MX_WWDG_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_TIM8_Init();
  MX_TIM4_Init();
  //MX_TIM2_Init();
  //MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  /* Init the low level hardware */
 // HAL_MspInit();HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

 // HAL_Delay(2000);




  LcdGpioKonfig();
#ifdef _LoRa
  LoRaGpioKonfig();
#endif
  TsopGpioKonfig();
  BuzGpioKonfig();

  TM_GPIO_SetPinAsOutput(GPIOB, GPIO_PIN_13);  //Flash_CS1,
  TM_GPIO_SetPinAsOutput(GPIOB, GPIO_PIN_12);  //Flash_CS2,
  TM_GPIO_SetPinAsOutput(GPIOB, GPIO_PIN_15);  //Flash_CS3,

  AT25SF_CSN_HIGH;
  AT25SF_2_CSN_HIGH;
  AT25SF_3_CSN_HIGH;

  TM_GPIO_SetPinAsInput(GPIOB, GPIO_PIN_14);  //Flash_SO,  ENC_SO
  ENC28j60_CE_HIGH


  sprintf(buf_p,"\r\nOdczyt Flash size: %d  ",_Size_Zmienne_zapisywalne);
 	dbg3(buf_p);
  Flash_xxx_read(_AdrFlashToRAM, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
 	dbg3("ok");

 	WpisPortElement();

 	RstGpioKonfig();
 	if(Const.s_Lan[1].param!=0x55)   // -> Pierwszy raz uruchamaimy Program Uzytkownika
 	{
 		Const.s_Rej[0].param=0;   // Rej OFF
 		ResetFlash();
 		ZapiszFlash();
 	}


   	if((Const.s_Lan[0].param&0x0C)==0)
   	{
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);  //WIFI   CH_PD down
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);  //start reset WIFI
       dbg3("\r\nWifi OFF  ");

    }



/*
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
    {
  	    __HAL_RCC_CLEAR_RESET_FLAGS();
  	    dbg3("\r\nPower Down ");
    }
    else
    {
  	    __HAL_RCC_CLEAR_RESET_FLAGS();
  	    dbg3("\r\nReset ");
    }
*/


    if((Const.s_Lan[0].param&0x0C)>0)
    {
    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);  //WIFI   CH_PD
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);  //start reset WIFI
          delay_funkc[34]=1;
    }


  dbg3("\r\nZaczynamy User Program ");

#ifndef _LCD_TFT
  lcd_reset();
  lcd_init();
  StringFont5x7(" STM Project ",13); NewLineFont5x7(13,0);
#else
 // init(); //ST7735
  LCD_init();  //ILI9341
  LCD_setOrientation(ORIENTATION_LANDSCAPE);
  DrawImageStart();
  YlcdWpis=41;
#endif



#ifdef _LoRa
  Lora_start();
#endif

  RTC_konfiguracja();


//------------  Autoryzacja  -------------------------------
 // if(strstr(Const.s_Kod[0].val,"15071979666666"))  //tu pomijamy
 // {
	   start_modem_uart();
 // }
 // else
 // {
	  // MX_USART3_UART_Init_115();
	  // start_modem_uart();   Autoryzacja();

	  // MX_USART3_UART_Init_460();
	  // start_modem_uart();
 // }
//--------------------------------------------


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

#ifdef _Neoway
  Reset_Modemu();
#endif

#ifdef _Fibocom
  Reset_Modemu();
#endif

#ifndef _Fibocom
  HAL_Delay(4000);
#endif


  init_timer_delay();


	if((Const.s_Lan[0].param&0x0C)>0){ HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET); }  //stop reset WIFI

	wybor2_bitowy|=0x0010;
   	Start_GPIO();
   	for(i=0;i<_Size__s_gpio;i++) Wybierz_Tryb_Portu(i+1);
   	for(i=0;i<_Size_we;i++) ServiceWE(i);
   	for(i=0;i<_Size_eol;i++) ServiceEOL(i);
   	for(i=0;i<_Size_wy;i++) GPIO_PK_start(i);
   	for(i=0;i<_Size_pwm;i++) GPIO_PWM(i);
   	wybor2_bitowy&=~0x0010;

   	if(RotacjaOnCzyOFF()==1) Redudancja0_po_RST();

    //PWM_3(1000,50);

#ifdef _ENC

		ConvertParamNET();
		WgrajMacENC();
			netstackInit(IPADDRESS, NETMASK, GATEWAY);     ip_addr_dest=GATEWAY;
		  	enc28j60RegDump();
		  	dbg3("\r\nKoniec init_ENC28 \r\n");

		  for(i=0; i<HTTP_task_LEN; i++)  replay_packet[i]=200;
		  //dhcpInit();
		  PORT_MAIL_S=5789;
#endif





delay_funkc[21]=1; //czekam na sygnal od ESP jezeli nie doczekam sie przez 5 sekund to go wylaczam
delay_funkc[15]=1;  //Send arp request
  wybor2_bitowy&=~0x0080;
  liczba_arp=0;
  delay_funkc[23]=1; //Sprawdzanie czy 'send arp request' dotarl

//if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0x32F2) dbg3("\r\nSEX !!!!");


		//  RTC_CalendarShow(buf_p, &buf_p[50]);
		//  dbg3(buf_p); dbg3(&buf_p[50]);

PobierzCzas(buf_p);
sprintf(&buf_p[100], "%02d:%02d:%02d %02d/%02d/%02d",buf_p[4],buf_p[5],buf_p[6] ,buf_p[0],buf_p[1],buf_p[2]); dbg3(&buf_p[100]);

ZapisZdarzenia(_ZdaRST,_PogrUrzytk,0);


for(i=0;i<_Ile_Czujek;i++)
{
	Lora[i]=0;
	start_odpytyw[i]=0;
}





//uint32_t a;
//ADC_kanal(ADC_CHANNEL_7);
  while (1)
  {

#ifdef _LoRa
	  Odbior_LoRa();
#endif

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	//  HAL_ADC_Start(&hadc1);   HAL_Delay(1);
	 // a= HAL_ADC_GetValue(&hadc1);
	        // sprintf(buf_p,"\r\nADC: %d",a); dbg3(buf_p);  HAL_Delay(500);

	  modem_uart();

	  if((wybor_bitowy&0x0400)>0) ObslugaTimer();

	  HAL_IWDG_Refresh(&hiwdg);


#ifdef _ENC

   if(email_enc>0)
   {  if(delay_funkc[28]==3)  //retr dla email
	  {
		  //netstackService();
		  delay_funkc[28]=0;
	  	   dbg3("\r\nRetr");
	  	     j =  ((unsigned short)buf_retrans[0]<<8)&0xFF00  |  ((unsigned short)buf_retrans[1]<<0)&0x00FF;
	  		             nicSend(j,  &buf_retrans[2]);  HAL_Delay(200);  delay_funkc[28]=1;
	  		//dbg3("\r\nRetr"); nicSend(j,  &buf_retrans[2]);  HAL_Delay(400);

	  	  //debugPrintHexTable(dowymazz, buf_retrans);

	 }}


	 if(delay_funkc[13]==10)  //email ENC STOP    //PRZY DUZYCH MAILI MUSI BYC CHYBA RETRANSMISJA PAKIETOW BO INACZEJ NIGDY NIE WYSLE  !!!!!
	 {
		if((wybor_bitowy&0x1000)==0)
		{
			dbg3("\r\nEmail ENC powtorka ");
			wybor_bitowy|=0x1000;
			if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o'))  wyslij_email_raport();
			else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z'))
			{
					                             if(Const.s_Rap[0].siec==0)  //LAN
						    	    		     {
						    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
						    	    		    	 {
						    	    		    		  sprintf(email_tytul,"RejZda");
						    	    		    		  Wyslij_email_ENC();
						    	    		    	 }
						    	    		     }

			}
			else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i'))
			{
					                             if(Const.s_Rap[0].siec==0)  //LAN
						    	    		     {
					                            	 if(Const.s_Rap[0].plik==1) //z Pomiarami
						    	    		    	 {
						    	    		    		 sprintf(email_tytul,"Pomiar");
						    	    		    		 Wyslij_email_ENC();
						    	    		    	 }
						    	    		     }

			}
			else wyslij_email();
		}
		else
		{
		   email_enc=0; wysylka_mail=0;
		   if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o'));
		   else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z'));
		   else{
		        for(j=0;j<_Ilosc_Mail;j++){ i_mail[j]=0; for(i=0;i<_Dlg_buf_Mail;i++) b_mail[i+_Dlg_buf_Mail*j]=0; }
		   }
	  	    dbg3("\r\nEmail ENC koniec ");
	  	   delay_funkc[13]=0;
		}
     }

//if((wybor_bitowy&0x0400)>0){  //start na rozpoczecie pomiarow
 if(email==0)
 {

	   netstackService();

	     for(i=0;i<HTTP_task_LEN;i++)
		 {
	       if(replay_packet[i]<200)
		   {
			  //replay_packet[i]++;

		     //if(replay_packet[i]>40)
			 if(replay_packet[i]>190)
		     {
				//  nicInit();
				  j =  ((unsigned short)buf_retrans[0]<<8)&0xFF00  |  ((unsigned short)buf_retrans[1]<<0)&0x00FF;
				  nicSend(j,  &buf_retrans[2]);
				  j =  ((unsigned short)buf_retrans2[0]<<8)&0xFF00  |  ((unsigned short)buf_retrans2[1]<<0)&0x00FF;
				  nicSend(j,  &buf_retrans2[2]);
				  sprintf(buf_p,"\r\nREPLAY:%d  ",i);
				  dbg3(buf_p);
				  HAL_Delay(50);
				  replay_packet[i]++;
				 // replay_packet[i]+=20;
	         }

	       }
		 }

 } //}
#endif



  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
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
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2){
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
   dbg3("\r\nRESET ZEGARA ");
    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 1;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0x32F2);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 0;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 0;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 0;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

}

/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 0;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim4);

}

/* TIM6 init function */
static void MX_TIM6_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM8 init function */
static void MX_TIM8_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 0;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 0;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim8);

}

/* UART4 init function */
static void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 460800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART3 init function */
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 460800;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* WWDG init function */
static void MX_WWDG_Init(void)
{

  hwwdg.Instance = WWDG;
  hwwdg.Init.Prescaler = WWDG_PRESCALER_1;
  hwwdg.Init.Window = 64;
  hwwdg.Init.Counter = 64;
  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
  if (HAL_WWDG_Init(&hwwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA2_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_3 
                          |GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE5 
                           PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC8 PC9 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 PD10 PD11 
                           PD12 PD13 PD14 PD3 
                           PD4 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_3 
                          |GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void MX_USART3_UART_Init_115(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void MX_USART3_UART_Init_460(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 460800;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

static void MX_IWDG_Init_moj(void)
{

 /* hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }*/

}

void RstGpioKonfig()
{
	SetPinAsInput(RST_GPIO, RST_GPIO_nr);
	HAL_Delay(100);
    if(TM_GPIO_GetInputPinValue(RST_GPIO, RST_GPIO_nr)==1)
    {  HAL_Delay(100);
       if(TM_GPIO_GetInputPinValue(RST_GPIO, RST_GPIO_nr)==1)
       {    HAL_Delay(100);
           if(TM_GPIO_GetInputPinValue(RST_GPIO, RST_GPIO_nr)==1)
           {   HAL_Delay(100);
               if(TM_GPIO_GetInputPinValue(RST_GPIO, RST_GPIO_nr)==1)
               {   HAL_Delay(100);
                   if(TM_GPIO_GetInputPinValue(RST_GPIO, RST_GPIO_nr)==1)
                   {
                	   dbg3("\r\nRST_FABR. !!! ...");
                	   Const.s_Lan[1].param=0;  //tak jakby pierszy raz uruchamiamy program uzytk.  !=0x55
                   }
               }
           }
       }
    }
}

uint32_t Mediana_pt(int nr)
{
	uint32_t buf[Size_probe_pt+1], out[Size_probe_pt+1], ile_pozost, k, n, i, j;
    ile_pozost = Size_probe_pt;

    for(j=0;j<ile_pozost;j++) buf[j]=tab_pt[nr][j];


				  n=0;

				  ghghyun:
				  for(j=0;j<ile_pozost;j++)
				  {
				     k=0;
				     for(i=0;i<ile_pozost;i++)
				     {
				       if(i!=j){
				           if(buf[j]<=buf[i]){ k++; }
				       }
				     }

				     if(k==ile_pozost-1)
				     {
				    	out[n++]=buf[j];
				        for(i=j;i<ile_pozost-1;i++)
				        {
				        	buf[i]=buf[i+1];
				        }
				        ile_pozost--;
				        if(ile_pozost==0){  break;  }
				        else{  goto ghghyun; }
				     }
				  }

//sprintf(buf_p,"--%d%d%d%d%d--",out[0],out[1],out[2],out[3],out[4]); dbg3(buf_p);


     return out[Size_probe_pt/2];

}

uint32_t Mediana_tt(int nr)
{
	uint32_t buf[Size_probe_tt+1], out[Size_probe_tt+1], ile_pozost, k, n, i, j;
    ile_pozost = Size_probe_tt;

    for(j=0;j<ile_pozost;j++) buf[j]=tab_tt[nr][j];


				  n=0;

				  ghghyun:
				  for(j=0;j<ile_pozost;j++)
				  {
				     k=0;
				     for(i=0;i<ile_pozost;i++)
				     {
				       if(i!=j){
				           if(buf[j]<=buf[i]){ k++; }
				       }
				     }

				     if(k==ile_pozost-1)
				     {
				    	out[n++]=buf[j];
				        for(i=j;i<ile_pozost-1;i++)
				        {
				        	buf[i]=buf[i+1];
				        }
				        ile_pozost--;
				        if(ile_pozost==0){  break;  }
				        else{  goto ghghyun; }
				     }
				  }

//sprintf(buf_p,"--%d%d%d%d%d--",out[0],out[1],out[2],out[3],out[4]); dbg3(buf_p);


     return out[Size_probe_tt/2];

}

int16_t Wpis_Pt1000(uint16_t res)
{
	uint16_t Pt[106];     int i,j,roz,itr,kon;   int16_t temp;

	Pt[0]=185;  Pt[10]=602;  Pt[20]=1000;  Pt[30]=1385;  Pt[40]=1758;
	Pt[1]=228;  Pt[11]=643;  Pt[21]=1039;  Pt[31]=1423;  Pt[41]=1795;
	Pt[2]=271;  Pt[12]=683;  Pt[22]=1078;  Pt[32]=1461;  Pt[42]=1832;
	Pt[3]=313;  Pt[13]=723;  Pt[23]=1117;  Pt[33]=1498;  Pt[43]=1868;
	Pt[4]=355;  Pt[14]=763;  Pt[24]=1155;  Pt[34]=1536;  Pt[44]=1905;
	Pt[5]=397;  Pt[15]=803;  Pt[25]=1194;  Pt[35]=1573;  Pt[45]=1941;
	Pt[6]=439;  Pt[16]=843;  Pt[26]=1232;  Pt[36]=1610;  Pt[46]=1977;
	Pt[7]=480;  Pt[17]=882;  Pt[27]=1271;  Pt[37]=1648;  Pt[47]=2013;
	Pt[8]=521;  Pt[18]=922;  Pt[28]=1309;  Pt[38]=1685;  Pt[48]=2049;
	Pt[9]=562;  Pt[19]=961;  Pt[29]=1347;  Pt[39]=1722;  Pt[49]=2085;

	Pt[50]=2120;  Pt[60]=2471;  Pt[70]=2810;  Pt[80]=3137;  Pt[90]=3453;
	Pt[51]=2156;  Pt[61]=2505;  Pt[71]=2843;  Pt[81]=3169;  Pt[91]=3484;
	Pt[52]=2191;  Pt[62]=2540;  Pt[72]=2876;  Pt[82]=3201;  Pt[92]=3515;
	Pt[53]=2227;  Pt[63]=2574;  Pt[73]=2909;  Pt[83]=3233;  Pt[93]=3545;
	Pt[54]=2262;  Pt[64]=2608;  Pt[74]=2942;  Pt[84]=3265;  Pt[94]=3576;
	Pt[55]=2297;  Pt[65]=2642;  Pt[75]=2975;  Pt[85]=3296;  Pt[95]=3606;
	Pt[56]=2332;  Pt[66]=2676;  Pt[76]=3007;  Pt[86]=3328;  Pt[96]=3637;
	Pt[57]=2367;  Pt[67]=2709;  Pt[77]=3040;  Pt[87]=3359;  Pt[97]=3667;
	Pt[58]=2402;  Pt[68]=2743;  Pt[78]=3072;  Pt[88]=3391;  Pt[98]=3697;
	Pt[59]=2436;  Pt[69]=2776;  Pt[79]=3105;  Pt[89]=3422;  Pt[99]=3727;

    Pt[100]=3757;
    Pt[101]=3787;
    Pt[102]=3816;
    Pt[103]=3846;
    Pt[104]=3875;
    Pt[105]=3905;

    kon=0;
    for(i=0;i<105;i++)
    {
    	if((res>=Pt[i])&&(res<Pt[i+1])){ kon=1; break; }
    }
    if(kon==0) return 0x7FFF;

    roz= 10*(Pt[i+1]-Pt[i]);    itr= roz/10;


    for(j=0;j<10;j++)
    {
    	if(10*res < (10*Pt[i]+j*itr)) break;
    }

    temp  =  ((i-20)*10) + (j-1);

    return temp;

}

void IntToChar(unsigned short b, unsigned char *buf)
{
	unsigned short x1,x2,x3,x4,i;

	x1=b/1000;
	x2=(b-1000*x1)/100;
	x3=(b-1000*x1-100*x2)/10;
	x4=(b-1000*x1-100*x2-10*x3);

	if(x4>5)
	{  b+=5;
	   x1=b/1000;
	   x2=(b-1000*x1)/100;
	   x3=(b-1000*x1-100*x2)/10;
	   x4=(b-1000*x1-100*x2-10*x3);
	}

    if(x1==0) sprintf(buf,"%d.%d",x2,x3);
    else      sprintf(buf,"%d%d.%d",x1,x2,x3);
}

int ADC_avr_TT(int nr)   //ZROB KOREKTE O +0.2V  dodac powyzej napiec jakis !!!!!!!!!!!!!!!!!!!!!!!!!
{
	uint32_t a,b,c,i;   uint32_t t1,t2,t3,t4,t5,r;

	HAL_ADC_Start(&hadc1);HAL_Delay(1);

	for(i=0;i<Size_probe_tt-1;i++)  tab_tt[nr][i] = tab_tt[nr][i+1];
	tab_tt[nr][Size_probe_tt-1]=HAL_ADC_GetValue(&hadc1);
	a= Mediana_tt(nr);


    	t1=(34000*a)/4096;    t1+=560;     t1=3*t1;  t1=t1/100;

    	if(a==0) t1=0;  //korekta dla najmniejszych napiec
    	else if(a<10) t1=10;
    	else if((a>9)&&(a<21)) t1=20;


    	     if((t1>=100)&&(t1<300)) t1+=10;  //korekta doswiadczalna
    	else if((t1>=300)&&(t1<600)) t1+=20;
    	else if(t1>=600)             t1+=30;

    	Const.s_ADC_TT[nr].val=t1;

    	//sprintf(buf_p,"\r\nB%d: %d   ",nr,a); dbg3(buf_p);

    return b;
}


int ADC_avr_pt(int nr)
{
	uint32_t a,b,c,i,j,bx[Size_probe_pt];   uint32_t t1,t2,t3,t4,t5,r;

    	HAL_ADC_Start(&hadc1);

    	a=0;
    	for(i=0;i<Size_probe_pt;i++)
    	{
    		czekaj_us(300);
    		//HAL_Delay(1);
    		bx[i]=HAL_ADC_GetValue(&hadc1);
    		a+=bx[i];
    	}
    	a=a/Size_probe_pt;


    	  t1=(34000*a)/4096;         //t1+=560;        // r = (t1*1Kohm)/(3,3V-t1)      t1-napiecie       a - wartosc z ADC
    	  r=(t1*1000)/(34000-t1);  r+=60;
    	  if(a>3500) r=0;

    	if(Wpis_Pt1000(r)!=0x7FFF)  //jezeli pomiar nie bedzie w przedziale dozwolonych napiec dla Pt1000  to nie przesuwaj pomiarów w medianie
    	{
    		for(i=0;i<Size_probe_pt-1;i++)  tab_pt[nr][i] = tab_pt[nr][i+1];
    	 	tab_pt[nr][Size_probe_pt-1]=a;

    	  //  a= Mediana_pt(nr);


    	 	c=0;  j=0;
    	 	for(i=0;i<Size_probe_pt;i++)
    	 	{
    	 		if(tab_pt[nr][i]<0xffff){   c+=tab_pt[nr][i];  j++; }
    	 	}
    	 	a=c/j;


        	  t1=(34000*a)/4096;         //t1+=560;        // r = (t1*1Kohm)/(3,3V-t1)      t1-napiecie       a - wartosc z ADC
        	  r=(t1*1000)/(34000-t1);  r+=60;
        	  if(a>3500) r=0;
    	}


    	  Const.s_ADC_pt[nr].val = Wpis_Pt1000(r);


    	  i= WyszukajNrCzujkiDlaPortu(t_pt[nr]);
    	  if(i!=-1)
    	  {
    		  if(Const.s_ADC_pt[nr].val!=0x7FFF){
    		     Const.s_ADC_pt[nr].val = (signed short)Const.s_ADC_pt[nr].val + Const.s_Czujki[i].korekcja;
        	     Var.value[i]= (signed short)Const.s_ADC_pt[nr].val;

          	    // sprintf(buf_p,"\r\nP %d  %d",a,Var.value[i]); dbg3(buf_p);   //wskazania wahaja sie o 10 z bezposr. wartosci ADC
    		  }
    		  else Var.value[i]=0x7FFF;
    	  }


    return b;
}

void ADC_avr_eol(int nr)
{
	int i;
	i=nr;
	uint32_t a[5];

	HAL_ADC_Start(&hadc1); 	while (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK); //czekaj_us(300);
	a[0]=HAL_ADC_GetValue(&hadc1);


		if(lifeLora[t_eol[i]-1]==0)
		{

			if(a[0]>(4096-(4096/12)))
			{
				Var.eol[i/16] |= (1<<(i-((i/16)*16)));
			}
			else
			{
				Var.eol[i/16] &= ~(1<<(i-((i/16)*16)));

				if(a[0]>(4096-(4096/5))){ Var.we[i/16] |= (1<<(i-((i/16)*16)));    }
				else   		  			 { Var.we[i/16] &= ~(1<<(i-((i/16)*16)));   }
			}
		}
}

void ConvertParamNET()
{
  int i,j,k;  char *ptr,buf[4];

  for(k=0;k<3;k++)
  {
	     if(k==0) ptr=Const.s_Lan[1].ip;
	else if(k==1) ptr=Const.s_Lan[1].mask;
	else if(k==2) ptr=Const.s_Lan[1].br;

	buf[0]=0;buf[1]=0;buf[2]=0;buf[3]=0;
	for(j=0;j<4;j++)
    {
		i=0; do{i++; if(i>5) break; ptr++; if(*ptr=='.') break; if(*ptr==0) break; }while(1);
            if(i==1) buf[j]=(*(ptr-1))&0x0f;
       else if(i==2) buf[j]=10*((*(ptr-2))&0x0f) + ((*(ptr-1))&0x0f);
       else if(i==3) buf[j]=100*((*(ptr-3))&0x0f) + 10*((*(ptr-2))&0x0f) + ((*(ptr-1))&0x0f);
       ptr++;
    }
          if(k==0){ IPADDRESS = IPDOT(buf[0],buf[1],buf[2],buf[3]);   sprintf(buf_p,"\r\nIP: %d.%d.%d.%d ",buf[0],buf[1],buf[2],buf[3] ); dbg3(buf_p); }
    else  if(k==1){ NETMASK   = IPDOT(buf[0],buf[1],buf[2],buf[3]);  sprintf(buf_p,"\r\nMa: %d.%d.%d.%d ",buf[0],buf[1],buf[2],buf[3] ); dbg3(buf_p); }
    else  if(k==2){ GATEWAY   = IPDOT(buf[0],buf[1],buf[2],buf[3]);  sprintf(buf_p,"\r\nBr: %d.%d.%d.%d ",buf[0],buf[1],buf[2],buf[3] ); dbg3(buf_p); }
  }
  PORT = wybierz_cyfry(Const.s_Lan[1].port);   sprintf(buf_p,"\r\nPo: %d ",PORT ); dbg3(buf_p);
}

uint8_t computeDayOfWeek(uint16_t y, uint8_t m, uint8_t d)
{
	uint32_t h;
	uint32_t j;
	uint32_t k;

   //January and February are counted as months 13 and 14 of the previous year
   if(m <= 2)
   {
      m += 12;
      y -= 1;
   }

   //J is the century
   j = y / 100;
   //K the year of the century
   k = y % 100;

   //Compute H using Zeller's congruence
   h = d + (26 * (m + 1) / 10) + k + (k / 4) + (5 * j) + (j / 4);

   //Return the day of the week
   return ((h + 5) % 7) + 1;
}
void UstawCzas(uint8_t rok, uint8_t mies, uint8_t dzien,  uint8_t godz, uint8_t min, uint8_t sec)
{
	  RTC_TimeTypeDef sTime;
	  RTC_DateTypeDef sDate;

	 // if (HAL_RTC_DeInit(&hrtc) != HAL_OK);
	  //MX_RTC_Init_moj();


	  sDate.WeekDay = computeDayOfWeek(rok,mies,dzien);     //oblicz_dzien();
	  sDate.Month = mies;
	  sDate.Date = dzien;
	  sDate.Year = rok;
	  if(HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK);//  Error_Handler();

	  sTime.Hours = godz;
	  sTime.Minutes = min;
	  sTime.Seconds = sec;
	  if(HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK);//  Error_Handler();




	  HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0x32F2);

	  /*
	  sDate.WeekDay = oblicz_dzien();
	  sDate.Month = mies;
	  sDate.Date = dzien;
	  sDate.Year = rok;
      if(HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK);//  Error_Handler();
*/

}


void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
  /* Display time Format : hh:mm:ss */
  sprintf((char *)showtime, "%2d:%2d:%2d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char *)showdate, "%2d-%2d-%2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
}

void PobierzCzas(uint8_t *buf)
{
	  RTC_TimeTypeDef sTime;
	  RTC_DateTypeDef sDate;

	  //HAL_RTC_WaitForSynchro(&hrtc);
	  if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)  Error_Handler();
	  if(HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)  Error_Handler();

	  buf[0]=sDate.Year;
	  buf[1]=sDate.Month;
	  buf[2]=sDate.Date;
	  buf[3]=sDate.WeekDay;

	  buf[4]=sTime.Hours;
	  buf[5]=sTime.Minutes;
	  buf[6]=sTime.Seconds;
}

void RTC_konfiguracja()
{
		  hrtc.Instance = RTC;
		   hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
		    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
		    if (HAL_RTC_Init_Konfig(&hrtc) != HAL_OK);

		  RTC_DateTypeDef  sdatestructure;
		      sdatestructure.Month = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
		      sdatestructure.Date  = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
		      sdatestructure.Year  = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
		   HAL_RTC_SetDate(&hrtc,&sdatestructure,RTC_FORMAT_BIN);

  HAL_Delay(300);

		   HAL_RTC_GetDate(&hrtc,&sdatestructure,RTC_FORMAT_BIN);
			    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, sdatestructure.Month);
			    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, sdatestructure.Date);
			    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, sdatestructure.Year);


}


void BuzGpioKonfig()
{
    SetPinAsOutput(GPIOE, GPIO_PIN_11);
    TM_GPIO_SetPinLow(GPIOE, GPIO_PIN_11);
}

void LcdGpioKonfig()
{
	SetPinAsOutput(GPIOE, GPIO_PIN_1);  //LCD_SCE
	SetPinAsOutput(GPIOB, GPIO_PIN_9);  //LCD_RST
	SetPinAsOutput(GPIOB, GPIO_PIN_8);  //LCD_D/C
	SetPinAsOutput(GPIOB, GPIO_PIN_6);   //LCD_SCK
	SetPinAsOutput(GPIOB, GPIO_PIN_4);   //LCD_MOSI

	  TM_GPIO_SetPinLow(GPIOE, GPIO_PIN_1);
	  TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_9);
	  TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_8);
	  TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_6);
	  TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_4);
}

void LoRaGpioKonfig()
{
	SetPinAsInput(GPIOB, GPIO_PIN_7);   //LoRa_DIO0
	SetPinAsOutput(GPIOB, GPIO_PIN_6);  //LoRa_SCK
	SetPinAsInput(GPIOB, GPIO_PIN_5);   //LoRa_MISO
	SetPinAsOutput(GPIOB, GPIO_PIN_4);  //LoRa_MOSI
	SetPinAsOutput(GPIOD, GPIO_PIN_3);  //LoRa_NSS

	  //TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_7);
	  TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_6);
	  //TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_5);
	  TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_4);
	  TM_GPIO_SetPinLow(GPIOD, GPIO_PIN_3);
}

void convertUnixTimeToDate(uint32_t t)
{

	uint32_t a;
   uint32_t b;
   uint32_t c;
   uint32_t d;
   uint32_t e;
   uint32_t f;

   //Negative Unix time values are not supported
   if(t < 1)
      t = 0;

   //Clear milliseconds
   date.milliseconds = 0;

   //Retrieve hours, minutes and seconds
   date.seconds = t % 60;
   t /= 60;
   date.minutes = t % 60;
   t /= 60;
   date.hours = t % 24;
   t /= 24;

   //Convert Unix time to date
   a = (uint32_t) ((4 * t + 102032) / 146097 + 15);
   b = (uint32_t) (t + 2442113 + a - (a / 4));
   c = (20 * b - 2442) / 7305;
   d = b - 365 * c - (c / 4);
   e = d * 1000 / 30601;
   f = d - e * 30 - e * 601 / 1000;

   //January and February are counted as months 13 and 14 of the previous year
   if(e <= 13)
   {
      c -= 4716;
      e -= 1;
   }
   else
   {
      c -= 4715;
      e -= 13;
   }

   //Retrieve year, month and day
   date.year = c;
   date.month = e;
   date.day = f;

   //Calculate day of week
   date.dayOfWeek = computeDayOfWeek(c, e, f);

}





uint32_t convertDateToUnixTime()
{
	 uint32_t y;
	 uint32_t m;
	 uint32_t d;
   uint32_t t;

   //Year
   y = date.year;
   //Month of year
   m = date.month;
   //Day of month
   d = date.day;

   //January and February are counted as months 13 and 14 of the previous year
   if(m <= 2)
   {
      m += 12;
      y -= 1;
   }


   //Convert years to days
   t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
   //Convert months to days
   t += (30 * m) + (3 * (m + 1) / 5) + d;
   //Unix time starts on January 1st, 1970
   t -= 719561;
   //Convert days to seconds
   t *= 86400;
   //Add hours, minutes and seconds
   t += (3600 * date.hours) + (60 * date.minutes) + date.seconds;
   //Return Unix time

  // sprintf(buf_p,"\r\nUNIX tIME:  %d  ", t );  dbg3(buf_p);
   return t;
}



int8_t ResetGateLogic(int nrPageOpLogic)
{
    for(int i=0;i<MAX_INITIAL_LOGIC_GATES;i++)
    { 	 for(int j=0;j<MAX_INPUT_INITIAL_LOGIC_GATE;j++)
    	 {
    		Const.Logic[nrPageOpLogic].InitGate[i].NameInputSignal[j]=0;
    		Const.Logic[nrPageOpLogic].InitGate[i].inputBuffer[j]=BUFFER;
    	 }
    	 Const.Logic[nrPageOpLogic].InitGate[i].logicalOperation=0;
    }

    for(int i=0;i<MAX_ADDITIONAL_LOGIC_GATES;i++)
    { 	 for(int j=0;j<MAX_INPUT_ADD_LOGIC_GATE;j++)
    	 {
    		Const.Logic[nrPageOpLogic].AddGate[i].NameInputSignal[j]=j+1;
    		Const.Logic[nrPageOpLogic].AddGate[i].inputBuffer[j]=BUFFER;
    	 }
    	 Const.Logic[nrPageOpLogic].AddGate[i].logicalOperation=0;
    }

    for(int i=0;i<MAX_FINAL_LOGIC_GATES;i++)
    { 	 for(int j=0;j<MAX_INPUT_FIN_LOGIC_GATE;j++)
    	 {
    		Const.Logic[nrPageOpLogic].FinGate[i].NameInputSignal[j]=j+1;
    		Const.Logic[nrPageOpLogic].FinGate[i].inputBuffer[j]=BUFFER;
    	 }
    	 Const.Logic[nrPageOpLogic].FinGate[i].logicalOperation=0;
    }
}

void ResetFlash()  //rst
{
   int i,j,k;   signed short Tmax,Tmin,hist,kor;

   GPIO_TypeDef* GPIOx;
   uint16_t GPIO_Pin;

   sprintf(Const.s_Lan[0].eth,"a220a632c276");   //Wifi
 	   	sprintf(Const.s_Lan[0].ip,"192.168.1.31");
 	   	sprintf(Const.s_Lan[0].br,"192.168.1.1");
 	   	sprintf(Const.s_Lan[0].mask,"255.255.255.0");
 	   	sprintf(Const.s_Lan[0].port,"80");
 	   	Const.s_Lan[0].param=0x01|0x0C;
 	    Const.s_Lan[0].param&=~0x04;   //0x04-STA tak/nie

 	   sprintf(Const.s_Lan[1].eth,"a020a632c275");   //LAN
 	   	   	sprintf(Const.s_Lan[1].ip,"192.168.1.32");
 	   	   	sprintf(Const.s_Lan[1].br,"192.168.1.1");
 	   	   	sprintf(Const.s_Lan[1].mask,"255.255.255.0");
 	   	   	sprintf(Const.s_Lan[1].port,"80");
 	   	   	Const.s_Lan[1].param=0x55;

 	     	Const.s_Wifi[0].ssid[0]=0;
 	        Const.s_Wifi[0].haslo[0]=0;

 	  //  sprintf(Const.s_Wifi[0].ssid,"NETGEAR");
 	  //  sprintf(Const.s_Wifi[0].haslo,"123456789abc");

    // sprintf(Const.s_Wifi[0].ssid,"PLAY INTERNET 4G LTE-0477");
 	  // 	sprintf(Const.s_Wifi[0].haslo,"LAJE1YDNJ5M");

 	    sprintf(Const.s_Wifi[0].AP_ssid, Wifi_AP_ssid);
 	    sprintf(Const.s_Wifi[0].AP_haslo, Wifi_AP_haslo);

 	    Const.s_Email[0].nad[0]=0;
 	    Const.s_Email[0].has[0]=0;
 	    Const.s_Email[0].ip[0]=0;
 	  //  Const.s_Email[0].nad2[0]=0;
 	   // Const.s_Email[0].has2[0]=0;
 	   // Const.s_Email[0].ip2[0]=0;
 	    Const.s_Email[0].port[0]=0;
 	    Const.s_Email[0].odb1[0]=0;
 	    Const.s_Email[0].odb2[0]=0;
 	    Const.s_Email[0].odb3[0]=0;
 	    Const.s_Email[0].odb4[0]=0;
 	    Const.s_Email[0].odb5[0]=0;
 	    Const.s_Email[0].odb6[0]=0;
 	    Const.s_Email[0].odb7[0]=0;
 	    Const.s_Email[0].odb8[0]=0;




 	   // for(i=1;i<7;i++) sprintf(Const.s_PK[i-1].nazwa, "Wyjscie Przekaznikowe nr %d", i);
 	   // for(i=7;i<11;i++) sprintf(Const.s_PK[i-1].nazwa, "Wyjscie typu OC,IR nr %d", i-6);
 	   // for(i=11;i<14;i++) sprintf(Const.s_PK[i-1].nazwa, "Wyjscie typu OC,IR,PWM nr %d", i-10);


 	 	for(i=0;i<_Size_wy;i++)
 	 	{
 	 	    sprintf(Const.s_PK[i].nazwa_on, "Stan ON");
 	 	    sprintf(Const.s_PK[i].nazwa_of, "Stan OFF");

 	 	    if((t_wy[i]==__Port_PWM_1)\
 	 	     ||(t_wy[i]==__Port_PWM_2)\
			 ||(t_wy[i]==__Port_PWM_3) )  sprintf(Const.s_PK[i].nazwa, "Wyjscie typu OC,IR,PWM");

 	 	    if((t_wy[i]==__Port_OC_1)\
 	 	     ||(t_wy[i]==__Port_OC_2)\
 	 	     ||(t_wy[i]==__Port_OC_3)\
			 ||(t_wy[i]==__Port_OC_4) )  sprintf(Const.s_PK[i].nazwa, "Wyjscie typu OC,IR");

 	 	    if((t_wy[i]==__Port_PK_1)\
 	 	     ||(t_wy[i]==__Port_PK_2)\
 	 	     ||(t_wy[i]==__Port_PK_3)\
 	 	     ||(t_wy[i]==__Port_PK_4)\
 	 	     ||(t_wy[i]==__Port_PK_5)\
			 ||(t_wy[i]==__Port_PK_6) )  sprintf(Const.s_PK[i].nazwa, "Wyjscie Przekaznikowe");

 	 	 }



 	   wartTemp("+99.9", &Tmax);
 	   wartTemp("-55.0", &Tmin);
 	   wartTemp("+00.5", &hist);
 	   wartTemp("+00.0", &kor);
   for(i=0;i<_Size__s_Czujki_info;i++)
   {
	    sprintf(Const.s_Czujki[i].nazwa,"Nazwa czujnika");
	    Const.s_Czujki[i].prog_max1= Tmax;
	    Const.s_Czujki[i].prog_max2= Tmax;
	    Const.s_Czujki[i].prog_min1= Tmin;
	    Const.s_Czujki[i].prog_min2= Tmin;
	    Const.s_Czujki[i].hist= hist;
	    Const.s_Czujki[i].korekcja= kor;
	    Const.s_Czujki[i].czas_reak= 1;
	    Const.s_Czujki[i].ilosc_alarm= 1;
	    Const.s_Czujki[i].alarm_pk= 0;

	    if((Const.s_Rej[0].param&0x01)==0){
	    	for(j=0;j<_Size__s_Czujki_info;j++) Const.s_Czujki[i].addr_DS[j]=0;
	    }

   }

   sprintf(Const.s_Tel[0].tel0,"+48000000000");
   sprintf(Const.s_Tel[0].tel1,"+48000000000");
   sprintf(Const.s_Tel[0].tel2,"+48000000000");
   sprintf(Const.s_Tel[0].tel3,"+48000000000");
   sprintf(Const.s_Tel[0].tel4,"+48000000000");
   sprintf(Const.s_Tel[0].tel5,"+48000000000");
   sprintf(Const.s_Tel[0].tel6,"+48000000000");
   sprintf(Const.s_Tel[0].tel7,"+48000000000");
   sprintf(Const.s_Tel[0].tel8,"+48000000000");

   for(i=0;i<_Size__s_gpio;i++)
   {
	   if((Const.s_Rej[0].param&0x01)>0){
		   if((Const.s_GPIO[i].val==9)||(Const.s_GPIO[i].val==2));
		   else Const.s_GPIO[i].val=6;  //ma byc 6
	   }
	   else Const.s_GPIO[i].val=6;  //ma byc 6

	   sprintf(Const.s_GPIO[i].nazwa,"Nazwa portu nr %d",i+1);
       Var.gpio[i]=0;
   }
   sprintf(Const.s_GPIO[15-1].nazwa+strlen(Const.s_GPIO[15-1].nazwa),"     0-10V");
   sprintf(Const.s_GPIO[16-1].nazwa+strlen(Const.s_GPIO[16-1].nazwa),"     0-10V");

   sprintf(Const.s_GPIO[17-1].nazwa+strlen(Const.s_GPIO[17-1].nazwa),"     E1");
   sprintf(Const.s_GPIO[18-1].nazwa+strlen(Const.s_GPIO[18-1].nazwa),"     E2");
   sprintf(Const.s_GPIO[19-1].nazwa+strlen(Const.s_GPIO[19-1].nazwa),"     E3");
   sprintf(Const.s_GPIO[20-1].nazwa+strlen(Const.s_GPIO[20-1].nazwa),"     E4");

   sprintf(Const.s_GPIO[21-1].nazwa+strlen(Const.s_GPIO[21-1].nazwa),"     PW1");
   sprintf(Const.s_GPIO[22-1].nazwa+strlen(Const.s_GPIO[22-1].nazwa),"     PW2");
   sprintf(Const.s_GPIO[23-1].nazwa+strlen(Const.s_GPIO[23-1].nazwa),"     PW3");

   sprintf(Const.s_GPIO[24-1].nazwa+strlen(Const.s_GPIO[24-1].nazwa),"     OC1");
   sprintf(Const.s_GPIO[25-1].nazwa+strlen(Const.s_GPIO[25-1].nazwa),"     OC2");
   sprintf(Const.s_GPIO[26-1].nazwa+strlen(Const.s_GPIO[26-1].nazwa),"     OC3");
   sprintf(Const.s_GPIO[27-1].nazwa+strlen(Const.s_GPIO[27-1].nazwa),"     OC4");

   sprintf(Const.s_GPIO[28-1].nazwa+strlen(Const.s_GPIO[28-1].nazwa),"     PK1");
   sprintf(Const.s_GPIO[29-1].nazwa+strlen(Const.s_GPIO[29-1].nazwa),"     PK2");
   sprintf(Const.s_GPIO[30-1].nazwa+strlen(Const.s_GPIO[30-1].nazwa),"     PK3");
   sprintf(Const.s_GPIO[31-1].nazwa+strlen(Const.s_GPIO[31-1].nazwa),"     PK4");
   sprintf(Const.s_GPIO[32-1].nazwa+strlen(Const.s_GPIO[32-1].nazwa),"     PK5");
   sprintf(Const.s_GPIO[33-1].nazwa+strlen(Const.s_GPIO[33-1].nazwa),"     PK6");




   for(i=0;i<_Size__s_pwm;i++)   //PWM tylko na 6 portach.  Port_6 nie ma PWM
   {    Const.s_PWM[i].freq=48000/8;
        Const.s_PWM[i].duty=50;
        //48000    - 1kHz
        //48000/2  - 2kHz
        //48000/4  - 4kHz
   }
   for(i=0;i<_Size_we;i++)
   {
	   sprintf(Const.s_WE[i].nazwa,"Nazwa dla Wejscia %d", i+1);
	   sprintf(Const.s_WE[i].nazwa_ro,"Stan OFF");
	   sprintf(Const.s_WE[i].nazwa_zw,"Stan ON");
	   sprintf(Const.s_WE[i].nazwa_uszk,"Uszkodzona linia");
	   Const.s_WE[i].czas_reak = 1;
	   Const.s_WE[i].czas_przyt = 0;
	   Const.s_WE[i].ilosc_alarm = 0;
	   Const.s_WE[i].pk = 0;
	   Const.s_WE[i].param = 0;
   }
   for(i=0;i<(_Size_wy);i++)
   {
	   wartTemp("+50.0", &Tmax);
 	   wartTemp("+00.5", &hist);
	   Const.s_Term[i].prog= Tmax;
	   Const.s_Term[i].hist= hist;
	   Const.s_Term[i].nr_temp= 0;
	   Const.s_Term[i].czas_reak = 1;
	   Const.s_Term[i].akt = 0;
   }
   for(i=0;i<(_Size_wy);i++)
   {
	   Const.s_Tim[i].godzON1 = 0;
	   Const.s_Tim[i].godzON2 = 0;
	   Const.s_Tim[i].godzON3 = 0;
	   Const.s_Tim[i].godzON4 = 0;
	     Const.s_Tim[i].godzOF1 = 0;
	     Const.s_Tim[i].godzOF2 = 0;
	     Const.s_Tim[i].godzOF3 = 0;
	     Const.s_Tim[i].godzOF4 = 0;
	   Const.s_Tim[i].akt = 0;
   }
   Const.s_Poz[0].ekran=0;
   Const.s_Poz[0].refr=1;

   for(i=0;i<_Ilosc_Tel;i++)
   {
	   for(j=0;j<(1+(_Size_te-1)/16);j++){
	     Const.s_Ma[i].sms[j]=0;
	     Const.s_Ma[i].mail[j]=0;
	   }
   }

   for(i=0;i<_Size_TT;i++)
   {
	   k=0;
	   for(j=0;j<100;j++)
	   {   Const.s_TT[i].dd[j]=j/10;
	       Const.s_TT[i].mm[j]=k;   k+=10;
	        if(k==100) k=0;

	   }
	   sprintf(Const.s_TT[i].jm,"V");
   }

   if((Const.s_Rej[0].param&0x01)==0){
      Const.s_Rej[0].intr=60;
      Const.s_Rej[0].param=0;
   }

   for(i=0;i<_Size__s_Czujki_info;i++)
   {
     Const.s_Wykr[i].prgmax=120;
     Const.s_Wykr[i].prgmin=50;
     Const.s_Wykr[i].zawezen= 1;
     Const.s_Wykr[i].szerwykr=5000;
   }

   for(i=0;i<_Ilosc_Log;i++)
   {
      sprintf(Const.s_Log[i].login,"1111");
      Const.s_Log[i].haslo[0]=0;
   }

   Const.s_Lora[0].freq=SX1278_434MHZ;
   Const.s_Lora[0].power=SX1278_POWER_14DBM;
   Const.s_Lora[0].spread=SX1278_LORA_SF_9;
   Const.s_Lora[0].bandwidth=SX1278_LORA_BW_20_8KHZ;
   Const.s_Lora[0].packetlen=1;   //packet len = 10


   for(i=0;i<_Size_imp;i++)
   {
	  Const.s_Imp[i].licz=1000;
      Const.s_Imp[i].wart=1;
      sprintf(Const.s_Imp[i].jedn,"KWh");

      for(j=0;j<_Imp_IleVal;j++)
      {
    	  Const.s_Imp[i].value[j]=0;
    	  Const.s_Imp[i].prog1_dzies[j]=10;
          Const.s_Imp[i].prog1_ulam[j]=3;
           Const.s_Imp[i].prog2_dzies[j]=10;
           Const.s_Imp[i].prog2_ulam[j]=3;
          Var.EtrVal[i][j]=0;
      }
   }

   for(i=0;i<(_Size_imp*_Imp_IleVal);i++)
   {
        Const.s_WykrEtr[i].prgmax=100;
        Const.s_WykrEtr[i].prgmin=50;
        Const.s_WykrEtr[i].zawezen= 1;
        Const.s_WykrEtr[i].szerwykr=5000;
   }


   #ifndef _REJ
   Const.s_Rej[0].param=0;   // Rej OFF
   #endif


   for(i=0;i<_IleLor;i++)
   {
         Const.s_Map[0].x[i]=0;
         Const.s_Map[0].y[i]=0;
   }
   Const.s_Map[0].resX=0;
   Const.s_Map[0].resY=0;



	for(i=0;i<_Size_pt;i++){
		for(j=0;j<5;j++){
			tab_pt[i][j]=0;
		}
	}

	for(i=0;i<_Size_TT;i++){
		for(j=0;j<5;j++){
			tab_tt[i][j]=0;
		}
	}

	for(i=0;i<_Size__s_Czujki_info;i++)  Var.value[i] = 0x7FFF;
	for(i=0;i<_Size_pt;i++)     Const.s_ADC_pt[i].val = 0x7FFF;
	for(i=0;i<(((_Size_we-1)/16)+1);i++)    Var.we[i] = 0xFFFF;

	for(i=0;i<_Size_wy;i++){
	   NrPortuNaPortZewn(t_wy[i], &GPIOx, &GPIO_Pin);
	   HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	   Const.s_WY[i/16].val&=~(1<<(i-16*(i/16)));
	}

	for(i=0;i<_Size_wy;i++) Const.s_Rot[0].ma[i]=0;
	Const.s_Rot[0].dob=0;
	Const.s_Rot[0].intrv=12*60;
	Const.s_Rot[0].sta=0;
	wartTemp("+50.0", &Tmax);  Const.s_Rot[0].max=Tmax;
	wartTemp("+10.0", &Tmin);  Const.s_Rot[0].min=Tmin;
	wartTemp("+00.5", &hist);  Const.s_Rot[0].hist=hist;
	Const.s_Rot[0].reak=0;
	RotCzasNext=0;  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, RotCzasNext);
	NrRot=0;        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, NrRot);
	RotDoby=0;      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, RotDoby);

	for(i=0;i<_Ilosc_Tel;i++) Const.s_Dzw[0].ma[i]=0;
	for(i=0;i<_Ilosc_Tel;i++) Const.s_Rap[0].sms[i]=0;
	for(i=0;i<_Ilosc_Mail;i++) Const.s_Rap[0].mail[i]=0;
	Const.s_Rap[0].godzON1=0;
	Const.s_Rap[0].plik=2;
	Const.s_Rap[0].siec=0;
	Const.s_Rap[0].czas=0;
	Const.s_Rap[0].godzina=0;
	Const.s_Rap[0].start_zd=0;
	Const.s_Rap[0].start_po=0;

	Const.s_GPRS[0].apn[0]=0;
	Const.s_GPRS[0].usr[0]=0;
	Const.s_GPRS[0].has[0]=0;

	for(int k=0;k<MAX_SIGOUT;k++)
		ResetGateLogic(k);

    Const.OutNameLogic[0].name=0xFF;
    //ResetGateLogic(Const.OutNameLogic[0].name);
}

int8_t GateLogicOperat(int nrPageOpLogic, int typeGate, int nrGate)
{
	int i;
	uint8_t output=0;
	uint8_t *pInputBuff, *pNameBuff;
	uint8_t binInput, logicOperation, maxInputSignal;


		switch(typeGate)
		{
		case INIT_GATE:
			pInputBuff=     Const.Logic[nrPageOpLogic].InitGate[nrGate].inputBuffer;
			pNameBuff=      Const.Logic[nrPageOpLogic].InitGate[nrGate].NameInputSignal;
			logicOperation= Const.Logic[nrPageOpLogic].InitGate[nrGate].logicalOperation;
			binInput=         Var.Logic[nrPageOpLogic].IGate[nrGate].binaryInputSignal;
			maxInputSignal = MAX_INPUT_INITIAL_LOGIC_GATE;
			break;
		case ADD_GATE:
			pInputBuff=     Const.Logic[nrPageOpLogic].AddGate[nrGate].inputBuffer;
			pNameBuff=      Const.Logic[nrPageOpLogic].AddGate[nrGate].NameInputSignal;
			logicOperation= Const.Logic[nrPageOpLogic].AddGate[nrGate].logicalOperation;
			binInput=         Var.Logic[nrPageOpLogic].AGate[nrGate].binaryInputSignal;
			maxInputSignal = MAX_INPUT_ADD_LOGIC_GATE;
			break;
		case FIN_GATE:
			pInputBuff=     Const.Logic[nrPageOpLogic].FinGate[nrGate].inputBuffer;
			pNameBuff=      Const.Logic[nrPageOpLogic].FinGate[nrGate].NameInputSignal;
			logicOperation= Const.Logic[nrPageOpLogic].FinGate[nrGate].logicalOperation;
			binInput=         Var.Logic[nrPageOpLogic].FGate[nrGate].binaryInputSignal;
			maxInputSignal = MAX_INPUT_FIN_LOGIC_GATE;
			break;
		default:
			pInputBuff=     Const.Logic[nrPageOpLogic].InitGate[nrGate].inputBuffer;
			pNameBuff=      Const.Logic[nrPageOpLogic].InitGate[nrGate].NameInputSignal;
			logicOperation= Const.Logic[nrPageOpLogic].InitGate[nrGate].logicalOperation;
			binInput=         Var.Logic[nrPageOpLogic].IGate[nrGate].binaryInputSignal;
			maxInputSignal = MAX_INPUT_INITIAL_LOGIC_GATE;
			break;
		}


	switch(logicOperation)
	{
	case NONE:
		output=0;
		break;
	case OR:
		output=0;
		for(i=0;i<maxInputSignal;i++)
		{
			if(0 < pNameBuff[i])
			{
				     if(NOT==pInputBuff[i])     output = ((~(binInput>>i))&0x01) | output;
				else if(BUFFER==pInputBuff[i])	output = ((binInput>>i)&0x01) | output;
			}
		}
		break;
	case AND:
		output=1;
		for(i=0;i<maxInputSignal;i++)
		{
			if(0 < pNameBuff[i])
			{
				     if(NOT==pInputBuff[i])    output = ((~(binInput>>i))&0x01) & output;
				else if(BUFFER==pInputBuff[i]) output = ((binInput>>i)&0x01) & output;
			}
		}
		break;
	case NOR:
		output=0;
		for(i=0;i<maxInputSignal;i++)
		{
			if(0 < pNameBuff[i])
			{
			    	if(NOT==pInputBuff[i])    output = ((~(binInput>>i))&0x01) | output;
			   else if(BUFFER==pInputBuff[i]) output = ((binInput>>i)&0x01) | output;
			}
		}
		output= (~output)&0x01;
		break;
	case NAND:
		output=1;
		for(i=0;i<maxInputSignal;i++)
		{
			if(0 < pNameBuff[i])
			{
			     	 if(NOT==pInputBuff[i])    output = ((~(binInput>>i))&0x01) & output;
			    else if(BUFFER==pInputBuff[i]) output = ((binInput>>i)&0x01) & output;
			}
		}
		output= (~output)&0x01;
		break;
	case XOR:
		output=0;
		for(i=0;i<maxInputSignal;i++)
		{
			if(0 < pNameBuff[i])
			{
		     	   if(NOT==pInputBuff[i])    output = ((~(binInput>>i))&0x01) ^ output;
		      else if(BUFFER==pInputBuff[i]) output = ((binInput>>i)&0x01) ^ output;
			}
		}
		break;
	}


 return (output&0x01);

}

void RewriteLogicInputs(void)
{
	int c,i,j,nrPortu;

	for(c=0;c<MAX_SIGOUT;c++)
	{
		for(i=0;i<MAX_INITIAL_LOGIC_GATES;i++)
	    {
			for(j=0;j<MAX_INPUT_INITIAL_LOGIC_GATE;j++)
			{
				nrPortu = Const.Logic[c].InitGate[i].NameInputSignal[j];
				if(nrPortu>0)
				{
					if(nrPortu<60)
					{
                                                     //WE
						if(( Const.s_GPIO[nrPortu-1].val==4))
						{
						   if(Var.logicWe&(1<<(nrPortu-1)))
							   Var.Logic[c].IGate[i].binaryInputSignal &= ~(1<<j);
						   else
							   Var.Logic[c].IGate[i].binaryInputSignal |= (1<<j);

						} 									  //Ti
						else if(( Const.s_GPIO[nrPortu-1].val==10))
						{
						   if(Var.logicTi&(1<<(nrPortu-20-1)))
							   Var.Logic[c].IGate[i].binaryInputSignal |= (1<<j);
						   else
							   Var.Logic[c].IGate[i].binaryInputSignal &= ~(1<<j);

						} 						            //DS                               //DHT							//Pt
						else if((Const.s_GPIO[nrPortu-1].val==2)||(Const.s_GPIO[nrPortu-1].val==9)||(Const.s_GPIO[nrPortu-1].val==3))
						{
						   if(Var.logicTemp&(1<<(nrPortu-1)))
							   Var.Logic[c].IGate[i].binaryInputSignal |= (1<<j);
						   else
							   Var.Logic[c].IGate[i].binaryInputSignal &= ~(1<<j);

						}
					}
					else
					{                                 //DHT
						if((Const.s_GPIO[nrPortu-60-1].val==9))
						{
						   if(Var.logicTemp&(((uint64_t)1<<(nrPortu-60-1))<<32))
							   Var.Logic[c].IGate[i].binaryInputSignal |= (1<<j);
						   else
							   Var.Logic[c].IGate[i].binaryInputSignal &= ~(1<<j);

						}

					}
				}

			}

	    }

	}

}

void FunctionLogicOperation(void)
{

	uint8_t nr_Gate;
	int c;

	for(c=0;c<MAX_SIGOUT;c++)
	//for(c=0;c<6;c++)  //MAX WY to 6
	{


		for(i=0;i<MAX_INPUT_INITIAL_LOGIC_GATE;i++)
				Var.Logic[c].IGate[i].binaryOutputSignal = GateLogicOperat(c,INIT_GATE,i);


		for(i=0;i<MAX_ADDITIONAL_LOGIC_GATES;i++)
		{
			Var.Logic[c].AGate[i].binaryInputSignal=0;
			for(int j=0;j<MAX_INPUT_ADD_LOGIC_GATE;j++)
			{
				nr_Gate= Const.Logic[c].AddGate[i].NameInputSignal[j];
				if(nr_Gate>0)
				{
					Var.Logic[c].AGate[i].binaryInputSignal |= (Var.Logic[c].IGate[nr_Gate-1].binaryOutputSignal << j);
				}
			}
		}





		for(i=0;i<MAX_ADDITIONAL_LOGIC_GATES;i++)
			Var.Logic[c].AGate[i].binaryOutputSignal = GateLogicOperat(c,ADD_GATE,i);


		for(i=0;i<MAX_FINAL_LOGIC_GATES;i++)
		{
			Var.Logic[c].FGate[i].binaryInputSignal=0;
			for(int j=0;j<MAX_INPUT_FIN_LOGIC_GATE;j++)
			{
				nr_Gate= Const.Logic[c].FinGate[i].NameInputSignal[j];
				if(nr_Gate>0)
				{
					Var.Logic[c].FGate[i].binaryInputSignal |= (Var.Logic[c].AGate[nr_Gate-1].binaryOutputSignal << j);
				}
			}
		}





		for(i=0;i<MAX_FINAL_LOGIC_GATES;i++)
				Var.Logic[c].FGate[i].binaryOutputSignal = GateLogicOperat(c,FIN_GATE,i);


		int i,j;
		i=c+20;

		if(Const.Logic[c].FinGate[0].logicalOperation > 0)
		{
                //PK                          IR                       PWM
			if((Const.s_GPIO[i].val==0)||(Const.s_GPIO[i].val==1)||(Const.s_GPIO[i].val==8))
			{
				if(WyszukujPortElement(t_wy,_Size_wy, i+1)!=-1)
				{
					j=WyszukujPortElement(t_wy,_Size_wy, i+1);

					 if(Var.Logic[c].FGate[0].binaryOutputSignal > 0)
					 {
						 Const.s_WY[0].val |= (1<<j);   GPIO_PK(j);
					 }
					 else
					 {
						 Const.s_WY[0].val &= ~(1<<j);   GPIO_PK(j);
					 }
				}
			}
		}


	}



}

void Start_GPIO()
{
   	int i;
	for(i=0;i<_Size__s_gpio;i++)
   	{
   		Var.gpio[i] = Const.s_GPIO[i].val;
   	}
}

void MX_RTC_Init_moj(void)
{

 /* RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

    /**Initialize RTC Only
/*
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 39;
  hrtc.Init.SynchPrediv = 1000;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    //Error_Handler();
  }*/
}
/*
int NRF_MasterOdbieraRAM()
{
   char *p1;  int i,j,k;    char md[33];
   p1= Const.s_Lan[0].eth;

	j=0;
    NRF_Start();
	for(i=0;i<100;i++)
	{
       HAL_Delay(10);
	   if(NRF_wyslij("MasterOdbieraRAM")==1)
	   {
		  if(i==0) k=NRF_odbierz(&buf_rx2[_Size_BufWifi+j],3000);
		  else     k=NRF_odbierz(&buf_rx2[_Size_BufWifi+j],200);

          if(k==1)
		  {
			   j+=30; Mrug(1); dbg3("a");
		  }
		  else{ dbg3("Z"); return 0; }
	   }
	   else{ dbg3("N"); return 0; }
	}
	if(NRF_odbierz(buf_p,200)==1)
	{
		hash_MD5(&buf_rx2[_Size_BufWifi],3000,md);
   		k=0;
   		for(i=0;i<16;i++)
   		{
   		   if(buf_p[i]==md[i]);
   		   else k=1;
   		}
		if(k==0){ dbg3("YESi  "); return 1; }  else{ dbg3("NOi!!  "); return 0; }
	}
	else{ dbg3("C"); return 0; }
	NRF_Stop();
}

int NRF_MasterWysylaRAM()
{
   char *p1;  int i,j,k;    char md[33];
   p1= Const.s_Lan[0].eth;

   for(i=0;i<3000;i++)  buf_rx2[_Size_BufWifi+i]=0x01;
   p1= &buf_rx2[_Size_BufWifi];
   hash_MD5(p1,3000,md);

	j=0;
    NRF_Start();
	for(i=0;i<(100+1);i++)
	{
       HAL_Delay(10);
       if(i==0) k= NRF_wyslij("MasterWysylaRAM");
       else     k= NRF_wyslij(p1+j);

	   if(k==1)
	   {
		  if(i==0) k=NRF_odbierz(buf_p,3000);
		  else     k=NRF_odbierz(buf_p,200);

		  if(k==1)
		  {
        	  if(strstr(buf_p,"OK")){  if(i>0) j+=30;    Mrug(1); dbg3("a");  }
		  }
		  else{ dbg3("Z"); return 0; }
	   }
	   else{ dbg3("N"); return 0; }
	}
	if(NRF_wyslij(md)==1){ dbg3("YES  "); return 1; } else{ dbg3("NO!!  "); return 0; }

	NRF_Stop();
}

int NRF_SlaveWysylaRAM()
{
   char *p1;  int i,j;    char md[33];    int czas;

   for(i=0;i<3000;i++)  buf_rx2[_Size_BufWifi+i]=0x01;
   p1= &buf_rx2[_Size_BufWifi];
   hash_MD5(p1,3000,md);

   j=0;
   NRF_Start();
   for(i=0;i<100;i++)
   {
	  if(i==0)  goto ominMOR;
	  else     czas=200;
	  if(NRF_odbierz(buf_p,czas)==1)
   	  {
   		 if(strstr(buf_p,"MasterOdbieraRAM"))
   		 {
   			ominMOR:
   			HAL_Delay(10);
   		    if(NRF_wyslij(p1+j)==1)
   		    {
   		       j+=30; Mrug(1); dbg3("a");
   		    }
   		    else{ dbg3("N"); return 0; }
   		 }
   		 else{ dbg3("X"); return 0; }
   	  }
   	  else{ dbg3("Z"); return 0; }
   }
   if(NRF_wyslij(md)==1){ dbg3("YES  "); return 1; } else{ dbg3("NO!!  "); return 0; }
   NRF_Stop();
}

int NRF_SlaveOdbieraRAM()
{
   char *p1;  int i,j,k;    char md[33]; int czas;

   j=0;
   NRF_Start();
   for(i=0;i<(100+1);i++)
   {
   	  if(i==0) goto ominMWR;
   	  else     czas=200;
	  if(NRF_odbierz(buf_p,czas)==1)
   	  {
   		if(i==0)
   		{
    		  if(strstr(buf_p,"MasterWysylaRAM"))
    		  {
    			  ominMWR:
    			  HAL_Delay(10);
    		      if(NRF_wyslij("OK")==1){ Mrug(1); dbg3("a");  }
    		      else{ dbg3("N"); return 0; }
    		  }
    		  else{ dbg3("X"); return 0; }
   		}
   		else
   		{
   			  for(k=0;k<30;k++) buf_rx2[_Size_BufWifi+j+k]=buf_p[k];
   			  j+=30;
   			  HAL_Delay(10);
		      if(NRF_wyslij("OK")==1){ Mrug(1); dbg3("a");  }
		      else{ dbg3("N"); return 0; }
   		}

   	  }
   	  else{ dbg3("Z"); return 0; }
   }
   if(NRF_odbierz(buf_p,200)==1)
   {
		hash_MD5(&buf_rx2[_Size_BufWifi],3000,md);
  		k=0;
  		for(i=0;i<16;i++)
  		{
  		   if(buf_p[i]==md[i]);
  		   else k=1;
  		}
		if(k==0){ dbg3("YESi  "); return 1; }  else{ dbg3("NOi!!  "); return 0; }
   }
   NRF_Stop();
}
 */

int NRF_MasterOdbieraRAM()
{
   char *p1;  int i,j,k,s,size;    char md[33];
   p1= Const.s_Lan[0].eth;      size=3900; //size=_Size_Zmienne_zapisywalne;

   if(size>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONE !!!");return 0; }

   if((size%30)>0) s= (size/30)+1;
   else            s=  size/30;

   sprintf(buf_p,"\r\nhh: %d\t\n",s); dbg3(buf_p);

	j=0;
    NRF_Start();
	for(i=0;i<s;i++)
	{
	   ppp1:
	   HAL_Delay(10);
	   if(NRF_wyslij("MasterOdbieraRAM")==1)
	   {
		  if(i==0) k=NRF_odbierz(&buf_rx2[_Size_BufWifi+j],3000);
		  else     k=NRF_odbierz(&buf_rx2[_Size_BufWifi+j],200);

          if(k==1)
		  {
			   j+=30; Mrug(1); dbg3("a");
		  }
		  else{ dbg3("Z"); HAL_Delay(500); goto ppp1; /*return 0;*/ }
	   }
	   else{ dbg3("N"); HAL_Delay(500); goto ppp1; /*return 0;*/ }
	}
	ppp1_:
	if(NRF_odbierz(buf_p,200)==1)
	{
		hash_MD5(&buf_rx2[_Size_BufWifi],3900,md);
   		k=0;
   		for(i=0;i<16;i++)
   		{
   		   if(buf_p[i]==md[i]);
   		   else k=1;
   		}
		if(k==0)
		{
	       dbg3("YESi  ");
	       for(i=0;i<3900;i++) *(p1+i)=buf_rx2[_Size_BufWifi+i];
		   return 1;
		}
		else{ dbg3("NOi!!  "); return 0; }
	}
	else{ dbg3("C"); HAL_Delay(500); goto ppp1_; /*return 0;*/ }
	NRF_Stop();
}

int NRF_MasterWysylaRAM()
{
   char *p1;  int i,j,k,s,size;    char md[33];
   p1= Const.s_Lan[0].eth;   size=3900; //size=_Size_Zmienne_zapisywalne;

   if(size>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONE !!!");return 0; }

   if((size%30)>0) s= (size/30)+1;
   else            s=  size/30;

   hash_MD5(p1,size,md);

   sprintf(buf_p,"\r\nhh: %d\t\n",s); dbg3(buf_p);

	j=0;
    NRF_Start();
	for(i=0;i<(s+1);i++)
	{
	   ppp2:
	   HAL_Delay(10);
       if(i==0) k= NRF_wyslij("MasterWysylaRAM");
       else     k= NRF_wyslij(p1+j);

	   if(k==1)
	   {
		  if(i==0) k=NRF_odbierz(buf_p,3000);
		  else     k=NRF_odbierz(buf_p,200);

		  if(k==1)
		  {
        	  if(strstr(buf_p,"OK")){  if(i>0) j+=30;    Mrug(1); dbg3("a");  }
		  }
		  else{ dbg3("Z"); HAL_Delay(500); goto ppp2;  /*return 0;*/ }
	   }
	   else{ dbg3("N");  HAL_Delay(500); goto ppp2;  /* return 0;*/ }
	}
	ppp2_:
	if(NRF_wyslij(md)==1){ dbg3("YES  "); return 1; } else{ dbg3("NO!!  ");  HAL_Delay(500); goto ppp2_; /*return 0;*/ }

	NRF_Stop();
}

int NRF_SlaveWysylaRAM()
{
   char *p1;  int i,j,s,size;    char md[33];    int czas;
   p1= Const.s_Lan[0].eth;   size=3900; //size=_Size_Zmienne_zapisywalne;

   if(size>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONE !!!");return 0; }

   if((size%30)>0) s= (size/30)+1;
   else            s=  size/30;

   hash_MD5(p1,3900,md);

   sprintf(buf_p,"\r\nhh: %d\t\n",s); dbg3(buf_p);

   j=0;
   NRF_Start();
   for(i=0;i<s;i++)
   {
	  ppp3:
	  if(i==0)  goto ominMOR;
	  else     czas=200;
	  if(NRF_odbierz(buf_p,czas)==1)
   	  {
   		 if(strstr(buf_p,"MasterOdbieraRAM"))
   		 {
   			ominMOR:
   			HAL_Delay(10);
   		    if(NRF_wyslij(p1+j)==1)
   		    {
   		       j+=30; Mrug(1); dbg3("a");
   		    }
   		    else{ dbg3("N");  HAL_Delay(500); goto ominMOR;  /*return 0;*/ }
   		 }
   		 else{ dbg3("X"); HAL_Delay(500); goto ppp3; /*return 0;*/ }
   	  }
   	  else{ dbg3("Z"); HAL_Delay(500); goto ppp3;  /*return 0;*/ }
   }
   ppp3_:
   HAL_Delay(200);
   if(NRF_wyslij(md)==1){ dbg3("YES  "); return 1; } else{ dbg3("NO!!  ");  HAL_Delay(500); goto ppp3_; /*return 0;*/ }
   NRF_Stop();
}

int NRF_SlaveOdbieraRAM()
{
   char *p1;  int i,j,k,s,size;    char md[33]; int czas;
   p1= Const.s_Lan[0].eth;   size=3900; //size=_Size_Zmienne_zapisywalne;

   if(size>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONE !!!");return 0; }

   if((size%30)>0) s= (size/30)+1;
   else            s=  size/30;

   sprintf(buf_p,"\r\nhh: %d\t\n",s); dbg3(buf_p);

   j=0;
   NRF_Start();
   for(i=0;i<(s+1);i++)
   {
	  ppp4:
	  if(i==0) goto ominMWR;
   	  else     czas=200;
	  if(NRF_odbierz(buf_p,czas)==1)
   	  {
   		if(i==0)
   		{
    		  if(strstr(buf_p,"MasterWysylaRAM"))
    		  {
    			  ominMWR:
    			  HAL_Delay(10);
    		      if(NRF_wyslij("OK")==1){ Mrug(1); dbg3("a");  }
    		      else{ dbg3("N");  HAL_Delay(500); goto ominMWR;  /*return 0;*/ }
    		  }
    		  else{ dbg3("X"); HAL_Delay(500); goto ppp4;  /*return 0;*/ }
   		}
   		else
   		{
   			  for(k=0;k<30;k++) buf_rx2[_Size_BufWifi+j+k]=buf_p[k];
   			  j+=30;
              ppp4__:
   			  HAL_Delay(10);
		      if(NRF_wyslij("OK")==1){ Mrug(1); dbg3("a");  }
		      else{ dbg3("N");  HAL_Delay(500); goto ppp4__;  /*return 0;*/ }
   		}

   	  }
   	  else{ dbg3("Z");  HAL_Delay(500); goto ppp4;   /*return 0;*/ }
   }
   ppp4_:
   if(NRF_odbierz(buf_p,200)==1)
   {
		hash_MD5(&buf_rx2[_Size_BufWifi],3900,md);
  		k=0;
  		for(i=0;i<16;i++)
  		{
  		   if(buf_p[i]==md[i]);
  		   else k=1;
  		}
		if(k==0)
		{
		   dbg3("YESi  ");
		   for(i=0;i<3900;i++) *(p1+i)=buf_rx2[_Size_BufWifi+i];
		   return 1;
		}
		else{ dbg3("NOi!!  "); return 0; }
   }
   else{ dbg3("C"); HAL_Delay(500); goto ppp4_; /*return 0;*/ }
   NRF_Stop();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
