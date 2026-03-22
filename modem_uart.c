/*
 * modem_uart.c
 *
 *  Created on: 23 lut 2018
 *      Author: ssss
 */


#include "stm32f1xx_hal.h"

#include "modem_uart.h"
#include "flash_in.h"
#include "http.h"
#include "define.h"
#include "gpio.h"
#include "lcd.h"
#include "enc28j60.h"
#include "eth.h"
#include "SX1278.h"
#include "pid.h"
#include "zdarzenia.h"
#include "wejscia_licznikowe.h"
#include "wykres.h"
#include "agh.h"
#include "irda.h"
#include "st7735/st7735.h"
#include "st7735/fonts.h"

/*
#ifdef _Neoway
#define __ATE0       1
#define __cpin_pyta  2
#define __cpin_wpis  3
#define __creg_pyta  4
#define __cmgf_wpis  5
#define __cnmi_wpis  6
#define __clip_wpis  7
#define __cops_pyta  8
#define __cnum       9
#define __csmp_wpis  10
#define __cscs       11
#define __cmgd_1     12
#define __cmgd_2     13
#define __cmgd_3     14
#define __csq        15
#endif*/

//#ifdef _Fibocom
#define __ATE0       1
#define __cpin_pyta  2
#define __cpin_wpis  3
#define __creg_pyta  4
#define __cmgf_wpis  5
#define __cnmi_wpis  6
#define __clip_wpis  7
#define __cops_pyta  8
#define __cnum       9
#define __csmp_wpis  10
#define __cmgd_1     11
#define __cmgd_2     12
#define __cmgd_3     13
#define __cmgd_4     14
#define __csq        15
//#endif

int wsk_co_sek, uartwifirst;   int ResetWifiAT;

#define HTTP_START        0
#define HTTP_STYLE        1
#define HTTP_BODY         2
#define HTTP_END          3
#define HTTP_UstTemp      4
#define HTTP_XXX          5

int cload_activ=0;

#define TM_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))
#define TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)

SX1278_hw_t SX1278_hw;
SX1278_t SX1278;    char ZmRstWifi;

int licz_cipsend,wskk,nr,iterix,sel_flash, gsm_flag;   int port_smtp_p,port_smtp;  int port_cload_p,port_cload;  int dzwon,param_dzwon,koniec_dzwon, idx_email_GPRS;

typedef  void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress, APPLICATION_ADDRESS;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;

extern RTC_HandleTypeDef hrtc;

extern IWDG_HandleTypeDef hiwdg;
//extern WWDG_HandleTypeDef hwwdg;

//extern ADC_HandleTypeDef hadc;

uint32_t Address_=0;

#define _Size_buf_rx  500
char Received[2],buf_rx[_Size_buf_rx],ab;
int li,pos,it,wsk,licz_start, licz_cclk;

char licz_brak_Sim;

uint32_t a;

char Received2[2];
int li2,pos2,it2;

#define _Size_buf_rx3  22
char Received3[2],buf_rx3[_Size_buf_rx3];
int li3,pos3,it3;

char dataIn[1];
extern int timer_main;

char *ptr,*ptr1,*ptr2, *ptr3;
int op;

int nr_kom,nr_kom2,licz_cpin,k,licz_g, licz_rst, wysylka_sms;
char at_send[30];

int LoraSynchro;

uint32_t sms_czas_pk[33], sms_czas_pk_start[33], flaga_pk_[33], dzwonBlok;
uint32_t godz_,min_,sec_;

void send_Http()
{

	//while(HAL_UART_Transmit_DMA(&huart2, &buf_http[PacketLen*FragmentPagePacketCounter], strlen(buf_http)-PacketLen*(FragmentPagePacketNumber-1))==HAL_BUSY);
}



void JumpToUserProgram(uint32_t adr)
{

	APPLICATION_ADDRESS=_JumpToProgr;
	// Test if user code is programmed starting from address "APPLICATION_ADDRESS"
    	  dbg3("\r\nSKOK...");  HAL_Delay(100);
    	                if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FE00000 ) == 0x20000000)
    	    		    {
    	    		      // Jump to user application
    	    		      JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
    	    		      JumpToApplication = (pFunction) JumpAddress;
    	    		      // Initialize user application's Stack Pointer
    	    		     // __disable_irq();
    	    		      __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
    	    		      JumpToApplication();
    	    		    }
    	                else dbg3("\r\nNIE !!!!...");
    	                HAL_Delay(5000);
}





void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==UART4)
	{
		buf_rx[it++]=Received[0];  if(it>(_Size_buf_rx-2)) it=0;
		HAL_UART_Receive_DMA(&huart4, Received, 1);
	}
	if(huart->Instance==USART1)
	{
	    buf_rx2[it2++]=Received2[0]; buf_rx2[it2]=0;    if(it2>(_Size_BufWifi-2)) it2=0;
	    HAL_UART_Receive_DMA(&huart1, Received2, 1);
	}
	if(huart->Instance==USART3)
	{
		buf_rx3[it3++]=Received3[0];  if(it3>(_Size_buf_rx3-2)) it3=0;
		HAL_UART_Receive_IT(&huart3, Received3, 1);
	}


}



void ResumptionReceiveDataFromGSM(void)
{
	int i;  //dbg3(buf_rx);
	for(i=0;i<_Size_buf_rx;i++) buf_rx[i]=0;
	HAL_UART_Receive_DMA(&huart4, Received, 1);
	it=0;
}
void ResumptionReceiveDataFromWifi(void)
{
	int i;
	for(i=0;i<_Size_BufWifi;i++) buf_rx2[i]=0;
	HAL_UART_Receive_DMA(&huart1, Received2, 1);
	it2=0;
}
void ResumptionReceiveDataFromDebug(void)
{
	int i;
	for(i=0;i<_Size_buf_rx3;i++) buf_rx3[i]=0;
	HAL_UART_Receive_IT(&huart3, Received3, 1);
	it3=0;
}

void StartReceiveDataFromWifi(void)
{
	HAL_UART_Receive_DMA(&huart1, Received2, 1);
}

void CzyJestCosNaWykresie()
{
   int i,j;

	    Var.wykres_nr=0xff;
	    for(i=0;i<_Size__s_gpio;i++)
	    {
	    	j= WyszukajNrCzujkiDlaPortu(i+1);
	    	if(j!=-1)
	    	{
	    		Var.wykres_nr = j;  break;
	    	}
	    }


	    for(i=0;i<_Size_imp;i++){  if(Const.s_GPIO[t_imp[i]-1].val==7){  WykrNrEtr=i*4;  break; }  }


}

void PrzeliczLicznik(int nr_licz, int nr_val, char *buf)
{
    int i,j;
	uint32_t poprzecinku, dzies, ulam, pom;

	i=nr_licz;
	j=nr_val;

      if(Const.s_Imp[i].licz<=10)     poprzecinku=10;
 else if(Const.s_Imp[i].licz<=100)    poprzecinku=100;
 else if(Const.s_Imp[i].licz<=1000)   poprzecinku=1000;
 else if(Const.s_Imp[i].licz<=10000)  poprzecinku=10000;
 else if(Const.s_Imp[i].licz<=100000) poprzecinku=100000;


    dzies=   (Const.s_Imp[i].value[j]*Const.s_Imp[i].wart)/Const.s_Imp[i].licz;


    pom=  ((Const.s_Imp[i].value[j]*Const.s_Imp[i].wart*Const.s_Imp[i].licz)/Const.s_Imp[i].licz)  -  (dzies*Const.s_Imp[i].licz);

  ulam= (pom*poprzecinku)/Const.s_Imp[i].licz;

if((Const.s_Imp[i].licz==1)&&(Const.s_Imp[i].wart==1)) sprintf(buf,"%10d %s               ",dzies,Const.s_Imp[i].jedn);
else
{
        if(Const.s_Imp[i].licz<=10)     sprintf(buf,"%10d.%01d %s         ",dzies,ulam,Const.s_Imp[i].jedn);
   else if(Const.s_Imp[i].licz<=100)    sprintf(buf,"%10d.%02d %s         ",dzies,ulam,Const.s_Imp[i].jedn);
   else if(Const.s_Imp[i].licz<=1000)   sprintf(buf,"%10d.%03d %s         ",dzies,ulam,Const.s_Imp[i].jedn);
   else if(Const.s_Imp[i].licz<=10000)  sprintf(buf,"%10d.%04d %s         ",dzies,ulam,Const.s_Imp[i].jedn);
   else if(Const.s_Imp[i].licz<=100000) sprintf(buf,"%10d.%05d %s         ",dzies,ulam,Const.s_Imp[i].jedn);
}

  //sprintf(buf,"%10d",ulam);
}


void PrzeliczLicznik_wstep(int nr_licz, int nr_val, char *buf)
{
    int i,j;
	uint32_t poprzecinku, dzies, ulam, pom;

	i=nr_licz;
	j=nr_val;

      if(Const.s_Imp[i].licz<=10)     poprzecinku=10;
 else if(Const.s_Imp[i].licz<=100)    poprzecinku=100;
 else if(Const.s_Imp[i].licz<=1000)   poprzecinku=1000;
 else if(Const.s_Imp[i].licz<=10000)  poprzecinku=10000;
 else if(Const.s_Imp[i].licz<=100000) poprzecinku=100000;


    dzies=   (Const.s_Imp[i].value[j]*Const.s_Imp[i].wart)/Const.s_Imp[i].licz;


    pom=  ((Const.s_Imp[i].value[j]*Const.s_Imp[i].wart*Const.s_Imp[i].licz)/Const.s_Imp[i].licz)  -  (dzies*Const.s_Imp[i].licz);

  ulam= (pom*poprzecinku)/Const.s_Imp[i].licz;

  if((Const.s_Imp[i].licz==1)&&(Const.s_Imp[i].wart==1)) sprintf(buf,"%d %s",dzies,Const.s_Imp[i].jedn);
  else
  {
          if(Const.s_Imp[i].licz<=10)     sprintf(buf,"%d.%01d %s",dzies,ulam,Const.s_Imp[i].jedn);
     else if(Const.s_Imp[i].licz<=100)    sprintf(buf,"%d.%02d %s",dzies,ulam,Const.s_Imp[i].jedn);
     else if(Const.s_Imp[i].licz<=1000)   sprintf(buf,"%d.%03d %s",dzies,ulam,Const.s_Imp[i].jedn);
     else if(Const.s_Imp[i].licz<=10000)  sprintf(buf,"%d.%04d %s",dzies,ulam,Const.s_Imp[i].jedn);
     else if(Const.s_Imp[i].licz<=100000) sprintf(buf,"%d.%05d %s",dzies,ulam,Const.s_Imp[i].jedn);
  }


  //sprintf(buf,"%10d",ulam);
}

void PrzeliczLicznikKolejny(int nr_licz, int nr_val, char *buf)
{
    int i,j,k;
	uint32_t poprzecinku, dzies, ulam, pom;

	i=nr_licz;
	j=nr_val;

      if(Const.s_Imp[i].licz<=10)     poprzecinku=10;
 else if(Const.s_Imp[i].licz<=100)    poprzecinku=100;
 else if(Const.s_Imp[i].licz<=1000)   poprzecinku=1000;
 else if(Const.s_Imp[i].licz<=10000)  poprzecinku=10000;
 else if(Const.s_Imp[i].licz<=100000) poprzecinku=100000;


    dzies=   (Const.s_Imp[i].value[j]*Const.s_Imp[i].wart)/Const.s_Imp[i].licz;


    pom=  ((Const.s_Imp[i].value[j]*Const.s_Imp[i].wart*Const.s_Imp[i].licz)/Const.s_Imp[i].licz)  -  (dzies*Const.s_Imp[i].licz);

  ulam= (pom*poprzecinku)/Const.s_Imp[i].licz;

if((Const.s_Imp[i].licz==1)&&(Const.s_Imp[i].wart==1)) sprintf(buf,"%10d      ",dzies);
else
{
        if(Const.s_Imp[i].licz<=10)     sprintf(buf,"%10d.%01d    ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=100)    sprintf(buf,"%10d.%02d   ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=1000)   sprintf(buf,"%10d.%03d  ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=10000)  sprintf(buf,"%10d.%04d ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=100000) sprintf(buf,"%10d.%05d",dzies,ulam);
}

if(j==0)
{
	  k=strlen(buf);   sprintf(&buf[k]," %s       ",Const.s_Imp[i].jedn);
}

  //sprintf(buf,"%10d",ulam);
}


void Autoryzacja()
{

	   dbg3("1234567890abcd");   HAL_Delay(500);
	                while(1)
	  		        {
	  	            	if(strstr(buf_rx3,"15071979666666"))   //kod OK :)
	  		            {
	  	                    HAL_Delay(100);
	  		        	    sprintf(Const.s_Kod[0].val,"15071979666666"); HAL_Delay(10);  ZapiszFlash();
	  		    			ResumptionReceiveDataFromDebug();
	  		    		    break;
	  		            }
	  		            HAL_IWDG_Refresh(&hiwdg);
	  		        }

}

void start_modem_uart()
{
	nr_kom=0;   nr_kom2=0;
	PacketLen=1480;
	li_sms=0;
	start=0;
	licz_brak_Sim=0;
	licz_cclk=0;
	wysylka_sms=0;
	WhichPage=6;
	itx_BufEmail=0;
	rej_tim=0;
	email=0;
	time_zone=0;
	NrRot=0;
	ekran_bit&=~0x02;  //wsztrzymuj wuswietlanie tft jesli idzie Http
	dzwon=0;
	koniec_dzwon=0;
	idx_email_GPRS=0;
	retremail=0;
	ZmRstWifi=1;
	wsk_co_sek=0;
	uartwifirst=0;
	ResetWifiAT=0;
	Lora_wstrz_synch=0;
	dzwonBlok=0;

	  SendLife_p=0;

	  for(i=0;i<_Ilosc_Tel;i++) Var.DzwMa[i]=0;

	  for(i=0; i<_Size_port; i++) lifeLora[i]=0;


	port_ux=-1;

	sprintf(zasieg_gsm,"--");
	gsm_log_licz=0;
	wpis_pin_licz=0;

	for(i=0;i<_Ilosc_Tel;i++) i_sms[i]=0;
	for(i=0;i<_Ilosc_Mail;i++) i_mail[i]=0;

	wybor_bitowy&=~0x1000;  //zerujemy powtorke email ENC
	wybor_bitowy&=~0x2000;  //zerujemy powtorke email ESP

	wybor2_bitowy&=~0x0400;  //Po restarcie odpytywanie wszystkich czujek lora


	ekran_tymczasowy=0;
	licz_delay_ekran=0;

	WpisPortElement();

	gsm_flag=0;


	port_smtp=4;
	port_smtp_p=100;

	port_cload=4;
	port_cload_p=100;


	for(i=0;i<_Size_pt;i++){
		for(j=0;j<Size_probe_pt;j++){
			tab_pt[i][j]=0xffff;
		}
	}

	for(i=0;i<_Size_TT;i++){
		for(j=0;j<Size_probe_tt;j++){
			tab_tt[i][j]=0;
		}
	}

	for(i=0;i<_Size__s_Czujki_info;i++){  Var.value[i] = 0x7FFF;  Var.delay_czujki[i]=0; Var.delay_czujki2[i]=0;  Var.wsk_czujki[i]=0; Var.wsk_czujki_t[i]=0;  }
	for(i=0;i<_Size_pt;i++)     Const.s_ADC_pt[i].val = 0x7FFF;

	for(i=0;i<(((_Size_we-1)/16)+1);i++){    Var.we[i] = 0xFFFF; Var.we_p[i] = 0xFFFF;   }
	for(i=0;i<(((_Size_eol-1)/16)+1);i++)    Var.eol[i] = 0xFFFF;
	for(i=0;i<_Size_we;i++){  Var.delay_we[i]=0;  Var.wsk_we[i]=0;  }

	Var.logicWe=0xFFFFFFFF;
	Var.logicTemp=0;
	Var.logicTi=0;

	adr_flash_SesjaSMTP=0;
	adr_flash_SesjaLogGSM=0;


	//musi byc 1000 dla routerow

	//dla  AP:  kom 1000 , komputer  1470

	wsk_1s=0;
	wsk_2s=0;

	APPLICATION_ADDRESS=ADDR_FLASH_PAGE_100;

	//  sprintf(buf_IPD,"+IPD,0,");  sprintf(buf__,"--");

	/*  buf_IPD[0]='+';
	  buf__[0]='-';
	  buf_IPD[1]='I';
	  buf__[1]='-';
	  buf_IPD[2]='P';
	  buf__[2]=0;
	  buf_IPD[3]='D';
	  buf_IPD[4]=',';
	  buf_IPD[5]='0';
	  buf_IPD[6]=',';
	  buf_IPD[7]=0;
*/

	if((Const.s_Lan[0].param&0x0C)==0) licz_htt=10000;
	else                               licz_htt=0;



///#########################  DO WYMAZANIA #############################################
/*
	licz_htt=1; //DO WYMAZANIA !!!!!  bo nie ma esp
	wybor_bitowy|=0x0400;
*/

	  Const.s_Imp[0].value[0]=0;
	  Const.s_Imp[1].value[0]=0;
	  Const.s_Imp[2].value[0]=0;
	  Const.s_Imp[3].value[0]=0;

	   for(i=0;i<_Size_imp;i++){
	     for(j=0;j<_Imp_IleVal;j++){

	    	 Var.EtrVal[i][j]=0;
	    	 Var.EtrPocz[i][j]= HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR11+5*i+j);
	   }}


	   PobierzCzas(buf_p);
	   for(i=0;i<_Size_imp;i++)
	   {
		   Var.wsk_etr[i]=0xFF;
		   Var.month_etr[i]=buf_p[1];
	   }

	   for(i=1;i<_Imp_IleVal;i++)  IteracjaFlash_3_OdczEtr(i);



//######################################################################################



	 while(HAL_UART_Transmit_IT(&huart3, "\r\nStart Program", strlen("\r\nStart Program"))==HAL_BUSY);


	    HAL_UART_Receive_DMA(&huart4, Received, 1);
	    StartReceiveDataFromWifi();
	    HAL_UART_Receive_IT(&huart3, Received3, 1);


	   IteracjaFlashZewnOdczyt(&adr_flash);         //dla pomiarow temp i wilg
	   IteracjaFlash_2_ZewnOdczyt(&adr_flash_2);    //dla zdarzen

	    CzyJestCosNaWykresie();


      if(Const.s_Rap[0].start_po==0xFFFFFFFF) Const.s_Rap[0].start_po=0;
      if(Const.s_Rap[0].start_zd==0xFFFFFFFF) Const.s_Rap[0].start_zd=0;

      LoraSynchro=0;

      for(i=0;i<33;i++) flaga_pk_[i]=0;




}

void dbg(char* txt)
{
	if(start==0)  //GSM ON
	{  if((wybor_bitowy&0x0020)==0)  //GSM ON
	   {
		  if(txt[0]>0){
		    ZapisSesjiLogGSM("-> ", 3, &adr_flash_SesjaLogGSM);
		    ZapisSesjiLogGSM(txt, strlen(txt), &adr_flash_SesjaLogGSM);
	      }
	}}
	while(HAL_UART_Transmit(&huart4,txt,strlen(txt),100)==HAL_BUSY);
}
void dbg2(char* txt)
{
	while(HAL_UART_Transmit_DMA(&huart1,txt,strlen(txt))==HAL_BUSY);
	//while(HAL_UART_Transmit(&huart2, txt, strlen(txt),10)==HAL_BUSY);
}
void dbg3(char* txt)
{
	while(HAL_UART_Transmit(&huart3,txt,strlen(txt),10)==HAL_BUSY);
}
void dbg3_roz(char* txt)
{
	while(HAL_UART_Transmit(&huart3,txt,strlen(txt),100)==HAL_BUSY);
}



int wpis_komend(int nr)
{
#ifdef _Neoway
	   if(nr==__ATE0){         sprintf(at_send,"ATE0\x0D");                }
  else if(nr==__cpin_pyta){    sprintf(at_send,"at+cpin?\x0D");             }
  else if(nr==__cpin_wpis){    sprintf(at_send,"at+cpin=\"1111\"\x0D");    }
  else if(nr==__creg_pyta){    sprintf(at_send,"at+creg?\x0D");            }
  else if(nr==__cmgf_wpis){    sprintf(at_send,"at+cmgf=1\x0D");            }
  else if(nr==__cnmi_wpis){    sprintf(at_send,"at+cnmi=2,2,0,1\x0D");            }
  else if(nr==__clip_wpis){    sprintf(at_send,"at+clip=1\x0D");            }
  else if(nr==__cops_pyta){    sprintf(at_send,"at+cops?\x0D");            }
  else if(nr==__cnum){         sprintf(at_send,"at+cnum\x0D");            }
  else if(nr==__csmp_wpis){    sprintf(at_send,"AT+CSMP=49,167\x0D");            }
  else if(nr==__cscs){         sprintf(at_send,"at+cscs=\"GSM\"\x0D");            }
  else if(nr==__cmgd_1){       sprintf(at_send,"at+cmgd=1\x0D");                   }
  else if(nr==__cmgd_2){       sprintf(at_send,"at+cmgd=2\x0D");                   }
  else if(nr==__cmgd_3){       sprintf(at_send,"at+cmgd=3\x0D");                   }
  else if(nr==__csq){          sprintf(at_send,"at+csq\x0D");                   }
  else{    return 0; }
#endif

#ifdef _Fibocom
	   if(nr==__ATE0){         sprintf(at_send,"ATE0\x0D");                }
  else if(nr==__cpin_pyta){    sprintf(at_send,"at+cpin?\x0D");             }
  else if(nr==__cpin_wpis){    sprintf(at_send,"at+cpin=\"1111\"\x0D");    }
  else if(nr==__creg_pyta){    sprintf(at_send,"at+creg?\x0D");            }
  else if(nr==__cmgf_wpis){    sprintf(at_send,"at+cmgf=1\x0D");            }
  else if(nr==__cnmi_wpis){    sprintf(at_send,"at+cnmi=2,2,0,1\x0D");            }
  else if(nr==__clip_wpis){    sprintf(at_send,"at+clip=1\x0D");            }
  else if(nr==__cops_pyta){    sprintf(at_send,"at+cops?\x0D");            }
  else if(nr==__cnum){         sprintf(at_send,"at+cnum\x0D");            }
  else if(nr==__csmp_wpis){    sprintf(at_send,"AT+CSMP=49,167\x0D");            }
  else if(nr==__cmgd_1){       sprintf(at_send,"at+cmgd=1\x0D");            }
  else if(nr==__cmgd_2){       sprintf(at_send,"at+cmgd=2\x0D");                   }
  else if(nr==__cmgd_3){       sprintf(at_send,"at+cmgd=3\x0D");                   }
  else if(nr==__cmgd_4){       sprintf(at_send,"at+cmgd=4\x0D");                   }
  else if(nr==__csq){          sprintf(at_send,"at+csq\x0D");                   }
  else{    return 0; }
#endif

  return 1;

}

void send_SMS(char *tel)
{
  unsigned char buffer[100];
	if(li_sms==0)
	{
 	   li_sms=30;
       sprintf(buffer,"at+cmgs=\"%s\"\x0D",tel);  dbg3(buffer);
       dbg(buffer);  HAL_Delay(500);
       ekran_tymczasowy=1;
  	   //HAL_UART_Receive_DMA(&huart1, Received, 1);
	}
}

void kopiuj_nr_tel(char *pp)
{
   int i; i=0;
    while(*(pp+i)!='"')
    {
    	tel_nr[i]=*(pp+i);
    	i++;
    }
    tel_nr[i]=0;
}

int wpis_sms__nazwa(unsigned char *tx,unsigned char *buf)
{
   unsigned char *ptr2; int i,yy;
   if(ptr2=strstr(buf,tx)){
            yy=strlen(tx);   ptr2+=yy; if(*ptr2==' ') ptr2++; if(*ptr2==' ') ptr2++;  if(*ptr2==' ') ptr2++;
      for(i=0;i<40;i++){ buf_p[i]= *(ptr2+i); if((buf_p[i]=='\r')||(buf_p[i]<0x20)) break; } buf_p[i]=0;
	  return 1;
   }
   return 0;
}
int wpis_sms__temp(unsigned char *tx,unsigned char *buf)
{
   unsigned char *ptr2;  int i,yy;      unsigned char bb[6];
   if(ptr2=strstr(buf,tx)){  dbg3("i");
            yy=strlen(tx);   ptr2+=yy; if(*ptr2==' ') ptr2++; if(*ptr2==' ') ptr2++;  if(*ptr2==' ') ptr2++;
       for(i=0;i<5;i++){ bb[i]=*(ptr2+i); if((bb[i]=='.')||(bb[i]=='+')||(bb[i]=='-')||((bb[i]>0x2f)&&(bb[i]<0x3A))); else break; } bb[i]=0;
       wzor_wpisu_temp(bb);  wartTemp(bb,&ttemp);


     //  sprintf(buf_p,"\r\n%s  ",bb);  dbg3(buf_p);
      // wartTemp_odwrotnie(&buf_p[50],*temp);
      // sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);

       return 1;
   }
   return 0;
}
int wpis_sms__temp_Pt(unsigned char *tx,unsigned char *buf)
{
   unsigned char *ptr2;  int i,yy;      unsigned char bb[6];
   if(ptr2=strstr(buf,tx)){  dbg3("i");
            yy=strlen(tx);   ptr2+=yy; if(*ptr2==' ') ptr2++; if(*ptr2==' ') ptr2++;  if(*ptr2==' ') ptr2++;
       for(i=0;i<4;i++){ bb[i]=*(ptr2+i); if((bb[i]=='+')||(bb[i]=='-')||((bb[i]>0x2f)&&(bb[i]<0x3A))); else break; } bb[i]=0;
       wzor_wpisu_temp_Pt(bb);  wartTemp_Pt(bb,&ttemp);
       return 1;
   }
   return 0;
}
int wpis_sms_minsec(unsigned char *tx,unsigned char *buf)
{
   unsigned char *ptr2; int i,yy;
   if(ptr2=strstr(buf,tx)){
            yy=strlen(tx);   ptr2+=yy; if(*ptr2==' ') ptr2++; if(*ptr2==' ') ptr2++;  if(*ptr2==' ') ptr2++;
      for(i=0;i<5;i++){ buf_p[i]= *(ptr2+i);  } buf_p[i]=0;
      wzor_wpisu_minsec(buf_p);
	  return 1;
   }
   return 0;
}
int wpis_sms_2x(unsigned char *tx,unsigned char *buf)
{
   unsigned char *ptr2; int i,yy;
   if(ptr2=strstr(buf,tx)){
            yy=strlen(tx);   ptr2+=yy; if(*ptr2==' ') ptr2++; if(*ptr2==' ') ptr2++;  if(*ptr2==' ') ptr2++;
      for(i=0;i<2;i++){ buf_p[i]= *(ptr2+i);  } buf_p[i]=0;
      buf_p[0]=buf_p[0]&0x0f;
      buf_p[1]=buf_p[1]&0x0f;
	  return 1;
   }
   return 0;
}
int wpis_sms_tak_nie(unsigned char *tx,unsigned char *buf)
{
   unsigned char *ptr2; int i,yy;
   if(ptr2=strstr(buf,tx)){
            yy=strlen(tx);   ptr2+=yy; if(*ptr2==' ') ptr2++; if(*ptr2==' ') ptr2++;  if(*ptr2==' ') ptr2++;
      for(i=0;i<3;i++){ buf_p[i]= *(ptr2+i);  } buf_p[i]=0;
	  return 1;
   }
   return 0;
}
int wpis_sms__on_off(unsigned char *tx,unsigned char *buf)
{
   unsigned char *ptr2; int i,yy;
   if(ptr2=strstr(buf,tx)){
            yy=strlen(tx);   ptr2+=yy; if(*ptr2==' ') ptr2++; if(*ptr2==' ') ptr2++;  if(*ptr2==' ') ptr2++;
           if((*(ptr2+0)=='o')&&(*(ptr2+1)=='n')&&(*(ptr2+2)=='/')&&(*(ptr2+3)=='o')&&(*(ptr2+4)=='f')) sprintf(buf_p,"on/off");
      else if((*(ptr2+0)=='o')&&(*(ptr2+1)=='n')) sprintf(buf_p,"on");
      else if((*(ptr2+0)=='o')&&(*(ptr2+1)=='f')) sprintf(buf_p,"off");
	  return 1;
   }
   return 0;
}

void Raport_Te_SMS()
{
  int k,ix, poz, i,j;

    k=0;  poz=0;  ix=0;
		for(i=0;i<_Size_te;i++)  //Temp i RHU
		{
			j= WyszukajNrCzujkiDlaPortu_2(t_te[i]);
			if(j!=-1)
			{
			    if(Const.s_GPIO[t_te[i]-1].val==9)
				{
			    	j= WyszukajNrCzujkiDlaPortu(t_te[i]);
			    	 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); sprintf(&buf_sms[ix],"P%d: %s",t_te[i],&buf_p[50]); ix+=strlen(&buf_sms[ix]);  if(ix>160-10) goto krrrr;
				    j= WyszukajNrCzujkiDlaPortu_2(t_te[i]);
                   wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  sprintf(&buf_sms[ix]," %s\r\n",&buf_p[60]); ix+=strlen(&buf_sms[ix]);           if(ix>160-10) goto krrrr;

				}
			}
		}

		for(i=0;i<_Size_te;i++)  //Temp
		{
			j= WyszukajNrCzujkiDlaPortu(t_te[i]);
			if(WyszukajNrCzujkiDlaPortu_2(t_te[i])==-1)
			{
			    if(Const.s_GPIO[t_te[i]-1].val==2)
				{
			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]); sprintf(&buf_sms[ix],"P%d: %s\r\n",t_te[i],&buf_p[50]); ix+=strlen(&buf_sms[ix]);     if(ix>160-10) goto krrrr;
				}
			    else if(Const.s_GPIO[t_te[i]-1].val==3)
			    {   wartTemp_odwrotnie_Pt(&buf_p[50],Var.value[j]); sprintf(&buf_sms[ix],"P%d: %s\r\n",t_te[i],&buf_p[50]); ix+=strlen(&buf_sms[ix]);  if(ix>160-10) goto krrrr;
			    }
			}
		}


    krrrr:
	sprintf(&buf_sms[ix],"\x1A");
	send_SMS(tel_nr);
}

void Raport_SMS()
{
  int k,ix, poz, i,j;

    k=0;  poz=0;  ix=0;
 		for(i=0;i<_Size_te;i++)  //Temp i RHU
 		{
 			j= WyszukajNrCzujkiDlaPortu_2(t_te[i]);
 			if(j!=-1)
 			{
 			    if(Const.s_GPIO[t_te[i]-1].val==9)
 				{
 			    	j= WyszukajNrCzujkiDlaPortu(t_te[i]);
 			    	 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); sprintf(&buf_sms[ix],"P%d: %s",t_te[i],&buf_p[50]); ix+=strlen(&buf_sms[ix]);  if(ix>160-60) goto krrrr;
 				    j= WyszukajNrCzujkiDlaPortu_2(t_te[i]);
                    wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  sprintf(&buf_sms[ix]," %s\r\n",&buf_p[60]); ix+=strlen(&buf_sms[ix]);           if(ix>160-60) goto krrrr;

 				}
 			}
 		}

 		for(i=0;i<_Size_te;i++)  //Temp
 		{
 			j= WyszukajNrCzujkiDlaPortu(t_te[i]);
 			if(WyszukajNrCzujkiDlaPortu_2(t_te[i])==-1)
 			{
 			    if(Const.s_GPIO[t_te[i]-1].val==2)
 				{
 			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]); sprintf(&buf_sms[ix],"P%d: %s\r\n",t_te[i],&buf_p[50]); ix+=strlen(&buf_sms[ix]);     if(ix>160-60) goto krrrr;
 				}
 			    else if(Const.s_GPIO[t_te[i]-1].val==3)
 			    {   wartTemp_odwrotnie_Pt(&buf_p[50],Var.value[j]); sprintf(&buf_sms[ix],"P%d: %s\r\n",t_te[i],&buf_p[50]); ix+=strlen(&buf_sms[ix]);  if(ix>160-60) goto krrrr;
 			    }
 			}
 		}


     krrrr:



		sprintf(&buf_sms[ix],"WE: ");  ix+=strlen(&buf_sms[ix]);
		 buf_p[0]=0;
		 for(i=0;i<_Size_we;i++)
		 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
		    {
			     if((Var.we[i/16]&(1<<(i-16*(i/16))))>0) buf_p[i]='0';
			 	 else                                    buf_p[i]='1';
		    }
		    else buf_p[i]=' ';
		 }
		 buf_p[i]=0;
		 sprintf(&buf_sms[ix],buf_p); ix+=strlen(&buf_sms[ix]);



		 sprintf(&buf_sms[ix],"\r\nWY: ");  ix+=strlen(&buf_sms[ix]);
		buf_p[0]=0;
		for(i=0;i<_Size_wy;i++)
	    {  if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11))   //WY
		   {
			    if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0) buf_p[i]='1';
			    else                                            buf_p[i]='0';
		   }
		   else buf_p[i]=' ';
		}
		buf_p[i]=0;
	    sprintf(&buf_sms[ix],buf_p); ix+=strlen(&buf_sms[ix]);



	sprintf(&buf_sms[ix],"\r\nGSM: %s\x1A",zasieg_gsm);

}

void Wyslij_SMS_Czujki(int nr_T, int po)
{
	int port;
	port= Const.s_Czujki[nr_T-1].addr_DS[8];
	if(Const.s_GPIO[port-1].val==3)
	{
		wartTemp_odwrotnie_Pt(&buf_p[6*0],Const.s_Czujki[nr_T-1].prog_max1);
		wartTemp_odwrotnie_Pt(&buf_p[6*1],Const.s_Czujki[nr_T-1].prog_max2);
		wartTemp_odwrotnie_Pt(&buf_p[6*2],Const.s_Czujki[nr_T-1].prog_min1);
		wartTemp_odwrotnie_Pt(&buf_p[6*3],Const.s_Czujki[nr_T-1].prog_min2);
		wartTemp_odwrotnie_Pt(&buf_p[6*4],Const.s_Czujki[nr_T-1].hist);
	    wartTemp_odwrotnie_Pt(&buf_p[6*5],Const.s_Czujki[nr_T-1].korekcja);
	}
	else
	{
		wartTemp_odwrotnie(&buf_p[6*0],Const.s_Czujki[nr_T-1].prog_max1);
	    wartTemp_odwrotnie(&buf_p[6*1],Const.s_Czujki[nr_T-1].prog_max2);
	    wartTemp_odwrotnie(&buf_p[6*2],Const.s_Czujki[nr_T-1].prog_min1);
	    wartTemp_odwrotnie(&buf_p[6*3],Const.s_Czujki[nr_T-1].prog_min2);
	    wartTemp_odwrotnie(&buf_p[6*4],Const.s_Czujki[nr_T-1].hist);
	    wartTemp_odwrotnie(&buf_p[6*5],Const.s_Czujki[nr_T-1].korekcja);
	}
    sprintf(&buf_p[6*6],"%02d:%02d",Const.s_Czujki[nr_T-1].czas_reak/60,  Const.s_Czujki[nr_T-1].czas_reak - 60*(Const.s_Czujki[nr_T-1].czas_reak/60) );
    sprintf(buf_sms,"111 T%d:\r\nNazwa: %s\r\nPmax1: %s\r\nPmax2: %s\r\nPmin1: %s\r\nPmin2: %s\r\nHist: %s\r\nKorekt: %s\r\nZwlok: %s\x1A",po,Const.s_Czujki[nr_T-1].nazwa, &buf_p[6*0],&buf_p[6*1],&buf_p[6*2],&buf_p[6*3],&buf_p[6*4],&buf_p[6*5],&buf_p[6*6]);
    send_SMS(tel_nr);
}
void Wyslij_SMS_Czujki_Rh(int nr_T, int po)
{
	int port;
	port= Const.s_Czujki[nr_T-1].addr_DS[8];
	if(Const.s_GPIO[port-1].val==3)
	{
		wartTemp_odwrotnie_Pt(&buf_p[6*0],Const.s_Czujki[nr_T-1].prog_max1);
		wartTemp_odwrotnie_Pt(&buf_p[6*1],Const.s_Czujki[nr_T-1].prog_max2);
		wartTemp_odwrotnie_Pt(&buf_p[6*2],Const.s_Czujki[nr_T-1].prog_min1);
		wartTemp_odwrotnie_Pt(&buf_p[6*3],Const.s_Czujki[nr_T-1].prog_min2);
		wartTemp_odwrotnie_Pt(&buf_p[6*4],Const.s_Czujki[nr_T-1].hist);
	    wartTemp_odwrotnie_Pt(&buf_p[6*5],Const.s_Czujki[nr_T-1].korekcja);
	}
	else
	{
		wartTemp_odwrotnie(&buf_p[6*0],Const.s_Czujki[nr_T-1].prog_max1);
	    wartTemp_odwrotnie(&buf_p[6*1],Const.s_Czujki[nr_T-1].prog_max2);
	    wartTemp_odwrotnie(&buf_p[6*2],Const.s_Czujki[nr_T-1].prog_min1);
	    wartTemp_odwrotnie(&buf_p[6*3],Const.s_Czujki[nr_T-1].prog_min2);
	    wartTemp_odwrotnie(&buf_p[6*4],Const.s_Czujki[nr_T-1].hist);
	    wartTemp_odwrotnie(&buf_p[6*5],Const.s_Czujki[nr_T-1].korekcja);
	}
    sprintf(&buf_p[6*6],"%02d:%02d",Const.s_Czujki[nr_T-1].czas_reak/60,  Const.s_Czujki[nr_T-1].czas_reak - 60*(Const.s_Czujki[nr_T-1].czas_reak/60) );
    sprintf(buf_sms,"111 Rh%d:\r\nNazwa: %s\r\nPmax1: %s\r\nPmax2: %s\r\nPmin1: %s\r\nPmin2: %s\r\nHist: %s\r\nKorekt: %s\r\nZwlok: %s\x1A",po,Const.s_Czujki[nr_T-1].nazwa, &buf_p[6*0],&buf_p[6*1],&buf_p[6*2],&buf_p[6*3],&buf_p[6*4],&buf_p[6*5],&buf_p[6*6]);
    send_SMS(tel_nr);
}
void Wyslij_SMS_LAN(int nr_T)
{
    sprintf(buf_sms,"111 LAN:\r\nIP: %s\r\nMaska: %s\r\nBrama: %s\r\nPort: %s\x1A",Const.s_Lan[nr_T-1].ip, Const.s_Lan[nr_T-1].mask, Const.s_Lan[nr_T-1].br, Const.s_Lan[nr_T-1].port);
    send_SMS(tel_nr);
}
void Wyslij_SMS_Wifi(int nr_T)
{
    if((Const.s_Lan[nr_T-1].param&0x04)>0) sprintf(&buf_p[6*0],"tak"); else sprintf(&buf_p[6*0],"nie");
    sprintf(buf_sms,"111 STA:\r\nssid: %s\r\nhaslo: %s\r\naktyw: %s\x1A",Const.s_Wifi[nr_T-1].ssid, Const.s_Wifi[nr_T-1].haslo, &buf_p[6*0]);
    send_SMS(tel_nr);
}
void Wyslij_SMS_AP(int nr_T)
{
    if((Const.s_Lan[nr_T-1].param&0x08)>0) sprintf(&buf_p[6*0],"tak"); else sprintf(&buf_p[6*0],"nie");
    sprintf(buf_sms,"111 AP:\r\nssid: %s\r\nhaslo: %s\r\naktyw: %s\x1A",Const.s_Wifi[nr_T-1].AP_ssid, Const.s_Wifi[nr_T-1].AP_haslo, &buf_p[6*0]);
    send_SMS(tel_nr);
}
void Wyslij_SMS_Termostat(int nr_T)
{
    int port;
    port= Const.s_Term[nr_T-1].nr_temp;
    if(Const.s_GPIO[port-1].val==3)
    {
		wartTemp_odwrotnie_Pt(&buf_p[6*0],Const.s_Term[nr_T-1].prog);
		wartTemp_odwrotnie_Pt(&buf_p[6*1],Const.s_Term[nr_T-1].hist);
	}
	else
	{
		wartTemp_odwrotnie(&buf_p[6*0],Const.s_Term[nr_T-1].prog);
	    wartTemp_odwrotnie(&buf_p[6*1],Const.s_Term[nr_T-1].hist);
	}
    sprintf(&buf_p[6*2],"%02d:%02d",Const.s_Term[nr_T-1].czas_reak/60,  Const.s_Term[nr_T-1].czas_reak - 60*(Const.s_Term[nr_T-1].czas_reak/60) );
    sprintf(buf_sms,"111 Te%d:\r\n%s\r\n%s\r\n%s\r\nProg zalacz: %s\r\nHistereza: %s\r\nCzas zwloki: %s\x1A", t_wy[nr_T-1], Const.s_PK[nr_T-1].nazwa, Const.s_PK[nr_T-1].nazwa_on, Const.s_PK[nr_T-1].nazwa_of, &buf_p[6*0],&buf_p[6*1],&buf_p[6*2]);
    send_SMS(tel_nr);
}
void Wyslij_SMS_WE(int nr_T)
{
    sprintf(&buf_p[10*0],"%02d:%02d",Const.s_WE[nr_T-1].czas_reak/60,  Const.s_WE[nr_T-1].czas_reak - 60*(Const.s_WE[nr_T-1].czas_reak/60) );
           if((Const.s_WE[nr_T-1].param&0x03)==0) sprintf(&buf_p[10*1],"on/off");
      else if((Const.s_WE[nr_T-1].param&0x03)==1) sprintf(&buf_p[10*1],"on");
      else if((Const.s_WE[nr_T-1].param&0x03)==2) sprintf(&buf_p[10*1],"off");
    sprintf(buf_sms,"111 We%d:\r\nTxt: %s\r\nON: %s\r\nOFF: %s\r\nCzas zwloki: %s\r\nReaguje: %s\x1A", t_we[nr_T-1], Const.s_WE[nr_T-1].nazwa, Const.s_WE[nr_T-1].nazwa_zw, Const.s_WE[nr_T-1].nazwa_ro, &buf_p[10*0], &buf_p[10*1]);
    send_SMS(tel_nr);
}
void Wyslij_SMS_WY(int nr_T)
{
    sprintf(buf_sms,"111 Wy%d:\r\nTxt: %s\r\nON: %s\r\nOFF: %s\x1A", t_wy[nr_T-1], Const.s_PK[nr_T-1].nazwa, Const.s_PK[nr_T-1].nazwa_on, Const.s_PK[nr_T-1].nazwa_of);
    send_SMS(tel_nr);
}

void Wyslij_SMS_Timer(int nr_T)
{
    sprintf(&buf_p[6*0],"%02d:%02d",Const.s_Tim[nr_T-1].godzON1/60,  Const.s_Tim[nr_T-1].godzON1 - 60*(Const.s_Tim[nr_T-1].godzON1/60) );
    sprintf(&buf_p[6*1],"%02d:%02d",Const.s_Tim[nr_T-1].godzON2/60,  Const.s_Tim[nr_T-1].godzON2 - 60*(Const.s_Tim[nr_T-1].godzON2/60) );
    sprintf(&buf_p[6*2],"%02d:%02d",Const.s_Tim[nr_T-1].godzON3/60,  Const.s_Tim[nr_T-1].godzON3 - 60*(Const.s_Tim[nr_T-1].godzON3/60) );
    sprintf(&buf_p[6*3],"%02d:%02d",Const.s_Tim[nr_T-1].godzON4/60,  Const.s_Tim[nr_T-1].godzON4 - 60*(Const.s_Tim[nr_T-1].godzON4/60) );
      sprintf(&buf_p[6*4],"%02d:%02d",Const.s_Tim[nr_T-1].godzOF1/60,  Const.s_Tim[nr_T-1].godzOF1 - 60*(Const.s_Tim[nr_T-1].godzOF1/60) );
      sprintf(&buf_p[6*5],"%02d:%02d",Const.s_Tim[nr_T-1].godzOF2/60,  Const.s_Tim[nr_T-1].godzOF2 - 60*(Const.s_Tim[nr_T-1].godzOF2/60) );
      sprintf(&buf_p[6*6],"%02d:%02d",Const.s_Tim[nr_T-1].godzOF3/60,  Const.s_Tim[nr_T-1].godzOF3 - 60*(Const.s_Tim[nr_T-1].godzOF3/60) );
      sprintf(&buf_p[6*7],"%02d:%02d",Const.s_Tim[nr_T-1].godzOF4/60,  Const.s_Tim[nr_T-1].godzOF4 - 60*(Const.s_Tim[nr_T-1].godzOF4/60) );
    sprintf(buf_sms,"111 Ti%d:\r\n%s\r\nON1: %s\r\nON2: %s\r\nON3: %s\r\nON4: %s\r\nOFF1: %s\r\nOFF2: %s\r\nOFF3: %s\r\nOFF4: %s\r\nPo %d\r\nWt %d\r\nSr %d\r\nCz %d\r\nPi %d\r\nSo %d\r\nNi %d\x1A", t_wy[nr_T-1], Const.s_PK[nr_T-1].nazwa, &buf_p[6*0], &buf_p[6*1], &buf_p[6*2], &buf_p[6*3], &buf_p[6*4], &buf_p[6*5], &buf_p[6*6], &buf_p[6*7],(Const.s_Tim[nr_T-1].akt>>0)&0x01,(Const.s_Tim[nr_T-1].akt>>1)&0x01,(Const.s_Tim[nr_T-1].akt>>2)&0x01,(Const.s_Tim[nr_T-1].akt>>3)&0x01,(Const.s_Tim[nr_T-1].akt>>4)&0x01,(Const.s_Tim[nr_T-1].akt>>5)&0x01,(Const.s_Tim[nr_T-1].akt>>6)&0x01);
    send_SMS(tel_nr);
}
/*
int strncmp_moj(char *in, char *out)
{
   int i,j,k,k2,x;   k=strlen(out);   k2=strlen(in);

   sprintf(buf_p,"%s %s",Const.s_PK[i].nazwa, Const.s_PK[i].nazwa_on);

   j=0;  x=0;
   for(i=0;i<k2;i++)
   {
	   if(in[i]==out[j]){ j++; if(j==k) return 1; }
	   else j=0;

   }
   return 0;
}*/

void sprawdz_SMS(char *ptr)  //sms    //AT+CUSD=1,"*101#",15  stan konta
{
    char *pt;  int i,j,nr_T,k,o,m,n, brk;  signed short tt;    int port;

    brk=0;



    for(i=0;i<_Size_wy;i++)
    {
    	if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11))  //PK. Timer, Termostat
    	{
    		sprintf(buf_p,"%s %s",Const.s_PK[i].nazwa, Const.s_PK[i].nazwa_on);
        	if(strstr(ptr, buf_p))
            {
        		  sprintf(buf_sms,"Port%d: %s %s\x1A",t_wy[i], Const.s_PK[i].nazwa, Const.s_PK[i].nazwa_on);   send_SMS(tel_nr); brk=1;  Const.s_WY[i/16].val|=(1<<(i-16*(i/16)));   GPIO_PK(i);
        		  break;
            }
        	sprintf(buf_p,"%s %s",Const.s_PK[i].nazwa, Const.s_PK[i].nazwa_of);
            if(strstr(ptr, buf_p))
            {
        		  sprintf(buf_sms,"Port%d: %s %s\x1A",t_wy[i], Const.s_PK[i].nazwa, Const.s_PK[i].nazwa_of);   send_SMS(tel_nr); brk=1;  Const.s_WY[i/16].val&=~(1<<(i-16*(i/16)));   GPIO_PK(i);
        		  break;
            }
    	}
    }



    //################   - SMS Wejscia -   ##################################
    if(ptr1=strstr(ptr,"\r\n111 We"))
    {
               if(   ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A)) && ((*(ptr1+9)>0x2F)&&(*(ptr1+9)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+8))&0x0f) + ((*(ptr1+9))&0x0f);  }
               else                                                                                 { k=0;   nr_T=     (*(ptr1+8))&0x0f;   }

               i= WyszukujPortElement(t_we,_Size_we,TabPort[nr_T-1]);
               if(i!=-1)
               {
                if(*(ptr1+9+k)==':')
                {
            	  if(strstr(ptr1,"Txt:")&&
            	   	 strstr(ptr1,"ON:")&&
            	   	 strstr(ptr1,"OFF:")&&
            	   	 strstr(ptr1,"Czas zwloki:")&&
            	   	 strstr(ptr1,"Reaguje:")    )
            	  {
            		  wpis_sms__nazwa("Txt:",ptr1);  sprintf(Const.s_WE[i].nazwa, buf_p);
            		  wpis_sms__nazwa("ON:",ptr1);   sprintf(Const.s_WE[i].nazwa_zw, buf_p);
            		  wpis_sms__nazwa("OFF:",ptr1);  sprintf(Const.s_WE[i].nazwa_ro, buf_p);
            		  wpis_sms_minsec("Czas zwloki:",ptr1);  Const.s_WE[i].czas_reak = 60*buf_p[0] + buf_p[1];
            		  wpis_sms__on_off("Reaguje:",ptr1);   Const.s_WE[i].param&=~0x03;   if(strstr(buf_p,"on/of")) Const.s_WE[i].param&=~0x03; else if(strstr(buf_p,"on")) Const.s_WE[i].param|=0x01; else if(strstr(buf_p,"of")) Const.s_WE[i].param|=0x02;
            		  delay_funkc[6]=1;  Wyslij_SMS_WE(i+1); brk=1;
            	  }
                }
                else{ Wyslij_SMS_WE(i+1); brk=1;     }
               }
    }
    else if((ptr1=strstr(ptr,"\r\n111 2"))||(ptr1=strstr(ptr,"\r\n111 3")))
    {

    	   if(strstr(tel_nr,&Const.s_Tel[0].tel1[3])||\
    			  strstr(tel_nr,&Const.s_Tel[0].tel2[3])||\
    			  strstr(tel_nr,&Const.s_Tel[0].tel3[3])||\
    			  strstr(tel_nr,&Const.s_Tel[0].tel4[3])||\
    			  strstr(tel_nr,&Const.s_Tel[0].tel5[3])||\
    			  strstr(tel_nr,&Const.s_Tel[0].tel6[3])||\
    			  strstr(tel_nr,&Const.s_Tel[0].tel7[3])||\
    			  strstr(tel_nr,&Const.s_Tel[0].tel8[3])	   )

    	   {
    	    	nr_T = 10*((*(ptr1+6))&0x0f)+(*(ptr1+7)&0x0f);
    	    	j= WyszukujPortElement(t_wy,_Size_wy,TabPort[nr_T-1]);
    	    	if(j!=-1){
    	    	if((*(ptr1+11)==':')&&(*(ptr1+14)==':'))
    	    	{
    	    		godz_ = 10*((*(ptr1+9))&0x0f)+((*(ptr1+10))&0x0f);
    	    		min_ = 10*((*(ptr1+12))&0x0f)+((*(ptr1+13))&0x0f);
    	    		sec_ = 10*((*(ptr1+15))&0x0f)+((*(ptr1+16))&0x0f);
    	    		sms_czas_pk[j]= (3600*godz_ + 60*min_ + sec_);
    	    		sms_czas_pk_start[j] = HAL_GetTick();
    	    		flaga_pk_[j]=1;   sprintf(buf_p,"\r\nSMS---%d-%d-%d-",nr_T, j, sms_czas_pk[j]); dbg3(buf_p);
    	    		Const.s_WY[0].val|=(1<<j);  GPIO_PK(j); ZapisZdarzenia(_ZdaPKsms,_PKsmson,j);

    	    		if(dzwonBlok==0)
    	    		{
    	    			delay_funkc[35]=1;  //dzwon
    	    			dzwonBlok=1;
    	    			dbg3("\r\nDzwonie ---");
    	    		}
    	    	}
    	    	else brk=1;
    	    	}
    	    	else brk=1;
    	   }


    }

    //################   - SMS Wyjscia -   ##################################
    else if(ptr1=strstr(ptr,"\r\n111 Wy"))
    {
               if(   ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A)) && ((*(ptr1+9)>0x2F)&&(*(ptr1+9)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+8))&0x0f) + ((*(ptr1+9))&0x0f);  }
               else                                                                                 { k=0;   nr_T=     (*(ptr1+8))&0x0f;   }

               i= WyszukujPortElement(t_wy,_Size_wy,TabPort[nr_T-1]);
               if(i!=-1)
               {
                if(*(ptr1+9+k)==':')
                {
            	  if(strstr(ptr1,"Txt:")&&
            	   	 strstr(ptr1,"ON:")&&
            	   	 strstr(ptr1,"OFF:")   )
            	  {
            		  wpis_sms__nazwa("Txt:",ptr1);  sprintf(Const.s_PK[i].nazwa, buf_p);
            		  wpis_sms__nazwa("ON:",ptr1);   sprintf(Const.s_PK[i].nazwa_on, buf_p);
            		  wpis_sms__nazwa("OFF:",ptr1);  sprintf(Const.s_PK[i].nazwa_of, buf_p);
            		  delay_funkc[6]=1;  Wyslij_SMS_WY(i+1); brk=1;
            	  }
                }
                else{ Wyslij_SMS_WY(i+1); brk=1;     }
               }
    }

    else if(ptr1=strstr(ptr,"\r\n111 RST"))
    {
   	    delay_funkc[1]=1;    //Restart ukladu
    }
    else if(ptr1=strstr(ptr,"\r\n111 ZGR"))  //+CDS: 49,5,"+48889691210",145,"17/06/27,08:33:50+08","17/06/27,08:33:53+08",0
    {
   	    dbg3("\r\nZEGAR   ");

  	     if(ptr2=strstr(ptr,"\","))    ptr2+=2;  else{ ResumptionReceiveDataFromGSM(); goto ominsdsdc; }
  	     if(ptr1 =strstr(ptr2,",\""))  ptr1+=2;  else{ ResumptionReceiveDataFromGSM(); goto ominsdsdc; }
  	     //UstawCzas(, , ,  , , );
  	     czas[0]= 10*(*(ptr1+0)&0x0f)+(*(ptr1+1)&0x0f);
  	     czas[1]= 10*(*(ptr1+3)&0x0f)+(*(ptr1+4)&0x0f);
  	     czas[2]= 10*(*(ptr1+6)&0x0f)+(*(ptr1+7)&0x0f);
  	     czas[4]= 10*(*(ptr1+9)&0x0f)+(*(ptr1+10)&0x0f);
  	     czas[5]= 10*(*(ptr1+12)&0x0f)+(*(ptr1+13)&0x0f);
  	     czas[6]= 10*(*(ptr1+15)&0x0f)+(*(ptr1+16)&0x0f);
  	     sprintf(buf_p,"\r\nCzas: %d/%d/%d  %d:%d:%d ",czas[0],czas[1],czas[2],czas[4],czas[5],czas[6]); dbg3(buf_p);
  	   delay_funkc[7]=1;   brk=1;  ResumptionReceiveDataFromGSM();
  	   ominsdsdc:
	   asm("nop");

    }
    else if(ptr1=strstr(ptr,"\r\n111 Z"))
    {
        if(   ((*(ptr1+7)>0x2F)&&(*(ptr1+7)<0x3A)) && ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+7))&0x0f) + ((*(ptr1+8))&0x0f);  }
        else                                                                                 { k=0;   nr_T=     (*(ptr1+7))&0x0f;   }

    	if((Const.s_GPIO[TabPort[nr_T-1]-1].val==0)||(Const.s_GPIO[TabPort[nr_T-1]-1].val==10)||(Const.s_GPIO[TabPort[nr_T-1]-1].val==11))  //PK. Timer, Termostat
    	{
    		j= WyszukujPortElement(t_wy,_Size_wy,TabPort[nr_T-1]);
    	    sprintf(buf_sms,"Port%d: %s %s\x1A",nr_T, Const.s_PK[j].nazwa, Const.s_PK[j].nazwa_on);   send_SMS(tel_nr); brk=1;  Const.s_WY[j/16].val|=(1<<(j-16*(j/16)));   GPIO_PK(j);
    	}
    }
    else if(ptr1=strstr(ptr,"\r\n111 W"))
    {
        if(   ((*(ptr1+7)>0x2F)&&(*(ptr1+7)<0x3A)) && ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+7))&0x0f) + ((*(ptr1+8))&0x0f);  }
        else                                                                                 { k=0;   nr_T=     (*(ptr1+7))&0x0f;   }

    	if((Const.s_GPIO[TabPort[nr_T-1]-1].val==0)||(Const.s_GPIO[TabPort[nr_T-1]-1].val==10)||(Const.s_GPIO[TabPort[nr_T-1]-1].val==11))  //PK. Timer, Termostat
    	{
    		j= WyszukujPortElement(t_wy,_Size_wy,TabPort[nr_T-1]);
    	    sprintf(buf_sms,"Port%d: %s %s\x1A",nr_T, Const.s_PK[j].nazwa, Const.s_PK[j].nazwa_of);   send_SMS(tel_nr); brk=1;  Const.s_WY[j/16].val&=~(1<<(j-16*(j/16)));   GPIO_PK(j);
    	}
    }
    else if(pt=strstr(ptr,"\r\n111 RapTe"))
    {
    	Raport_Te_SMS(); brk=1;
    }
    else if(pt=strstr(ptr,"\r\n111 Rap"))
    {
    	Raport_SMS(); send_SMS(tel_nr);  brk=1;
    }



//################   - SMS Termostat -   ##################################
     else if(ptr1=strstr(ptr,"\r\n111 Te"))
     {

       if(   ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A)) && ((*(ptr1+9)>0x2F)&&(*(ptr1+9)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+8))&0x0f) + ((*(ptr1+9))&0x0f);  }
       else                                                                                 { k=0;   nr_T=     (*(ptr1+8))&0x0f;   }

       i= WyszukujPortElement(t_wy,_Size_wy,TabPort[nr_T-1]);
       if(i!=-1)
       {

        if(*(ptr1+9+k)==':')
        {
    	  if( strstr(ptr1,"Txt:")&&
         	  strstr(ptr1,"ON:")&&
         	  strstr(ptr1,"OFF:")&&
    		  strstr(ptr1,"Prog zalacz:")&&
    	   	  strstr(ptr1,"Histereza:")&&
    	   	  strstr(ptr1,"Czas zwloki:")   )
    	  {

    		  wpis_sms__nazwa("Txt:",ptr1);  sprintf(Const.s_PK[i].nazwa, buf_p);
    		  wpis_sms__nazwa("ON:",ptr1);   sprintf(Const.s_PK[i].nazwa_on, buf_p);
    		  wpis_sms__nazwa("OFF:",ptr1);  sprintf(Const.s_PK[i].nazwa_of, buf_p);

    		  port= Const.s_Term[i].nr_temp;
    		  if(Const.s_GPIO[port-1].val==3)
    		  {
        		  wpis_sms__temp_Pt("Prog zalacz:",ptr1);    Const.s_Term[i].prog=ttemp;
        		  wpis_sms__temp_Pt("Histereza:",ptr1);    Const.s_Term[i].hist=ttemp;
    		  }
    		  else
    		  {
        		  wpis_sms__temp("Prog zalacz:",ptr1);    Const.s_Term[i].prog=ttemp;
        		  wpis_sms__temp("Histereza:",ptr1);    Const.s_Term[i].hist=ttemp;
    		  }

    		  wpis_sms_minsec("Czas zwloki:",ptr1);    Const.s_Term[i].czas_reak = 60*buf_p[0] + buf_p[1];
    		  delay_funkc[6]=1;  Wyslij_SMS_Termostat(i+1);  brk=1;
    	  }
        }
        else{ Wyslij_SMS_Termostat(i+1); brk=1;  }
       }
     }


    //################   - SMS Timer -   ##################################
             else if(ptr1=strstr(ptr,"\r\n111 Ti"))
             {

               if(   ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A)) && ((*(ptr1+9)>0x2F)&&(*(ptr1+9)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+8))&0x0f) + ((*(ptr1+9))&0x0f);  }
               else                                                                                 { k=0;   nr_T=     (*(ptr1+8))&0x0f;   }

               i= WyszukujPortElement(t_wy,_Size_wy,TabPort[nr_T-1]);
               if(i!=-1)
               {

                if(*(ptr1+9+k)==':')
                {
            	  if(strstr(ptr1,"ON1:")&&
            	   	 strstr(ptr1,"ON2:")&&
            	   	 strstr(ptr1,"ON3:")&&
            	   	 strstr(ptr1,"ON4:")&&
            	   	 strstr(ptr1,"OFF1:")&&
            	   	 strstr(ptr1,"OFF2:")&&
            	   	 strstr(ptr1,"OFF3:")&&
            	   	 strstr(ptr1,"OFF4:")&&
            	   	 strstr(ptr1,"Po ")&&
            	   	 strstr(ptr1,"Wt ")&&
            	   	 strstr(ptr1,"Sr ")&&
            	   	 strstr(ptr1,"Cz ")&&
            	   	 strstr(ptr1,"Pi ")&&
            	   	 strstr(ptr1,"So ")&&
            	   	 strstr(ptr1,"Ni ")&&
            	   	 strstr(ptr1,"Akt:")    )
            	  {
            		  wpis_sms_minsec("ON1:",ptr1);  Const.s_Tim[i].godzON1 = 60*buf_p[0] + buf_p[1];
            		  wpis_sms_minsec("ON2:",ptr1);  Const.s_Tim[i].godzON2 = 60*buf_p[0] + buf_p[1];
            		  wpis_sms_minsec("ON3:",ptr1);  Const.s_Tim[i].godzON3 = 60*buf_p[0] + buf_p[1];
            		  wpis_sms_minsec("ON4:",ptr1);  Const.s_Tim[i].godzON4 = 60*buf_p[0] + buf_p[1];
            		    wpis_sms_minsec("OFF1:",ptr1);  Const.s_Tim[i].godzOF1 = 60*buf_p[0] + buf_p[1];
            		    wpis_sms_minsec("OFF2:",ptr1);  Const.s_Tim[i].godzOF2 = 60*buf_p[0] + buf_p[1];
            		    wpis_sms_minsec("OFF3:",ptr1);  Const.s_Tim[i].godzOF3 = 60*buf_p[0] + buf_p[1];
            		    wpis_sms_minsec("OFF4:",ptr1);  Const.s_Tim[i].godzOF4 = 60*buf_p[0] + buf_p[1];
            		    wpis_sms_2x("Po ",ptr1);     if(buf_p[0]==1) Const.s_Tim[i].akt|=0x01; else Const.s_Tim[i].akt&=~0x01;
            		    wpis_sms_2x("Wt ",ptr1);     if(buf_p[0]==1) Const.s_Tim[i].akt|=0x02; else Const.s_Tim[i].akt&=~0x02;
            		    wpis_sms_2x("Sr ",ptr1);     if(buf_p[0]==1) Const.s_Tim[i].akt|=0x04; else Const.s_Tim[i].akt&=~0x04;
            		    wpis_sms_2x("Cz ",ptr1);     if(buf_p[0]==1) Const.s_Tim[i].akt|=0x08; else Const.s_Tim[i].akt&=~0x08;
            		    wpis_sms_2x("Pi ",ptr1);     if(buf_p[0]==1) Const.s_Tim[i].akt|=0x10; else Const.s_Tim[i].akt&=~0x10;
            		    wpis_sms_2x("So ",ptr1);     if(buf_p[0]==1) Const.s_Tim[i].akt|=0x20; else Const.s_Tim[i].akt&=~0x20;
            		    wpis_sms_2x("Ni ",ptr1);     if(buf_p[0]==1) Const.s_Tim[i].akt|=0x40; else Const.s_Tim[i].akt&=~0x40;
              		  delay_funkc[6]=1;  Wyslij_SMS_Timer(i+1); brk=1;
            	  }
                }
                else{ Wyslij_SMS_Timer(i+1); brk=1; }
               }
             }




//################   - SMS Czujki -   ##################################
 else if(ptr1=strstr(ptr,"\r\n111 T"))
 {

   if(   ((*(ptr1+7)>0x2F)&&(*(ptr1+7)<0x3A)) && ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+7))&0x0f) + ((*(ptr1+8))&0x0f); }
   else                                                                                 { k=0;   nr_T=     (*(ptr1+7))&0x0f;      }

   j=WyszukujPortElement(t_te,_Size_te,TabPort[nr_T-1]);
   if(j!=-1)
   {
   	 i=WyszukajNrCzujkiDlaPortu(t_te[j]);
   if(i!=-1)
   {
	//sprintf(buf_p,"\r\nnr_T: %d  %d",nr_T,k); dbg3(buf_p);

    if(*(ptr1+8+k)==':')
    {
	  if(strstr(ptr1,"Pmax1:")&&
	   	 strstr(ptr1,"Pmax2:")&&
	   	 strstr(ptr1,"Pmin1:")&&
	   	 strstr(ptr1,"Pmin2:")&&
	   	 strstr(ptr1,"Hist:")&&
	   	 strstr(ptr1,"Korekt:")&&
	   	 strstr(ptr1,"Zwlok:")  )
	  {
		 wpis_sms__nazwa("Nazwa:",ptr1);  sprintf(Const.s_Czujki[i].nazwa,buf_p);// dbg3(Const.s_Czujki[nr_T-1].nazwa);
		 port= Const.s_Czujki[i].addr_DS[8];
		 if(Const.s_GPIO[port-1].val==3)
		 {
			  wpis_sms__temp_Pt("Pmax1:",ptr1);    Const.s_Czujki[i].prog_max1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp_Pt("Pmax2:",ptr1);    Const.s_Czujki[i].prog_max2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp_Pt("Pmin1:",ptr1);    Const.s_Czujki[i].prog_min1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp_Pt("Pmin2:",ptr1);    Const.s_Czujki[i].prog_min2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp_Pt("Hist:",ptr1);    Const.s_Czujki[i].hist=ttemp;
			  wpis_sms__temp_Pt("Korekt:",ptr1);  Const.s_Czujki[i].korekcja=ttemp;
		 }
		 else
		 {
			  wpis_sms__temp("Pmax1:",ptr1);    Const.s_Czujki[i].prog_max1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp("Pmax2:",ptr1);    Const.s_Czujki[i].prog_max2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp("Pmin1:",ptr1);    Const.s_Czujki[i].prog_min1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp("Pmin2:",ptr1);    Const.s_Czujki[i].prog_min2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
			  wpis_sms__temp("Hist:",ptr1);    Const.s_Czujki[i].hist=ttemp;
			  wpis_sms__temp("Korekt:",ptr1);  Const.s_Czujki[i].korekcja=ttemp;
		 }
		  wpis_sms_minsec("Zwlok:",ptr1);  Const.s_Czujki[i].czas_reak = 60*buf_p[0] + buf_p[1];
		  delay_funkc[6]=1;  Wyslij_SMS_Czujki(i+1,nr_T); brk=1;
	  }
    }
    else{ Wyslij_SMS_Czujki(i+1,nr_T); brk=1; }
   }}
 }
    //################   - SMS Czujki Wilgot-   ##################################
     else if(ptr1=strstr(ptr,"\r\n111 Rh"))
     {

       if(   ((*(ptr1+8)>0x2F)&&(*(ptr1+8)<0x3A)) && ((*(ptr1+9)>0x2F)&&(*(ptr1+9)<0x3A))  ){ k=1;   nr_T= 10*((*(ptr1+8))&0x0f) + ((*(ptr1+9))&0x0f); }
       else                                                                                 { k=0;   nr_T=     (*(ptr1+8))&0x0f;      }

       j=WyszukujPortElement(t_te,_Size_te,TabPort[nr_T-1]);
       if(j!=-1)
       {
       	 i=WyszukajNrCzujkiDlaPortu_2(t_te[j]);
       if(i!=-1)
       {
    	//sprintf(buf_p,"\r\nnr_T: %d  %d",nr_T,k); dbg3(buf_p);

        if(*(ptr1+9+k)==':')
        {
    	  if(strstr(ptr1,"Pmax1:")&&
    	   	 strstr(ptr1,"Pmax2:")&&
    	   	 strstr(ptr1,"Pmin1:")&&
    	   	 strstr(ptr1,"Pmin2:")&&
    	   	 strstr(ptr1,"Hist:")&&
    	   	 strstr(ptr1,"Korekt:")&&
    	   	 strstr(ptr1,"Zwlok:")  )
    	  {
    		 wpis_sms__nazwa("Nazwa:",ptr1);  sprintf(Const.s_Czujki[i].nazwa,buf_p);// dbg3(Const.s_Czujki[nr_T-1].nazwa);
    		 port= Const.s_Czujki[i].addr_DS[8];
    		 if(Const.s_GPIO[port-1].val==3)
    		 {
    			  wpis_sms__temp_Pt("Pmax1:",ptr1);    Const.s_Czujki[i].prog_max1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp_Pt("Pmax2:",ptr1);    Const.s_Czujki[i].prog_max2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp_Pt("Pmin1:",ptr1);    Const.s_Czujki[i].prog_min1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp_Pt("Pmin2:",ptr1);    Const.s_Czujki[i].prog_min2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp_Pt("Hist:",ptr1);    Const.s_Czujki[i].hist=ttemp;
    			  wpis_sms__temp_Pt("Korekt:",ptr1);  Const.s_Czujki[i].korekcja=ttemp;
    		 }
    		 else
    		 {
    			  wpis_sms__temp("Pmax1:",ptr1);    Const.s_Czujki[i].prog_max1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp("Pmax2:",ptr1);    Const.s_Czujki[i].prog_max2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_max2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp("Pmin1:",ptr1);    Const.s_Czujki[i].prog_min1=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min1); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp("Pmin2:",ptr1);    Const.s_Czujki[i].prog_min2=ttemp; //wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[nr_T-1].prog_min2); sprintf(buf_p," %s  ",&buf_p[50]);  dbg3(buf_p);
    			  wpis_sms__temp("Hist:",ptr1);    Const.s_Czujki[i].hist=ttemp;
    			  wpis_sms__temp("Korekt:",ptr1);  Const.s_Czujki[i].korekcja=ttemp;
    		 }
    		  wpis_sms_minsec("Zwlok:",ptr1);  Const.s_Czujki[i].czas_reak = 60*buf_p[0] + buf_p[1];
    		  delay_funkc[6]=1;  Wyslij_SMS_Czujki_Rh(i+1,nr_T); brk=1;
    	  }
        }
        else{ Wyslij_SMS_Czujki_Rh(i+1,nr_T); brk=1; }
       }}
     }

    //################   - SMS Wifi -   ##################################
     else if(ptr1=strstr(ptr,"\r\n111 STA"))
     {
     	nr_T=1;
        if(*(ptr1+9)==':')
        {
    	  if(strstr(ptr1,"ssid:")&&
    	   	 strstr(ptr1,"haslo:")&&
    	   	 strstr(ptr1,"aktyw:")  )
    	  {
    		  wpis_sms__nazwa("ssid:",ptr1);  sprintf(Const.s_Wifi[nr_T-1].ssid,buf_p);
    		  wpis_sms__nazwa("haslo:",ptr1);  sprintf(Const.s_Wifi[nr_T-1].haslo,buf_p);
    		  wpis_sms_tak_nie("aktyw:",ptr1);   if(strstr(buf_p,"tak")) Const.s_Lan[nr_T-1].param|=0x04;  else Const.s_Lan[nr_T-1].param&=~0x04;
    		  delay_funkc[6]=1;  Wyslij_SMS_Wifi(nr_T); brk=1;
    	  }
        }
        else{ Wyslij_SMS_Wifi(nr_T); brk=1; }
     }

    //################   - SMS AP -   ##################################
     else if(ptr1=strstr(ptr,"\r\n111 AP"))
     {
    	nr_T=1;
    	if(*(ptr1+8)==':')
        {
    	  if(strstr(ptr1,"ssid:")&&
    	   	 strstr(ptr1,"haslo:")&&
    	   	 strstr(ptr1,"aktyw:")  )
    	  {
    		  wpis_sms__nazwa("ssid:",ptr1);  sprintf(Const.s_Wifi[nr_T-1].AP_ssid,buf_p);
    		  wpis_sms__nazwa("haslo:",ptr1);  sprintf(Const.s_Wifi[nr_T-1].AP_haslo,buf_p);
    		  wpis_sms_tak_nie("aktyw:",ptr1);   if(strstr(buf_p,"tak")) Const.s_Lan[nr_T-1].param|=0x08;  else Const.s_Lan[nr_T-1].param&=~0x08;
    		  delay_funkc[6]=1;  Wyslij_SMS_AP(nr_T); brk=1;
    	  }
        }
        else{ Wyslij_SMS_AP(nr_T); brk=1; }
     }

    //################   - SMS AP -   ##################################
     else if(ptr1=strstr(ptr,"\r\n111 LAN"))
     {
    	nr_T=2;
    	if(*(ptr1+9)==':')
        {
    	  if(strstr(ptr1,"IP:")&&
    	   	 strstr(ptr1,"Maska:")&&
    	   	 strstr(ptr1,"Brama:")&&
    	   	 strstr(ptr1,"Port:") )
    	  {
    		  wpis_sms__nazwa("IP:",ptr1);  sprintf(Const.s_Lan[nr_T-1].ip,buf_p);
    		  wpis_sms__nazwa("Maska:",ptr1);  sprintf(Const.s_Lan[nr_T-1].mask,buf_p);
    		  wpis_sms__nazwa("Brama:",ptr1);  sprintf(Const.s_Lan[nr_T-1].br,buf_p);
    		  wpis_sms__nazwa("Port:",ptr1);  sprintf(Const.s_Lan[nr_T-1].port,buf_p);
    		  delay_funkc[6]=1;  Wyslij_SMS_LAN(nr_T); brk=1;
    	  }
        }
        else{ Wyslij_SMS_LAN(nr_T); brk=1; }
     }

     if(brk==0)
     {
    	 if(strstr(ptr,"\r\n111 "));
    	 else
    	 {
    		 if(Const.s_Tel[0].tel1[3]!='0')
    		 {
        		 snprintf(buf_sms,158,"%s",ptr); buf_sms[strlen(buf_sms)]=0x1A;
        	     send_SMS(Const.s_Tel[0].tel1);
        	     goto ominrdfxzg;
    		 }
    	 }
    	 ResumptionReceiveDataFromGSM();
    	 ominrdfxzg:
		 asm("nop");
     }




}

void SendDataToModemWifi(char *txt)
{
	if((email>0)&&(debug_smtp_esp==1))
	{
		ZapisSesjiSMTP("-> ", 3, &adr_flash_SesjaSMTP);
		ZapisSesjiSMTP(txt, strlen(txt), &adr_flash_SesjaSMTP);
	}
	debug_smtp_esp=0;
	while(HAL_UART_Transmit_DMA(&huart1, txt, strlen(txt))==HAL_BUSY);
	//while(HAL_UART_Transmit(&huart1, txt, strlen(txt),10)==HAL_BUSY);
	//dbg3("OK  ");
}
/*
void XXX(void)
{	    	  wsk=0;
	    	    while(li2!=pos2)
	    	    {  wsk=1;
	    	  	 buf_rx2[it2++]=buf_tx2[pos2];     pos2++; if(pos2>8000) pos2=0;   if(it2>8000) it2=0;
	    	    }
	    	    buf_rx2[it2]=0;
}
*/

int liczba_char_na_int()
{
  if(buf_p[1]==0) return buf_p[0]&0x0f;
  if(buf_p[2]==0) return 10*(buf_p[0]&0x0f)+(buf_p[1]&0x0f);
  if(buf_p[3]==0) return 100*(buf_p[0]&0x0f)+10*(buf_p[1]&0x0f)+(buf_p[2]&0x0f);
  if(buf_p[4]==0) return 1000*(buf_p[0]&0x0f)+100*(buf_p[1]&0x0f)+10*(buf_p[2]&0x0f)+(buf_p[3]&0x0f);
}

char *strstr_hex(char *p1, char *p2)
{
  // char *pp1;  pp1=p1;

	int i,j,i1,i2;   k=strlen(p2);   i2=0;

	if(licz_htt==0) HAL_Delay(1000);

   for(i1=0;i1<100;i1++)
   {
	   if(*(p1+i1)==*(p2+i2)) i2++;
	   else i2=0;

	   if(i2==k) return (p1+i1+1-k);
   }
  // sprintf(buf_p,"\r\nXEROX: %d  %d  %d",i1,i2,k);
   return 0;

}

/*char przeszukuj_adresy_DS18B20(char *adrbuf)
{
	int i,j;  char k;

	for(i=0;i<_Size__s_Czujki_info;i++)
	{
		k=1;
		for(j=0;j<8;j++){ if(Const.s_Czujki[i].addr_DS[j]==adrbuf[j]); else k=0; }
		if(k==1) return 1;
	}
    return 0;
}

void Zaprogramuj_DS18B20()
{
  int i,j,n; char k,buf[9];

	 for(j=0;j<_Size__s_gpio;j++)
  	 {
  	    if(Const.s_GPIO[j].val==3)
  		{
  			  for(i=0;i<_Size__s_Czujki_info;i++)
  			  {
  				 if(Const.s_Czujki[i].addr_DS[8]==0) break;
  			  }
  			  if(i==_Size__s_Czujki_info) return;

  			       if(j==0) k= DS18B20_pobierz_adres(buf, GPIOA, GPIO_PIN_4);
  			  else if(j==1) k= DS18B20_pobierz_adres(buf, GPIOA, GPIO_PIN_5);
  			  else if(j==2) k= DS18B20_pobierz_adres(buf, GPIOA, GPIO_PIN_6);
  			  else if(j==3) k= DS18B20_pobierz_adres(buf, GPIOC, GPIO_PIN_4);
  			  else if(j==4) k= DS18B20_pobierz_adres(buf, GPIOC, GPIO_PIN_5);
  			  else if(j==5) k= DS18B20_pobierz_adres(buf, GPIOA, GPIO_PIN_12);
  			  else if(j==6) k= DS18B20_pobierz_adres(buf, GPIOD, GPIO_PIN_2);

  			  if(k==2)
  		      {
  				if(przeszukuj_adresy_DS18B20(buf)==1);
  				else
  				{
  					for(n=0;n<8;n++)  Const.s_Czujki[i].addr_DS[n]=buf[n];
    		    	Const.s_Czujki[i].addr_DS[8]=j+1;
  				}
				wybor_bitowy|=0x0002;

				dbg3("\r\nZapis...");
				Flash_in_progr_char_na_32(ADDR_FLASH_PAGE_124, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
				dbg3("ok ");
  		     }
  	    }
  	 }
}*/

void ZapiszFlash()
{
	dbg3("\r\nZapis...");
	Flash_in_progr_char_na_32(_AdrFlashToRAM, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
	dbg3("ok ");
}

char var_DS[_Size__s_gpio],start_DS[_Size__s_gpio],i_DS[_Size__s_gpio];


//@@@@@@@@@@@@@@@@@@@@@@@@@@@   -- Tu Zmieniamy Porty -- @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@   -- Tu Zmieniamy Porty -- @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@   -- Tu Zmieniamy Porty -- @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


void NrPortuNaPortZewn(int port, GPIO_TypeDef** GPIOx, uint16_t *GPIO_Pin)
{

      if(port==__Port_Ogoln_1){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_5; }
 else if(port==__Port_Ogoln_2){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_4; }
 else if(port==__Port_Ogoln_3){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_7; }
 else if(port==__Port_Ogoln_4){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_6; }
 else if(port==__Port_Ogoln_5){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_5; }
 else if(port==__Port_Ogoln_6){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_4; }
 else if(port==__Port_Ogoln_7){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_3; }
 else if(port==__Port_Ogoln_8){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_2; }
 else if(port==__Port_Ogoln_9){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_1; }
 else if(port==__Port_Ogoln_10){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_0; }
 else if(port==__Port_Ogoln_11){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_3; }
 else if(port==__Port_Ogoln_12){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_2; }
 else if(port==__Port_Ogoln_13){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_1; }
 else if(port==__Port_Ogoln_14){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_0; }

 else if(port==__Port_Ogoln_Nap_1){ *GPIOx=GPIOB;  *GPIO_Pin=GPIO_PIN_0; }
 else if(port==__Port_Ogoln_Nap_2){ *GPIOx=GPIOB;  *GPIO_Pin=GPIO_PIN_1; }

 else if(port==__Port_Ogoln_Etr_1){ *GPIOx=GPIOE;  *GPIO_Pin=GPIO_PIN_7; }
 else if(port==__Port_Ogoln_Etr_2){ *GPIOx=GPIOA;  *GPIO_Pin=GPIO_PIN_15; }
 else if(port==__Port_Ogoln_Etr_3){ *GPIOx=GPIOD;  *GPIO_Pin=GPIO_PIN_2; }
 else if(port==__Port_Ogoln_Etr_4){ *GPIOx=GPIOE;  *GPIO_Pin=GPIO_PIN_0; }

 else if(port==__Port_PWM_1){ *GPIOx=GPIOD;  *GPIO_Pin=GPIO_PIN_15; }
 else if(port==__Port_PWM_2){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_6; }
 else if(port==__Port_PWM_3){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_7; }

 else if(port==__Port_OC_1){ *GPIOx=GPIOD;  *GPIO_Pin=GPIO_PIN_13; }
 else if(port==__Port_OC_2){ *GPIOx=GPIOD;  *GPIO_Pin=GPIO_PIN_14; }
 else if(port==__Port_OC_3){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_8; }
 else if(port==__Port_OC_4){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_9; }

 else if(port==__Port_PK_1){ *GPIOx=GPIOC;  *GPIO_Pin=GPIO_PIN_13; }
 else if(port==__Port_PK_2){ *GPIOx=GPIOE;  *GPIO_Pin=GPIO_PIN_6; }
 else if(port==__Port_PK_3){ *GPIOx=GPIOE;  *GPIO_Pin=GPIO_PIN_5; }
 else if(port==__Port_PK_4){ *GPIOx=GPIOE;  *GPIO_Pin=GPIO_PIN_4; }
 else if(port==__Port_PK_5){ *GPIOx=GPIOE;  *GPIO_Pin=GPIO_PIN_3; }
 else if(port==__Port_PK_6){ *GPIOx=GPIOE;  *GPIO_Pin=GPIO_PIN_2; }


}
void pokaz_ADC_pt()
{
	int i;
	for(i=0;i<_Size_pt;i++)
	{
	    if(Const.s_GPIO[ t_pt[i]-1 ].val==3)
		{
			     if(t_pt[i]==__Port_Ogoln_1){ ADC_kanal(ADC_CHANNEL_15);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_2){ ADC_kanal(ADC_CHANNEL_14); ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_3){ ADC_kanal(ADC_CHANNEL_7);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_4){ ADC_kanal(ADC_CHANNEL_6);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_5){ ADC_kanal(ADC_CHANNEL_5);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_6){ ADC_kanal(ADC_CHANNEL_4);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_7){ ADC_kanal(ADC_CHANNEL_3);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_8){ ADC_kanal(ADC_CHANNEL_2);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_9){ ADC_kanal(ADC_CHANNEL_1);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_10){ ADC_kanal(ADC_CHANNEL_0);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_11){ ADC_kanal(ADC_CHANNEL_13);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_12){ ADC_kanal(ADC_CHANNEL_12);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_13){ ADC_kanal(ADC_CHANNEL_11);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_14){ ADC_kanal(ADC_CHANNEL_10);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_Nap_1){ ADC_kanal(ADC_CHANNEL_9);  ADC_avr_pt(i); }
			else if(t_pt[i]==__Port_Ogoln_Nap_2){ ADC_kanal(ADC_CHANNEL_8);  ADC_avr_pt(i); }
		}
	}
}

void pokaz_ADC_eol()
{
	int i;
	for(i=0;i<_Size_eol;i++)
	{
	    if(Const.s_GPIO[ t_eol[i]-1 ].val==13)
		{
			     if(t_eol[i]==__Port_Ogoln_1){ ADC_kanal(ADC_CHANNEL_15);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_2){ ADC_kanal(ADC_CHANNEL_14); ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_3){ ADC_kanal(ADC_CHANNEL_7);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_4){ ADC_kanal(ADC_CHANNEL_6);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_5){ ADC_kanal(ADC_CHANNEL_5);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_6){ ADC_kanal(ADC_CHANNEL_4);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_7){ ADC_kanal(ADC_CHANNEL_3);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_8){ ADC_kanal(ADC_CHANNEL_2);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_9){ ADC_kanal(ADC_CHANNEL_1);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_10){ ADC_kanal(ADC_CHANNEL_0);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_11){ ADC_kanal(ADC_CHANNEL_13);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_12){ ADC_kanal(ADC_CHANNEL_12);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_13){ ADC_kanal(ADC_CHANNEL_11);  ADC_avr_eol(i); }
			else if(t_eol[i]==__Port_Ogoln_14){ ADC_kanal(ADC_CHANNEL_10);  ADC_avr_eol(i); }
		}
	}
}
void pokaz_ADC_TT()
{
	int i;
	for(i=0;i<_Size_TT;i++)
	{
	    if(Const.s_GPIO[ t_TT[i]-1 ].val==5)
		{
		        if(t_TT[i]==__Port_Ogoln_1){ ADC_kanal(ADC_CHANNEL_15);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_2){ ADC_kanal(ADC_CHANNEL_14); ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_3){ ADC_kanal(ADC_CHANNEL_7);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_4){ ADC_kanal(ADC_CHANNEL_6);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_5){ ADC_kanal(ADC_CHANNEL_5);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_6){ ADC_kanal(ADC_CHANNEL_4);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_7){ ADC_kanal(ADC_CHANNEL_3);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_8){ ADC_kanal(ADC_CHANNEL_2);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_9){ ADC_kanal(ADC_CHANNEL_1);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_10){ ADC_kanal(ADC_CHANNEL_0);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_11){ ADC_kanal(ADC_CHANNEL_13);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_12){ ADC_kanal(ADC_CHANNEL_12);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_13){ ADC_kanal(ADC_CHANNEL_11);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_14){ ADC_kanal(ADC_CHANNEL_10);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_Nap_1){ ADC_kanal(ADC_CHANNEL_9);  ADC_avr_TT(i); }
		   else if(t_TT[i]==__Port_Ogoln_Nap_2){ ADC_kanal(ADC_CHANNEL_8);  ADC_avr_TT(i); }
		}
	}
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

char pokaz_temp(int port)
{
	int i,j,k,o;
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;

	k=0;
	for(i=0;i<_Size__s_gpio;i++)
	{
		if((Const.s_GPIO[i].val==3)||(Const.s_GPIO[i].val==2)) k=1;
	}
	if(k==0)
	{
	    var_DS[port-1]=0;
		i_DS[port-1]=0;
	    start_DS[port-1]=0;
		return 0;
	}

	NrPortuNaPortZewn(port, &GPIOx, &GPIO_Pin);

	if(start_DS[port-1]==1)
	{
	    o=0; while(DS18B20_odczyt_T(Const.s_Czujki[i_DS[port-1]].addr_DS,&ttemp,GPIOx, GPIO_Pin)==4){ o++; if(o>2) break; };
		//DS18B20_odczyt_T(Const.s_Czujki[i_DS[port-1]].addr_DS,&ttemp,GPIOx, GPIO_Pin);
	    wartTemp_odwrotnie(&buf_p[30],ttemp);
	    Var.value[i_DS[port-1]]=ttemp+Const.s_Czujki[i_DS[port-1]].korekcja;
	    //sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS[port-1]].addr_DS[8],i_DS[port-1],&buf_p[30]);
	   // dbg3(buf_p);
		start_DS[port-1]=0;
	}

    k=0;
    powtorz_pokaz:
	for(i=var_DS[port-1];i<_Size__s_Czujki_info;i++)
	{
		if(Const.s_Czujki[i].addr_DS[8]==port)
		{
		   k=1;
		   if(Const.s_GPIO[   Const.s_Czujki[i].addr_DS[8]-1   ].val==33)
		   {
			   DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOx, GPIO_Pin);
			   if(i<(_Size__s_Czujki_info-1)) var_DS[port-1]=i+1; else var_DS[port-1]=0;
			   i_DS[port-1]=i;
			   start_DS[port-1]=1;
			   return 0;
		   }
		   else if(Const.s_GPIO[   Const.s_Czujki[i].addr_DS[8]-1   ].val==2)
		   {
			   o=0; while(odczyt_temp(&ttemp,GPIOx, GPIO_Pin)==4){ o++;  if(o>2) break; };

			   if(lifeLora[port-1]==0) Var.value[i]=ttemp;
			  // Var.value[i]=ttemp;


			   //odczyt_temp(&ttemp,GPIOx, GPIO_Pin);
			   if(ttemp!=0x7fff)
			   {

				   //wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp+Const.s_Czujki[i].korekcja;   //sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);
				   if(lifeLora[port-1]==0)  Var.value[i]=ttemp+Const.s_Czujki[i].korekcja;


			   }
			   start_DS18B20(GPIOx, GPIO_Pin);
			   var_DS[port-1]=0;
			   i_DS[port-1]=0;
			   start_DS[port-1]=0;  //dbg3("X");
			   return 0;
		   }
		}
	}
    if((k==0)&&(var_DS[port-1]>0)){ var_DS[port-1]=0; goto powtorz_pokaz;  }
    var_DS[port-1]=0;
	i_DS[port-1]=0;
    start_DS[port-1]=0;
}

char pokaz_DHT11(int port)
{
	int i,j,k,o,lk1,lk2;
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;

	NrPortuNaPortZewn(port, &GPIOx, &GPIO_Pin);

	o=0; lk2=0;
    for(i=0;i<_Size__s_Czujki_info;i++)
	{
		if(Const.s_Czujki[i].addr_DS[8]==port)
		{
		   if(Const.s_GPIO[   Const.s_Czujki[i].addr_DS[8]-1   ].val==9)
		   {
			  if(lifeLora[port-1]>0);
			  else
			  {
			     if(lk2==0){ lk2=1;  lk1=DHT22_read(&buf_p[30], &buf_p[90], GPIOx, GPIO_Pin); } //Odczyt Temp

			      if(lk1==1)
			      {
			          if(o==0)
			          {                    //sprintf(&buf_p[50],"+%02d.0",buf_p[30]);
			    	      sprintf(&buf_p[50],&buf_p[30]);
			              wartTemp(&buf_p[50],&ttemp);  Var.value[i]=ttemp+Const.s_Czujki[i].korekcja;
			             // sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[50]);  dbg3(buf_p);
			              o++;
			          }
			          else
			          {
			              sprintf(&buf_p[50],"+%02d.0",buf_p[90]);
			              wartTemp(&buf_p[50],&ttemp);  Var.value[i]=ttemp+Const.s_Czujki[i].korekcja;
			             // sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[50]);  dbg3(buf_p);
			          }

			      }
			      else
			      {
			    	  if(o==0)
			    	  {
			    	     sprintf(&buf_p[50],"--.-");  Var.value[i]=0x7fff;
			    	    // sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[50]);  dbg3(buf_p);
			    	     o++;
			    	  }
			    	  else
			    	  {
			    	     sprintf(&buf_p[50],"--");    Var.value[i]=0x7fff;
			    	     //sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[50]);  dbg3(buf_p);
			    	  }

			      }
			  }
		   }

		}
	}
}

/*
void pokaz_temp()
{
	int i,j;  char k;

	if(start_DS==1)
	{
		start_DS=0;
		     if(port_DS==1){ DS18B20_odczyt_T(Const.s_Czujki[i_DS].addr_DS,&ttemp,GPIOA, GPIO_PIN_4);  wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i_DS]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS].addr_DS[8],i_DS,&buf_p[30]);  dbg3(buf_p); }
		else if(port_DS==2){ DS18B20_odczyt_T(Const.s_Czujki[i_DS].addr_DS,&ttemp,GPIOA, GPIO_PIN_5);  wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i_DS]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS].addr_DS[8],i_DS,&buf_p[30]);  dbg3(buf_p); }
		else if(port_DS==3){ DS18B20_odczyt_T(Const.s_Czujki[i_DS].addr_DS,&ttemp,GPIOA, GPIO_PIN_6);  wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i_DS]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS].addr_DS[8],i_DS,&buf_p[30]);  dbg3(buf_p); }
		else if(port_DS==4){ DS18B20_odczyt_T(Const.s_Czujki[i_DS].addr_DS,&ttemp,GPIOC, GPIO_PIN_4);  wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i_DS]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS].addr_DS[8],i_DS,&buf_p[30]);  dbg3(buf_p); }
		else if(port_DS==5){ DS18B20_odczyt_T(Const.s_Czujki[i_DS].addr_DS,&ttemp,GPIOC, GPIO_PIN_5);  wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i_DS]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS].addr_DS[8],i_DS,&buf_p[30]);  dbg3(buf_p); }
		else if(port_DS==6){ DS18B20_odczyt_T(Const.s_Czujki[i_DS].addr_DS,&ttemp,GPIOA, GPIO_PIN_12); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i_DS]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS].addr_DS[8],i_DS,&buf_p[30]);  dbg3(buf_p); }
		else if(port_DS==7){ DS18B20_odczyt_T(Const.s_Czujki[i_DS].addr_DS,&ttemp,GPIOD, GPIO_PIN_2);  wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i_DS]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i_DS].addr_DS[8],i_DS,&buf_p[30]);  dbg3(buf_p); }
		i_DS++;
		if(i_DS<_Size__s_Czujki_info) var1_DS=i_DS;
		else                          var1_DS=0;
	}

     k=0;
     powtorz_pokaz:
	 for(i=var1_DS;i<_Size__s_Czujki_info;i++)
	 {
		if(Const.s_Czujki[i].addr_DS[8]>0)
		{
		  k=1;
		  if(Const.s_GPIO[0].val[   Const.s_Czujki[i].addr_DS[8]-1   ]==3)
		  {
			      if(Const.s_Czujki[i].addr_DS[8]==1){ start_DS=1; port_DS=1; DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOA, GPIO_PIN_4);  i_DS=i; goto sdsdsd; }
		     else if(Const.s_Czujki[i].addr_DS[8]==2){ start_DS=1; port_DS=2; DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOA, GPIO_PIN_5);  i_DS=i; goto sdsdsd; }
		     else if(Const.s_Czujki[i].addr_DS[8]==3){ start_DS=1; port_DS=3; DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOA, GPIO_PIN_6);  i_DS=i; goto sdsdsd; }
		     else if(Const.s_Czujki[i].addr_DS[8]==4){ start_DS=1; port_DS=4; DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOC, GPIO_PIN_4);  i_DS=i; goto sdsdsd; }
		     else if(Const.s_Czujki[i].addr_DS[8]==5){ start_DS=1; port_DS=5; DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOC, GPIO_PIN_5);  i_DS=i; goto sdsdsd; }
		     else if(Const.s_Czujki[i].addr_DS[8]==6){ start_DS=1; port_DS=6; DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOA, GPIO_PIN_12); i_DS=i; goto sdsdsd; }
		     else if(Const.s_Czujki[i].addr_DS[8]==7){ start_DS=1; port_DS=7; DS18B20_conv_T(Const.s_Czujki[i].addr_DS, GPIOD, GPIO_PIN_2);  i_DS=i; goto sdsdsd; }
		  }
		  else if(Const.s_GPIO[0].val[   Const.s_Czujki[i].addr_DS[8]-1   ]==2)
		  {
			      if(Const.s_Czujki[i].addr_DS[8]==1){ odczyt_temp(&ttemp,GPIOA, GPIO_PIN_4); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);  start_DS18B20(GPIOA, GPIO_PIN_4);  }
			 else if(Const.s_Czujki[i].addr_DS[8]==2){ odczyt_temp(&ttemp,GPIOA, GPIO_PIN_5); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);   start_DS18B20(GPIOA, GPIO_PIN_5);  }
			 else if(Const.s_Czujki[i].addr_DS[8]==3){ odczyt_temp(&ttemp,GPIOA, GPIO_PIN_6); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);   start_DS18B20(GPIOA, GPIO_PIN_6);  }
			 else if(Const.s_Czujki[i].addr_DS[8]==4){ odczyt_temp(&ttemp,GPIOC, GPIO_PIN_4); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);   start_DS18B20(GPIOC, GPIO_PIN_4);  }
			 else if(Const.s_Czujki[i].addr_DS[8]==5){ odczyt_temp(&ttemp,GPIOC, GPIO_PIN_5); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);   start_DS18B20(GPIOC, GPIO_PIN_5);  }
			 else if(Const.s_Czujki[i].addr_DS[8]==6){ odczyt_temp(&ttemp,GPIOA, GPIO_PIN_12); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);   start_DS18B20(GPIOA, GPIO_PIN_12); }
			 else if(Const.s_Czujki[i].addr_DS[8]==7){ odczyt_temp(&ttemp,GPIOD, GPIO_PIN_2); wartTemp_odwrotnie(&buf_p[30],ttemp); Var.value[i]=ttemp; sprintf(buf_p,"\r\n%02d-%02d: %s",Const.s_Czujki[i].addr_DS[8],i,&buf_p[30]);  dbg3(buf_p);   start_DS18B20(GPIOD, GPIO_PIN_2);  }
		  }
		}
	 }
	 if((k==0)&&(var1_DS>0)){ var1_DS=0; goto powtorz_pokaz;  }
	 var1_DS=0;
	 sdsdsd:
	 asm("nop");
}//if((i+1)<_Size__s_Czujki_info){ if(Const.s_Czujki[i].addr_DS[8]!=Const.s_Czujki[i+1].addr_DS[8]); else goto sdsdsd; }
*/

unsigned char hex_to_ASCII (unsigned char kkk)
{
    if(kkk==0x01) return '1';
    else if(kkk==0x02) return '2';
	else if(kkk==0x03) return '3';
	else if(kkk==0x04) return '4';
	else if(kkk==0x05) return '5';
	else if(kkk==0x06) return '6';
	else if(kkk==0x07) return '7';
	else if(kkk==0x08) return '8';
	else if(kkk==0x09) return '9';
	else if(kkk==0x00) return '0';
	else if(kkk==0x0A) return 'A';
	else if(kkk==0x0B) return 'B';
	else if(kkk==0x0C) return 'C';
	else if(kkk==0x0D) return 'D';
	else if(kkk==0x0E) return 'E';
	else if(kkk==0x0F) return 'F';
	return 0;
}

char ASCII_to_hex (char kkk)
{
    if(kkk=='1') return 0x01;
    else if(kkk=='2') return 0x02;
	else if(kkk=='3') return 0x03;
	else if(kkk=='4') return 0x04;
	else if(kkk=='5') return 0x05;
	else if(kkk=='6') return 0x06;
	else if(kkk=='7') return 0x07;
	else if(kkk=='8') return 0x08;
	else if(kkk=='9') return 0x09;
	else if(kkk=='0') return 0x00;
	else if((kkk=='A')||(kkk=='a')) return 0x0A;
	else if((kkk=='B')||(kkk=='b')) return 0x0B;
	else if((kkk=='C')||(kkk=='c')) return 0x0C;
	else if((kkk=='D')||(kkk=='d')) return 0x0D;
	else if((kkk=='E')||(kkk=='e')) return 0x0E;
	else if((kkk=='F')||(kkk=='f')) return 0x0F;
	return 0;
}


void init_ENC()
{
  		ConvertParamNET();
  		WgrajMacENC();
  			netstackInit(IPADDRESS, NETMASK, GATEWAY);     ip_addr_dest=GATEWAY;
  		  	//enc28j60RegDump();
  		  	dbg3("\r\nKoniec init_ENC28 \r\n");

  		  for(i=0; i<HTTP_task_LEN; i++)  replay_packet[i]=200;
  		  //dhcpInit();
  		  PORT_MAIL_S=5789;

  		delay_funkc[15]=1;  //Send arp request
}

void InitUartDbg(void)
{
	UART_HandleTypeDef huart3;
      HAL_UART_MspDeInit(&huart3);
	  HAL_UART_MspInit(&huart3);

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
	  ResumptionReceiveDataFromDebug();

}

void InitUartWifi(void)
{
	UART_HandleTypeDef huart1;

	DMA_HandleTypeDef hdma_usart1_rx;
	DMA_HandleTypeDef hdma_usart1_tx;

	dbg3("\r\nRST  UART_2 ");

	    HAL_UART_MspDeInit(&huart1);
    	  HAL_UART_MspInit(&huart1);

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


// ResumptionReceiveDataFromWifi();
    	 // for(i=0;i<_Size_WskFragmentPage;i++) buf_rx2[_Size_BufWifi+i]='a';
// SendHttpData(&buf_rx2[_Size_BufWifi], PacketLen);

    	  //for(i=0;i<_Size_WskFragmentPage;i++) buf_rx2[_Size_BufWifi+i]=0;
    	  //SendDataToModemWifi("AT\x0D\x0A");
    	 // HAL_Delay(300);

    	   ResumptionReceiveDataFromWifi();
    	  dbg3("\r\nAT ");
    	      	  SendDataToModemWifi("AT\x0D\x0A");
    	   		  HAL_Delay(10);
    	   		  dbg3(buf_rx2);
    	   		if(strstr(buf_rx2,"busy")||strstr(buf_rx2,"OK"));
    	   		else
    	   		{
    	   			//ZapisZdarzenia(_ZdaTest,_Esp2,0);
    	   			if(ResetWifiAT==1)
    	   			{
    	   				//delay_funkc[33]=1;
    	   				delay_funkc[1]=1;
    	   			}
    	   			else
    	   			{
    	   				delay_funkc[1]=1;
    	   			}

    	   		}
    	   		ResumptionReceiveDataFromWifi();
}

void InitUartWifiHttp(void)
{
	UART_HandleTypeDef huart1;

	DMA_HandleTypeDef hdma_usart1_rx;
	DMA_HandleTypeDef hdma_usart1_tx;

	dbg3("\r\nRST  UART_2 ");

	    HAL_UART_MspDeInit(&huart1);
    	  HAL_UART_MspInit(&huart1);

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


     ResumptionReceiveDataFromWifi();
    	 // for(i=0;i<_Size_WskFragmentPage;i++) buf_rx2[_Size_BufWifi+i]='a';
    SendHttpData(&buf_rx2[_Size_BufWifi], PacketLen);

    	  //for(i=0;i<_Size_WskFragmentPage;i++) buf_rx2[_Size_BufWifi+i]=0;
    	  //SendDataToModemWifi("AT\x0D\x0A");
    	 // HAL_Delay(300);

    	   ResumptionReceiveDataFromWifi();
    	  dbg3("\r\nAT ");
    	      	  SendDataToModemWifi("AT\x0D\x0A");
    	   		  HAL_Delay(10);
    	   		  dbg3(buf_rx2);
    	   		if(strstr(buf_rx2,"busy")||strstr(buf_rx2,"OK"));
    	   		else
    	   		{
    	   			delay_funkc[1]=1;
    	   		}
    	   		ResumptionReceiveDataFromWifi();
}

void InitUartGSM(void)
{
	UART_HandleTypeDef huart4;

	DMA_HandleTypeDef hdma_uart4_rx;

	dbg3("\r\nRST  UART_4 ");

	    HAL_UART_MspDeInit(&huart4);
    	  HAL_UART_MspInit(&huart4);

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

    	  ResumptionReceiveDataFromGSM();

}




int WyszukajNrCzujkiDlaPortu(int port)
{
  int i;
  for(i=0;i<_Size__s_Czujki_info;i++)
  {  if(Const.s_Czujki[i].addr_DS[8]==port) return i;
  }
  return -1;
}
int WyszukajNrCzujkiDlaPortu_2(int port)
{
  int i,c;  c=0;
  for(i=0;i<_Size__s_Czujki_info;i++)
  {
	  if(c==0){ if(Const.s_Czujki[i].addr_DS[8]==port) c=1;  }
	  else    { if(Const.s_Czujki[i].addr_DS[8]==port) return i;  }
  }
  return -1;
}

int HttpMobilePK(char ni)  //mobile/pk
{
     if((Const.s_WY[ni/16].val&(1<<(ni-16*(ni/16))))>0)
     {	 Const.s_WY[ni/16].val&=~(1<<(ni-16*(ni/16)));
         sprintf(buf_PK, "<font color=\"#000\">%s</font>",Const.s_PK[ni].nazwa_of);
         ZapisZdarzenia(_ZdaPKwww,_PKoff,ni);
#ifdef _LoRa
  if(lifeLora[t_wy[ni]-1]>0) Lora_Send_Przek(t_wy[ni],0);
#endif
     }
     else
     {	 Const.s_WY[ni/16].val|=(1<<(ni-16*(ni/16)));
         sprintf(buf_PK, "<font color=\"#eee\">%s</font>",Const.s_PK[ni].nazwa_on);
         ZapisZdarzenia(_ZdaPKwww,_PKon,ni);
#ifdef _LoRa
  if(lifeLora[t_wy[ni]-1]>0) Lora_Send_Przek(t_wy[ni],1);
#endif
     }

	/*  if((Const.s_WY[0].val&(1<<ni))>0)
	{	 Const.s_WY[0].val&=~(1<<ni);
		 sprintf(buf_PK, "<font color=\"#000\">%s</font>",Const.s_PK[ni].nazwa_of);
		 Lora_send_cycle=10002;
	}
	else
	{	 Const.s_WY[0].val|=(1<<ni);
		sprintf(buf_PK, "<font color=\"#eee\">%s</font>",Const.s_PK[ni].nazwa_on);
		Lora_send_cycle=10001;
	}*/
    GPIO_PK(ni);
    //ZapiszFlash();
    FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) );
    return strlen(buf_PK);
}

void ZmianaTTnaTME(int port, char *buf)
{
	unsigned short dd; unsigned char mm,c,buf_p[20];  int cal;

	IntToChar(Const.s_ADC_TT[port].val, buf_p); //dbg3("\r\n"); dbg3(buf_p); dbg3("   ");
	if(buf_p[1]=='.')
	{
		dd = buf_p[0]&0x0f;
		mm = buf_p[2]&0x0f;
	}
	else
	{
		dd = 10*(buf_p[0]&0x0f)+(buf_p[1]&0x0f);
		mm = buf_p[3]&0x0f;
	}
	cal= 10*dd + mm;  if(cal>99){ cal=99; dd=9; mm=9;  }

	if((Const.s_TT[port].dd[cal]&0x8000)==0) c=' ';
	else                                     c='-';
	sprintf(buf,"%c %04d.%02d ",c,0x7FFF&Const.s_TT[port].dd[cal],Const.s_TT[port].mm[cal]);  // + 0000.00
	if(buf[2]=='0')
	{  buf[2]=' ';  buf[1]=c; buf[0]=' ';
	   if(buf[3]=='0')
	   {  buf[3]=' ';  buf[2]=c; buf[1]=' ';
	      if(buf[4]=='0')
	      {     buf[4]=' ';  buf[3]=c; buf[2]=' ';
	      }
	   }
	}
	if(buf[8]=='0') buf[8]=' ';
}

char KolorTemp(int i)
{
  if(Var.value[i]!=0x7fff)
  {
	  if(Var.value[i] > Const.s_Czujki[i].prog_max1)
	  {
		  return 0x31;
	  }
	  else if( (Var.value[i] < Const.s_Czujki[i].prog_max1) && (Var.value[i] > Const.s_Czujki[i].prog_min1) )
	  {
          return 0x30;
	  }
	  else if(Var.value[i]< Const.s_Czujki[i].prog_min1)
	  {
		  return 0x32;
	  }
	  else return 0x30;
  }
  else return 0x30;
}

int HttpDynTME()  //TME.txt
{  int itx,i,j,a;  itx=0;  char tt,ww, *pkh;


												  pkh= &buf_TME[0];
												  PobierzCzas(buf_p);
												  sprintf(pkh, "%02d:%02d:%02d &nbsp;%02d/%02d/%02d%s",buf_p[4],buf_p[5],buf_p[6] ,buf_p[0],buf_p[1],buf_p[2],zasieg_gsm);     pkh+=strlen(pkh);
												  //sprintf(pkh, "10:35:09 &nbsp;01/01/0900");     pkh+=strlen(pkh);

												  for(i=0;i<_Size_wy;i++)   //WY
												  {
												  	if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){ if(lifeLora[t_wy[i]-1]>0) *(pkh+i)='3'; else *(pkh+i)='1';  }
												  	else                                           { if(lifeLora[t_wy[i]-1]>0) *(pkh+i)='2'; else *(pkh+i)='0';  }
												  }
												  *(pkh+i)=' '; *(pkh+i+1)=0;   pkh+=strlen(pkh);


												  for(i=0;i<_Size_we;i++)   //WE
												  {
												  	if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){ if(lifeLora[t_we[i]-1]>0) *(pkh+i)='3'; else *(pkh+i)='1';  }
												  	else                                   { if(lifeLora[t_we[i]-1]>0) *(pkh+i)='2'; else *(pkh+i)='0';  }
												  }
												  *(pkh+i)=' '; *(pkh+i+1)=0;   pkh+=strlen(pkh);


												  for(i=0;i<_Size_eol;i++)   //WE EOL
												  {
													if((Var.eol[i/16]&(1<<(i-16*(i/16))))>0)
													{
														if(lifeLora[t_eol[i]-1]>0) *(pkh+i)='6'; else *(pkh+i)='5';
													}
													else
													{
													  	if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){ if(lifeLora[t_eol[i]-1]>0) *(pkh+i)='3'; else *(pkh+i)='1';  }
													  	else                                   { if(lifeLora[t_eol[i]-1]>0) *(pkh+i)='2'; else *(pkh+i)='0';  }

													}
												  }
												  *(pkh+i)=0;   pkh+=strlen(pkh);



												  for(i=0;i<_Size_te;i++)   //Te
												  {
												  	 j= WyszukajNrCzujkiDlaPortu(t_te[i]);
												  	 if(j!=-1)
												  	 {
												  		 if(Const.s_GPIO[t_te[i]-1].val==2)
												  		 {
												  			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';
												  			 if((Var.value[j]>0x063f)&&(Var.value[j]<0x7FFF)){ buf_p[50+4]=' '; buf_p[50+5]=0; }
												  		 	  buf_p[60]='-';
												  		 	  buf_p[61]='-';
												  		 	  buf_p[62]='%';
												  		 	  buf_p[63]=0;
														 	  if(lifeLora[t_te[i]-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
												  		     sprintf(pkh," %s%c%c%s0", &buf_p[50], KolorTemp(j),buf_p[70] ,&buf_p[60]);  pkh+=strlen(pkh);
												  		 }
												  		 else if(Const.s_GPIO[t_te[i]-1].val==9)
												     	     {
												  			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';  tt=KolorTemp(j);
												     	         j= WyszukajNrCzujkiDlaPortu_2(t_te[i]);
												     	         if(j!=-1)
												     	         {
												     	    	     wartTemp_odwrotnie(&buf_p[60],Var.value[j]); if(buf_p[60+1]=='0') buf_p[60+1]=' ';  ww=KolorTemp(j);
												     	         }
												    		 	 if(lifeLora[t_te[i]-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
												     	         sprintf(pkh," %s%c%c%c%c",&buf_p[50],tt, buf_p[70],buf_p[61],buf_p[62]);
												     	         a=strlen(pkh); pkh[a++]='%'; pkh[a++]=ww; pkh[a]=0;   pkh+=strlen(pkh);
												     	     }
												  		 else
												  		 {
												  	 	    buf_p[60]='-';
												  		    buf_p[61]='-';
												  		    buf_p[62]='%';
												  		    buf_p[63]=0;
												  		    sprintf(pkh,"  --.-0 %s0",&buf_p[60]);   pkh+=strlen(pkh);
												  		 }
												  	 }
												  	 else
												  	 {
												  	    buf_p[60]='-';
												  	    buf_p[61]='-';
												  	    buf_p[62]='%';
												  	    buf_p[63]=0;
												  	    sprintf(pkh,"  --.-0 %s0",&buf_p[60]);   pkh+=strlen(pkh);
												  	 }
												  }
												  *(pkh+0)=' '; *(pkh+1)=' '; *(pkh+2)=0;   pkh+=strlen(pkh);




												  for(i=0;i<_Size_TT;i++)
												  {
												  	ZmianaTTnaTME(i,buf_p);
												  	sprintf(pkh,"%s",buf_p);  pkh+=strlen(pkh);
												  }
												  *(pkh+0)=' '; *(pkh+1)=0;   pkh+=strlen(pkh);



												  for(i=0;i<_Size_pt;i++)
												  {
												  	 if(Const.s_ADC_pt[i].val==0x7FFF){ sprintf(pkh,"   ---   0");  pkh+=strlen(pkh); }
												  	 else
												  	 {
												  	     sprintf(buf_p,"          ");
												  	     if((int16_t)Const.s_ADC_pt[i].val<0)
												  	     {
												  	    	 buf_p[0]='-';
												  	    	 sprintf(&buf_p[1],"%d",(int16_t)Const.s_ADC_pt[i].val);   buf_p[1]=' '; buf_p[1+strlen(&buf_p[1])]=' ';
												  	     }
												  	     else
												  	     {
												  	    	 buf_p[0]='+';
												  	    	 sprintf(&buf_p[2],"%d",(int16_t)Const.s_ADC_pt[i].val);   buf_p[1]=' '; buf_p[2+strlen(&buf_p[2])]=' ';
												  	     }
												  	     tt=KolorTemp( WyszukajNrCzujkiDlaPortu(t_pt[i]));
												  	     buf_p[9]=tt;
												  	     buf_p[10]=0;
												  	     sprintf(pkh,"%s",buf_p);  pkh+=strlen(pkh);
												  	 }
												  }
												  *(pkh+0)=' '; *(pkh+1)=0;   pkh+=strlen(pkh);



												  for(i=0;i<_Size_TT;i++)   //nazwy jdn.adc
												  {
												   	sprintf(pkh,"%s",Const.s_TT[i].jm);  pkh+=strlen(pkh);
												   	for(j=0;j<(5-strlen(Const.s_TT[i].jm));j++) *(pkh+j)=' ';
												   	*(pkh+j+0)=' ';
												   	*(pkh+j+1)=0;
												   	pkh+=strlen(pkh);
												  }

												  for(i=0;i<_Size_imp;i++)
												  {
												  	for(j=0;j<_Imp_IleVal;j++)
												  	{
												  		    PrzeliczLicznikKolejny(i,j,pkh);
												  		    if(j==0)
												  		    {
												  	 	        *(pkh+17+5+0)=' ';
												  	 	        *(pkh+17+5+1)=0;
												  		    }
												  		    else
												  		    {
												  		    	if((Var.etrwsk[i][j]>0)||(Var.etrwsk2[i][j]>0)) *(pkh+16+0+0)='1';
												  		    	else                                            *(pkh+16+0+0)='0';
												  		    	*(pkh+16+1+0)=' ';
												   	            *(pkh+16+1+1)=0;
												  		    }
												   	        pkh+=strlen(pkh);
												      }
												  }

  if(strlen(pkh)> _Size_buf_TME) dbg3("\r\nPRZEKROCZONE buf_TME !!!!");
}

/*
void NewPanelID(int i)  //main nowy
{
   int itx,j,c;  itx=0;  j=0;

        if(i==0) buf_p[50]='a';
   else if(i==1) buf_p[50]='b';
   else if(i==2) buf_p[50]='c';
   else if(i==3) buf_p[50]='d';
   else if(i==4) buf_p[50]='e';
   else if(i==5) buf_p[50]='f';
   else if(i==6) buf_p[50]='g';
   else if(i==7) buf_p[50]='h';

//----------------------  PK  --------------
   if((Const.s_WY[0].val&(1<<i))>0){
		if(i<7) sprintf(&buf_p[10],"PK%d_ON",i+1); else sprintf(&buf_p[10],"PK%d_ON prze",i+1);
		sprintf(&buf_p[30],"fff");
   }
   else{
	    if(i<7) sprintf(&buf_p[10],"PK%d_OFF",i+1); else sprintf(&buf_p[10],"PK%d_OFF prze",i+1);
		sprintf(&buf_p[30],"000");
   }
   sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"00%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='gsm' class=\"shiny-button\">%d</a></td><td width=\"1px\"></td><td><a href=\"lpc.cgi/pprzek%02d\"  onclick=\"my_scroll('lpc.cgi/pprzek%02d\'); return false\" class=\"c\"><font color=\"#%s\">%s</font></a></td><td width=\"10px\"></td><td></td></tr></table></div>",buf_p[50],i+1,i,i,&buf_p[30],&buf_p[10]);
   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//----------------------  Temp  --------------
   c=0;
   for(j=0;j<_Size__s_Czujki_info;j++)
   {   if(Const.s_GPIO[   Const.s_Czujki[j].addr_DS[8]-1   ].val==2){
	      if(Const.s_Czujki[j].addr_DS[8]==(i+1)){ c=1; break; }
       }
   }
   if(i<7)
   {   if(c==1) wartTemp_odwrotnie(buf_p,Var.value[j]);
       else     sprintf(buf_p,"---.-");
       sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"02%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='temp' class=\"shiny-button\">%d</a></td><td width=\"10px\"></td><td><button class=\"m\">%c %s °C</button></td><td width=\"10px\"></td><td></td></tr></table></div>",buf_p[50],i+1,buf_p[0],&buf_p[1]);
   }
   else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"02%c\" style=\"display: none;\"></div>\r\n",buf_p[50]);
   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//----------------------  Temp i Wilg  --------------
   c=0;
   for(j=0;j<_Size__s_Czujki_info;j++)
   {   if(Const.s_GPIO[   Const.s_Czujki[j].addr_DS[8]-1   ].val==9){
	      if(Const.s_Czujki[j].addr_DS[8]==(i+1)){ c=1; break; }
       }
   }
   if(i<7)
   {   if(c==1)
	   {   wartTemp_odwrotnie(buf_p,Var.value[j]);
           wartTemp_odwrotnie(&buf_p[30],Var.value[j+1]);
	   }
	   else
	   {   sprintf(buf_p,"---.-");
	       sprintf(&buf_p[31],"--");
	   }
       buf_p[99]='%';
       sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"09%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='temp' class=\"shiny-button\">%d</a></td><td width=\"10px\"></td><td><button class=\"m\">%c %s °C</button></td><td width=\"10px\"></td><td><button class=\"m\"> %c%c %</button></td></tr></table></div>",buf_p[50],i+1,buf_p[0],&buf_p[1], buf_p[31],buf_p[32]);
   }
   else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"09%c\" style=\"display: none;\"></div>\r\n",buf_p[50]);
   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//----------------------  Timer --------------
    if((Const.s_WY[0].val&(1<<i))>0){
    	if(i<7) sprintf(&buf_p[10],"PK%d_ON",i+1); else sprintf(&buf_p[10],"PK%d_ON timer",i+1);
   		sprintf(&buf_p[30],"fff");
    }
    else{
    	if(i<7) sprintf(&buf_p[10],"PK%d_OFF",i+1); else sprintf(&buf_p[10],"PK%d_OFF timer",i+1);
   		sprintf(&buf_p[30],"000");
    }
    sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"10%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='tim' class=\"shiny-button\">%d</a></td><td width=\"1px\"></td><td><a href=\"lpc.cgi/pprzek%02d\"  onclick=\"my_scroll('lpc.cgi/pprzek%02d\'); return false\" class=\"c\"><font color=\"#%s\">%s</font></a></td><td width=\"10px\"></td><td></td></tr></table></div>",buf_p[50],i+1,i,i,&buf_p[30],&buf_p[10]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//----------------------  Termostat --------------
        if((Const.s_WY[0].val&(1<<i))>0){
        	if(i<7) sprintf(&buf_p[10],"PK%d_ON",i+1); else sprintf(&buf_p[10],"PK%d_ON termost",i+1);
       		sprintf(&buf_p[30],"fff");
        }
        else{
        	if(i<7) sprintf(&buf_p[10],"PK%d_OFF",i+1); else sprintf(&buf_p[10],"PK%d_OFF termost",i+1);
       		sprintf(&buf_p[30],"000");
        }
        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"11%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='term' class=\"shiny-button\">%d</a></td><td width=\"1px\"></td><td><a href=\"lpc.cgi/pprzek%02d\"  onclick=\"my_scroll('lpc.cgi/pprzek%02d\'); return false\" class=\"c\"><font color=\"#%s\">%s</font></a></td><td width=\"10px\"></td><td></td></tr></table></div>",buf_p[50],i+1,i,i,&buf_p[30],&buf_p[10]);
        itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//------------------- WE ------------------------------
      if(i<7)
      {  if((Var.we&(0x01<<i))>0) sprintf(buf_p,"%s",Const.s_WE[i].nazwa_ro);
   	     else                     sprintf(buf_p,"%s",Const.s_WE[i].nazwa_zw);
         sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"04%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='we' class=\"shiny-button\">%d</a></td><td width=\"1px\"></td><td><button class=\"m\">%s</button></td><td width=\"10px\"></td><td></td></tr></table></div>",buf_p[50],i+1,buf_p);
      }
 	  else    sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"04%c\" style=\"display: none;\"></div>\r\n",buf_p[50]);
   	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//------------------- PWM ------------------------------
   	  if((i<7)&&(i!=5))
   	  {  if(i==6) j=5; else j=i;     c=i;
   	     sprintf(buf_p,"%d",Const.s_PWM[j].duty);
   	     sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"01%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='main' class=\"shiny-button\">%d</a></td><td width=\"12px\"></td><td width=\"42px\"><output  class=\"v\" id=\"volume%d\">%s</output></td><td><input type=\"range\" name=\"n_pwm%d\"  min=\"0\" max=\"100\"  step=\"1\"  value=\"%s\"   oninput=\"outputUpdate%d(value)\" onchange=\"x%d(this.value);\"  onclick=\"my_scroll('lpc.cgi/n_pwm%d=' + this.value + '&n_115_zapisz='); return false\" ></input></td></tr></table></div>",buf_p[50],c,j+1,buf_p,j+1,buf_p,j+1,j+1,j+1);
   	  }
   	  else  sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"01%c\" style=\"display: none;\"></div>\r\n",buf_p[50]);
 	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//------------------- ADC ------------------------------
 	 if(i<7)
 	 {   IntToChar(Const.s_ADC[i].val, buf_p);
         sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"05%c\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='main' class=\"shiny-button\">%d</a></td><td width=\"10px\"></td><td><button class=\"m\"> %s V</button></td><td width=\"10px\"></td><td></td></tr></table></div>",buf_p[50],i+1,buf_p);
 	 }
     else  sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"05%c\" style=\"display: none;\"></div>\r\n",buf_p[50]);
	 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//-------------- Wybierz funkcje ------------------------------
	 if(i<7) sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"06%c\" style=\"display: none;\"><table><tr><td width=\"50px\"></td><td><font size=\"4\" color=\"white\"  >---&nbsp; Wybierz funkcje&nbsp; ---</font></td></tr></table></div>\r\n",buf_p[50]);
	 else    sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"06%c\" style=\"display: none;\"></div>\r\n",buf_p[50]);
	 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO !!!"); HAL_Delay(3000); return; }
}
*/

void NewPanelID_Dyn(int i)  //main nowy dynamiczny
{
   int itx,j,c,h,h2; char sss[20]; itx=0;  j=0;  int kolejn;

   if(TabPort[i]==5) kolejn=1;
   if(TabPort[i]==6) kolejn=2;
   if(TabPort[i]==7) kolejn=3;
   if(TabPort[i]==8) kolejn=4;
   if(TabPort[i]==9/*17*/) kolejn=5;
   if(TabPort[i]==10/*18*/) kolejn=6;
   if(TabPort[i]==28) kolejn=7;
   if(TabPort[i]==21) kolejn=8;
   if(TabPort[i]==22/*15*/) kolejn=9;
   if(TabPort[i]==23) kolejn=10;
   if(TabPort[i]==24) kolejn=11;
   if(TabPort[i]==33) kolejn=12;

   //if(i<9) sprintf(sss,"&nbsp;&nbsp;%d",TabPort[i]); else sprintf(sss,"%d",TabPort[i]);
   if(kolejn<10) sprintf(sss,"&nbsp;&nbsp;%d",kolejn); else sprintf(sss,"%d",kolejn);



  if((wybor_bitowy&0x0400)>0){
   //  pokaz_ADC(i);
   //  ServiceWE(i);
    // pokaz_temp(i+1);
  }

//----------------------  PK  --------------
   j= WyszukujPortElement(t_wy,_Size_wy,TabPort[i]);
   if(j!=-1)
   {
	    if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0)
	    {
	    	sprintf(&buf_p[0],Const.s_PK[j].nazwa_on);
	 		sprintf(&buf_p[30],"fff");
	    }
	    else{
	    	sprintf(&buf_p[0],Const.s_PK[j].nazwa_of);
	 		sprintf(&buf_p[30],"000");
	    }
	   if(Const.s_GPIO[TabPort[i]-1].val==8) sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"00p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='wypk%03d' class=\"shiny-button\">%s</a></td><td width=\"1px\"></td><td><a href=\"#\" rel=\"external\" onClick=\"xpk%03d(); this.href='javascript:return false;';\" class=\"c\"><div id=\"uu_pk%03d\">",TabPort[i],j+1,sss,TabPort[i],TabPort[i],TabPort[i],TabPort[i]);
	   else                                  sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"00p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='wypk%03d' class=\"shiny-button\">%s</a></td><td width=\"1px\"></td><td><a href=\"#\" rel=\"external\" onClick=\"xpk%03d(); this.href='javascript:return false;';\" class=\"c\"><div id=\"tu_pk%03d\">",TabPort[i],j+1,sss,TabPort[i],TabPort[i],TabPort[i],TabPort[i]);
       itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
       sprintf(&buf_rx2[_Size_BufWifi+itx],"<font color=\"#%s\">%s</font></div></a></td><td width=\"1px\"></td><td><div id=\"tu_lora%03dwy\"></div></td><td width=\"10px\"></td><td></td></tr></table></div>",&buf_p[30],&buf_p[0],TabPort[i]);
   }
   else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"00p%03d\" style=\"display: none;\"></div>",TabPort[i]);
   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


//----------------------  Temp  --------------
   j=WyszukujPortElement(t_te,_Size_te,TabPort[i]);
   if(j!=-1)
   {
	   h=WyszukajNrCzujkiDlaPortu(t_te[j]);
	   h2=WyszukajNrCzujkiDlaPortu_2(t_te[j]);
	   if((h!=-1)&&(h2==-1))
	   {
		   wartTemp_odwrotnie(&buf_p[100],Var.value[h]);   if(buf_p[100+1]=='0') buf_p[100+1]=' ';
		   h2=KolorTemp(h);
		        if(h2=='1') sprintf(buf_p,"<font color='#d00'> %c %s °C</font>",buf_p[100],&buf_p[101]);
		   else if(h2=='0') sprintf(buf_p,"<font color='#000'> %c %s °C</font>",buf_p[100],&buf_p[101]);
		   else if(h2=='2') sprintf(buf_p,"<font color='#00d'> %c %s °C</font>",buf_p[100],&buf_p[101]);
	   }
	   else sprintf(buf_p," --.-");
       sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"02p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='temp%03d' class=\"shiny-button\">%s</a></td><td width=\"10px\"></td><td><button class=\"m\"><div id=\"tu_temp%03da\">%s</div></button></td><td width=\"1px\"></td><td><div id=\"tu_lora%03da\"></div></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],TabPort[i],sss,TabPort[i],buf_p,TabPort[i]);
   }
   else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"02p%03d\" style=\"display: none;\"></div>",TabPort[i]);
   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


//----------------------  Temp i Wilg  --------------
    j=WyszukujPortElement(t_te,_Size_te,TabPort[i]);
    if(j!=-1)
    {
       h2=WyszukajNrCzujkiDlaPortu_2(t_te[j]);
       h=WyszukajNrCzujkiDlaPortu(t_te[j]);
 	   if((h!=-1)&&(h2!=-1))
 	   {
 		   wartTemp_odwrotnie(&buf_p[1],  Var.value[h]);    buf_p[0]=buf_p[1]; buf_p[1]=' '; if(buf_p[2]=='0') buf_p[2]=' ';
		   h2=KolorTemp(h);
		        if(h2=='1') sprintf(&buf_p[100],"<font color='#d00'> %s °C</font>",buf_p);
		   else if(h2=='0') sprintf(&buf_p[100],"<font color='#000'> %s °C</font>",buf_p);
		   else if(h2=='2') sprintf(&buf_p[100],"<font color='#00d'> %s °C</font>",buf_p);

		   h=WyszukajNrCzujkiDlaPortu_2(t_te[j]);
	 	   if(h!=-1)
	 	   {
 		       wartTemp_odwrotnie(&buf_p[30], Var.value[h]);      buf_p[30+0]=' '; buf_p[30+3]=0;   if(buf_p[30+1]=='0') buf_p[30+1]=' ';
		       h2=KolorTemp(h);
		            if(h2=='1') sprintf(&buf_p[50],"<font color='#d00'> %s %%%%</font>",&buf_p[30]);
		       else if(h2=='0') sprintf(&buf_p[50],"<font color='#000'> %s %%%%</font>",&buf_p[30]);
		       else if(h2=='2') sprintf(&buf_p[50],"<font color='#00d'> %s %%%%</font>",&buf_p[30]);
	 	   }
 	   }
 	   else
 	   {
 	       sprintf(&buf_p[100]," --.-");
 		   sprintf(&buf_p[50]," -- ");
 	   }
       sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"09p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='temp%03d' class=\"shiny-button\">%s</a></td><td width=\"10px\"></td><td><button class=\"m\"><div id=\"tu_temp%03db\">%s</div></button></td><td width=\"10px\"></td><td><button class=\"m\"> <div id=\"tu_temp%03dc\">%s</div></button></td><td width=\"1px\"></td><td><div id=\"tu_lora%03db\"></div></td><td width=\"10px\"></td></tr></table></div>",TabPort[i],TabPort[i],sss,TabPort[i],&buf_p[100], TabPort[i], &buf_p[50], TabPort[i]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"09p%03d\" style=\"display: none;\"></div>",TabPort[i]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


//----------------------  Timer --------------
    j= WyszukujPortElement(t_wy,_Size_wy,TabPort[i]);
    if(j!=-1)
    {
	    if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0)
	    {
	    	sprintf(&buf_p[0],Const.s_PK[j].nazwa_on);
	 		sprintf(&buf_p[30],"fff");
	    }
	    else{
	    	sprintf(&buf_p[0],Const.s_PK[j].nazwa_of);
	 		sprintf(&buf_p[30],"000");
	    }

        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"10p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='tim%03d' class=\"shiny-button\">%s</a></td><td width=\"1px\"></td><td><a class=\"c\"><div id=\"tu_ti%03d\"><font color=\"#%s\">%s</font></div></a></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],j+1,sss,TabPort[i],&buf_p[30],&buf_p[0]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"10p%03d\" style=\"display: none;\"></div>",TabPort[i]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


//----------------------  Termostat --------------
    j= WyszukujPortElement(t_wy,_Size_wy,TabPort[i]);
    if(j!=-1)
    {
	    if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0)
	    {
	    	sprintf(&buf_p[0],Const.s_PK[j].nazwa_on);
	 		sprintf(&buf_p[30],"fff");
	    }
	    else{
	    	sprintf(&buf_p[0],Const.s_PK[j].nazwa_of);
	 		sprintf(&buf_p[30],"000");
	    }

        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"11p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='term%03d' class=\"shiny-button\">%s</a></td><td width=\"1px\"></td><td><a class=\"c\"><div id=\"tu_te%03d\"><font color=\"#%s\">%s</font></div></a></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],j+1,sss,TabPort[i],&buf_p[30],&buf_p[0]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"11p%03d\" style=\"display: none;\"></div>",TabPort[i]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


 //----------------------  Sterowanie IR --------------
    j= WyszukujPortElement(t_ir,_Size_ir,TabPort[i]);
    if(j!=-1)
    {  j= WyszukujPortElement(t_wy,_Size_wy,TabPort[i]);
       if(j!=-1)
       {
 	       if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0)
 	       {
 	    	 sprintf(&buf_p[0],Const.s_PK[j].nazwa_on);
 	 		 sprintf(&buf_p[30],"fff");
 	       }
 	       else{
 	    	 sprintf(&buf_p[0],Const.s_PK[j].nazwa_of);
 	 		 sprintf(&buf_p[30],"000");
 	       }
           sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"08p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='wypk%03d' class=\"shiny-button\">%s</a></td><td width=\"1px\"></td><td><a href=\"#\" rel=\"external\" onClick=\"xpk%03d(); this.href='javascript:return false;';\" class=\"c\"><div id=\"tu_pk%03d\">",TabPort[i],j+1,sss,TabPort[i],TabPort[i],TabPort[i],TabPort[i]);
           itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
           sprintf(&buf_rx2[_Size_BufWifi+itx],"<font color=\"#%s\">%s</font></div></a></td><td width=\"10px\"></td><td></td></tr></table></div>",&buf_p[30],&buf_p[0]);
       }
       else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"08p%03d\" style=\"display: none;\"></div>",TabPort[i]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"08p%03d\" style=\"display: none;\"></div>",TabPort[i]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
        itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);



//------------------- WE ------------------------------

    j= WyszukujPortElement(t_we,_Size_we,TabPort[i]);
    if(j!=-1)
    {

        if((Var.we[j/16]&(1<<(j-16*(j/16))))>0) sprintf(buf_p,"<font color='#000'>%s</font>",Const.s_WE[j].nazwa_ro);
        else                                    sprintf(buf_p,"<font color='#d00'>%s</font>",Const.s_WE[j].nazwa_zw);
        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"04p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='we%03d' class=\"shiny-button\">%s</a></td><td width=\"1px\"></td><td><button class=\"m\"><div id=\"tu_we%03d\">%s</div></button></td><td width=\"1px\"></td><td><div id=\"tu_lora%03dwe\"></div></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],j+1,sss,TabPort[i],buf_p,TabPort[i]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"04p%03d\" style=\"display: none;\"></div>",TabPort[i]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);



    //------------------- WE EOL------------------------------

        j= WyszukujPortElement(t_eol,_Size_we,TabPort[i]);
        if(j!=-1)
        {

            if((Var.we[j/16]&(1<<(j-16*(j/16))))>0) sprintf(buf_p,"<font color='#000'>%s</font>",Const.s_WE[j].nazwa_ro);
            else                                    sprintf(buf_p,"<font color='#d00'>%s</font>",Const.s_WE[j].nazwa_zw);
            sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"13p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='eol%03d' class=\"shiny-button\">%s</a></td><td width=\"1px\"></td><td><button class=\"m\"><div id=\"tu_eol%03d\">%s</div></button></td><td width=\"1px\"></td><td><div id=\"tu_lora%03deol\"></div></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],j+1,sss,TabPort[i],buf_p,TabPort[i]);
        }
        else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"13p%03d\" style=\"display: none;\"></div>",TabPort[i]);
        itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//------------------- PWM ------------------------------

    j= WyszukujPortElement(t_pwm,_Size_pwm,TabPort[i]);
    if(j!=-1)
    {
   	      sprintf(buf_p,"%d",Const.s_PWM[j].duty);
   	      sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"01p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='3main' class=\"shiny-button\">%s</a></td><td width=\"12px\"></td><td width=\"42px\"><output  class=\"v\" id=\"volume%03d\">%s</output></td><td><input type=\"range\" name=\"n_pwm%d\"  min=\"0\" max=\"100\"  step=\"1\"  value=\"%s\"   oninput=\"outputUpdate%03d(value)\" onchange=\"x%03d(this.value);\"></input></td></tr></table></div>",TabPort[i],sss,TabPort[i],buf_p,j+1,buf_p,TabPort[i],TabPort[i],TabPort[i]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"01p%03d\" style=\"display: none;\"></div>",TabPort[i]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

//------------------- ADC ------------------------------

    j= WyszukujPortElement(t_TT,_Size_TT,TabPort[i]);
    if(j!=-1)
    {
 	    //sprintf(buf_p,"10.0 V");
 	    ZmianaTTnaTME(j,buf_p);
 	  	sprintf(&buf_p[20],"%s %s",buf_p,Const.s_TT[j].jm);
        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"05p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='nap%03d' class=\"shiny-button\">%s</a></td><td width=\"10px\"></td><td><button class=\"m\"> <div id=\"tu_adc%03d\">%s</div></button></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],j+1,sss,TabPort[i],&buf_p[20]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"05p%03d\" style=\"display: none;\"></div>",TabPort[i]);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 //------------------- Pt1000 ------------------------------

    j= WyszukujPortElement(t_pt,_Size_pt,TabPort[i]);
    if(j!=-1)
    {
 	    h=WyszukajNrCzujkiDlaPortu(t_pt[j]);
 	    if(h!=-1)
 	    {
 	      if(Var.value[h]==0x7FFF) sprintf(&buf_p[50]," --- °C");
 	      else
 	      {
 	    	 if(Var.value[h]<0)
 	    	 {
 	    		 buf_p[0]='-';
 	    		 sprintf(&buf_p[1],"%d °C",Var.value[h]);   buf_p[1]=' ';
 	    	 }
 	    	 else
 	    	 {
 	    		 buf_p[0]='+';
 	    		 sprintf(&buf_p[2],"%d °C",Var.value[h]);   buf_p[1]=' ';
 	    	 }
 	    	 h2=KolorTemp(h);
		          if(h2=='1') sprintf(&buf_p[50],"<font color='#d00'> %s </font>",buf_p);
		     else if(h2=='0') sprintf(&buf_p[50],"<font color='#000'> %s </font>",buf_p);
		     else if(h2=='2') sprintf(&buf_p[50],"<font color='#00d'> %s </font>",buf_p);
 	      }
 	    }
 	    else sprintf(&buf_p[50]," --.-");
	    sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"03p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='temp%03d' class=\"shiny-button\">%s</a></td><td width=\"10px\"></td><td><button class=\"m\"> <div id=\"tu_ptt%03d\">%s</div></button></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],TabPort[i],sss,TabPort[i],&buf_p[50]);
    }
    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"03p%03d\" style=\"display: none;\"></div>",TabPort[i]);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


//------------------- Impuls ------------------------------

	    j= WyszukujPortElement(t_imp,_Size_imp,TabPort[i]);
	    if(j!=-1)
	    {
	    	PrzeliczLicznik_wstep(j,0,&buf_p[50]);
	    	//sprintf(&buf_p[50],"%d %s",Const.s_Imp[j].value[0], Const.s_Imp[j].jedn);
	        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"07p%03d\" style=\"display: none;\"><table><tr><td width=\"0px\"></td><td width=\"20px\"><a href='etr%03d' class=\"shiny-button\">%s</a></td><td width=\"10px\"></td><td><button class=\"m\"> <div id=\"tu_imp0_%03d\">%s</div></button></td><td width=\"10px\"></td><td></td></tr></table></div>",TabPort[i],j+1,sss,TabPort[i],&buf_p[50]);
	    }
	    else sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"07p%03d\" style=\"display: none;\"></div>",TabPort[i]);
		itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


//-------------- Wybierz funkcje ------------------------------
	  sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div id=\"06p%03d\" style=\"display: none;\"><table><tr><td width=\"50px\"></td><td><font size=\"4\" color=\"white\"  >---&nbsp; Wybierz funkcje&nbsp; ---</font></td></tr></table></div>\r\n",TabPort[i]);

	 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	 if(itx>4000) dbg3("\r\nPRZEKROCZONO itx !!!");  sprintf(buf_p,"\r\nItx: %d   %d  ",itx,WhichFragmentPage); dbg3(buf_p);
}


int MainPanel_temp(int *point)  //main
{
  int itx,i,j,h2;  itx=0;  buf_rx2[_Size_BufWifi]=0;

  for(i=*point ;i<ddo; i++)
  {
    if(WyszukujPortElement(t_te,_Size_te,TabPort[i])!=-1)
    {
	  j=WyszukajNrCzujkiDlaPortu(TabPort[i]);
	  if(j!=-1)
	  {
		  if(Const.s_GPIO[TabPort[i]-1].val==9)  //DHT
		  {
			  if((itx+500)>=4000){ *point=i; return itx; }

			   wartTemp_odwrotnie(&buf_p[1],Var.value[j]); buf_p[0]=buf_p[1]; buf_p[1]=' '; if(buf_p[2]=='0') buf_p[2]=' ';
			   h2=KolorTemp(j);
			        if(h2=='1') sprintf(&buf_p[100],"<font color='#f88'> %s °C</font>",buf_p);
			   else if(h2=='0') sprintf(&buf_p[100],"<font color='#eee'> %s °C</font>",buf_p);
			   else if(h2=='2') sprintf(&buf_p[100],"<font color='#8cf'> %s °C</font>",buf_p);
			   sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\" style=\"height:300px\"><div class=\"x\" style=\"height:270px\"><font class=\"a\"><b><div id=\"tu_temp%03db\">%s</div></b></font><br><font class=\"b\">%s</font><div id=\"tu_lora%03dc\"></div><br>\r\n",TabPort[i],&buf_p[100],Const.s_Czujki[j].nazwa,TabPort[i]);
			  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			   if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

			  j=WyszukajNrCzujkiDlaPortu_2(TabPort[i]);
			  if(j!=-1)
		      {
				   wartTemp_odwrotnie(buf_p,Var.value[j]); buf_p[0]=' '; buf_p[3]=' ';buf_p[4]=' '; if(buf_p[1]=='0') buf_p[1]=' ';
				   h2=KolorTemp(j);
				        if(h2=='1') sprintf(&buf_p[100],"<font color='#f88'> %s %%%%</font>",buf_p);
				   else if(h2=='0') sprintf(&buf_p[100],"<font color='#eee'> %s %%%%</font>",buf_p);
				   else if(h2=='2') sprintf(&buf_p[100],"<font color='#8cf'> %s %%%%</font>",buf_p);

				   sprintf(&buf_rx2[_Size_BufWifi+itx],"<font class=\"a\"><b><div id=\"tu_temp%03dc\">%s</div></b></font><br><font class=\"b\">%s</font><div id=\"tu_lora%03db\"></div></div></div>\r\n",TabPort[i],&buf_p[100],Const.s_Czujki[j].nazwa,TabPort[i]);
				   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
				   if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

					  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03da\" style=\"display: none;\"></div><div id=\"tu_lora%03da\"></div>",TabPort[i],TabPort[i]);
					  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
					  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

		      }

		  }
		  else if(Const.s_GPIO[TabPort[i]-1].val==2)  //DS
		  {
			  if((itx+500)>=4000){ *point=i; return itx; }

			   wartTemp_odwrotnie(&buf_p[1],Var.value[j]); buf_p[0]=buf_p[1]; buf_p[1]=' '; if(buf_p[2]=='0') buf_p[2]=' ';
			   h2=KolorTemp(j);
			        if(h2=='1') sprintf(&buf_p[100],"<font color='#f88'> %s °C</font>",buf_p);
			   else if(h2=='0') sprintf(&buf_p[100],"<font color='#eee'> %s °C</font>",buf_p);
			   else if(h2=='2') sprintf(&buf_p[100],"<font color='#8cf'> %s °C</font>",buf_p);
			  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><font class=\"a\"><b><div id=\"tu_temp%03da\">%s</div></b></font><br><font class=\"b\">%s</font></div></div><div id=\"tu_lora%03da\"></div>\r\n",TabPort[i],&buf_p[100],Const.s_Czujki[j].nazwa,TabPort[i]);
			  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

			  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03db\" style=\"display: none;\"></div><div id=\"tu_lora%03dc\"></div>",TabPort[i],TabPort[i]);
			  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

			  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03dc\" style=\"display: none;\"></div><div id=\"tu_lora%03db\"></div>",TabPort[i],TabPort[i]);
			  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

		  }
		  else if(Const.s_GPIO[TabPort[i]-1].val==3)   //Pt
		  {
			  if((itx+500)>=4000){ *point=i; return itx; }

			  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03da\" style=\"display: none;\"></div><div id=\"tu_lora%03da\"></div>",TabPort[i],TabPort[i]);
			  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

			  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03db\" style=\"display: none;\"></div><div id=\"tu_lora%03dc\"></div>",TabPort[i],TabPort[i]);
			  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

			  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03dc\" style=\"display: none;\"></div><div id=\"tu_lora%03db\"></div>",TabPort[i],TabPort[i]);
			  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

		  }

	  }
	  else
	  {
		  if((itx+500)>=4000){ *point=i; return itx; }

		  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03da\" style=\"display: none;\"></div><div id=\"tu_lora%03da\"></div>",TabPort[i],TabPort[i]);
		  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

		  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03db\" style=\"display: none;\"></div><div id=\"tu_lora%03dc\"></div>",TabPort[i],TabPort[i]);
		  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }

		  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_temp%03dc\" style=\"display: none;\"></div><div id=\"tu_lora%03db\"></div>",TabPort[i],TabPort[i]);
		  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Te !!!"); HAL_Delay(3000); return; }
	  }
    }
  }
  if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }
  return -1;
}

int MainPanel_licznik(int *point)
{
	 int itx,i,j,h2;  itx=0;  buf_rx2[_Size_BufWifi]=0;

	  for(i=odd-1 ;i<ddo; i++)
	  {
	      j=WyszukujPortElement(t_imp,_Size_imp,TabPort[i]);
	      if(j!=-1)
	      {
		      if(Const.s_GPIO[TabPort[i]-1].val==7)  //Licznik impulsów
	          {
		    	  if((itx+900)>=_Size_WskFragmentPage){ *point=i; return itx; }
		    	  PrzeliczLicznikKolejny(j,0,&buf_p[0]);
		    	  PrzeliczLicznikKolejny(j,1,&buf_p[50]);
		    	  PrzeliczLicznikKolejny(j,2,&buf_p[100]);
		    	  PrzeliczLicznikKolejny(j,3,&buf_p[150]);
		    	  PrzeliczLicznikKolejny(j,4,&buf_p[200]);
	              sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\" style=\"height:330px\"><div class=\"x\" style=\"height:300px\"><font class=\"z\" style=\"font-size: 43px;\"><b><div id=\"tu_imp0_%03d\">%s</div></font></b><br><font class=\"z\" style=\"color:#ccc\"><div id=\"tu_imp1_%03d\">%s</div></font><font color=\"#ccc\" size=\"4\"> &nbsp;&nbsp;&nbsp;&nbsp;godzinny</font><br><font class=\"z\" style=\"color:#ccc\"><div id=\"tu_imp2_%03d\">%s</div></font><font color=\"#ccc\" size=\"4\"> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;dobowy</font><br><font class=\"z\" style=\"color:#ccc\"><div id=\"tu_imp3_%03d\">%s</div></font><font color=\"#ccc\" size=\"4\"> &nbsp;tygodniowy</font><br><font class=\"z\" style=\"color:#ccc\"><div id=\"tu_imp4_%03d\">%s</div></font><font color=\"#ccc\" size=\"4\"> &nbsp;&nbsp;miesieczny</font><br><font class=\"b\">%s</font></div></div>\r\n",TabPort[i],&buf_p[0],TabPort[i],&buf_p[50],TabPort[i],&buf_p[100],TabPort[i],&buf_p[150],TabPort[i],&buf_p[200],Const.s_GPIO[TabPort[i]-1].nazwa);
	  	          itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	  	          if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Li !!!"); HAL_Delay(3000); return; }
	          }
	          else
	          {
	        	  if((itx+400)>=_Size_WskFragmentPage){ *point=i; return itx; }
	        	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_imp0_%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	        	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_imp1_%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	        	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_imp2_%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	        	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_imp3_%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	        	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_imp4_%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	    	      if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Li !!!"); HAL_Delay(3000); return; }
	          }
	      }
	  }
	  if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }
	  return -1;
}

void MainPanel_pk()
{
   int itx,i,j;  itx=0;  buf_rx2[_Size_BufWifi]=0;

   for(i=odd-1 ;i<ddo; i++)
   {
      j=WyszukujPortElement(t_wy,_Size_wy,TabPort[i]);
      if(j!=-1)
      {
	      if((Const.s_GPIO[TabPort[i]-1].val==0)||(Const.s_GPIO[TabPort[i]-1].val==8))  // PK  ||  IR
	      {
	    	   if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){
	    	     		sprintf(&buf_p[0],Const.s_PK[j].nazwa_on);
	    	     		sprintf(&buf_p[30],"fff");
	    	   }
	    	   else{
	    	     	    sprintf(&buf_p[0],Const.s_PK[j].nazwa_of);
	    	     		sprintf(&buf_p[30],"000");
	    	   }
	          sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><a href=\"#\" rel=\"external\" onClick=\"xpk%03d(); this.href='javascript:return false;';\" class=\"c\" style=\"font-size: 25px;\"><b><div id=\"tu_pk%03d\"><font color=\"#%s\">%s</font></div></b></a><br><font class=\"b\">%s</font></div></div><div id=\"tu_lora%03dwy\"></div>\r\n",TabPort[i],TabPort[i],&buf_p[30],&buf_p[0],Const.s_PK[j].nazwa,TabPort[i]);
	 	      itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	 	      if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO PK !!!"); HAL_Delay(3000); return; }
	      }
	      else
	      {
	    	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_pk%03d\" style=\"display: none;\"></div><div id=\"tu_lora%03dwy\"></div>\r\n",TabPort[i],TabPort[i]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	    	  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO PK !!!"); HAL_Delay(3000); return; }
	      }
      }
   }
   if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }


}

void MainPanel_poz()  //Timer i Termostat
{
	  int itx,i,j;  itx=0;  buf_rx2[_Size_BufWifi]=0;


	   for(i=odd-1 ;i<ddo; i++)
	   {
	      j=WyszukujPortElement(t_wy,_Size_wy,TabPort[i]);
	      if(j!=-1)
	      {
		      if(Const.s_GPIO[TabPort[i]-1].val==10)  //Timer
		      {
		    	   if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){
		    	     		sprintf(&buf_p[0],Const.s_PK[j].nazwa_on);
		    	     		sprintf(&buf_p[30],"fff");
		    	   }
		    	   else{
		    	     	    sprintf(&buf_p[0],Const.s_PK[j].nazwa_of);
		    	     		sprintf(&buf_p[30],"000");
		    	   }
		    	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><a class=\"c\" style=\"font-size: 25px;\"><b><div id=\"tu_ti0%02d\"><font color=\"#%s\">%s</font></div></b></a><br><font class=\"b\">%s</font></div></div>\r\n",TabPort[i],&buf_p[30],&buf_p[0],Const.s_PK[j].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		    	  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Ti !!!"); HAL_Delay(3000); return; }
		      }
		      else
		      {
		    	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_ti0%02d\" style=\"display: none;\"></div>\r\n",TabPort[i]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		    	  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Ti !!!"); HAL_Delay(3000); return; }
		      }
	      }
	   }


	   for(i=odd-1 ;i<ddo; i++)
	   {
		      j=WyszukujPortElement(t_wy,_Size_wy,TabPort[i]);
		      if(j!=-1)
		      {
			      if(Const.s_GPIO[TabPort[i]-1].val==11)  //Termostat
			      {
			    	   if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){
			    	     		sprintf(&buf_p[0],Const.s_PK[j].nazwa_on);
			    	     		sprintf(&buf_p[30],"fff");
			    	   }
			    	   else{
			    	     	    sprintf(&buf_p[0],Const.s_PK[j].nazwa_of);
			    	     		sprintf(&buf_p[30],"000");
			    	   }
			    	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><a class=\"c\" style=\"font-size: 25px;\"><b><div id=\"tu_te0%02d\"><font color=\"#%s\">%s</font></div></b></a><br><font class=\"b\">%s</font></div></div>\r\n",TabPort[i],&buf_p[30],&buf_p[0],Const.s_PK[j].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			    	  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Term !!!"); HAL_Delay(3000); return; }
			      }
			      else
			      {
			    	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_te0%02d\" style=\"display: none;\"></div>\r\n",TabPort[i]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			    	  if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Term !!!"); HAL_Delay(3000); return; }
			      }
		      }
	   }
	   if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }

	   sprintf(buf_p,"\r\nHHHHH: %d ", itx); dbg3(buf_p);

}

void MainPanel_we()
{
	  int itx,i,j;  itx=0;  buf_rx2[_Size_BufWifi]=0;

	  for(i=odd-1 ;i<ddo; i++)
	  {
	      j=WyszukujPortElement(t_we,_Size_we,TabPort[i]);
	      if(j!=-1)
	      {
		      if(Const.s_GPIO[TabPort[i]-1].val==4)  //WE
	          {
	  	          if((Var.we[j/16]&(1<<(j-16*(j/16))))>0) sprintf(buf_p,"<font color='#eee'>%s</font>",Const.s_WE[j].nazwa_ro);
	  	          else                                    sprintf(buf_p,"<font color='#f88'>%s</font>",Const.s_WE[j].nazwa_zw);
	              sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><font class=\"z\"><b><div id=\"tu_we%03d\">%s</div></b></font><br><font class=\"b\">%s</font></div></div><div id=\"tu_lora%03dwe\"></div>\r\n",TabPort[i],buf_p,Const.s_WE[j].nazwa,TabPort[i]);
	  	          itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	  	          if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO We !!!"); HAL_Delay(3000); return; }
	          }
	          else
	          {
	    	      sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_we%03d\" style=\"display: none;\"></div><div id=\"tu_lora%03dwe\"></div>\r\n",TabPort[i],TabPort[i]);
	    	      itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	    	      if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO We !!!"); HAL_Delay(3000); return; }
	          }
	      }
	  }
	  if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }
}

void MainPanel_eol()
{
	  int itx,i,j;  itx=0;  buf_rx2[_Size_BufWifi]=0;

	  for(i=odd-1 ;i<ddo; i++)
	  {
	      j=WyszukujPortElement(t_eol,_Size_eol,TabPort[i]);
	      if(j!=-1)
	      {
		      if(Const.s_GPIO[TabPort[i]-1].val==13)  //WE EOL
	          {
	  	          if((Var.we[j/16]&(1<<(j-16*(j/16))))>0) sprintf(buf_p,"<font color='#eee'>%s</font>",Const.s_WE[j].nazwa_ro);
	  	          else                                    sprintf(buf_p,"<font color='#f88'>%s</font>",Const.s_WE[j].nazwa_zw);
	              sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><font class=\"z\"><b><div id=\"tu_eol%03d\">%s</div></b></font><br><font class=\"b\">%s</font></div></div><div id=\"tu_lora%03deol\"></div>\r\n",TabPort[i],buf_p,Const.s_WE[j].nazwa,TabPort[i]);
	  	          itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	  	          if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO We !!!"); HAL_Delay(3000); return; }
	          }
	          else
	          {
	    	      sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_eol%03d\" style=\"display: none;\"></div><div id=\"tu_lora%03deol\"></div>\r\n",TabPort[i],TabPort[i]);
	    	      itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	    	      if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO We !!!"); HAL_Delay(3000); return; }
	          }
	      }
	  }
	  if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }
}

void MainPanel_adc()
{
	  int itx,i,j,h2;  itx=0;  buf_rx2[_Size_BufWifi]=0;

		for(i=odd-1 ;i<ddo; i++)
		{
			j=WyszukujPortElement(t_TT,_Size_TT,TabPort[i]);
		    if(j!=-1)
		    {
		       if(Const.s_GPIO[TabPort[i]-1].val==5)  //TT
		       {
			  	   IntToChar(Const.s_ADC_TT[j].val, buf_p);
			       sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><font class=\"a\"><b><div id=\"tu_adc%03d\">%s %s</div></b></font><br><font class=\"b\">%s</font></div></div>\r\n",TabPort[i],buf_p, Const.s_TT[j].jm, Const.s_GPIO[TabPort[i]-1].nazwa);
			       itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			       if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO TT !!!"); HAL_Delay(3000); return; }
		       }
			   else
		       {
				   sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_adc%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
				   if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO TT !!!"); HAL_Delay(3000); return; }
			   }
		    }
		}



	for(i=odd-1 ;i<ddo; i++)
	{
      if(WyszukujPortElement(t_pt,_Size_pt,TabPort[i])!=-1)
      {
	     j=WyszukajNrCzujkiDlaPortu(TabPort[i]);
	     if(j!=-1)
	     {
	       if(Const.s_GPIO[TabPort[i]-1].val==3)  //Pt1000
	       {
	    	   if(Var.value[j]==0x7FFF) sprintf(&buf_p[50]," --- °C");
	    	   else
	    	   {
	    		   if((int16_t)Var.value[j]<0){ sprintf(&buf_p[1],"%d °C",(int16_t)Var.value[j]); buf_p[0]='-'; buf_p[1]=' ';  }
	    	       else                         sprintf(buf_p,"+ %d °C",(int16_t)Var.value[j]);

		   	    	 h2=KolorTemp(j);
		  		        if(h2=='1') sprintf(&buf_p[50],"<font color='#f88'> %s </font>",buf_p);
		  		   else if(h2=='0') sprintf(&buf_p[50],"<font color='#eee'> %s </font>",buf_p);
		  		   else if(h2=='2') sprintf(&buf_p[50],"<font color='#8cf'> %s </font>",buf_p);
	    	   }
	    	   sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><font class=\"a\"><b><div id=\"tu_ptt%03d\">%s</div></b></font><br><font class=\"b\">%s</font></div></div>\r\n",TabPort[i],&buf_p[50],Const.s_Czujki[j].nazwa);
	           itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	           if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Pt !!!"); HAL_Delay(3000); return; }
	       }
		   else
	       {
		      sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_ptt%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		      if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Pt !!!"); HAL_Delay(3000); return; }
	       }
	     }
		 else
	     {
		     sprintf(&buf_rx2[_Size_BufWifi+itx],"<div id=\"tu_ptt%03d\" style=\"display: none;\"></div>\r\n",TabPort[i]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		     if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Pt !!!"); HAL_Delay(3000); return; }
	     }
      }
	}
	if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }

}

void MainPanel_pwm()
{
	  int itx,i,j;  itx=0;  buf_rx2[_Size_BufWifi]=0;

	for(i=odd-1 ;i<ddo; i++)
	{
		j=WyszukujPortElement(t_pwm,_Size_pwm,TabPort[i]);
	    if(j!=-1)
	    {
	       if(Const.s_GPIO[TabPort[i]-1].val==1)   //slider PWM
	       {
		        sprintf(buf_p,"%d",Const.s_PWM[j].duty);
		 	    sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"y\"><div class=\"x\"><table><tr><td width=\"100px\"><output  class=\"v\" style=\"font-size:36px;\" id=\"volume%03d\">%s</output></td><td></td><td><input type=\"range\" name=\"n_pwm%d\"  min=\"0\" max=\"100\"  step=\"1\"  value=\"%s\"   oninput=\"outputUpdate%03d(value)\" onchange=\"x%03d(this.value);\"></input></td></tr></table><font class=\"b\">%s</font></div></div>\r\n",TabPort[i],buf_p,j+1,buf_p,TabPort[i],TabPort[i],Const.s_GPIO[TabPort[i]-1].nazwa);
		 	    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 	    if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Pwm !!!"); HAL_Delay(3000); return; }
	       }
		   else
	       {
			   sprintf(&buf_rx2[_Size_BufWifi+itx]," ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 	    if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Pwm !!!"); HAL_Delay(3000); return; }
		   }
	    }
		else
	    {
			sprintf(&buf_rx2[_Size_BufWifi+itx]," ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	 	    if(itx>_Size_WskFragmentPage){ dbg3("\r\nPRZEKROCZONO Pwm !!!"); HAL_Delay(3000); return; }
		}
	}
    if(itx==0){  sprintf(&buf_rx2[_Size_BufWifi+itx],"   ");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }

}




void SearchWifiFunction()
{
  int m;
  			if((wybor_bitowy&0x0001)>0)
  			{
  				ResumptionReceiveDataFromWifi();
  				wybor_bitowy&=~0x0001;
  				j=0;
  				ptr1=&buf_rx2[_Size_BufWifi+3000];
  				powt_cwlap:
  				if(ptr=strstr(ptr1,"+CWLAP:"))
  				{
  					     if(*(ptr+8)=='0') sprintf(&buf_rx2[j*50+0],"OPEN");
  					else if(*(ptr+8)=='1') sprintf(&buf_rx2[j*50+0],"WEP");
  					else if(*(ptr+8)=='2') sprintf(&buf_rx2[j*50+0],"WPA_PSK");
  					else if(*(ptr+8)=='3') sprintf(&buf_rx2[j*50+0],"WPA2_PSK");
  					else if(*(ptr+8)=='4') sprintf(&buf_rx2[j*50+0],"WPA_WPA2_PSK");
  					else if(*(ptr+8)=='5') sprintf(&buf_rx2[j*50+0],"WPA2_Enterpr");

  					i=0; do{  buf_rx2[j*50+22+i]=*(ptr+11+i); i++; }while(*(ptr+11+i)!='"'); buf_rx2[j*50+22+i]=0;  m=i;
  					k=11+i+2;
  					i=0; do{  buf_rx2[j*50+14+i]=*(ptr+k+i); i++; }while(*(ptr+k+i)!=',');  buf_rx2[j*50+14+i]=0;

  					//k=k+i+1;
  					  //i=0; do{  buf_rx2[j*40+19+i]=*(ptr+k+i); i++; }while(*(ptr+k+i)!=')');  buf_rx2[j*40+19+i]=0;
  					k=k+i+1+20;
  					i=0; do{  buf_rx2[j*50+19+i]=*(ptr+k+i); i++; }while(*(ptr+k+i)!=',');  buf_rx2[j*50+19+i]=0;

  					ptr1=ptr+10;
  					 j++;
  					 goto powt_cwlap;
  				}
  			}
  			else{ for(i=0;i<3000;i++) buf_rx2[i]=0;  }
/*
sprintf(&buf_rx2[_Size_BufWifi],"<!DOCTYPE html PUBLIC \"-//WAPFORUM//DTD XHTML Mobile 1.2//EN\"\"http://www.openmobilealliance.org/tech/DTD/xhtml-mobile12.dtd\"><html><head><link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAABGdBTUEAALGPC/xhBQAAACRJREFUOE9jZGj4z0AaAGogCZGmGuQckowf1UBkcI0GKxEBBQA55GigBl4jdAAAAABJRU5ErkJggg==\"><meta http-equiv=\"refresh\" content=\"10\"></head><body><br><font size=\"5\" color=\"blue\">Przeszukuje ...<br><br></font>\
<table><tr><td width=\"100px\"><b>Encrypt</b></td><td width=\"70px\"><b>RSSI</b></td><td width=\"70px\"><b>Channel</b></td><td width=\"300px\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>SSID</b></td></tr>\
<tr><td width=\"100px\">--------------------</td><td width=\"70px\">---------</td><td width=\"70px\">---------</td><td width=\"300px\"> --------------------------------</tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>\
</table><br> <a href=\"/net\"; return false\"><button type=\"button\">Wroc</button></a> </body></html>"\
,&buf_rx2[0*50],&buf_rx2[0*50+14],&buf_rx2[0*50+19],&buf_rx2[0*50+22]\
,&buf_rx2[1*50],&buf_rx2[1*50+14],&buf_rx2[1*50+19],&buf_rx2[1*50+22]\
,&buf_rx2[2*50],&buf_rx2[2*50+14],&buf_rx2[2*50+19],&buf_rx2[2*50+22]\
,&buf_rx2[3*50],&buf_rx2[3*50+14],&buf_rx2[3*50+19],&buf_rx2[3*50+22]\
,&buf_rx2[4*50],&buf_rx2[4*50+14],&buf_rx2[4*50+19],&buf_rx2[4*50+22]\
,&buf_rx2[5*50],&buf_rx2[5*50+14],&buf_rx2[5*50+19],&buf_rx2[5*50+22]\
,&buf_rx2[6*50],&buf_rx2[6*50+14],&buf_rx2[6*50+19],&buf_rx2[6*50+22]\
,&buf_rx2[7*50],&buf_rx2[7*50+14],&buf_rx2[7*50+19],&buf_rx2[7*50+22]\
,&buf_rx2[8*50],&buf_rx2[8*50+14],&buf_rx2[8*50+19],&buf_rx2[8*50+22]\
,&buf_rx2[9*50],&buf_rx2[9*50+14],&buf_rx2[9*50+19],&buf_rx2[9*50+22]\
,&buf_rx2[10*50],&buf_rx2[10*50+14],&buf_rx2[10*50+19],&buf_rx2[10*50+22]\
,&buf_rx2[11*50],&buf_rx2[11*50+14],&buf_rx2[11*50+19],&buf_rx2[11*50+22]\
,&buf_rx2[12*50],&buf_rx2[12*50+14],&buf_rx2[12*50+19],&buf_rx2[12*50+22]\
,&buf_rx2[13*50],&buf_rx2[13*50+14],&buf_rx2[13*50+19],&buf_rx2[13*50+22]\
,&buf_rx2[14*50],&buf_rx2[14*50+14],&buf_rx2[14*50+19],&buf_rx2[14*50+22] );

}*/

sprintf(&buf_rx2[_Size_BufWifi],"<!DOCTYPE html PUBLIC \"-//WAPFORUM//DTD XHTML Mobile 1.2//EN\"\"http://www.openmobilealliance.org/tech/DTD/xhtml-mobile12.dtd\"><html><head><link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAABGdBTUEAALGPC/xhBQAAACRJREFUOE9jZGj4z0AaAGogCZGmGuQckowf1UBkcI0GKxEBBQA55GigBl4jdAAAAABJRU5ErkJggg==\"><meta http-equiv=\"refresh\" content=\"10\"></head><body><br><font size=\"5\" color=\"blue\">Przeszukuje ...<br><br></font>\
<table><tr><td width=\"100px\"><b>Encrypt</b></td><td width=\"70px\"><b>RSSI</b></td><td width=\"70px\"><b>Channel</b></td><td width=\"300px\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>SSID</b></td></tr>\
<tr><td width=\"100px\">--------------------</td><td width=\"70px\">---------</td><td width=\"70px\">---------</td><td width=\"300px\"> --------------------------------</tr>");

for(i=0;i<15;i++)
{
   sprintf(&buf_rx2[   _Size_BufWifi + strlen(&buf_rx2[_Size_BufWifi])   ],"\
<tr><td width=\"170px\">%s</td><td width=\"60px\">%s</td><td width=\"70px\">%s</td><td width=\"300px\">%s</td></tr>"\
,&buf_rx2[i*50],&buf_rx2[i*50+14],&buf_rx2[i*50+19],&buf_rx2[i*50+22] );
}

sprintf(&buf_rx2[   _Size_BufWifi + strlen(&buf_rx2[_Size_BufWifi])   ],"</table><br> <a href=\"/wifi\"; return false\"><button type=\"button\">Wroc</button></a> </body></html>");

}

void show_hex(char *ptr)
{
	unsigned char bg[40];
	 sprintf(bg,"\r\nhh:%02x %02x %02x %02x %02x %02x %02x %02x",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);
	                                         dbg3(bg);
	     sprintf(bg,"%02x %02x %02x %02x %02x %02x %02x %02x  ",ptr[8],ptr[9],ptr[10],ptr[11],ptr[12],ptr[13],ptr[14],ptr[15]);
	                                         dbg3(bg);
}

//int Polacz_z_IP(char *ip)
//{
//  int i;
//	  ResumptionReceiveDataFromWifi();
//	  sprintf(buf_p,"AT+CIPSTART=0,\"TCP\",\"%s\",80\x0D\x0A",ip);   SendDataToModemWifi(buf_p);
//	  i=0; while(strstr(buf_rx2,"\r\nOK")==0){ HAL_Delay(5); i++; if(i>1000) return 1; }  dbg3("\r\n99");
//	  return 0;
//}

int Polacz_z_IP(char *ip)
{
  int i;

  if(ptr=strstr(buf_rx2,"\r\n+IPD,")) port_cload_p= *(ptr+7)&0x0f;
       if(port_cload_p==3) port_cload=4;
  else if(port_cload_p==4) port_cload=3;
  else                    port_cload=4;
  sprintf(buf_p,"AT+CIPSTART=%d,\"TCP\",\"%s\",80\x0D\x0A",port_cload,ip);

	  ResumptionReceiveDataFromWifi();
	   SendDataToModemWifi(buf_p);
	  i=0; while(strstr(buf_rx2,"\r\nOK")==0){ HAL_Delay(10); i++; if(i>500){  dbg3("\r\nXXXX"); return 1; } }  dbg3("\r\n99");
	  return 0;
}

//int ZakonczPolaczenieTCP()
//{
//	sprintf(buf_p,"AT+CIPCLOSE=%c\x0D\x0A",HttpPort); dbg3("   "); dbg3(buf_p);
//	SendDataToModemWifi(buf_p);
//}

int ZakonczPolaczenieTCP()
{
	sprintf(buf_p,"AT+CIPCLOSE=%d\x0D\x0A",port_cload); dbg3("   "); dbg3(buf_p);
	SendDataToModemWifi(buf_p);
}

int WyslijKomend(char *cmd)
{
	  sprintf(buf_p,"AT+CIPSEND=%d,%d\x0D\x0A",port_cload,strlen(cmd));   dbg3("\r\n"); dbg3(buf_p);  SendDataToModemWifi(buf_p);  dbg3("\r\n00");
	  i=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); i++; if(i>300) return 1;}  dbg3("\r\n55");
	  ResumptionReceiveDataFromWifi();
	  SendHttpData(cmd,strlen(cmd));   dbg3(" **");
	  i=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); i++; if(i>3000){ dbg3(" koniec"); return 1; } }  dbg3("  AAAA ");
	  return 0;
}

int WyslijDane(char *data, int size)
{
	int i,j, pakiet;     pakiet=2000;  char md[17];

	  for(j=0;j<(size/pakiet);j++)
	  {
		 sprintf(buf_p,"AT+CIPSEND=0,%d\x0D\x0A",pakiet);  SendDataToModemWifi(buf_p);
		 i=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); i++; if(i>300) return 1;}
		 ResumptionReceiveDataFromWifi();
		 SendHttpData(data+(pakiet*j),pakiet);
		 i=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); i++; if(i>300) return 1; }
		 HAL_Delay(10);  dbg3("i");
	  }
	  if(size==(j*pakiet));
	  else
	  {
		     sprintf(buf_p,"AT+CIPSEND=0,%d\x0D\x0A",size-(j*pakiet));  SendDataToModemWifi(buf_p);
	         i=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); i++; if(i>300) return 1;}
		     ResumptionReceiveDataFromWifi();
		     SendHttpData(data+(pakiet*j),size-(j*pakiet));
		     i=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); i++; if(i>300) return 1; }  dbg3("i");
	  }

	  hash_MD5(data,size,md);
	     sprintf(buf_p,"AT+CIPSEND=0,16\x0D\x0A");  SendDataToModemWifi(buf_p);
         i=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); i++; if(i>300) return 1;}
	     ResumptionReceiveDataFromWifi();
	     SendHttpData(md,16);
	     i=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); i++; if(i>300) return 1; }  dbg3("i");

      return 0;
}


int Wyslij_email_ENC()
{
	int i;
	NRF_Start();
	if(((Const.s_Email[0].ip[0])!=0)&&((Const.s_Email[0].nad[0])!=0)&&((Const.s_Email[0].port[0])!=0))
	{ if(((Const.s_Email[0].odb1[0])!=0)||((Const.s_Email[0].odb2[0])!=0)||((Const.s_Email[0].odb3[0])!=0)||((Const.s_Email[0].odb4[0])!=0)||((Const.s_Email[0].odb5[0])!=0)||((Const.s_Email[0].odb6[0])!=0)||((Const.s_Email[0].odb7[0])!=0)||((Const.s_Email[0].odb8[0])!=0))
	  {
	      email_enc=1;  delay_funkc[13]=1;  info_email[0]=0;

#ifdef _LCD_TFT
	DrawInfoE("Wysylam E-mail");
#endif

	      PORT_MAIL_S++;
	      for(i=0;i<12;i++) Operation_DATA_SRAM[_Net_stack_start+0x36+i]=0;
	      if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')){ adr_flash_SesjaSMTP=0; debug_smtp=1; AT25SF_3_BlockErase(_Sesja_SMTP_Start);  HAL_Delay(200);  }
	       tcpSend11_2( IpMyConfig.ip, zamiana_txt_na_iplong_v2(Const.s_Email[0].ip), PORT_MAIL_S, wybierz_cyfry(Const.s_Email[0].port), 12, &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN] , 0 , 1, 40000);
	       debug_smtp=0;
	      dbg3("\r\nmail\r\n");  ekran_tymczasowy=3;
	      return 1;
	  }
	  else return 0;
	}
	else return 0;
	NRF_Stop();
}

int Wyslij_email_ESP()
{
    if((Const.s_Email[0].ip[0]!=0)&&(Const.s_Email[0].nad[0]!=0))
    { if(((Const.s_Email[0].odb1[0])!=0)||((Const.s_Email[0].odb2[0])!=0)||((Const.s_Email[0].odb3[0])!=0)||((Const.s_Email[0].odb4[0])!=0)||((Const.s_Email[0].odb5[0])!=0)||((Const.s_Email[0].odb6[0])!=0)||((Const.s_Email[0].odb7[0])!=0)||((Const.s_Email[0].odb8[0])!=0))
      {
         email=1;  delay_funkc[12]=1;  info_email[0]=0;

#ifdef _LCD_TFT
	DrawInfoE("Wysylam E-mail");
#endif

         dbg3("\r\nmail\r\n"); ekran_tymczasowy=3;
         if(ptr=strstr(buf_rx2,"\r\n+IPD,")) port_smtp_p= *(ptr+7)&0x0f;
              if(port_smtp_p==3) port_smtp=4;
         else if(port_smtp_p==4) port_smtp=3;
         else                    port_smtp=4;
        /* if((wybor_bitowy&0x2000)>0) sprintf(buf_p,"AT+CIPSTART=4,\"TCP\",\"217.74.64.236\",%s\x0D\x0A",Const.s_Email[0].port);
         else  */                      sprintf(buf_p,"AT+CIPSTART=%d,\"TCP\",\"%s\",%s\x0D\x0A",port_smtp,Const.s_Email[0].ip,Const.s_Email[0].port);
           ResumptionReceiveDataFromWifi();
           if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')){  debug_smtp_esp=1; adr_flash_SesjaSMTP=0;  AT25SF_3_BlockErase(_Sesja_SMTP_Start);  HAL_Delay(200);  }
           SendDataToModemWifi(buf_p); dbg3(buf_p);  HAL_Delay(1000); //dbg3(buf_rx2);
           if((wybor_bitowy&0x2000)>0) delay_funkc[30]=1;   //jezeli esp powtorka
         return 1;
      }
	  else return 0;
    }
	else return 0;
}

int Wyslij_email_GPRS()
{
    ResumptionReceiveDataFromGSM();
    	    sprintf(buf_p,"AT+MIPCALL=1,\"%s\",\"%s\",\"%s\"\x0D",Const.s_GPRS[0].apn, Const.s_GPRS[0].usr, Const.s_GPRS[0].has);
    	    dbg(buf_p);     delay_funkc[26]=1; idx_email_GPRS=1;   ekran_tymczasowy=3;
    	    dbg3("\r\nGPRS");  info_email[0]=0;
            #ifdef _LCD_TFT
	           DrawInfoE("Wysylam E-mail");
            #endif
}


int wyslij_email()  //mail
{
	  sprintf(email_tytul,"Zdarzenie");
	  if((Const.s_Lan[0].param&0x10)==0){   Wyslij_email_ENC();  }
	  else                                   delay_funkc[18]=1;  //przez ESP
}

int wyslij_email_raport()  //mail
{
	  sprintf(email_tytul,"Raport");
	  if((Const.s_Lan[0].param&0x10)==0){   Wyslij_email_ENC();  }
	  else                                   delay_funkc[18]=1;  //przez ESP
}

int OdbierzDane(char *pp)
{
	int i,j,go,k;  char *ptr,*ptr1;  char md[17];
	 go=0;
	     ptr=pp;
   		 for(j=0;j<20;j++)
   		 {
   			if(ptr1=strstr(ptr,"\r\n+IPD,"))
   			{
   				while(*ptr1!=',') ptr1++;  ptr1++;
   				while(*ptr1!=',') ptr1++;  ptr1++;
   				i=0; while(*ptr1!=':'){ buf_p[i++]=*ptr1;  ptr1++;}  ptr1++;  buf_p[i]=0;
   				k=liczba_char_na_int();
   				sprintf(buf_p,"\r\nS: %d ",k ); dbg3(buf_p);
   				for(i=0;i<k;i++) buf_rx2[go+i]= *(ptr1+i);
   				go+=k;
                ptr=ptr1;
   			}
   			else break;
   		 }
   		 buf_rx2[go]=0;

   		hash_MD5(&buf_rx2[12],go-12-16,md);
   		k=0;
   		for(i=0;i<16;i++)
   		{
   		   if((buf_rx2[i+go-16])==md[i]);
   		   else k=1;
   		}
   		return k;
}

//int OdbierzDane(char *pp)
//{
//	int i,j,go,k;  char *ptr,*ptr1;  char md[17];
//	 go=0;
//	     ptr=pp;
//   		 for(j=0;j<20;j++)
//   		 {
//   			if(ptr1=strstr(ptr,"\r\n+IPD,"))
//   			{
//   				while(*ptr1!=',') ptr1++;  ptr1++;
//   				while(*ptr1!=',') ptr1++;  ptr1++;
//   				i=0; while(*ptr1!=':'){ buf_p[i++]=*ptr1;  ptr1++;}  ptr1++;  buf_p[i]=0;
//   				k=liczba_char_na_int();
//   				sprintf(buf_p,"\r\nS: %d ",k ); dbg3(buf_p);
//   				for(i=0;i<k;i++) buf_rx2[go+i]= *(ptr1+i);
//   				go+=k;
//                ptr=ptr1;
//   			}
//   			else break;
//   		 }
//   		 buf_rx2[go]=0;
//
//   		hash_MD5(&buf_rx2[12],go-12-16,md);
//   		k=0;
//   		for(i=0;i<16;i++)
//   		{
//   		   if((buf_rx2[i+go-16])==md[i]);
//   		   else k=1;
//   		}
//   		return k;
//}

void TasksRealize(void)   //op
{
	char *pc;
 //################### --  TASK`s   delay_funkc  -- ##########################

  if(delay_funkc[0]==4) //Zapisz parametry LAN
  {
	 test_lora=0;

	  /* ResumptionReceiveDataFromWifi();

   	   sprintf(buf_p,"AT+CIPSTAMAC_CUR=\"  :  :  :  :  :  \"\x0D\x0A");
   	   buf_p[18]=Const.s_Lan[0].eth[0];
   	   buf_p[19]=Const.s_Lan[0].eth[1];
   	   buf_p[21]=Const.s_Lan[0].eth[2];
   	   buf_p[22]=Const.s_Lan[0].eth[3];
   	   buf_p[24]=Const.s_Lan[0].eth[4];
   	   buf_p[25]=Const.s_Lan[0].eth[5];
   	   buf_p[27]=Const.s_Lan[0].eth[6];
   	   buf_p[28]=Const.s_Lan[0].eth[7];
   	   buf_p[30]=Const.s_Lan[0].eth[8];
   	   buf_p[31]=Const.s_Lan[0].eth[9];
   	   buf_p[33]=Const.s_Lan[0].eth[10];
   	   buf_p[34]=Const.s_Lan[0].eth[11];
   	   SendDataToModemWifi(buf_p);

		 dbg3("\r\ndddddd\r\n"); HAL_Delay(500);

	 if((Const.s_Lan[0].param&0x01)>0) licz_htt=7004;
	 else                              licz_htt=7003;*/
	 delay_funkc[0]=0;
  }

  if(delay_funkc[1]==6) //Restart
  {
	  dbg3("\r\nRestart"); HAL_Delay(200);
	 /*   hiwdg.Instance = IWDG;
	    hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
	    hiwdg.Init.Window = 4095;
	    hiwdg.Init.Reload = 4095;
	   HAL_IWDG_Init(&hiwdg);
	   HAL_IWDG_Start(&hiwdg);*/
	  while(1);
  }

  if(delay_funkc[2]==4) //lists AP`s
  {
	 ResumptionReceiveDataFromWifi();
	 SendDataToModemWifi("AT+CWLAP\x0D\x0A");
	 delay_funkc[2]=0;
  }

  if(delay_funkc[3]==4) //send 'AT+CIPSERVER=1'
  {
	 ResumptionReceiveDataFromWifi();
	 sprintf(buf_p,"AT+CIPSERVER=1,%s\x0D\x0A",Const.s_Lan[0].port);
	 SendDataToModemWifi(buf_p);
	 delay_funkc[3]=0;
  }

  if(delay_funkc[4]==20) //send 'AT+CIPSERVER=1' bo nie polaczylem sie z jakims routerem
  {
	  dbg3("\r\nWIFI:  CIFSR ");
	  ResumptionReceiveDataFromWifi();  SendDataToModemWifi("AT+CIFSR\x0D\x0A");
	  licz_htt=5;
	 delay_funkc[4]=0;
  }

  if(delay_funkc[5]==4)
  {


#ifdef _Neoway
      if(nr_kom==1){ nr_kom2++; sprintf(&buf_p[70],"__ATE0");       licz_cpin=0; }
 else if(nr_kom==2){ nr_kom2++;  sprintf(&buf_p[70],"__cpin_pyta");  licz_cpin++; }
 else if(nr_kom==3){ nr_kom2++;  sprintf(&buf_p[70],"__cpin_wpis");}
 else if(nr_kom==4){ nr_kom2++;  sprintf(&buf_p[70],"__creg_pyta");}
 else if(nr_kom==5){ nr_kom2++;  sprintf(&buf_p[70],"__cmgf_wpis");}
 else if(nr_kom==6){ nr_kom2++;  sprintf(&buf_p[70],"__cnmi_wpis");}
 else if(nr_kom==7){ nr_kom2++;  sprintf(&buf_p[70],"__clip_wpis");}
 else if(nr_kom==8){ nr_kom2++;  sprintf(&buf_p[70],"__cops_pyta");}
 else if(nr_kom==9){ nr_kom2++;  sprintf(&buf_p[70],"__cnum");}
 else if(nr_kom==10){ nr_kom2++;  sprintf(&buf_p[70],"__csmp_wpis");}
 else if(nr_kom==11){ nr_kom2++;  sprintf(&buf_p[70],"__cscs");}
 else if(nr_kom==12){ nr_kom2++;  sprintf(&buf_p[70],"__cmgd_1");}
 else if(nr_kom==13){ nr_kom2++;  sprintf(&buf_p[70],"__cmgd_2");}
 else if(nr_kom==14){ nr_kom2++;  sprintf(&buf_p[70],"__cmgd_3");}
 else if(nr_kom==15){ nr_kom2++;  sprintf(&buf_p[70],"__csq");}
#endif


#ifdef _Fibocom
	      if(nr_kom==1){ nr_kom2++; sprintf(&buf_p[70],"__ATE0");       licz_cpin=0; }
	 else if(nr_kom==2){ nr_kom2++;  sprintf(&buf_p[70],"__cpin_pyta");  licz_cpin++; }
	 else if(nr_kom==3){ nr_kom2++;  sprintf(&buf_p[70],"__cpin_wpis");}
	 else if(nr_kom==4){ nr_kom2++;  sprintf(&buf_p[70],"__creg_pyta");}
	 else if(nr_kom==5){ nr_kom2++;  sprintf(&buf_p[70],"__cmgf_wpis");}
	 else if(nr_kom==6){ nr_kom2++;  sprintf(&buf_p[70],"__cnmi_wpis");}
	 else if(nr_kom==7){ nr_kom2++;  sprintf(&buf_p[70],"__clip_wpis");}
	 else if(nr_kom==8){ nr_kom2++;  sprintf(&buf_p[70],"__cops_pyta");}
	 else if(nr_kom==9){ nr_kom2++;  sprintf(&buf_p[70],"__cnum");}
	 else if(nr_kom==10){ nr_kom2++;  sprintf(&buf_p[70],"__csmp_wpis");}
	 else if(nr_kom==11){ nr_kom2++;  sprintf(&buf_p[70],"__cmgd_1");}
	 else if(nr_kom==12){ nr_kom2++;  sprintf(&buf_p[70],"__cmgd_2");}
	 else if(nr_kom==13){ nr_kom2++;  sprintf(&buf_p[70],"__cmgd_3");}
	 else if(nr_kom==14){ nr_kom2++;  sprintf(&buf_p[70],"__cmgd_4");}
	 else if(nr_kom==15){ nr_kom2++;  sprintf(&buf_p[70],"__csq");}
#endif



	  sprintf(buf_p,"\r\nGSM:  Replay %s  ",&buf_p[70]); dbg3(buf_p);
	    if(nr_kom==3)
	    {
		    if(wpis_pin_licz==1)
		    {
		    	delay_funkc[5]=0;
		    	dbg3("\r\nGSM: Pin Error !!! ");
		    	//Start na rozpoczecie pomiarów    //send arp request
		        wybor_bitowy|=0x0400;              delay_funkc[15]=1;
		        wybor_bitowy|=0x0020;  dbg3("\r\nGSM OFF !!!");          licz_brak_Sim=100;
#ifndef _LCD_TFT
    StringFont5x7("GSM: Pin Error",14);NewLineFont5x7(14,0);
#else
    lcd_tft("GSM: Pin Error",ST7735_WHITE);
#endif
		        sprintf(info_gsm,"GSM: Pin Error");
		        nr_kom=100;  nr_kom2=0;
		    	goto koniec_gsm_inicj;

		    }
		    else
		    {
		    	wpis_komend(nr_kom);
		    }

	    }

	  ResumptionReceiveDataFromGSM();  dbg(at_send);
	 delay_funkc[5]=1;

	 if(( nr_kom2>3)&&(licz_cpin==0)) Reset_Modemu();

     if(licz_cpin>5){ delay_funkc[5]=0;   Reset_Modemu();   }

     koniec_gsm_inicj:
	 asm("nop");

  }

  if(delay_funkc[6]==4) //Zapisz Flash
  {
	  ZapiszFlash();
	 delay_funkc[6]=0;
  }

  if(delay_funkc[7]==4)
  {
	 HAL_Delay(30);  UstawCzas(czas[0],czas[1],czas[2], czas[4],czas[5],czas[6]);  HAL_Delay(30);
	 PobierzCzas(buf_p);
  #ifdef _Neoway
	 sprintf(buf_p,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d\"\x0D",buf_p[0],buf_p[1],buf_p[2], buf_p[4],buf_p[5],buf_p[6]);   // AT+CCLK=\"08/06/17,14:50:00\"\x0D
	  dbg(buf_p);dbg3(buf_p);
	  HAL_Delay(300);
	  dbg3(buf_rx);
  #endif
  #ifdef _Fibocom
	 sprintf(buf_p,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+00\"\x0D",buf_p[0],buf_p[1],buf_p[2], buf_p[4],buf_p[5],buf_p[6]);   // +CCLK: "18/02/26,09:42:01+04"
	  dbg(buf_p);dbg3(buf_p);
	  HAL_Delay(300);
	  dbg3(buf_rx);
  #endif
	 delay_funkc[7]=0;

	 RTC_DateTypeDef sdatestructure;
	 HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
	    sprintf(buf_p,"\r\n2: %d, %d, %d",sdatestructure.Month,sdatestructure.Date,sdatestructure.Year); dbg3(buf_p);
	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, sdatestructure.Month);
	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, sdatestructure.Date);
	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, sdatestructure.Year);

  }

  if(delay_funkc[8]==7)  //Webserver OK
  {
	  wybor_bitowy|=0x10;
	 delay_funkc[8]=0;
  }

  if(delay_funkc[9]==20)  //SMS do siebie
  {
	/*if((wybor2_bitowy&0x0002)>0)
	{
	  if(wysylka_sms==0)
	  {
	   if(Const.s_Tel[0].tel0[3]!='0')
	   {
	     sprintf(buf_sms,"ZGR\x1A");
 	     ResumptionReceiveDataFromGSM();
 	     wysylka_sms=99; send_SMS(Const.s_Tel[0].tel0);
	   }
	   delay_funkc[9]=0;
	  }
	}*/
  }

  if(delay_funkc[10]==3)
  {
	   Const.s_WY[5/16].val&=~(1<<(5-16*(5/16)));  //6 PK oFF dla dzwonienia
	   GPIO_PK(5);
	 delay_funkc[10]=0;
	 ResumptionReceiveDataFromGSM(); dbg("ATH\x0D");
  }

 /* if(delay_funkc[11]==7)
  {
		if(Polacz_z_IP("192.168.4.2")==0);  else goto koniecop7;
		if(WyslijKomend("GET /TRANS_1")==0);  else goto koniecop7;
		for(i=0;i<3999;i++) buf_rx2[_Size_BufWifi+i]='A';  buf_rx2[_Size_BufWifi+i]=0;
		buf_rx2[_Size_BufWifi+0]='1';
		buf_rx2[_Size_BufWifi+1999]='2';
		buf_rx2[_Size_BufWifi+3998]='3';
		if(WyslijDane(&buf_rx2[_Size_BufWifi],3999)==0);  else goto koniecop7;

	 koniecop7:
	 dbg3("\r\nKoniec_op11  ");
  	 delay_funkc[11]=0;
  }*/
  if(delay_funkc[12]==20)  //email ESP STOP     powtorz email dopiero za czas  delay_funkc[12]
  {
  	        if((wybor_bitowy&0x2000)==0)
  			{
  				dbg3("\r\nEmail ESP powtorka ");
  				wybor_bitowy|=0x2000;
  				     if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o')) wyslij_email_raport();
  				else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z'))
  				{
  					                             if(Const.s_Rap[0].siec==1)  //WiFi
  						    	    		     {
  						    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
  						    	    		    	 {
  						    	    		    		  sprintf(email_tytul,"RejZda");
  						    	    		    		  Wyslij_email_ESP();
  						    	    		    	 }

  						    	    		     }
  				}
  				else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i'))
  				{
  					                             if(Const.s_Rap[0].siec==1)  //WiFi
  						    	    		     {
  						    	    		    	 if(Const.s_Rap[0].plik==1)      //z Pomiarami
  						    	    		    	 {
  						    	    		    		  sprintf(email_tytul,"Pomiar");
  						    	    		    		  Wyslij_email_ESP();
  						    	    		    	 }

  						    	    		     }
  				}
  				else wyslij_email();
  			}
  			else
  			{
  				 email=0; wysylka_mail=0;
  				     if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o'));
  				else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z'));
  				else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i'));
  				else{
  				   for(j=0;j<_Ilosc_Mail;j++){ i_mail[j]=0; for(i=0;i<_Dlg_buf_Mail;i++) b_mail[i+_Dlg_buf_Mail*j]=0; }
  				}
  				// InitUartWifi();
  			 	   RstWifi();
  				 dbg3("\r\nEmail ESP stop");
  			}
  	       delay_funkc[12]=0;

  }
  if(delay_funkc[14]==10)  //email test
  {
	 sprintf(email_tytul,"Test");
	 if((Const.s_Lan[0].param&0x10)==0) Wyslij_email_ENC();
	 else{                              Wyslij_email_ESP();  wybor2_bitowy|=0x0004;  }
  	 delay_funkc[14]=0;
  }
  if(delay_funkc[15]==4)
  {
     #ifdef _ENC
	    NRF_Start();
	    SEND_arp_request(ip_addr_dest,&Operation_DATA_SRAM[_Net_stack_start+0]);
	    NRF_Stop();
     #endif
  	 delay_funkc[15]=0;
  }
  if(delay_funkc[16]==5)   //mac ESP do ENC i na strone
  {
	  ResumptionReceiveDataFromWifi();
	  SendDataToModemWifi("AT+CIPSTAMAC?\x0D\x0A");
  	  delay_funkc[16]=0;
  }
  if(delay_funkc[17]==2)   //ciag email ENC lub ESP
  {
  	  delay_funkc[17]=0;
  	  email_enc=0;  ObslugaMail();
  }
  if(delay_funkc[18]==2)  //Zapocz¹tkowanie wysylania Emaila przez ESP
  {
  	  delay_funkc[18]=0;
  	  Wyslij_email_ESP();
  }
  if(delay_funkc[19]==6)   //zmiana init_lora przez Http
  {
  	  delay_funkc[19]=0;
  /*	NRF_Start();
  	 	SX1278_begin(&SX1278, Const.s_Lora[0].freq, Const.s_Lora[0].power, Const.s_Lora[0].spread, Const.s_Lora[0].bandwidth, lora_lenTrans());
  	  dbg3("\r\nDone configuring LoRaModule  ");
  	  i = SX1278_LoRaEntryRx(&SX1278, lora_lenTrans(), 20000);
  	  if(i==1) dbg3("  OK recv");  else dbg3("  NO !!! ");
*/
  	//testLL=99;



  	 NRF_Stop();
  }
  if(delay_funkc[20]==10)
  {
  	  delay_funkc[20]=0;

  	  dbg3("\r\nStart GSM...");
  	  licz_cpin=0;
  	  	  delay_funkc[5]=0;
  	  		nr_kom=0;
  	  		nr_kom2=0;
  	  		li_sms=0;
  	  		start=0;
  	  		licz_start=0;
  	  		licz_brak_Sim=0;
  	  		licz_cclk=0;
  	  		wysylka_sms=0;

  	  		if(nr_kom==0){  nr_kom=__ATE0;  wpis_komend(nr_kom);  HAL_Delay(100);  dbg(at_send); delay_funkc[5]=1;  }

  }
  if(delay_funkc[21]==5)  //Wylaczam ESp bo nie ma od niego zadnej odpowiedzi
  {
  	  delay_funkc[21]=0;
  	  licz_htt=10000;
  }
  if(delay_funkc[22]==3)  //przy kazdym polaczeniu HTTP:    "if(http_wstrz==0) all_HTTP_clear();"
  {
  	  delay_funkc[22]=0;
  	  http_wstrz=0;  dbg3("\r\nhttp_wstrz=0  ");
  	  port_ux=-1;
  }
  if(delay_funkc[23]==8)  //Sprawdzam czy sygnal 'arp request' zosta³ odebrany jesli nie to powtorz wysylke
  {
  	  if((wybor2_bitowy&0x0080)>0){  delay_funkc[23]=0;   }
  	  else
  	  {
  		  if(liczba_arp > _IloscPowtARP){  delay_funkc[23]=0;  /* dbg3("\r\nNie dostalem .ARP Request.");*/ }
  		  else
  		  {
  			  delay_funkc[15]=1;  //Send arp request
  			  delay_funkc[23]=1;
  			  liczba_arp++;
  			// dbg3("\r\nPowtarzam .ARP Request.");
  		  }
  	  }
  }
  if(delay_funkc[24]==4)  //przy kazdym polaczeniu HTTP:    "if(http_wstrz==0) all_HTTP_clear();"
  {
  	  delay_funkc[24]=0;
  	  ekran_bit&=~0x02;
  }
  if(delay_funkc[25]==30)  //flaga dzwon na 0 gdy przekroczy czas 30s
  {
  	  delay_funkc[25]=0;   ResumptionReceiveDataFromGSM(); dbg("ATH\x0D");
  	dbg3("\r\nKONIEC DZWONIENIA timer ");    Var.DzwMa[param_dzwon]=0;  koniec_dzwon=10;  dzwonBlok=0;
  }
  if(delay_funkc[26]==35)  //zerowanie flagi 'idx_email_GPRS' a zatem zamykanie polaczenia GPRS jesli sam sie nie zamknie
  {
  	  delay_funkc[26]=0;
  	  dbg3("\r\nZamykam sesje SMTP");  idx_email_GPRS=9999;   GprsSend("AT+MIPCALL=0\x0D");    idx_email_GPRS=0;

  }
  if(delay_funkc[27]==10)  //zerowanie flagi 'idx_email_GPRS' a zatem zamykanie polaczenia GPRS jesli sam sie nie zamknie
  {
  	  delay_funkc[27]=0;

 	 dbg3("\r\nDzwonienie...");  ekran_tymczasowy=5;
 	 dzwon=1;
	     delay_funkc[25]=1;  //gdyby dzwon nie wrocilo do 0
	     pc= Const.s_Tel[0].tel1;
	     ResumptionReceiveDataFromGSM();
	     sprintf(buf_p,"atd%s\x0D",pc+17*param_dzwon);  dbg3("\r\n");dbg3(buf_p);dbg3("\r\n");
	     dbg(buf_p);

  }
  if(delay_funkc[29]==5)  // zawieszenie sie podczas mail ESP
  {
  	  delay_funkc[29]=0;
  	  email=0;
  	  wybor_bitowy&=~0x2000;  //zerujemy powtorke
  	  delay_funkc[12]=0;

  	  InitUartWifi();   RstWifi();

  }
  if(delay_funkc[30]==20)  // // zawieszenie   mail ESP podczas powtorki AT+CIPSTART.... i nic wiecej
  {
  	  delay_funkc[30]=0;
  	  email=0;
  	  wybor_bitowy&=~0x2000;  //zerujemy powtorke
  	  delay_funkc[12]=0;

  }
  if(delay_funkc[31]==5)  //Test wifi komend¹ AT  czy ok
  {
  	  delay_funkc[31]=0;  dbg3("\r\nRST uarty   ");  /* ZapisZdarzenia(_ZdaTest,_Esp1,0);*/   ResetWifiAT=1;
  	  InitUartWifi();
  	  InitUartDbg();

  }
  if(delay_funkc[32]==20)  //Odblokuj 'Wyslij_zdarzenia'
  {
  	  delay_funkc[32]=0;


  }
  if(delay_funkc[33]==5)  //RST wifi bo nie odpowiada na AT
  {
  	  delay_funkc[33]=0;



  	 UART_HandleTypeDef huart1;

  	DMA_HandleTypeDef hdma_usart1_rx;
  	DMA_HandleTypeDef hdma_usart1_tx;

  	   HAL_UART_MspDeInit(&huart1);
       HAL_UART_MspInit(&huart1);

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
          ResumptionReceiveDataFromWifi();




      RstWifi();   ResetWifiAT=0;

  }
  if(delay_funkc[34]==120)  //
  {
  	  delay_funkc[34]=0;

	  dbg3("\r\nwhile(1)");

	  hiwdg.Instance = IWDG;
	  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
	  hiwdg.Init.Reload = 4095;
	  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }


	  while(1);
  }
  if(delay_funkc[35]==10)  //zerowanie flagi 'idx_email_GPRS' a zatem zamykanie polaczenia GPRS jesli sam sie nie zamknie
  {
  	  delay_funkc[35]=0;

 	 dbg3("\r\nDzwonienie...");  ekran_tymczasowy=5;
 	 dzwon=1;
	     delay_funkc[25]=1;  //gdyby dzwon nie wrocilo do 0
	     ResumptionReceiveDataFromGSM();
	     sprintf(buf_p,"atd%s\x0D",tel_nr);  dbg3("\r\n");dbg3(buf_p);dbg3("\r\n");
	     dbg(buf_p);

  }






  //################### --  TASK`s   wybor_bitowy  -- ##########################
  if((wybor_bitowy&0x0008)>0)
  {
	  wybor_bitowy&=~0x0008;
	  Reset_Modemu();

  }
  if((wybor_bitowy&0x0004)>0)
  {
  	wybor_bitowy&=~0x0004;
  	wpis_komend(__csq); dbg(at_send); licz_rst++;

  }
  if((wybor_bitowy&0x0800)>0)   //SMS`y wyslane i start obsluga email
  {
	  if(wysylka_sms==0)
	  {
		  if((Const.s_Lan[0].param&0x10)>0)
		  {
			  if((wybor_bitowy&0x8000)>0){  Wysylka_Mail_na_skrzynki(0);    wybor_bitowy&=~0x0800;   }

		  }
		  else
		  {
			  Wysylka_Mail_na_skrzynki(0);
			  wybor_bitowy&=~0x0800;
		  }

	  }
  }
  /*if((wybor_bitowy&0x8000)>0)
  {
  	  wybor_bitowy&=~0x8000;
  	 NRF_Start(); IteracjaFlashZewnOdczyt(&adr_flash);  NRF_Stop();
  }*/
  if((wybor2_bitowy&0x0001)>0)
  {
  	  wybor2_bitowy&=~0x0001;
    	NRF_Start();  AT25SF_BlockErase(0); HAL_Delay(200);  AT25SF_BlockErase(4096); NRF_Stop();
  	 adr_flash=0;  adr_flash_end=0;  adr_flash_start=0;
  	 dbg3("\r\nKasuje Rejestr temp !!!");
  }
  if((wybor2_bitowy&0x0008)>0)
  {
  	  wybor2_bitowy&=~0x0008;
      AT25SF_2_BlockErase(0); HAL_Delay(200);  AT25SF_2_BlockErase(4096);
  	 adr_flash_2=0;  adr_flash_2_end=0;  adr_flash_2_start=0;
  	 Const.s_Rap[0].start_zd=0;  ZapiszFlash();
  	 dbg3("\r\nKasuje Rejestr zdarzen !!!");
  }







}

void send_buf_sms()
{
  int i,j;

#ifdef _Neoway
  i=0; j=0;
  do
  {
     buf_p[j]=buf_sms[i];
     if(buf_p[j]=='\r')
     {
    	 buf_p[j+1]=0;
    	 ResumptionReceiveDataFromGSM();
    	 dbg3(buf_p);  dbg(buf_p);     HAL_Delay(100); dbg3(buf_rx); while(strstr(buf_rx,"\r\n>")==0);
    	 j=0;
     }
     else j++;
  }
  while(buf_sms[i++]!=0);
  buf_p[j+2]=0;
  ResumptionReceiveDataFromGSM();
  dbg3(buf_p); dbg(buf_p);
#endif


#ifdef _Fibocom
  ResumptionReceiveDataFromGSM();
 // sprintf(buf_p,"\r\nXXXX: %d     ",strlen(buf_sms));  dbg3(buf_p);
  dbg3_roz(buf_sms);
  //dbg3("\r\nTRESC... ");
  dbg(buf_sms);
#endif


}

int odczyt_liczby_cgi(char *p)
{
  int i,j;
  i=0;
  do{ buf_p[i]=p[i]; i++; }while((p[i]!='&')&&(p[i]!='.'));   buf_p[i]=0;
  return liczba_char_na_int();
}

int oblicz_dzien(void)
{
	unsigned char rok,mies,dzie;
	unsigned short aak,bbk,cck,xx; unsigned char dzien=0;

	PobierzCzas(buf_p);
	rok=buf_p[0];
	mies=buf_p[1];
	dzie=buf_p[2];


    if(rok>9)
	{
           xx = 31+28;

        for(j=0;j<rok-10;j++)
        {
            for(i=1;i<13;i++)
		    {
               if((i==1)||(i==3)||(i==5)||(i==7)||(i==8)||(i==10)||(i==12)) xx += 31;
			   else if((i==4)||(i==6)||(i==9)||(i==11)) xx += 30;
			   else if(i==2)
			   {
                  if(((10+j)%4)==0) xx += 29;
				  if(((10+j)%4)!=0) xx += 28;
			   }
		    }
        }

            for(i=1;i<mies;i++)
		    {
               if((i==1)||(i==3)||(i==5)||(i==7)||(i==8)||(i==10)||(i==12)) xx += 31;
			   else if((i==4)||(i==6)||(i==9)||(i==11)) xx += 30;
			   else if(i==2)
			   {
                  if((rok%4)==0) xx += 29;
				  if((rok%4)!=0) xx += 28;
			   }
		    }


		   xx += (unsigned short)dzie;

           aak=xx/7;
		   bbk=xx%7;

           if(bbk!=0)
	       {
               cck = xx-(aak*7);
			   dzien = 0x01<<cck;
		   }
		   else dzien = 0x01;


             if(dzien==0x01){ return 1;}
		else if(dzien==0x02){ return 2;}
		else if(dzien==0x04){ return 3;}
		else if(dzien==0x08){ return 4;}
		else if(dzien==0x10){ return 5;}
		else if(dzien==0x20){ return 6;}
		else if(dzien==0x40){ return 7;}
	    else                { return 0;}

	}
}

int podaj_dzien(int rrok, int mmies, int ddzie)
{
	unsigned char rok,mies,dzie;
	unsigned short aak,bbk,cck,xx; unsigned char dzien=0;

	rok=rrok;
	mies=mmies;
	dzie=ddzie;


    if(rok>9)
	{
           xx = 31+28;

        for(j=0;j<rok-10;j++)
        {
            for(i=1;i<13;i++)
		    {
               if((i==1)||(i==3)||(i==5)||(i==7)||(i==8)||(i==10)||(i==12)) xx += 31;
			   else if((i==4)||(i==6)||(i==9)||(i==11)) xx += 30;
			   else if(i==2)
			   {
                  if(((10+j)%4)==0) xx += 29;
				  if(((10+j)%4)!=0) xx += 28;
			   }
		    }
        }

            for(i=1;i<mies;i++)
		    {
               if((i==1)||(i==3)||(i==5)||(i==7)||(i==8)||(i==10)||(i==12)) xx += 31;
			   else if((i==4)||(i==6)||(i==9)||(i==11)) xx += 30;
			   else if(i==2)
			   {
                  if((rok%4)==0) xx += 29;
				  if((rok%4)!=0) xx += 28;
			   }
		    }


		   xx += (unsigned short)dzie;

           aak=xx/7;
		   bbk=xx%7;

           if(bbk!=0)
	       {
               cck = xx-(aak*7);
			   dzien = 0x01<<cck;
		   }
		   else dzien = 0x01;


             if(dzien==0x01){ return 1;}
		else if(dzien==0x02){ return 2;}
		else if(dzien==0x04){ return 3;}
		else if(dzien==0x08){ return 4;}
		else if(dzien==0x10){ return 5;}
		else if(dzien==0x20){ return 6;}
		else if(dzien==0x40){ return 7;}
	    else                { return 0;}

	}
}

void WgrajMacENC()
{
	  ETHADDRESS[0] =(ASCII_to_hex(Const.s_Lan[1].eth[0])<<4) | ASCII_to_hex(Const.s_Lan[1].eth[1]);
	  ETHADDRESS[1] =(ASCII_to_hex(Const.s_Lan[1].eth[2])<<4) | ASCII_to_hex(Const.s_Lan[1].eth[3]);
	  ETHADDRESS[2] =(ASCII_to_hex(Const.s_Lan[1].eth[4])<<4) | ASCII_to_hex(Const.s_Lan[1].eth[5]);
	  ETHADDRESS[3] =(ASCII_to_hex(Const.s_Lan[1].eth[6])<<4) | ASCII_to_hex(Const.s_Lan[1].eth[7]);
	  ETHADDRESS[4] =(ASCII_to_hex(Const.s_Lan[1].eth[8])<<4) | ASCII_to_hex(Const.s_Lan[1].eth[9]);
	  ETHADDRESS[5] =(ASCII_to_hex(Const.s_Lan[1].eth[10])<<4) | ASCII_to_hex(Const.s_Lan[1].eth[11]);
}

void WyznaczDzienTygodnia(void)
{
     int i;
     i=oblicz_dzien();
	         if(i==1) sprintf(&buf_p[90],"Po");
        else if(i==2) sprintf(&buf_p[90],"Wt");
        else if(i==3) sprintf(&buf_p[90],"Sr");
        else if(i==4) sprintf(&buf_p[90],"Cz");
        else if(i==5) sprintf(&buf_p[90],"Pi");
        else if(i==6) sprintf(&buf_p[90],"So");
        else if(i==7) sprintf(&buf_p[90],"Ni");
        else          sprintf(&buf_p[90],"--");
}

int RaportStanu(int *part_len, int rodz)
{

    int i,j,k,f,itx,itx2,SIZE,psize;    itx=0;  itx2=0;   SIZE=800;

	   k=0;
		  for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
		  {
			j= WyszukajNrCzujkiDlaPortu(i+1);
			if(j!=-1)
			{
			    if(Const.s_GPIO[i].val==9)
				{
			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
			    	sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",&buf_p[50],Const.s_Czujki[j].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;    if(itx2>SIZE) goto konczRaportStanu;

			    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
			    	if(j!=-1)
			    	{
			    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);
			    		     if(rodz==0){   buf_p[60]=' ';buf_p[63]=' ';buf_p[64]=' ';buf_p[65]=0;  }
			    		else if(rodz==1){   buf_p[60]=' ';buf_p[63]=' ';buf_p[64]='%';buf_p[65]='%';buf_p[66]='%';buf_p[67]='%';buf_p[68]=0;  }
				    	sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",&buf_p[60],Const.s_Czujki[j].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;     if(itx2>SIZE) goto konczRaportStanu;
			    	}
				}
			}
		  }
		  for(i=0;i<_Size__s_gpio;i++)  //Temp
		  {
			j= WyszukajNrCzujkiDlaPortu(i+1);
		    if(j!=-1)
			{
			    if(Const.s_GPIO[i].val==2)
				{
			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
			    	sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",&buf_p[50],Const.s_Czujki[j].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;     if(itx2>SIZE) goto konczRaportStanu;
				}
			    else if(Const.s_GPIO[i].val==3)
				{
			    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);
			    	sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",&buf_p[50],Const.s_Czujki[j].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;     if(itx2>SIZE) goto konczRaportStanu;


				}
			}
		  }


	 for(i=0;i<_Size_we;i++)
	 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
	 	{
		   if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){
			   sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",Const.s_WE[i].nazwa_ro, Const.s_WE[i].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;     if(itx2>SIZE) goto konczRaportStanu;
		   }
		   else{
			   sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",Const.s_WE[i].nazwa_zw, Const.s_WE[i].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;     if(itx2>SIZE) goto konczRaportStanu;
		   }
	   }
	}

	for(i=0;i<_Size_wy;i++)  //WY
	{  if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11))   //WY
	   {
		   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){
			   sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",Const.s_PK[i].nazwa_on, Const.s_PK[i].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;     if(itx2>SIZE) goto konczRaportStanu;
		   }
		   else{
			   sprintf(&buf_rx2[_Size_BufWifi+3000+itx2],"%s   %s\r\n",Const.s_PK[i].nazwa_of, Const.s_PK[i].nazwa);   psize=strlen(&buf_rx2[_Size_BufWifi+3000+itx2]);  itx+=psize;   if(itx>*part_len) itx2+=psize;     if(itx2>SIZE) goto konczRaportStanu;
		   }
	   }
	}

	if(itx==0){ sprintf(&buf_rx2[_Size_BufWifi],"Brak konfiguracji\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi]);  *part_len=itx;  return 1; }


	/*  buf_rx2[_Size_BufWifi+itx+0]='\r';
	  buf_rx2[_Size_BufWifi+itx+1]='\n';
	  buf_rx2[_Size_BufWifi+itx+2]='\x1A';
	  buf_rx2[_Size_BufWifi+itx+3]=0;
*/
	// if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO RaportStanu itx !!!");


	 konczRaportStanu:
	 sprintf(&buf_rx2[_Size_BufWifi],&buf_rx2[_Size_BufWifi+3000]);
	 *part_len += itx2;

	 if(itx2<=SIZE) return 1;
	 else           return 0;

}

void RRRRRWIFI()
{
	  if(buf_p[4]==3)
	  {  if(ZmRstWifi==0){  ZmRstWifi=1;    init_ENC();   InitUartWifi(); RstWifi(); }
	  }
	  else ZmRstWifi=0;
}

void wyswietl_ekran()   //ekr
{
	int lo,k,i,j,o,poz,x,y;   lo=0;         if((wybor_bitowy&0x4000)>0) wybor_bitowy&=~0x4000;  else wybor_bitowy|=0x4000;

	if((wybor2_bitowy&0x0400)==0){   wybor2_bitowy|=0x0400;   czujki_timer=(_Czas_Zycia_Czujek/4);   }

	if(ekran_tymczasowy>0)
	{
		if(ekran_tymczasowy==1)  //111111111111111
		{
#ifndef _LCD_TFT
			lcd_reset();
	        lcd_init();
			StringFont5x7("Wysylam SMS ...",15);
#else
			DrawInfo("Wysylam SMS ...");
#endif

			ekran_tymczasowy=101;
		}
		else if(ekran_tymczasowy==2)  //22222222222222222
		{
#ifndef _LCD_TFT
			lcd_reset();
			lcd_init();
			StringFont5x7("SMS OK",6);
#else
			DrawInfo("SMS OK");
#endif
			ekran_tymczasowy=102;
		}
		else if(ekran_tymczasowy==5)  //55555555555
		{
#ifndef _LCD_TFT
			lcd_reset();
			lcd_init();
			StringFont5x7("Dzwonie...",10);
#else
			DrawInfo("Dzwonie...");
#endif
			ekran_tymczasowy=105;
		}
		else if(ekran_tymczasowy==4)  //44444444444444
		{
#ifndef _LCD_TFT
			lcd_reset();
			lcd_init();
			StringFont5x7("SMS ERROR",9);
#else
			DrawInfo("SMS ERROR");
#endif
			ekran_tymczasowy=104;
		}
		else if(ekran_tymczasowy==3)  //3333333333333333333
		{
#ifndef _LCD_TFT
			lcd_reset();
			lcd_init();
			StringFont5x7("Wysylam E-mail",14);
#else
			//DrawInfoE("Wysylam E-mail");
#endif
			j=strlen(info_email);
			for(i=0;i<j;i++){ if((info_email[i]>0x1F)&&(info_email[i]<0x80)); else info_email[i]=' ';  } info_email[i]=0;

#ifndef _LCD_TFT
			StringFont5x7(info_email,j);
#else
			ST7735_WriteString(5, 25, "                                                                         ",   Font_11x18, ST7735_COLOR565(0xB0,0xB0,0xB0), ST7735_COLOR565(0x20,0x20,0x20));
			ST7735_WriteString(5, 25, info_email,   Font_11x18, ST7735_COLOR565(0xB0,0xB0,0xB0), ST7735_COLOR565(0x20,0x20,0x20));
#endif
			ekran_tymczasowy=103;
		}
		else licz_delay_ekran++;

		if((ekran_tymczasowy==101)||(ekran_tymczasowy==102)){ if(licz_delay_ekran>3){ ekran_tymczasowy=0; licz_delay_ekran=0; }}
		if((ekran_tymczasowy==103)||(ekran_tymczasowy==104)||(ekran_tymczasowy==105)){ if(licz_delay_ekran>6){ ekran_tymczasowy=0; licz_delay_ekran=0; }}
		ekran_bit&=~0x01;
	}
	else
	{

	if(Const.s_Poz[0].ekran==0)   //2xtemp  wejscia i wyjscia  #####################################################
	{
#ifndef _LCD_TFT

        #ifndef _LCD
            lcd_reset();
        #endif
		if((ekran_bit&0x01)>0) lcd_init_2();
		else                 { lcd_reset(); lcd_init(); ekran_bit|=0x01;}

		if((wybor_bitowy&0x4000)==0){ StringFont5x7("Czujniki     *",14);  NewLineFont5x7(14,0); }
		else                        { StringFont5x7("Czujniki      ",14);  NewLineFont5x7(14,0); }


        k=0;  poz=0; x=0;
		for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
		{
			j= WyszukajNrCzujkiDlaPortu(i+1);
			if(j!=-1)
			{
			    if(Const.s_GPIO[i].val==9)
				{
			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]); korekta_lcd(&buf_p[50]); k++; StringFont8Bold(&buf_p[50],5);

			    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
			    	if(j!=-1)
			    	{
			    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  buf_p[60]=' ';buf_p[63]='%';buf_p[64]=0;  k++;  StringFont8Bold(" ",1); StringFont8Bold(&buf_p[60],4);
			    	   lcd_start(); s(0);s(0);s(0);s(0);s(0);s(0);s(0); lcd_stop();
			    	}
			    	x++;x++; if(x>1) goto ominnnnss;
				}
			}
		}

		for(i=0;i<_Size__s_gpio;i++)  //Temp
		{
			j= WyszukajNrCzujkiDlaPortu(i+1);
		    if(j!=-1)
			{
			    if(Const.s_GPIO[i].val==2)
				{
			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]); k++; StringFont8Bold(&buf_p[50],5);
                    if((k%2)==0) NewLine(3);
                    else  NewLine(7);
			    	x++; if(x>1) goto ominnnnss;
				}
			    else if(Const.s_GPIO[i].val==3)
				{
			    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]); k++; StringFont8Bold(&buf_p[50],5);
                    if((k%2)==0);
                    else NewLine(4);
			    	x++; if(x>1) goto ominnnnss;
				}
			}
		}

             if(x==0) NewLine(17*5);
        else if(x==1) NewLine(8*5);

      ominnnnss:

	  StringFont5x7("Wejscia",7);  NewLine(8*5+1); //NewLineFont5x7(7,0);
	  		 buf_p[0]=0; x=0;
	  		 for(i=0;i<_Size_we;i++)
	  		 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
	  		    {
	  			    if((Var.we[i/16]&(1<<(i-16*(i/16))))>0) buf_p[x]='0';
	  			    else                                    buf_p[x]='1';
	  			    x++; if(x>9) break;
	  		    }
	  		 }
	  		 for(j=0;j<(10-x);j++) buf_p[x+j]=' ';     buf_p[x+j]=0;
	  		StringFont8Bold(buf_p,strlen(buf_p));   NewLine(1*5);   // NewLineFont8Bold(strlen(buf_p),1);



	  		StringFont5x7("Przekazniki",11); NewLine(4*5-2);  //  NewLineFont5x7(11,0);
	  		buf_p[0]=0;  x=0;
	  		for(i=0;i<_Size_wy;i++)
	  		{  if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY Ti Te IR
	  		   {
	  			   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0) buf_p[x]='1';
	  			   else                                            buf_p[x]='0';
	  			   x++; if(x>9) break;
	  		   }
	  		}
	  		for(j=0;j<(10-x);j++) buf_p[x+j]=' ';     buf_p[x+j]=0;
	  		StringFont8Bold(buf_p,strlen(buf_p));

#else  //--------------------------------------------------------------------------------------------------

			if((ekran_bit&0x01)>0)
			{  if((ekran_bit&0x02)==0)
		 	   {
				   EkranLcdZgrTempWeWy();

		 	   }
			}
			else
			{     ekran_bit|=0x01;

			     EkranLcdZgrTempWeWyStart();
			}

#endif

	}
	else if(Const.s_Poz[0].ekran==1)   //tylko 10xtemp  #####################################################
	{
#ifndef _LCD_TFT

        #ifndef _LCD
		  lcd_reset();
        #endif
		if((ekran_bit&0x01)>0) lcd_init_2();
		else                 { lcd_reset(); lcd_init(); ekran_bit|=0x01;}

		if((wybor_bitowy&0x4000)==0){ StringFont5x7("Czujniki     *",14);  NewLineFont5x7(14,0); }
		else                        { StringFont5x7("Czujniki      ",14);  NewLineFont5x7(14,0); }

        k=0;  poz=0; x=0; y=0;
		for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
		{
			j= WyszukajNrCzujkiDlaPortu(i+1);
			if(j!=-1)
			{
		        //x++;
		       // if(x>10) break;
			    if(Const.s_GPIO[i].val==9)
				{
			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]); k++; StringFont8Bold(&buf_p[50],5);  y++;

			    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
			    	if(j!=-1)
			    	{
			    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  buf_p[60]=' ';buf_p[63]='%';buf_p[64]=0;  k++;  StringFont8Bold(" ",1); StringFont8Bold(&buf_p[60],4);  y++;
                        #ifdef _LCD
			    		   if(y<10) NewLine(7);
                        #endif
                        #ifndef _LCD
                            NewLine(7);
                        #endif
			    	}
				}
			}
		}

		for(i=0;i<_Size__s_gpio;i++)  //Temp
		{
			j= WyszukajNrCzujkiDlaPortu(i+1);
		    if(j!=-1)
			{
			    if(Const.s_GPIO[i].val==2)
				{
			    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]); k++; StringFont8Bold(&buf_p[50],5);  y++;
                    #ifdef _LCD
                       if(y<10){
			    	   if((k%2)==0) NewLine(3);
                       else NewLine(7); }
                    #endif
                    #ifndef _LCD
			    	   if((k%2)==0) NewLine(3);
                       else NewLine(7);
                    #endif
				}
			    else if(Const.s_GPIO[i].val==3)
				{
			    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]); k++; StringFont8Bold(&buf_p[50],5);  y++;
                   #ifdef _LCD
			    	   if(y<10){
			    	   if((k%2)==0){ }
                       else  NewLine(4);  }
                   #endif
                   #ifndef _LCD
			    	   if((k%2)==0){ }
			           else  NewLine(4);
                   #endif


				}
			}
		}



     #ifdef _LCD
		if((y==8)||(y==9)) NewLineFont5x7(14,0);
		else if((y==6)||(y==7))
	    {
	    	  StringFont5x7("              ",14);
	    }
		else if((y==4)||(y==5))
		{
			StringFont5x7("              ",14);  NewLineFont5x7(14,0);
			StringFont5x7("              ",14);
		}
		else if((y==2)||(y==3))
		{
			StringFont5x7("              ",14);  NewLineFont5x7(14,0);
			StringFont5x7("              ",14);  NewLineFont5x7(14,0);
			StringFont5x7("              ",14);
		}
		else if((y==0)||(y==1))
		{
			StringFont5x7("              ",14);  NewLineFont5x7(14,0);
			StringFont5x7("              ",14);  NewLineFont5x7(14,0);
			StringFont5x7("              ",14);  NewLineFont5x7(14,0);
			StringFont5x7("              ",14);
		}
     #endif

#else   //-------------------------------------------------------------------------------------------
		PobierzCzas(buf_p);  RRRRRWIFI();
		if((ekran_bit&0x01)>0)
		{  if((ekran_bit&0x02)==0)
	 	   {
		      k=IloscCzujnikow()/2;
		      if(k<3)                EkranLcdTempBig();
		      else if((k>2)&&(k<9))  EkranLcdTemp();
		      else                   EkranLcdTempSmall(14);

	 	   }
		}
		else
		{     ekran_bit|=0x01;
	          k=IloscCzujnikow()/2;
	          if(k<3)                EkranLcdTempBigStart();
	          else if((k>2)&&(k<9))  EkranLcdTempStart();
	          else                   EkranLcdTempSmallStart(14);
		}

#endif

	}
	else if(Const.s_Poz[0].ekran==2)   //zegar  #####################################################
	{

#ifndef _LCD_TFT

        #ifndef _LCD
           lcd_reset();
        #endif
		if((ekran_bit&0x01)>0) lcd_init_2();
		else                 { lcd_reset(); lcd_init(); ekran_bit|=0x01;}


		PobierzCzas(buf_p);
		sprintf(&buf_p[10],"%02d.%02d.%02d",buf_p[0],buf_p[1],buf_p[2]);
		NewLine(2*5); StringFont5x7(&buf_p[10],strlen(&buf_p[10]));   NewLine(6*5); // NewLineFont5x7(strlen(&buf_p[10]),4);
		WyznaczDzienTygodnia();
		sprintf(&buf_p[10],"%02d:%02d:%02d %s",buf_p[4],buf_p[5],buf_p[6],&buf_p[90]);
		NewLine(2*5); StringFont5x7(&buf_p[10],strlen(&buf_p[10]));  NewLine(2*5-2);   //    NewLineFont5x7(strlen(&buf_p[10]),4);



		 StringFont5x7("Wejscia",7);  NewLine(8*5+1); //NewLineFont5x7(7,0);
		 buf_p[0]=0; x=0;
		 for(i=0;i<_Size_we;i++)
		 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
		    {
			    if((Var.we[i/16]&(1<<(i-16*(i/16))))>0) buf_p[x]='0';
			    else                                    buf_p[x]='1';
			    x++; if(x>9) break;
		    }
		 }
		 for(j=0;j<(10-x);j++) buf_p[x+j]=' ';     buf_p[x+j]=0;
		StringFont8Bold(buf_p,strlen(buf_p));   NewLine(1*5);   // NewLineFont8Bold(strlen(buf_p),1);




		StringFont5x7("Przekazniki",11); NewLine(4*5-2);  //  NewLineFont5x7(11,0);
		buf_p[0]=0;  x=0;
		for(i=0;i<_Size_wy;i++)
		{  if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY  Te  Ti  IR
		   {
			   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0) buf_p[x]='1';
			   else                                            buf_p[x]='0';
			   x++; if(x>9) break;
		   }
		}
		for(j=0;j<(10-x);j++) buf_p[x+j]=' ';     buf_p[x+j]=0;
		StringFont8Bold(buf_p,strlen(buf_p));


#else  //---------------------------------------------------------------------------------------------------------
		PobierzCzas(buf_p);  RRRRRWIFI();
		if((ekran_bit&0x01)>0)
		{  if((ekran_bit&0x02)==0)
	 	   {
			   k= IloscWE()+IloscWY();
			   if(k<9) EkranLcdZegar();
			   else    EkranLcdZegarSmall(14);

	 	   }
		}
		else
		{     ekran_bit|=0x01;

		      k= IloscWE()+IloscWY();
		      if(k<9)  EkranLcdZegarStart();
		      else     EkranLcdZegarStartSmall(14);
		}

#endif
	}
	/*else if(Const.s_Poz[0].ekran==3)   //test LoRa  #####################################################
	{
		lcd_reset();
		if((ekran_bit&0x01)>0) lcd_init_2();
		else                 { lcd_init(); ekran_bit|=0x01;}

		if((wybor_bitowy&0x4000)==0){ StringFont5x7("Test LoRa    *",14);  NewLineFont5x7(14,0); }
		else                        { StringFont5x7("Test LoRa     ",14);  NewLineFont5x7(14,0); }

		if(test_lora==1) sprintf(buf_p,"Recv: OK   ");
		else             sprintf(buf_p,"Recv: FALSE");

		StringFont5x7(buf_p,11);

	}*/
	}
}

void SygnalizacjaPK_dla_temp(int stan, int i)
{
 /*  int nr;

   if((Const.s_Czujki[i].alarm_pk&0x01)>0)
   {
	   nr= Const.s_Czujki[i].ilosc_alarm;
	   nr--;
	   if(Const.s_GPIO[nr].val==0)
	   {
		      if(stan==0) Const.s_WY[0].val&=~(1<<nr);
		      else        Const.s_WY[0].val|=(1<<nr);

		      if(Const.s_GPIO[nr].val==0)  GPIO_PK(nr);
	   }
   }*/
}

int Wysylka_SMS_na_numery(int k)
{
       if(k==0) goto wsmsnr0;
  else if(k==1) goto wsmsnr1;
  else if(k==2) goto wsmsnr2;
  else if(k==3) goto wsmsnr3;
  else if(k==4) goto wsmsnr4;
  else if(k==5) goto wsmsnr5;
  else if(k==6) goto wsmsnr6;
  else if(k==7) goto wsmsnr7;
  else if(k==8) return 0;

  wsmsnr0: if((Const.s_Tel[0].tel1[3]!='0')&&(i_sms[0]!=0)){ wysylka_sms=1;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel1); return 1; }
  wsmsnr1: if((Const.s_Tel[0].tel2[3]!='0')&&(i_sms[1]!=0)){ wysylka_sms=2;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel2); return 2; }
  wsmsnr2: if((Const.s_Tel[0].tel3[3]!='0')&&(i_sms[2]!=0)){ wysylka_sms=3;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel3); return 3; }
  wsmsnr3: if((Const.s_Tel[0].tel4[3]!='0')&&(i_sms[3]!=0)){ wysylka_sms=4;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel4); return 4; }
  wsmsnr4: if((Const.s_Tel[0].tel5[3]!='0')&&(i_sms[4]!=0)){ wysylka_sms=5;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel5); return 5; }
  wsmsnr5: if((Const.s_Tel[0].tel6[3]!='0')&&(i_sms[5]!=0)){ wysylka_sms=6;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel6); return 6; }
  wsmsnr6: if((Const.s_Tel[0].tel7[3]!='0')&&(i_sms[6]!=0)){ wysylka_sms=7;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel7); return 7; }
  wsmsnr7: if((Const.s_Tel[0].tel8[3]!='0')&&(i_sms[7]!=0)){ wysylka_sms=8;ekran_tymczasowy=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel8); return 8; }

  return 0;
}

void ZapiszFlashZewn(uint32_t* adres, uint8_t* data, uint16_t len)
{
	uint32_t  adr_p,adrr,adr,k1,k2;  uint8_t  buf[256];

	if((*adres+len)>(_Temp_Flash_Size-4096))
	{    dbg3("\r\nPRZEWIJAM Flash !!!"); *adres=0;
	     AT25SF_BlockErase(0); HAL_Delay(200); AT25SF_BlockErase(4096); dbg3("  ERASE block 4K for iter. ");
	     adr_flash=0;  adr_flash_end=0;  adr_flash_start=0;
	     Const.s_Rap[0].start_po= 0;  delay_funkc[6]=1;
	     return;
	}

	adr=*adres;    adr_p= 256*(adr/256);    adrr= adr_p+256-adr;

	NRF_Start();

	k1= (adr+  0)/4096;
	k2= (adr+len)/4096;

	if(k1<k2){ AT25SF_BlockErase(4096+4096*k2); HAL_Delay(500); dbg3("\r\nERASE block 4K "); }

	AT25SF_ReadArray(4096+adr_p, buf, 256 );

	if(len<=adrr)
	{	AT25SF_PageProgram(4096+adr, data, len );
	}
	else
	{   AT25SF_PageProgram(4096+adr, data, adrr );
        AT25SF_PageProgram(4096+adr_p+256, &data[adrr], len-adrr );
	}
	*adres+=len;

	NRF_Stop();
}



void ZakonczPomiaryTempAvr()
{
  int i,j,k;   k=0;

  for(j=0;j<_Size__s_Czujki_info;j++)
  {
	  if(temp_avr[2*j+1]>0) temp_avr[2*j]= temp_avr[2*j]/temp_avr[2*j+1];
	  else temp_avr[2*j]=0x7FFF;
  }

  ZamianaCzasuNaFlash();
	buf_p[k++]=time>>24;
	buf_p[k++]=time>>16;
	buf_p[k++]=time>>8;
	buf_p[k++]=time>>0;

	for(i=0;i<_Size__s_gpio;i++)
	{
		       j= WyszukajNrCzujkiDlaPortu(i+1);
			   if(j!=-1)
			   {
		    	  if(Const.s_GPIO[i].val==9)
		    	  {
			    	 buf_p[k++]=temp_avr[2*j]>>8;  //temp z DHT
			    	 buf_p[k++]=temp_avr[2*j]>>0;

		    	     j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	     if(j!=-1)
		    	     {
		    	    	 buf_p[k++]=temp_avr[2*j]>>8;    //Wilg z DHT
		    	    	 buf_p[k++]=temp_avr[2*j]>>0;
		    	     }
		    	  }
			      else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))
			      {
			    	  buf_p[k++]=temp_avr[2*j]>>8;    //temp z DS z Pt
			    	  buf_p[k++]=temp_avr[2*j]>>0;
			      }
			   }
	}
	ZapiszFlashZewn(&adr_flash,buf_p, k);
	IteracjaFlashZewnZapis(adr_flash);

	for(j=0;j<_Size__s_Czujki_info;j++)
	{
	   temp_avr[2*j]= 0;
	   temp_avr[2*j+1]= 0;
	}

}

void PomiaryTempAvr()
{
  int i,j,k,itx;  k=0;

	for(i=0;i<_Size__s_gpio;i++)
	{
		       j= WyszukajNrCzujkiDlaPortu(i+1);
			   if(j!=-1)
			   {
		    	  if(Const.s_GPIO[i].val==9)
		    	  {
		    		  if(Var.value[j]!=0x7FFF){
		    		     temp_avr[2*j]= temp_avr[2*j]+Var.value[j];  //temp z DHT
		    		     temp_avr[2*j+1]++;
		    		  }

		    	     j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	     if(j!=-1)
		    	     {
		    	    	 if(Var.value[j]!=0x7FFF){
		    	    	     temp_avr[2*j]= temp_avr[2*j]+Var.value[j];  //Wilg z DHT
			    		     temp_avr[2*j+1]++;
			    		 }
		    	     }
		    	  }
			      else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))
			      {
		    	    	 if(Var.value[j]!=0x7FFF){
			    	         temp_avr[2*j]= temp_avr[2*j]+Var.value[j];  //temp z DS
			    		     temp_avr[2*j+1]++;
			    		 }
			      }
			   }
	}
}

void ZapisPomarowFlashZewn()
{
  int i,j,k,itx;  k=0;
	ZamianaCzasuNaFlash();
	buf_p[k++]=time>>24;
	buf_p[k++]=time>>16;
	buf_p[k++]=time>>8;
	buf_p[k++]=time>>0;

	for(i=0;i<_Size__s_gpio;i++)
	{
		       j= WyszukajNrCzujkiDlaPortu(i+1);
			   if(j!=-1)
			   {
		    	  if(Const.s_GPIO[i].val==9)
		    	  {
		    		 buf_p[k++]=Var.value[j]>>8;  //temp z DHT
		    		 buf_p[k++]=Var.value[j]>>0;

		    	     j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	     if(j!=-1)
		    	     {
		    	    	 buf_p[k++]=Var.value[j]>>8;  //Wilg z DHT
		    	    	 buf_p[k++]=Var.value[j]>>0;
		    	     }
		    	  }
			      else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))
			      {
			    	  buf_p[k++]=Var.value[j]>>8;  //temp z DS
			          buf_p[k++]=Var.value[j]>>0;
			      }
			   }
	}

	ZapiszFlashZewn(&adr_flash,buf_p, k);
	IteracjaFlashZewnZapis(adr_flash);

}

int IloscCzujnikow()
{
	int i,j,k;  k=0;

	for(i=0;i<_Size__s_gpio;i++)
	{
		       j= WyszukajNrCzujkiDlaPortu(i+1);
			   if(j!=-1)
			   {
		    	  if(Const.s_GPIO[i].val==9)
		    	  {
		    		 k+=2;

		    	     j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	     if(j!=-1)
		    	     {
		    	    	 k+=2;
		    	     }
		    	  }
			      else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))
			      {
			    	  k+=2;
			      }
			   }
	}
	return k;
}

char SprawdzKtoraCzujkaZapisu(int nr)
{
  int i,j,nr_p;

	nr_p=0;
    for(i=0;i<_Size__s_gpio;i++)
   	{
   		       j= WyszukajNrCzujkiDlaPortu(i+1);
   			   if(j!=-1)
   			   {
   		    	  if(Const.s_GPIO[i].val==9)  //DHT
   		    	  {
   		    		 if(nr_p==nr) return 'T';
   		    	     else nr_p++;
		    	     j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	     if(j!=-1)
		    	     {
		    	    	 if(nr_p==nr) return 'D';
		    	         else nr_p++;
		    	     }
   		    	  }
   			      else if(Const.s_GPIO[i].val==2) //DS
   			      {
   			    	 if(nr_p==nr) return 'T';
   			         else nr_p++;
   			      }
   			      else if(Const.s_GPIO[i].val==3) //Pt
   			      {
   			    	 if(nr_p==nr) return 'P';
   			         else nr_p++;
   			      }
   			   }
   	}
}

void OdczytPomiarowFlashZewn_p(int k, int* j)
{
	int i; char a;

	AT25SF_ReadArray(4096+adr_flash_read, buf_p, k );
	adr_flash_read+=k;

	time=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
	ZamianaCzasuZFlash(time);

	sprintf(&buf_rx2[_Size_BufWifi+*j],"%02d.%02d.%02d  %02d:%02d:%02d",rok,mies,dzien,godz,min,sec);  *j+=strlen(&buf_rx2[_Size_BufWifi+*j]);

	for(i=0;i<(k-4);i++)   //4 bo czas zapisywany jest w 4-rech bajtach
	{
    	ttemp=   0xff00&(((uint16_t)buf_p[4+i+0])<<8) | 0x00ff&(((uint16_t)buf_p[4+i+1])<<0);

    	 a=SprawdzKtoraCzujkaZapisu(i/2);
    	      if(a=='T'){ wartTemp_odwrotnie(&buf_p[90], ttemp);   if(buf_p[90+1]=='0'){  buf_p[90+1]=buf_p[90+0];  buf_p[90+0]=' '; }   }
    	 else if(a=='D'){ wartTemp_odwrotnie(&buf_p[90], ttemp);   buf_p[90+0]=' '; buf_p[90+3]=' ';  buf_p[90+4]=' ';  if(buf_p[90+1]=='0') buf_p[90+1]=' '; }
    	 else if(a=='P'){ wartTemp_odwrotnie_Pt(&buf_p[90], ttemp);   }

       sprintf(&buf_rx2[_Size_BufWifi+*j],"   %s",&buf_p[90]);  *j+=strlen(&buf_rx2[_Size_BufWifi+*j]);
       i++;
	}
}


void OdczytPomiarowFlashZewn_p_CSV(int k, int* j)
{
	int i;

	AT25SF_ReadArray(4096+adr_flash_read, buf_p, k );
	adr_flash_read+=k;

	time=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
	ZamianaCzasuZFlash(time);

	sprintf(&buf_rx2[_Size_BufWifi+*j],"%02d.%02d.%02d %02d:%02d:%02d;",rok,mies,dzien,godz,min,sec);  *j+=strlen(&buf_rx2[_Size_BufWifi+*j]);

	for(i=0;i<(k-4);i++)
	{
    	ttemp=   0xff00&(((uint16_t)buf_p[4+i+0])<<8) | 0x00ff&(((uint16_t)buf_p[4+i+1])<<0);

   	    a=SprawdzKtoraCzujkaZapisu(i/2);
   	         if(a=='T'){  if(ttemp!=0x7fff){ wartTemp_odwrotnie(&buf_p[0], ttemp);  sprintf(&buf_p[90],"%d,%c",ttemp>>4,buf_p[4]);      }  else sprintf(&buf_p[90],"null");  }
   	    else if(a=='D'){  if(ttemp!=0x7fff){ wartTemp_odwrotnie(&buf_p[90], ttemp);   buf_p[90+0]=buf_p[90+1]; buf_p[90+1]=buf_p[90+2]; buf_p[90+2]=0; } else sprintf(&buf_p[90],"null");  }
   	    else if(a=='P'){  if(ttemp!=0x7fff) sprintf(&buf_p[90],"%d",ttemp);   else sprintf(&buf_p[90],"null"); }

        if(i==(k-4-2)) sprintf(&buf_rx2[_Size_BufWifi+*j],"%s",&buf_p[90]);
    	else           sprintf(&buf_rx2[_Size_BufWifi+*j],"%s;",&buf_p[90]);

    	*j+=strlen(&buf_rx2[_Size_BufWifi+*j]);
        i++;

	}
}

/*int OdczytPomiarowFlashZewn()
{
	int i,j,k;

	NRF_Start();

	k=IloscCzujnikow();
	if(k==0) return 0;
	k+=3;

	adr_flash_read=0;    if(adr_flash==0) return 0;

	poesdfdfgg:
	j=0;

	OdczytPomiarowFlashZewn_p(k,&j);

	buf_rx2[_Size_BufWifi+j]=0;
	dbg3("\r\n");  dbg3(&buf_rx2[_Size_BufWifi]);

	if(adr_flash_read < adr_flash) goto poesdfdfgg;

	 NRF_Stop();

	 return 1;

}
*/

void IteracjaFlashZewnZapis(uint32_t adr)
{
  uint8_t  buf[256], *p1;  p1=buf;
  int i,j;

  NRF_Start();
  powtorzIFZ:
  for(j=0;j<16;j++)
  {
	 AT25SF_ReadArray(256*j, p1, 256 );
	 for(i=0;i<64;i++)
	 {
	    if(*(p1+4*i)==0xFF)
	    {
		   *(p1+4*i+0)= 0x00;
		   *(p1+4*i+1)= adr>>16;
		   *(p1+4*i+2)= adr>>8;
		   *(p1+4*i+3)= adr>>0;

		   AT25SF_PageProgram(256*j+4*i, p1+4*i, 4 ); // dbg3("\r\nZapis adr_flash");
		   goto ominIFZ;
	    }
	 }
  }
  AT25SF_BlockErase(0); HAL_Delay(200);  dbg3("\r\nERASE block 4K for iter. "); goto powtorzIFZ;
  ominIFZ:
  NRF_Stop();
}

void IteracjaFlashZewnOdczyt(uint32_t* adr)
{
  uint8_t  buf[256],a,b,c,d, *p1;  p1=buf;
  int i,j;  a=0; b=0; c=0; d=0;


  for(j=0;j<16;j++)
  {
	 AT25SF_ReadArray(256*j, p1, 256 );
	 for(i=0;i<64;i++)
	 {
	    if(*(p1+4*i)==0x00)
	    {
		   a= *(p1+4*i+0);
		   b= *(p1+4*i+1);
		   c= *(p1+4*i+2);
		   d= *(p1+4*i+3);
	    }
	    else if(*(p1+4*i)==0xFF)
	    {
           goto ominIFZ;
	    }
	 }
  }

  ominIFZ:
  *adr=0;
  *adr= 0x00FF0000&(((uint32_t)b)<<16) | 0x0000FF00&(((uint32_t)c)<<8) | 0x000000FF&(((uint32_t)d)<<0);

}


void IteracjaFlash_2_ZewnZapis(uint32_t adr)
{
  uint8_t  buf[256], *p1;  p1=buf;
  int i,j;

  NRF_Start();
  powtorzIFZ2:
  for(j=0;j<16;j++)
  {
	 AT25SF_2_ReadArray(256*j, p1, 256 );
	 for(i=0;i<64;i++)
	 {
	    if(*(p1+4*i)==0xFF)
	    {
		   *(p1+4*i+0)= 0x00;
		   *(p1+4*i+1)= adr>>16;
		   *(p1+4*i+2)= adr>>8;
		   *(p1+4*i+3)= adr>>0;

		   AT25SF_2_PageProgram(256*j+4*i, p1+4*i, 4 );  dbg3("\r\nZapis adr_flash_2");
		   goto ominIFZ2;
	    }
	 }
  }
  AT25SF_2_BlockErase(0); HAL_Delay(200);  dbg3("\r\nERASE block 4K for iter. "); goto powtorzIFZ2;
  ominIFZ2:
  NRF_Stop();
}

void IteracjaFlash_2_ZewnOdczyt(uint32_t* adr)
{
  uint8_t  buf[256],a,b,c,d, *p1;  p1=buf;
  int i,j;  a=0; b=0; c=0; d=0;


  for(j=0;j<16;j++)
  {
	 AT25SF_2_ReadArray(256*j, p1, 256 );
	 for(i=0;i<64;i++)
	 {
	    if(*(p1+4*i)==0x00)
	    {
		   a= *(p1+4*i+0);
		   b= *(p1+4*i+1);
		   c= *(p1+4*i+2);
		   d= *(p1+4*i+3);
	    }
	    else if(*(p1+4*i)==0xFF)
	    {
           goto ominIFZ2;
	    }
	 }
  }

  ominIFZ2:
  *adr=0;
  *adr= 0x00FF0000&(((uint32_t)b)<<16) | 0x0000FF00&(((uint32_t)c)<<8) | 0x000000FF&(((uint32_t)d)<<0);

}




void ZamianaCzasuNaFlash()
{
  PobierzCzas(buf_p);
  //time= 0x000F0000&(((uint32_t)buf_p[1])<<16) | 0x0000F800&(((uint32_t)buf_p[2])<<11) | 0x000007C0&(((uint32_t)buf_p[4])<<6) | 0x0000003F&((uint32_t)buf_p[5]);
  time= (63<<26)&(((uint32_t)buf_p[0])<<26) | (15<<22)&(((uint32_t)buf_p[1])<<22) | (31<<17)&(((uint32_t)buf_p[2])<<17) | (31<<12)&(((uint32_t)buf_p[4])<<12) | (63<<6)&(((uint32_t)buf_p[5])<<6) | (63<<0)&((uint32_t)buf_p[6]);
  //time= 0x03C00000&(((uint32_t)buf_p[1])<<22) | 0x00370000&(((uint32_t)buf_p[2])<<17) | 0x0001F000&(((uint32_t)buf_p[4])<<12) | 0x00000FC0&(((uint32_t)buf_p[5])<<6) | 0x0000003F&((uint32_t)buf_p[6]);
}

void ZamianaCzasuZFlash(uint32_t time)
{
	sec=  time&0x0000003F;
	min=  (time>>6)&0x0000003F;
	godz= (time>>12)&0x0000001F;
	dzien= (time>>17)&0x0000001F;
	mies= (time>>22)&0x0000000F;
	rok= (time>>26)&0x0000003F;
}

int Wysylka_Mail_na_skrzynki(int k)
{
       if(k==0) goto wsmsnr0a;
  else if(k==1) goto wsmsnr1a;
  else if(k==2) goto wsmsnr2a;
  else if(k==3) goto wsmsnr3a;
  else if(k==4) goto wsmsnr4a;
  else if(k==5) goto wsmsnr5a;
  else if(k==6) goto wsmsnr6a;
  else if(k==7) goto wsmsnr7a;
  else if(k==8) return 0;

  wsmsnr0a: if((Const.s_Email[0].odb1[0]!=0)&&(i_mail[0]!=0)){ wysylka_mail=1; wyslij_email(); return 1; }
  wsmsnr1a: if((Const.s_Email[0].odb2[0]!=0)&&(i_mail[1]!=0)){ wysylka_mail=2; wyslij_email(); return 2; }
  wsmsnr2a: if((Const.s_Email[0].odb3[0]!=0)&&(i_mail[2]!=0)){ wysylka_mail=3; wyslij_email(); return 3; }
  wsmsnr3a: if((Const.s_Email[0].odb4[0]!=0)&&(i_mail[3]!=0)){ wysylka_mail=4; wyslij_email(); return 4; }
  wsmsnr4a: if((Const.s_Email[0].odb5[0]!=0)&&(i_mail[4]!=0)){ wysylka_mail=5; wyslij_email(); return 5; }
  wsmsnr5a: if((Const.s_Email[0].odb6[0]!=0)&&(i_mail[5]!=0)){ wysylka_mail=6; wyslij_email(); return 6; }
  wsmsnr6a: if((Const.s_Email[0].odb7[0]!=0)&&(i_mail[6]!=0)){ wysylka_mail=7; wyslij_email(); return 7; }
  wsmsnr7a: if((Const.s_Email[0].odb8[0]!=0)&&(i_mail[7]!=0)){ wysylka_mail=8; wyslij_email(); return 8; }

  return 0;
}

void wyslij_zdarzenia()   //zda
{
	int i,j,ix,h,g,nr_czujki,port,p,full,full2,m,s,zgoda;   ix=0;  j=0;  char *pp;    full=0;  full2=0;    m=0; s=0;

  //if((wysylka_sms==0)&&(start==1))
  if((wysylka_sms==0)&&(wysylka_mail==0)&&(dzwon==0)&&(idx_email_GPRS==0))
  { if((li_sms==0)&&(gsm_flag==0))
    {
	    zgoda=0;
	    if(((Const.s_Email[0].ip[0])!=0)&&((Const.s_Email[0].nad[0])!=0)&&((Const.s_Email[0].port[0])!=0))
	  	{ if(((Const.s_Email[0].odb1[0])!=0)||((Const.s_Email[0].odb2[0])!=0)||((Const.s_Email[0].odb3[0])!=0)||((Const.s_Email[0].odb4[0])!=0)||((Const.s_Email[0].odb5[0])!=0)||((Const.s_Email[0].odb6[0])!=0)||((Const.s_Email[0].odb7[0])!=0)||((Const.s_Email[0].odb8[0])!=0))
	  	  {
	  		  if((Const.s_Lan[0].param&0x10)==0) zgoda=1;   //ENC mail
	  		  else    //ESP mail
	  		  {
	  			  if((Const.s_Lan[0].param&0x04)>0)    //if STA on
	  			  {
	  				    zgoda=1;
	  			  }
	  		  }
	  	  }
	  	}


	  //for(i=0;i<(_Size_sms_wsk-1)/3;i++)
	  for(i=0;i<sms_wsk[_Size_sms_wsk-1]/3;i++)
	  {



		 if(sms_wsk[3*i+0]=='T')    //-------Zdarzenie temp 1
	     {
			   nr_czujki = sms_wsk[3*i+1]-1;
               port= Const.s_Czujki[nr_czujki].addr_DS[8];
			 if(Const.s_GPIO[port-1].val==3) wartTemp_odwrotnie_Pt(&buf_p[60],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_max1); else wartTemp_odwrotnie(&buf_p[60],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_max1);
			 if(Const.s_GPIO[port-1].val==3) wartTemp_odwrotnie_Pt(&buf_p[50],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_min1); else wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_min1);
	    	      if(sms_wsk[3*i+2]==3){ sprintf(&buf_p[77],"Powyzej %s",&buf_p[60]);                SygnalizacjaPK_dla_temp(1,sms_wsk[3*i+1]-1); }
	    	 else if(sms_wsk[3*i+2]==2){ sprintf(&buf_p[77],"w normie %s %s",&buf_p[60],&buf_p[50]); SygnalizacjaPK_dla_temp(0,sms_wsk[3*i+1]-1); }
	    	 else if(sms_wsk[3*i+2]==1){ sprintf(&buf_p[77],"ponizej %s",&buf_p[50]);                SygnalizacjaPK_dla_temp(1,sms_wsk[3*i+1]-1); }
	    	 sprintf(buf_p,"\r\n%s %s", Const.s_Czujki[sms_wsk[3*i+1]-1].nazwa, &buf_p[77]);




	    	 if(start==1){
				pp= Const.s_Tel[0].tel1;
			    nr_czujki = sms_wsk[3*i+1]-1;
                port= Const.s_Czujki[nr_czujki].addr_DS[8];
                if((Const.s_GPIO[port-1].val==2)||(Const.s_GPIO[port-1].val==9)||(Const.s_GPIO[port-1].val==3))
                {
				   for(p=0;p<_Ilosc_Tel;p++)
				   { if(*(pp+p*17+3)!='0')
				     {
					     h=WyszukujPortElement(t_te,_Size_te,port);
					   	 if(h!=-1)
					   	 {
					        if(((Const.s_Ma[p].sms[h/16]>>(h-16*(h/16)))&0x01)>0)
				            {
				    	       if(i_sms[p]+strlen(buf_p)<160)
				    	       {
				    		       sprintf(&b_sms[160*p+i_sms[p]],buf_p);
					               i_sms[p]= i_sms[p]+strlen(buf_p);
					               s=1;
	 					           if(Const.s_Dzw[0].ma[p]>0) Var.DzwMa[p]=1;
				    	       }
				    	       else{ dbg3("\r\ni_sms !!!"); full=1; goto koniec_zda_full; }
				            }
					   	 }

				     }
				   }
				}}


	    	 if(zgoda>0){
                     pp= Const.s_Email[0].odb1;
                     nr_czujki = sms_wsk[3*i+1]-1;
                     port= Const.s_Czujki[nr_czujki].addr_DS[8];
	                 if((Const.s_GPIO[port-1].val==2)||(Const.s_GPIO[port-1].val==9)||(Const.s_GPIO[port-1].val==3))
	                 {
	 				   for(p=0;p<_Ilosc_Mail;p++)
	 				   { if(*(pp+p*41+0)!=0)
	 				     {
	 					     h=WyszukujPortElement(t_te,_Size_te,port);
	 					     if(h!=-1)
	 					  	 {
	 					        if(((Const.s_Ma[p].mail[h/16]>>(h-16*(h/16)))&0x01)>0)
	 				            {
	 				               if(i_mail[p]+strlen(buf_p)<_Dlg_buf_Mail)
				    	           {
	 					              sprintf(&b_mail[_Dlg_buf_Mail*p+i_mail[p]],buf_p);
	 					              i_mail[p]= i_mail[p]+strlen(buf_p);
	 					              m=1;
	 					           }
	 				    	       else{ dbg3("\r\ni_mail !!!"); full2=1; goto koniec_zda_full; }
	 				            }
	 					  	 }

	 				     }
	 				   }
	 				}
	    	 }

	         j+=3;

	     }
		 else if(sms_wsk[3*i+0]=='D')    //-------Zdarzenie temp 2
	     {
			    nr_czujki = sms_wsk[3*i+1]-1;
                port= Const.s_Czujki[nr_czujki].addr_DS[8];
             if(Const.s_GPIO[port-1].val==3) wartTemp_odwrotnie_Pt(&buf_p[60],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_max2); else wartTemp_odwrotnie(&buf_p[60],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_max2);
             if(Const.s_GPIO[port-1].val==3) wartTemp_odwrotnie_Pt(&buf_p[50],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_min2); else wartTemp_odwrotnie(&buf_p[50],Const.s_Czujki[sms_wsk[3*i+1]-1].prog_min2);
	    	      if(sms_wsk[3*i+2]==3){ sprintf(&buf_p[77],"Powyzej %s",&buf_p[60]);                SygnalizacjaPK_dla_temp(1,sms_wsk[3*i+1]-1); }
	    	 else if(sms_wsk[3*i+2]==2){ sprintf(&buf_p[77],"w normie %s %s",&buf_p[60],&buf_p[50]); SygnalizacjaPK_dla_temp(0,sms_wsk[3*i+1]-1); }
	    	 else if(sms_wsk[3*i+2]==1){ sprintf(&buf_p[77],"ponizej %s",&buf_p[50]);                SygnalizacjaPK_dla_temp(1,sms_wsk[3*i+1]-1); }
	    	 sprintf(buf_p,"\r\n%s %s", Const.s_Czujki[sms_wsk[3*i+1]-1].nazwa, &buf_p[77]);




		     if(start==1){
	 		    pp= Const.s_Tel[0].tel1;
			    nr_czujki = sms_wsk[3*i+1]-1;
                port= Const.s_Czujki[nr_czujki].addr_DS[8];
                if((Const.s_GPIO[port-1].val==2)||(Const.s_GPIO[port-1].val==9)||(Const.s_GPIO[port-1].val==3))
                {
				   for(p=0;p<_Ilosc_Tel;p++)
				   { if(*(pp+p*17+3)!='0')
				     {
					     h=WyszukujPortElement(t_te,_Size_te,port);
					     if(h!=-1)
					  	 {
					        if(((Const.s_Ma[p].sms[h/16]>>(h-16*(h/16)))&0x01)>0)
				            {
				    	       if(i_sms[p]+strlen(buf_p)<160)
				    	       {
				    	           sprintf(&b_sms[160*p+i_sms[p]],buf_p);
					               i_sms[p]= i_sms[p]+strlen(buf_p);
					               s=1;
	 					           if(Const.s_Dzw[0].ma[p]>0) Var.DzwMa[p]=1;
				    	       }
				    	       else{ dbg3("\r\ni_sms !!!"); full=1; goto koniec_zda_full; }
				            }
					  	 }

				     }
				   }
				}}


		        if(zgoda>0){
                     pp= Const.s_Email[0].odb1;
                     nr_czujki = sms_wsk[3*i+1]-1;
                     port= Const.s_Czujki[nr_czujki].addr_DS[8];
	                 if((Const.s_GPIO[port-1].val==2)||(Const.s_GPIO[port-1].val==9)||(Const.s_GPIO[port-1].val==3))
	                 {
	 				   for(p=0;p<_Ilosc_Mail;p++)
	 				   { if(*(pp+p*41+0)!=0)
	 				     {
						     h=WyszukujPortElement(t_te,_Size_te,port);
						     if(h!=-1)
						  	 {
	 					         if(((Const.s_Ma[p].mail[h/16]>>(h-16*(h/16)))&0x01)>0)
	 				             {
	 				                if(i_mail[p]+strlen(buf_p)<_Dlg_buf_Mail)
				    	            {
	 					               sprintf(&b_mail[_Dlg_buf_Mail*p+i_mail[p]],buf_p);
	 					               i_mail[p]= i_mail[p]+strlen(buf_p);
						               m=1;
	 					            }
	 				    	        else{ dbg3("\r\ni_mail !!!"); full2=1; goto koniec_zda_full; }
	 				             }
						  	 }

	 				     }
	 				   }
	 				}
		     }

	    	j+=3;

	     }
		 else if(sms_wsk[3*i+0]=='W')    //------Wejscia
	     {
			 g=WyszukujPortElement(t_we, _Size_we, sms_wsk[3*i+1]);
			 if(g!=-1)
			 {
			      if(sms_wsk[3*i+2]==1){ sprintf(buf_p,"\r\n%s %s", Const.s_WE[g].nazwa, Const.s_WE[g].nazwa_ro);   }  //rozwarcie
	    	 else if(sms_wsk[3*i+2]==2){ sprintf(buf_p,"\r\n%s %s", Const.s_WE[g].nazwa, Const.s_WE[g].nazwa_zw);  }  //zwarcie
	    	 else if(sms_wsk[3*i+2]==3){ sprintf(buf_p,"\r\n%s %s", Const.s_WE[g].nazwa, Const.s_WE[g].nazwa_uszk);  }  //zwarcie




	    	      if(start==1){
	 				 pp= Const.s_Tel[0].tel1;
	                 port= sms_wsk[3*i+1];
	                 if((Const.s_GPIO[port-1].val==4)||(Const.s_GPIO[port-1].val==13))
	                 {
	 				   for(p=0;p<_Ilosc_Tel;p++)
	 				   { if(*(pp+p*17+3)!='0')
	 				     {
						     h=WyszukujPortElement(t_te,_Size_te,port);
						     if(h!=-1)
						  	 {
	 					        if(((Const.s_Ma[p].sms[h/16]>>(h-16*(h/16)))&0x01)>0)
	 				            {
	 				               if(i_sms[p]+strlen(buf_p)<160)
				    	           {
	 					              sprintf(&b_sms[160*p+i_sms[p]],buf_p);
	 					              i_sms[p]= i_sms[p]+strlen(buf_p);
	 					              s=1;
	 					              if(Const.s_Dzw[0].ma[p]>0) Var.DzwMa[p]=1;
	 					           }
	 				    	       else{ dbg3("\r\ni_sms !!!"); /*sprintf(&buf_p[200],"\r\ni_sms !!!  %d-%d   ",p,i_sms[p]); dbg3(&buf_p[200]);*/  full=1; goto koniec_zda_full; }
	 				            }
						  	 }

	 				     }
	 				   }
	 				}}


	    	        if(zgoda>0){
	                     pp= Const.s_Email[0].odb1;
		                 port= sms_wsk[3*i+1];
		                 if((Const.s_GPIO[port-1].val==4)||(Const.s_GPIO[port-1].val==13))
		                 {
		 				   for(p=0;p<_Ilosc_Mail;p++)
		 				   { if(*(pp+p*41+0)!=0)
		 				     {
							     h=WyszukujPortElement(t_te,_Size_te,port);
							     if(h!=-1)
							  	 {
		 					        if(((Const.s_Ma[p].mail[h/16]>>(h-16*(h/16)))&0x01)>0)
		 				            {
		 				               if(i_mail[p]+strlen(buf_p)<_Dlg_buf_Mail)
					    	           {
		 					              sprintf(&b_mail[_Dlg_buf_Mail*p+i_mail[p]],buf_p);
		 					              i_mail[p]= i_mail[p]+strlen(buf_p);
		 					              m=1;
		 					           }
		 				    	       else{ dbg3("\r\ni_mail !!!"); full2=1; goto koniec_zda_full; }
		 				            }
							  	 }

		 				     }
		 				   }
		 				}
		    	    }



			 }
	         j+=3;
	     }
 /*  #ifdef _LoRa
		 else if(sms_wsk[3*i+0]=='L')    //------  LoRa Wejscia
	     {
			 // sms_wsk[3*i+1]  -> nr czujki
			 // sms_wsk[3*i+2]  -> 0x7F & nr wejscia dla danej czujki       0x80 &     1 zwarcie   0 rozwarcie

              p=sms_wsk[3*i+1];
			  if((p==5)||(p==22)||(p==23))
		      {
			 	        if(p==5)  sprintf(&buf_p[150]," portiernia");
			 	   else if(p==22) sprintf(&buf_p[150]," pokoj");
			 	   else if(p==23) sprintf(&buf_p[150]," sekretariat");
			 	   p=5;
			  }
		      else if(p==21) sprintf(&buf_p[150]," sekretariat");
		      else           sprintf(&buf_p[150]," portiernia");


	    	     if(p==1) sprintf(&buf_p[100]," 'Olimp'");
	    	else if(p==2) sprintf(&buf_p[100]," 'Babilon'");
	    	else if(p==3) sprintf(&buf_p[100]," 'Akropol'");
	    	else if(p==4) sprintf(&buf_p[100]," 'Filutek'");
	    	else if(p==5) sprintf(&buf_p[100]," 'Strumyk'");
	    	else if(p==6) sprintf(&buf_p[100]," 'Bratek'");
	    	else if(p==7) sprintf(&buf_p[100]," 'Zascianek'");
	    	else if(p==8) sprintf(&buf_p[100]," 'Stokrotka'");
	    	else if(p==9) sprintf(&buf_p[100]," 'Omega'");
	    	else if(p==10) sprintf(&buf_p[100]," 'Hajduczek'");
	    	else if(p==11) sprintf(&buf_p[100]," 'Bonus'");
	    	else if(p==12) sprintf(&buf_p[100]," 'Promyk'");
	    	else if(p==13) sprintf(&buf_p[100]," 'Straszny Dwor'");
	    	else if(p==14) sprintf(&buf_p[100]," 'Kapitol'");
	    	else if(p==15) sprintf(&buf_p[100]," 'Maraton'");
	    	else if(p==16) sprintf(&buf_p[100]," 'Itaka'");
	    	else if(p==17) sprintf(&buf_p[100]," 'Arkadia'");
	    	else if(p==18) sprintf(&buf_p[100]," ");
	    	else if(p==19) sprintf(&buf_p[100]," ");
	    	else if(p==20) sprintf(&buf_p[100]," 'Alfa'");
	    	else if(p==21) sprintf(&buf_p[100]," 'Budynek Administracyjny'");


			       if(sms_wsk[3*i+2]==(1|0x80)) sprintf(buf_p,"\r\nDS-%d%s%s:  ALARM II st.",p,&buf_p[100],&buf_p[150]);
			  else if(sms_wsk[3*i+2]==(2|0x80)){ if(sms_wsk[3*i+1]==21) sprintf(buf_p,"\r\nDS-%d%s%s:  ALARM WLAMANIOWY",p,&buf_p[100],&buf_p[150]);  else sprintf(buf_p,"\r\nDS-%d%s%s:  ALARM I st.",p,&buf_p[100],&buf_p[150]);  }
			  else if(sms_wsk[3*i+2]==(3|0x80)){ if(sms_wsk[3*i+1]==23) sprintf(buf_p,"\r\nDS-%d%s%s:  NAPAD",p,&buf_p[100],&buf_p[150]);  else sprintf(buf_p,"\r\nDS-%d%s%s:  NAPAD",p,&buf_p[100],&buf_p[150]);   }
			  else if(sms_wsk[3*i+2]==1)        sprintf(buf_p,"\r\nDS-%d%s%s:  alarm II st. odwolany ",p,&buf_p[100],&buf_p[150]);
			  else if(sms_wsk[3*i+2]==2)       { if(sms_wsk[3*i+1]==21) sprintf(buf_p,"\r\nDS-%d%s%s:  alarm wlamaniowy odwolany",p,&buf_p[100],&buf_p[150]);  else sprintf(buf_p,"\r\nDS-%d%s%s:  alarm I st. odwolany",p,&buf_p[100],&buf_p[150]);   }
			  else if(sms_wsk[3*i+2]==3)       { if(sms_wsk[3*i+1]==23) sprintf(buf_p,"\r\nDS-%d%s%s:  alarm wlamaniowy odwolany",p,&buf_p[100],&buf_p[150]);  else sprintf(buf_p,"\r\nDS-%d%s%s:  napad odwolany",p,&buf_p[100],&buf_p[150]);  }


		    	      if(start==1){
		 				 pp= Const.s_Tel[0].tel1;
		 				   for(p=0;p<_Ilosc_Tel;p++)
		 				   { if(*(pp+p*17+3)!='0')
		 				     {
		 				               if(i_sms[p]+strlen(buf_p)<160)
					    	           {
		 					              sprintf(&b_sms[160*p+i_sms[p]],buf_p);
		 					              i_sms[p]= i_sms[p]+strlen(buf_p);
		 					              s=1;
		 					           }
		 				    	       else{ dbg3("\r\ni_sms !!!");   full=1; goto koniec_zda_full; }
		 				     }
		 				   }
		 				}



		                     pp= Const.s_Email[0].odb1;
			 				   for(p=0;p<_Ilosc_Mail;p++)
			 				   { if(*(pp+p*41+0)!=0)
			 				     {

			 				               if(i_mail[p]+strlen(buf_p)<_Dlg_buf_Mail)
						    	           {
			 					              sprintf(&b_mail[_Dlg_buf_Mail*p+i_mail[p]],buf_p);
			 					              i_mail[p]= i_mail[p]+strlen(buf_p);
			 					              m=1;
			 					           }
			 				    	       else{ dbg3("\r\ni_mail !!!"); full2=1; goto koniec_zda_full; }

			 				     }
			 				   }



		 	         j+=3;

	     }
   #endif*/




	  }

	  if(j==0)
	  {
		     for(p=0;p<_Ilosc_Tel;p++)
			 {
		    	if(i_sms[p]>0) full=1;
			 }

		     for(p=0;p<_Ilosc_Mail;p++)
		     {
		     	   if(i_mail[p]>0) full2=1;
		     }

	  }

	  if(j>0){  if(s==1) full=1;   if(m==1) full2=1;  }
	  koniec_zda_full:
	  if((full==1)||(full2==1))
	  {
	     for(i=0;i<(_Size_sms_wsk-1-j);i++)   //usuwanie wpisów zdarzen, ktore zostalu juz wyzej zinterpretowane
	     {
		    sms_wsk[i]=sms_wsk[j+i];
	     }
		 sms_wsk[_Size_sms_wsk-1] = sms_wsk[_Size_sms_wsk-1]-j;   // w  'sms_wsk[_Size_sms_wsk-1]' jest liczba wpisów zdarzen


	     for(p=0;p<_Ilosc_Tel;p++)
		 {
	    	if(i_sms[p]>0) b_sms[160*p+i_sms[p]]=0x1A;
		 }

	     for(p=0;p<_Ilosc_Mail;p++)
	     {
	     	   if(i_mail[p]>0) b_mail[_Dlg_buf_Mail*p+i_mail[p]]=0;
	     }





		 //dbg3(buf_sms);  buf_sms[0]=0;



	    if(full==1){   if(start==1){    Wysylka_SMS_na_numery(0);  } }

		if(full2==1) wybor_bitowy|=0x0800;  //czekamy az sms`y zostana wyslane i wtedy obsluga email   //Wysylka_Mail_na_skrzynki(0);

/*
			  if(Const.s_Tel[0].tel1[3]!='0'){ wysylka_sms=1; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel1);   }
		 else if(Const.s_Tel[0].tel2[3]!='0'){ wysylka_sms=2; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel2);   }
		 else if(Const.s_Tel[0].tel3[3]!='0'){ wysylka_sms=3; ResumptionReceiveDataFromGSM(); send_SMS(Const.s_Tel[0].tel3);   }
*/
	  }

  }}


  if((wysylka_sms==0)&&(wysylka_mail==0))
  {  //dbg3("x");
      if(koniec_dzwon==1) dzwon=0;
	  if((start==1)&&(wysylka_sms==0)&&(li_sms==0)&&(dzwon==0)&&(idx_email_GPRS==0))
	  {
	     for(i=0;i<_Ilosc_Tel;i++)
         {
	         if(Var.DzwMa[i]>0)
	         {   dbg3("\r\nZadanie Dzwonienia...");
	        	 delay_funkc[27]=1;
	        	 dzwon=1;
	        	 param_dzwon=i;
	        	 koniec_dzwon=0;
	        	 goto omindsfdf333;

	         }
         }
	     dzwon=0; koniec_dzwon=0;
	     omindsfdf333:
		 asm("nop");
	  }
  }



}



void ObslugaSMS()
{
  int k,i,j;

	  //if(wysylka_sms>0)
	 // {
		  k= Wysylka_SMS_na_numery(wysylka_sms);
		  if(k==0)
		  {     wysylka_sms=0;
		        for(j=0;j<_Ilosc_Tel;j++){ i_sms[j]=0; for(i=0;i<160;i++) b_sms[i+160*j]=0; }
	      }
	 // }
}

void ObslugaMail()
{
  int k,i,j;

	  if((wysylka_mail>0)&&(wysylka_sms==0))
	  {
		  k= Wysylka_Mail_na_skrzynki(wysylka_mail);
		  if(k==0)
		  {     wysylka_mail=0;
		        for(j=0;j<_Ilosc_Mail;j++){ i_mail[j]=0; for(i=0;i<_Dlg_buf_Mail;i++) b_mail[i+_Dlg_buf_Mail*j]=0; }
	      }
	  }
}

void WskazPortHttp(char *ptr1)
{
   int i;    char *ptr;
  	    i=0;
     		  		  porrr4:
     		  		  if(*(ptr1-i++)=='\n'); else goto porrr4;
     		  		  ptr=(ptr1-i); ptr1=strstr(ptr,"\r\n+IPD,");
     		  		  HttpPort=ptr1[7];
}
void TM_GPIO_SetPinAsInputi(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
  	uint8_t i;
  	i=0; while(GPIO_Pin>>=1) i++;
   // GPIOx->MODER &= ~(0x03 << (2 * i));
}
void TM_GPIO_SetPinAsOutputi(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	uint8_t i;
	i=0; while(GPIO_Pin>>=1) i++;
   // GPIOx->MODER = (GPIOx->MODER & ~(0x03 << (2 * i))) | (0x01 << (2 * i));

}

void RstWifi()
{
    	  dbg3("\r\nRST wifi ");     wybor_bitowy&=~0x8000;    delay_funkc[34]=1;

    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);  //WIFI   CH_PD
    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);  //start reset WIFI
    	  HAL_Delay(5000);
    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
    	  licz_htt=0;

    	    dbg3("  ok.. ");  // delay_funkc[1]=1;
}

void debugPrintHexTable(unsigned short length, unsigned char *buffer)
{
	unsigned char i;
	unsigned short j;
	unsigned char *buf;
	unsigned char s;

	buf = buffer;

	dbg3("\r\n");
	// print the low order address indicies and ASCII header
	dbg3("     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF\r\n");
	dbg3("     -----------------------------------------------  ---- ASCII -----\r\n");

	// print the data
	for(j=0; j<((length+15)>>4); j++)
	{
		// print the high order address index for this line
		dbg3u16(j<<4);
		dbg3(" ");

		// print the hex data
		for(i=0; i<0x10; i++)
		{
			// be nice and print only up to the exact end of the data
			if( ((j<<4)+i) < length)
			{
				// print hex byte
				dbg3u08(buf[(j<<4)+i]);
				dbg3(" ");
			}
			else
			{
				// we're past the end of the data's length
				// print spaces
				dbg3("   ");
			}
		}

		// leave some space
		dbg3(" ");

		// print the ascii data
		for(i=0; i<0x10; i++)
		{
			// be nice and print only up to the exact end of the data
			if( ((j<<4)+i) < length)
			{
				// get the character
				s = buf[(j<<4)+i];
				// make sure character is printable
				if(s >= 0x20)
				    while(HAL_UART_Transmit(&huart3,&s,1,10)==HAL_BUSY);
				else
					dbg3(".");
			}
			else
			{
				// we're past the end of the data's length
				// print a space
				//rprintf("\r\n");
			}
		}
		dbg3("\r\n");
	}

}

void debugPrintHexTable_(unsigned short length, unsigned char *buffer)
{
	unsigned char i;
	unsigned short j;
	unsigned char *buf;
	unsigned char s;   int sx;

	buf = buffer;
    sx=0;

	sprintf(&buf_rx2[_Size_BufWifi+sx],"\r\n");   sx+=strlen(&buf_rx2[_Size_BufWifi+sx]);
	// print the low order address indicies and ASCII header
	sprintf(&buf_rx2[_Size_BufWifi+sx],"     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF\r\n");   sx+=strlen(&buf_rx2[_Size_BufWifi+sx]);
	sprintf(&buf_rx2[_Size_BufWifi+sx],"     -----------------------------------------------  ---- ASCII -----\r\n");   sx+=strlen(&buf_rx2[_Size_BufWifi+sx]);

	// print the data
	for(j=0; j<((length+15)>>4); j++)
	{
		// print the high order address index for this line

		buf_rx2[_Size_BufWifi+sx]=hex_to_ASCII(((j<<4)>>12)&0x000F);  sx++;
		buf_rx2[_Size_BufWifi+sx]=hex_to_ASCII(((j<<4)>>8)&0x000F);   sx++;
		buf_rx2[_Size_BufWifi+sx]=hex_to_ASCII(((j<<4)>>4)&0x000F);   sx++;
		buf_rx2[_Size_BufWifi+sx]=hex_to_ASCII((j<<4)&0x000F);        sx++;

		buf_rx2[_Size_BufWifi+sx]=' ';   sx++;

		// print the hex data
		for(i=0; i<0x10; i++)
		{
			// be nice and print only up to the exact end of the data
			if( ((j<<4)+i) < length)
			{
				// print hex byte
				buf_rx2[_Size_BufWifi+sx]=hex_to_ASCII(((buf[(j<<4)+i])>>4)&0x0F);   sx++;
				buf_rx2[_Size_BufWifi+sx]=hex_to_ASCII((buf[(j<<4)+i])&0x0F);        sx++;

				buf_rx2[_Size_BufWifi+sx]=' ';   sx++;
			}
			else
			{
				// we're past the end of the data's length
				// print spaces
				buf_rx2[_Size_BufWifi+sx]=' ';   sx++;
				buf_rx2[_Size_BufWifi+sx]=' ';   sx++;
				buf_rx2[_Size_BufWifi+sx]=' ';   sx++;
			}
		}

		// leave some space
		buf_rx2[_Size_BufWifi+sx]=' ';   sx++;

		// print the ascii data
		for(i=0; i<0x10; i++)
		{
			// be nice and print only up to the exact end of the data
			if( ((j<<4)+i) < length)
			{
				// get the character
				s = buf[(j<<4)+i];
				// make sure character is printable
				if(s >= 0x20){
				    //while(HAL_UART_Transmit(&huart3,&s,1,10)==HAL_BUSY);
					 buf_rx2[_Size_BufWifi+sx]=s;   sx++;   }
				else{
					buf_rx2[_Size_BufWifi+sx]='.';   sx++;  }
			}
			else
			{
				// we're past the end of the data's length
				// print a space
				//rprintf("\r\n");
			}
		}
		buf_rx2[_Size_BufWifi+sx]='\r';   sx++;
		buf_rx2[_Size_BufWifi+sx]='\n';   sx++;
		buf_rx2[_Size_BufWifi+sx]=0;

		if(sx>2000) dbg3("\r\nDBG  PRZEKROCZONE !!! ");
	}

}

void zmiana_ekranu()
{
		 if(WhichPage==13)  //   ../czas
		 {
			 Const.s_Poz[0].ekran++; ekran_bit&=~0x01;  if(Const.s_Poz[0].ekran>(_Ile_Ekranow-1)) Const.s_Poz[0].ekran=0;
			ZapiszFlash();
			WhichPage=79;
		 }
}

void HttpAuthEsp(char *pp)
{
	   sprintf(buf_p,"%s:%s",Const.s_Log[0].login, Const.s_Log[0].haslo);
	  	  base64_encode(buf_p, &buf_p[50], strlen(buf_p));   // rrprintf("\r\n%s\r\n",buf_p);
	  	  sprintf(&buf_p[100],"Authorization: Basic %s", &buf_p[50]);
	  	  if(strstr(buf_rx2,&buf_p[100]));
	  	  else
	  	  {
	  		  WhichPage=20;
	  	  }
}

void WpisDatyNaPodtrzymanie()
{
	 RTC_DateTypeDef sdatestructure;
	 HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, sdatestructure.Month);
	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, sdatestructure.Date);
	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, sdatestructure.Year);
}

void GprsSend(char *px)
{
    int i,j;  char *pg;
	HAL_Delay(300);
    dbg3(buf_rx);
    if(idx_email_GPRS!=9999){  idx_email_GPRS++;  delay_funkc[26]=1;    ekran_tymczasowy=3; }


    if(pg=strstr(buf_rx,"+MIPSEND:"))  //+MIPSEND: 1,0,2048
    {
    	pg=pg+20;
    }
    else  pg=&buf_rx[0];

    i=0; j=0;
    while(pg[i]!=0)
    {   if(pg[i]=='\r') info_email[j++]=' ';
        else if(pg[i]=='\n');
        else info_email[j++]=pg[i];
        i++;  if(i>73) break;
    }
    info_email[j]=0;
   // dbg3("\r\n#######< "); dbg3(info_email);  dbg3("\r\n#######> ");

   #ifdef _LCD_TFT
     if(ekran_tymczasowy==0){
    	 if(idx_email_GPRS!=9999) DrawInfoE("Wysylam E-mail");
     }
   #endif

   ResumptionReceiveDataFromGSM();
   dbg(px);
}



void modem_uart()
{
	if(email_enc>0) goto omin_mmm1;

	   if(li_WifiCipsend[0]>6)
	   {
		   li_WifiCipsend[0]=0; // InitUartWifi();
	   }

	   if(li_WifiCipsend[1]>6)
	   {
		   li_WifiCipsend[1]=0; // InitUartWifi();
	   }


    TasksRealize();       WpisPortElement();



    omin_mmm1:

  if(wsk_1s==1)  //cosek
  {


	  for(j=0;j<_Size_wy;j++)
	  {
	    	  if(flaga_pk_[j]==1)
	    	  {
	    		  if(HAL_GetTick()-sms_czas_pk_start[j] > (sms_czas_pk[j]*1000))
	    		  {
	    			  Const.s_WY[0].val&=~(1<<j);  GPIO_PK(j);  ZapisZdarzenia(_ZdaPKsms,_PKsmsoff,j);
	    			  flaga_pk_[j]=0;
	    		  }
	    	  }
	  }




	  if((wybor_bitowy&0x8000)>0)  //wifi server ok
	  {
		 if(uartwifirst>(60*10))
	     {
		     if((email_enc==0)&&(email==0))
		     {
		    	 SendDataToModemWifi("AT\x0D\x0A");    delay_funkc[31]=1;  dbg3("\r\nSEND AT wifi ...");   uartwifirst=0;  ResetWifiAT=0;  ResumptionReceiveDataFromDebug();
		     }
	     }
	     else uartwifirst++;

		 if(((uartwifirst%2)==0)&&(uartwifirst>2))
		 {

#ifdef _CLOUD
   if(cload_activ==0)
   {	 	dbg3("\r\nWYSYLAM SSSSSSSS ");  cload_activ=1;

			if(Polacz_z_IP("192.168.1.32")==0) dbg3("  OK!!!  ");  else{ cload_activ=0; goto koniecop7;  }
			dbg3("\r\nDDDDDDDDDDDDDDDDD ");
			if(WyslijKomend("GET /TME.txt")==0) dbg3("  SSSSSS!!!  ");  else{ cload_activ=0;  goto koniecop7; }

		     koniecop7:
		     dbg3("\r\nKoniec_op11  ");
   }
#endif

		 }

	  }



      if(koniec_dzwon>1) koniec_dzwon--;

	  WpisDatyNaPodtrzymanie();

	  if(email_enc==0){ if((wybor_bitowy&0x0080)>0) Mrug(3);  }


	  if((wybor_bitowy&0x0400)>0)
	  {
		  if((wybor2_bitowy&0x0200)==0)  //Wpis czasu STM`a do GSM`a tylko raz przy restarcie
		  {   wybor2_bitowy|=0x0200;
		      PobierzCzas(buf_p);
		      #ifdef _Fibocom
		 	    sprintf(buf_p,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+00\"\x0D",buf_p[0],buf_p[1],buf_p[2], buf_p[4],buf_p[5],buf_p[6]);   // +CCLK: "18/02/26,09:42:01+04"
		 	    dbg(buf_p);dbg3(buf_p);
		 	    HAL_Delay(300);
		 	    dbg3(buf_rx);
		      #endif
		  }

		 ObslugaLicznikow();

		 for(i=0;i<_Size_we;i++)
	     {
		    ServiceWE(i);
	     }

    	 for(i=0;i<_Size_te;i++)  pokaz_temp(t_te[i]);
    	 pokaz_ADC_pt();
    	 pokaz_ADC_TT();
    	 pokaz_ADC_eol();

	     sprawdz_prog_temp();  dbg3("i");

	     if((email_enc==0)&&(email==0)&&(delay_funkc[31]==0)&&(delay_funkc[32]==0)){    wyslij_zdarzenia();  }

	     ObslugaTermostatu();
	     WejsciaSMS();


		  RewriteLogicInputs();
		  FunctionLogicOperation();


#ifdef _REJ
		 if(((Const.s_Rej[0].param&0x01)>0)&&((Const.s_Rej[0].param&0x02)==0))  //Rejestrator ON i kontynyowany
		 {

			 j=0;
			 for(i=0;i<_Size__s_gpio;i++)   //czy sa jakies czujniki temp wilg zaznaczone aby z nich zapisywac pomiary
			 {
			     if(WyszukajNrCzujkiDlaPortu(i+1)!=-1){ j=1; break; }
			 }

             if(j==1)
             {
			    rej_tim++;
			    if(rej_tim > Const.s_Rej[0].intr)
			    {
				   //ZapisPomarowFlashZewn();
				   ZakonczPomiaryTempAvr();
				   rej_tim=1;
			    }
			    PomiaryTempAvr();
             }
             else
               Var.wykres_nr=0xff;

		 }
		 else rej_tim=1;
#endif


		 if(Lora_send_cycle<10000) Lora_send_cycle++;


	 }

	 if(email_enc>0) goto omin_mmm2;

	    if(start==1)
	    {
	    	if(li_sms>0)
	    	{
	    		sprintf(buf_p,"\r\nSMS: %d",li_sms);  dbg3(buf_p);
	    	}
	    	if(li_sms==1)
	    	{
	    	   li_sms=0;  InitUartGSM();  HAL_Delay(200);
	    	   if((wysylka_sms>0)&&(wysylka_sms<10)){ wysylka_sms--; ObslugaSMS();  }
	    	   else if(wysylka_sms==99){ delay_funkc[9]=1; }
	    	}
	    }

	   // StringFont5x7("i",1);



	  if((wybor_bitowy&0x0020)==0)  //gsm tak
	  {
		  if(((Const.s_Lan[0].param&0x04)>0)||((Const.s_Lan[0].param&0x08)>0))  //STA tak  lub   AP tak
		  {
			  if((start==1)&&((wybor_bitowy&0x10)>0)) wyswietl_ekran();  //GSM zalogowany  i   Server OK
		  }
		  else   //STA nie  i  AP nie
		  {
			  if(start==1) wyswietl_ekran();
		  }
	  }
	  else   //gsm nie
	  {
		  if(((Const.s_Lan[0].param&0x04)>0)||((Const.s_Lan[0].param&0x08)>0))  //STA tak  lub   AP tak
		  {
			  if((wybor_bitowy&0x10)>0) wyswietl_ekran();      //Server OK
		  }
		  else
		  {
			  wyswietl_ekran();
		  }

	  }

	  if(RotacjaOnCzyOFF()==1) Rotacja();


/*
	NRF_Start();
	  if(NRF_odbierz(buf_p,10)==1)
	  {
	      if(strstr(buf_p,"MasterOdbieraRAM"))
	      {
	    	  if(NRF_SlaveWysylaRAM()==1);
	      }
	      else if(strstr(buf_p,"MasterWysylaRAM"))
	      {
	    	  if(NRF_SlaveOdbieraRAM()==1);
	      }
	  }
    NRF_Stop();
#endif
*/
    #ifdef _LoRa   //synch
	  LoraSynchro++;
	  if(LoraSynchro>_LoRaSynch)
	  {
		  LoraSynchro=0;
		 // Lora_Send_Synchr(); dbg3("\r\nSynchro  ");
	  }
    #endif



    omin_mmm2:
    wsk_1s=0;
  }



  if(wsk_2s==2)  //co2sek
  {
	  if((wybor_bitowy&0x0400)>0)
	  {
	     /*for(i=0;i<_Size__s_gpio;i++)
	     {
		    pokaz_DHT11(i+1);
	     }*/
	     for(i=0;i<_Size_te;i++) pokaz_DHT11(t_te[i]);



		 if((wysylka_sms==0)&&(wysylka_mail==0)&&(dzwon==0)&&(idx_email_GPRS==0))
		 {    if((li_sms==0)&&(gsm_flag==0))
			           ObslugaTimerRaport();
		 }
		 if((wysylka_sms==0)&&(wysylka_mail==0)&&(dzwon==0)&&(idx_email_GPRS==0))
		 {    if((li_sms==0)&&(gsm_flag==0))
			           ObslugaTimerRejZda();
		 }


	  }
	 wsk_2s=0;
  }


#ifndef _Fibocom   //TU DO WYMAZANIA !!!!!!!!!!!!!!!  na chwile wylaczony GSM
  wybor_bitowy|=0x0020; //GSM OFF
  wybor_bitowy|=0x0400;   //Rozpoczecie pomiarów
	gsm_flag=0;  //odpytywanie CSQ i CCLK
#endif

//#ifdef _Master

  if(email_enc>0) return;





    //######################  -- Modem GSM ---- ####################################################
  if(start==0)  //GSM ON/OFF
  {  if((wybor_bitowy&0x0020)==0){
	 if(buf_rx[0]>0)
     { HAL_Delay(100);
	    if((wybor2_bitowy&0x0020)==0)
        {
		   wybor2_bitowy|=0x0020;
		   adr_flash_SesjaLogGSM=0;
		   AT25SF_3_BlockErase(_Log_GSM_Start); HAL_Delay(100);
        }
	    ZapisSesjiLogGSM(buf_rx, strlen(buf_rx), &adr_flash_SesjaLogGSM);
     }}
  }


#ifdef _Fibocom
  if(ptr=strstr(buf_rx,"+CCLK: \""))   //+CCLK: "18/09/30,16:06:07+04"      //+CCLK: "18/09/30,14:07:54+08"
  {
	  HAL_Delay(10);  dbg3("\r\n"); dbg3(ptr);       // 2*((( 10*(*(ptr+26)&0x0f)+(*(ptr+27)&0x0f) )/4)-1)
	  gsm_flag=0;

      if( ((*(ptr+26)>0x29)&&(*(ptr+26)<0x3A)) && ((*(ptr+27)>0x29)&&(*(ptr+27)<0x3A)) )
  	  {
  		  i= 10*(*(ptr+26)&0x0f)+(*(ptr+27)&0x0f);
  	  }
  	  else
  	  {
  		  i= (*(ptr+26)&0x0f);
  	  }
      if(*(ptr+25)=='-') i= i*(-1);
      if(i!=0)
      {

    		 date.year=2000 + 10*(*(ptr+8)&0x0f)+(*(ptr+9)&0x0f);
    		 date.month=      10*(*(ptr+11)&0x0f)+(*(ptr+12)&0x0f);
    		 date.day=        10*(*(ptr+14)&0x0f)+(*(ptr+15)&0x0f);

    		 date.hours=      10*(*(ptr+17)&0x0f)+(*(ptr+18)&0x0f);
    		 date.minutes=    10*(*(ptr+20)&0x0f)+(*(ptr+21)&0x0f);
    		 date.seconds=    10*(*(ptr+23)&0x0f)+(*(ptr+24)&0x0f);

    	     time=convertDateToUnixTime();
    	     time+=(15*60*i);
   	         convertUnixTimeToDate(time);

   	         UstawCzas(date.year-2000, date.month, date.day,    date.hours, date.minutes, date.seconds);


      }
      else
      {
    	  UstawCzas(10*(*(ptr+8)&0x0f)+(*(ptr+9)&0x0f), 10*(*(ptr+11)&0x0f)+(*(ptr+12)&0x0f), 10*(*(ptr+14)&0x0f)+(*(ptr+15)&0x0f),  10*(*(ptr+17)&0x0f)+(*(ptr+18)&0x0f), 10*(*(ptr+20)&0x0f)+(*(ptr+21)&0x0f), 10*(*(ptr+23)&0x0f)+(*(ptr+24)&0x0f));

      }
    //  HAL_Delay(20); dbg3("\r\nGSM_+CCLK: ");




  /*
	  if( ((*(ptr+26)>0x29)&&(*(ptr+26)<0x3A)) && ((*(ptr+27)>0x29)&&(*(ptr+27)<0x3A)) )
	  {
		  i= 10*(*(ptr+26)&0x0f)+(*(ptr+27)&0x0f);
	  }
	  else
	  {
		  i= (*(ptr+26)&0x0f);
	  }
      if(*(ptr+25)=='-') i= i*(-1);   //time zone  korekcja

	  if(time_zone!=i)
	  {   sprintf(buf_p,"\r\nZMIANA CZASU:  %d -> %d   ", time_zone, i);   dbg3(buf_p);
		  ZamianaCzasuNaFlash();
	      time= time+(15*60)*(i-time_zone);   time_zone= i;
	      ZamianaCzasuZFlash(time);
	      UstawCzas(rok,mies,dzien,godz,min,sec);

	      PobierzCzas(buf_p);
	       		 sprintf(buf_p,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+08\"\x0D",buf_p[0],buf_p[1],buf_p[2], buf_p[4],buf_p[5],buf_p[6]);   // +CCLK: "18/02/26,09:42:01+04"
	       		  dbg(buf_p);dbg3(buf_p);
	       		  HAL_Delay(300);
	       		  dbg3(buf_rx);
	  }
*/



	  if(ptr=strstr(buf_rx,"\x0D\x0A>"))   //>
	  {
		  dbg3("\r\nGSM:  Wysylam SMS o tresci:\r\n"); //dbg3(buf_sms);   dbg3("\r\n\r\n");
		  ResumptionReceiveDataFromGSM();
		  if((wysylka_sms>0)&&(wysylka_sms<10)){
			    for(i=0;i<160;i++) buf_sms[i]=b_sms[160*(wysylka_sms-1)+i];   /*i_sms[wysylka_sms-1]=0;*/   }
		  send_buf_sms();  HAL_Delay(500);

	  }

	  ResumptionReceiveDataFromGSM();

  }
#endif


if((wybor_bitowy&0x0020)==0)  //GSM ON
{

#ifdef _Neoway
  if(strstr(buf_rx,"MODEM:STARTUP"))   //gsm
  {
	  ResumptionReceiveDataFromGSM();
	  dbg3("\r\nGSM:  MODEM:STARTUP");
	  nr_kom=0;
	  if(nr_kom==0){  nr_kom=__ATE0;  wpis_komend(nr_kom);  HAL_Delay(100);  dbg(at_send); delay_funkc[5]=1;  }
  }
#endif

#ifdef _Fibocom
  if((strstr(&buf_rx[1],"AT command ready"))||(strstr(&buf_rx[0],"AT command ready")))   //gsm
  {
	  ResumptionReceiveDataFromGSM();
	  dbg3("\r\nGSM:  AT command ready");
	  delay_funkc[20]=0;

	  licz_cpin=0;
	  delay_funkc[5]=0;
		nr_kom=0;
		nr_kom2=0;
		li_sms=0;
		start=0;
		licz_start=0;

	  if(nr_kom==0){  nr_kom=__ATE0;  wpis_komend(nr_kom);  HAL_Delay(100);  dbg(at_send); delay_funkc[5]=1;  }
  }
#endif


  if(strstr(buf_rx,"+PBREADY"))
  {
	 // ResumptionReceiveDataFromGSM();
	 // dbg3("\r\n+PBREADY");

  }



  if(ptr=strstr(buf_rx,"+CMT:"))   //Recv SMS
  { if(li_sms==0)
    {
	  HAL_Delay(300);
	  dbg3(ptr);

	/*  if(strstr(ptr,"ZGR"))  //+CDS: 49,5,"+48889691210",145,"17/06/27,08:33:50+08","17/06/27,08:33:53+08",0
	  {
	     if(ptr2=strstr(ptr,"\","))    ptr2+=2;  else{ ResumptionReceiveDataFromGSM(); goto ominsdsd; }
	     if(ptr1 =strstr(ptr2,",\""))  ptr1+=2;  else{ ResumptionReceiveDataFromGSM(); goto ominsdsd; }
	     //UstawCzas(, , ,  , , );
	     czas[0]= 10*(*(ptr1+0)&0x0f)+(*(ptr1+1)&0x0f);
	     czas[1]= 10*(*(ptr1+3)&0x0f)+(*(ptr1+4)&0x0f);
	     czas[2]= 10*(*(ptr1+6)&0x0f)+(*(ptr1+7)&0x0f);
	     czas[4]= 10*(*(ptr1+9)&0x0f)+(*(ptr1+10)&0x0f);
	     czas[5]= 10*(*(ptr1+12)&0x0f)+(*(ptr1+13)&0x0f);
	     czas[6]= 10*(*(ptr1+15)&0x0f)+(*(ptr1+16)&0x0f);
	     sprintf(buf_p,"\r\nCzas: %d/%d/%d  %d:%d:%d ",czas[0],czas[1],czas[2],czas[4],czas[5],czas[6]); dbg3(buf_p);
	     delay_funkc[7]=1;
	     wybor_bitowy|=0x0040;
	     ResumptionReceiveDataFromGSM();
	  }
	  ominsdsd:*/

	  if(ptr1=strstr(buf_rx,"+CMGS:"))
	  {
		  ResumptionReceiveDataFromGSM();
		  HAL_Delay(200);
		  dbg3("\r\nkoniec SMS");
		  if(li_sms>0) li_sms=0;
		  if((wysylka_sms>0)&&(wysylka_sms<10))  ObslugaSMS();
		  else if(wysylka_sms==99) wysylka_sms=0;
		  else{ for(i=0;i<160;i++) buf_sms[i]=0; }
		  ekran_tymczasowy=2;    sms_reply_end=0;
	  }

	  kopiuj_nr_tel(ptr+7);
	  sprawdz_SMS(ptr+5);
	  //ResumptionReceiveDataFromGSM();

    }
  }
  if(ptr=strstr(buf_rx,"+CDS:"))
  {
	  HAL_Delay(50);     //Tu moze buc tez aktualizacja czasu
	  dbg3(ptr+0);
	 /* if(strstr(ptr,Const.s_Tel[0].tel0))  //+CDS: 49,5,"+48889691210",145,"17/06/27,08:33:50+08","17/06/27,08:33:53+08",0
	  {
	     if(ptr2=strstr(ptr,"\","))    ptr2+=2;  else goto ominsdsd2;
	     if(ptr1 =strstr(ptr2,",\""))  ptr1+=2;  else goto ominsdsd2;
	     //UstawCzas(10*(*(ptr1+0)&0x0f)+(*(ptr1+1)&0x0f), 10*(*(ptr1+3)&0x0f)+(*(ptr1+4)&0x0f), 10*(*(ptr1+6)&0x0f)+(*(ptr1+7)&0x0f),  10*(*(ptr1+9)&0x0f)+(*(ptr1+10)&0x0f), 10*(*(ptr1+12)&0x0f)+(*(ptr1+13)&0x0f), 10*(*(ptr1+15)&0x0f)+(*(ptr1+16)&0x0f));
       #ifdef _Neoway
	      k=0;
       #endif
       #ifdef _Fibocom
          k=2;
       #endif
	     czas[0]= 10*(*(ptr1+k+0)&0x0f)+(*(ptr1+k+1)&0x0f);
	     czas[1]= 10*(*(ptr1+k+3)&0x0f)+(*(ptr1+k+4)&0x0f);
	     czas[2]= 10*(*(ptr1+k+6)&0x0f)+(*(ptr1+k+7)&0x0f);
	     czas[4]= 10*(*(ptr1+k+9)&0x0f)+(*(ptr1+k+10)&0x0f);
	     czas[5]= 10*(*(ptr1+k+12)&0x0f)+(*(ptr1+k+13)&0x0f);
	     czas[6]= 10*(*(ptr1+k+15)&0x0f)+(*(ptr1+k+16)&0x0f);
	     delay_funkc[7]=1;
	     wybor_bitowy|=0x0040;
	  }*/
	  ominsdsd2:
	  asm("nop");

	  if(ptr=strstr(buf_rx,"+CMGS:"))
	  {
		  HAL_Delay(200);
		  dbg3("\r\nkoniec SMS");
		  if(li_sms>0) li_sms=0;
		  if((wysylka_sms>0)&&(wysylka_sms<10))  ObslugaSMS();
		  else if(wysylka_sms==99) wysylka_sms=0;
		  else{ for(i=0;i<160;i++) buf_sms[i]=0; }
		  ekran_tymczasowy=2;   sms_reply_end=0;
	  }
	  if(ptr=strstr(buf_rx,"\x0D\x0A>"))   //>
	  {
		  dbg3("\r\nGSM:  Wysylam SMS o tresci:\r\n"); //dbg3(buf_sms);   dbg3("\r\n\r\n");
		  ResumptionReceiveDataFromGSM();
		  if((wysylka_sms>0)&&(wysylka_sms<10)){
			    for(i=0;i<160;i++) buf_sms[i]=b_sms[160*(wysylka_sms-1)+i];   /*i_sms[wysylka_sms-1]=0;*/   }
		  send_buf_sms();  HAL_Delay(500);

	  }

	  ResumptionReceiveDataFromGSM();

  }
  if(ptr=strstr(buf_rx,"+CSQ"))
  {
	  licz_rst=0;
	  gsm_flag=0;
	  HAL_Delay(200);
      dbg3("\r\nGSM:  CSQ ");
	  zasieg_gsm[0]= *(ptr+6);
	  if(*(ptr+7)==',') zasieg_gsm[1]=' '; else zasieg_gsm[1]= *(ptr+7);
	  zasieg_gsm[2]= 0;
	  dbg3(zasieg_gsm);

	  if((zasieg_gsm[0]=='9')&&(zasieg_gsm[1]=='9')){ sprintf(info_gsm,"Blad lub brak karty SIM !!!");  }
	  else                                            sprintf(info_gsm,"GSM: Zalogowany");

	 // ResumptionReceiveDataFromGSM();

  }
  if(strstr(buf_rx,"ERROR"))
  {

     if( idx_email_GPRS>0){ idx_email_GPRS=9999; GprsSend("AT+MIPCALL=0\x0D"); idx_email_GPRS=0; delay_funkc[26]=0;  }

	  ResumptionReceiveDataFromGSM();
	 if(nr_kom==__cpin_pyta)
	 {     if((start_lcd&0x01)==0)
	       {
		       sprintf(info_gsm,"GSM: SIM ERROR");
               #ifndef _LCD_TFT
	              StringFont5x7("GSM: SIM ERROR",14);NewLineFont5x7(14,0);
               #else
	              lcd_tft("GSM: SIM ERROR",ST7735_RED);
               #endif
	          start_lcd|=001;
	       }
	       dbg3("\r\nGSM: SIM ERROR"); licz_brak_Sim++;   licz_cpin=0; nr_kom2=0;
	 }
	 else{ dbg3("\r\nERROR");  if(nr_kom==__cpin_wpis)  wpis_pin_licz++;   /* if(nr_kom==__cnum){  nr_kom++;  wpis_komend(nr_kom); HAL_Delay(2000); dbg(at_send); delay_funkc[5]=1; }*/  }
	  if(li_sms>0)
	  {
		  li_sms=0;
		  if( sms_reply_end<10)
		  {
		     if((wysylka_sms>0)&&(wysylka_sms<10)){ wysylka_sms--; dbg3("\r\nPowtarzam wysylke SMS"); ObslugaSMS();  sms_reply_end++;  }
		     else if(wysylka_sms==99){ delay_funkc[9]=1; wysylka_sms=0;  sms_reply_end++; }
		  }
		  else
		  {
			  wysylka_sms=0;    sms_reply_end=0;
			  for(i=0;i<160;i++) buf_sms[i]=0;
			  for(j=0;j<_Ilosc_Tel;j++){ i_sms[j]=0;  for(i=0;i<160;i++) b_sms[i+160*j]=0; }
			  ekran_tymczasowy=4;
		  }
	  }

  }
  if(strstr(buf_rx,"+CPIN: READY"))
  {
	  ResumptionReceiveDataFromGSM();
	  if(nr_kom==__cpin_pyta)
	  {   nr_kom2=0;
          #ifndef _LCD_TFT
	         StringFont5x7("GSM: Pin  OK",12);NewLineFont5x7(12,0);
          #else
             lcd_tft("GSM: Pin  OK",ST7735_WHITE);
          #endif

	      dbg3("\r\nGSM:  +CPIN: READY");  nr_kom=__creg_pyta;  wpis_komend(nr_kom); HAL_Delay(2000); dbg(at_send); delay_funkc[5]=1;
	  }

  }
  if(strstr(buf_rx,"NO CARRIER")||strstr(buf_rx,"BUSY")||strstr(buf_rx,"NO ANSWER"))
  {
	  dbg3("\r\nKONIEC DZWONIENIA ");  Var.DzwMa[param_dzwon]=0;  koniec_dzwon=1;  delay_funkc[27]=0; delay_funkc[25]=0;  dzwonBlok=0;

	  ResumptionReceiveDataFromGSM();
  }
  if(strstr(buf_rx,"+SIM READY"))
  {
	  ResumptionReceiveDataFromGSM();
  }
  if(strstr(buf_rx,"+CPIN: SIM PIN"))
  {
	  ResumptionReceiveDataFromGSM();
	 if(nr_kom==__cpin_pyta){ nr_kom2=0;  dbg3("\r\nGSM:  +CPIN: SIM PIN");   nr_kom=__cpin_wpis;  wpis_komend(nr_kom); HAL_Delay(500);  dbg(at_send); delay_funkc[5]=1;  }

  }
  if(strstr(buf_rx,"+CREG: 0,0")||strstr(buf_rx,"+CREG: 0,2")||strstr(buf_rx,"+CREG: 0,3"))
  {
	  if((start_lcd&0x02)==0)
	  {  sprintf(info_gsm,"GSM: Logowanie...");

         #ifndef _LCD_TFT
            StringFont5x7("GSM: Logowanie",14);NewLineFont5x7(14,0);
         #else
            lcd_tft("GSM: Logowanie",ST7735_WHITE);
         #endif

	      start_lcd|=0x02;
	  }
	  dbg3(buf_rx);
	  ResumptionReceiveDataFromGSM();  delay_funkc[5]=1;
	  nr_kom2=0;                 //Start na rozpoczecie pomiarów    //send arp request
	       if(gsm_log_licz==10){  wybor_bitowy|=0x0400;              delay_funkc[15]=1;    gsm_log_licz=11; }
	  else if(gsm_log_licz<10) gsm_log_licz++;

  }
  if(strstr(buf_rx,"+CREG: 0,1")||strstr(buf_rx,"+CREG: 0,5"))
  {
	  ResumptionReceiveDataFromGSM();       //Start na rozpoczecie pomiarów    //send arp request
	  if(nr_kom==__creg_pyta){ nr_kom2=0;   wybor_bitowy|=0x0400;             delay_funkc[15]=1;    sprintf(info_gsm,"GSM: Zalogowany");
     #ifndef _LCD_TFT
	     StringFont5x7("GSM ZALOGOWANO",14);NewLineFont5x7(14,0);
     #else
         lcd_tft("GSM: ZALOGOWANO",ST7735_YELLOW);
     #endif

	  dbg3("\r\nGSM:  ZALOGOWANO  ");
	  nr_kom=__cmgf_wpis;  wpis_komend(nr_kom);  HAL_Delay(300); dbg(at_send);  delay_funkc[5]=1;  }

  }
  if(strstr(buf_rx,"+COPS:"))
  {
	  HAL_Delay(300); dbg3(buf_rx);

     #ifdef _LCD_TFT
	    i=0;
	    while(1){ if(buf_rx[i]=='"') break;   i++; if(i>100) break; } j=i; i++;
	    while(1){ if(buf_rx[i]=='"') break;   i++; if(i>100) break; } buf_rx[i+1]=0;
	    sprintf(buf_p,"GSM: %s",&buf_rx[j]);
	    lcd_tft(buf_p,ST7735_YELLOW);
     #endif

	  ResumptionReceiveDataFromGSM();
	  if(nr_kom==__cops_pyta){ nr_kom2=0;  nr_kom=__cnum;  wpis_komend(nr_kom);  HAL_Delay(500); dbg(at_send); delay_funkc[5]=1; }

  }
  if(strstr(buf_rx,"+CNUM:"))
  {
	  ResumptionReceiveDataFromGSM();
	  if(nr_kom==__cnum){ nr_kom2=0;  nr_kom=__csmp_wpis;  wpis_komend(nr_kom);  HAL_Delay(500); dbg(at_send);  delay_funkc[5]=1;  }

  }
  if(ptr=strstr(buf_rx,"+CLIP:"))   // RING
  {
	   HAL_Delay(1000);
	   dbg3(ptr);
	   //sprintf(buf_p,Const.s_Tel[0].tel1);
	   if(strstr(ptr,&Const.s_Tel[0].tel1[3])||\
		  strstr(ptr,&Const.s_Tel[0].tel2[3])||\
		  strstr(ptr,&Const.s_Tel[0].tel3[3])||\
		  strstr(ptr,&Const.s_Tel[0].tel4[3])||\
		  strstr(ptr,&Const.s_Tel[0].tel5[3])||\
		  strstr(ptr,&Const.s_Tel[0].tel6[3])||\
		  strstr(ptr,&Const.s_Tel[0].tel7[3])||\
		  strstr(ptr,&Const.s_Tel[0].tel8[3])	   )
	   {
		   Const.s_WY[5/16].val|=(1<<(5-16*(5/16)));  //6-ty PK  on  dla dzwonienia
		   GPIO_PK(5);

	       delay_funkc[10]=1;
	   }

	  ResumptionReceiveDataFromGSM();

  }
  if(ptr=strstr(buf_rx,"+CMGS:"))
  {
	  ResumptionReceiveDataFromGSM(); HAL_Delay(30);
	  dbg3("\r\nkoniec SMS");
	  if(li_sms>0) li_sms=0;
	  if((wysylka_sms>0)&&(wysylka_sms<10)){  i_sms[wysylka_sms-1]=0;   ObslugaSMS();  }
	  else if(wysylka_sms==99) wysylka_sms=0;
	  else{ for(i=0;i<160;i++) buf_sms[i]=0; }
	  ekran_tymczasowy=2;   sms_reply_end=0;

	  HAL_Delay(30);

  }
  if(ptr=strstr(buf_rx,"\x0D\x0A>"))   //>
  {
	  if(idx_email_GPRS>0)
	  {
		  if(idx_email_GPRS==4){  ResumptionReceiveDataFromGSM();   dbg("EHLO ElektronikaRM\r\n\x1A");   idx_email_GPRS++;   }
		  if(idx_email_GPRS==6){  ResumptionReceiveDataFromGSM();   dbg("auth login\r\n\x1A");           idx_email_GPRS++;   }
		  if(idx_email_GPRS==8){  base64_encode(Const.s_Email[0].nad, buf_p, strlen(Const.s_Email[0].nad)); sprintf(&buf_p[100],"%s\r\n\x1A",buf_p);\
			                      ResumptionReceiveDataFromGSM();   dbg(&buf_p[100]);                    idx_email_GPRS++;   }
		  if(idx_email_GPRS==10){ base64_encode(Const.s_Email[0].has, buf_p, strlen(Const.s_Email[0].has)); sprintf(&buf_p[100],"%s\r\n\x1A",buf_p);\
			                      ResumptionReceiveDataFromGSM();   dbg(&buf_p[100]);                    idx_email_GPRS++;   }
		  if(idx_email_GPRS==12){ ResumptionReceiveDataFromGSM();   sprintf(buf_p,"MAIL From: <%s>\r\n\x1A",Const.s_Email[0].nad);  dbg(buf_p);   idx_email_GPRS++;   }
		  if(idx_email_GPRS==14){ ResumptionReceiveDataFromGSM();   sprintf(buf_p,"RCPT To: <%s>\r\n\x1A",Const.s_Email[0].odb1);dbg(buf_p);     idx_email_GPRS++;   }
		  if(idx_email_GPRS==16){ ResumptionReceiveDataFromGSM();   dbg("DATA\r\n\x1A");                              idx_email_GPRS++;   }
		  if(idx_email_GPRS==18){ ResumptionReceiveDataFromGSM();   sprintf(buf_p,"From: <%s>\r\n\x1A",Const.s_Email[0].nad);  dbg(buf_p);        idx_email_GPRS++;   }
		  if(idx_email_GPRS==20){ ResumptionReceiveDataFromGSM();  sprintf(buf_p,"To: <%s>\r\n\x1A",Const.s_Email[0].odb1); dbg(buf_p);          idx_email_GPRS++;   }
		  if(idx_email_GPRS==22){ ResumptionReceiveDataFromGSM();   if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')){ dbg("Subject: Rejestr Zdarzen\r\n\x1A"); adr_flash_2_read= Const.s_Rap[0].start_zd; } else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){ dbg("Subject: Pomiary temperatury i wilgotnosci\r\n\x1A"); idx_email_GPRS=200-1;  adr_flash_read= Const.s_Rap[0].start_po;  }             idx_email_GPRS++;   }
		  if(idx_email_GPRS==201){ ResumptionReceiveDataFromGSM();  sprintf(buf_p,"Content-Type: text/plain\r\n\x1A" ); dbg(buf_p);          idx_email_GPRS++;   }
		  if(idx_email_GPRS==203){ ResumptionReceiveDataFromGSM();  sprintf(buf_p,"Content-Disposition: attachment; filename=POMIARY.txt\r\n\x1A" ); dbg(buf_p);          idx_email_GPRS=23;   }
		  if(idx_email_GPRS==24){ ResumptionReceiveDataFromGSM();   dbg("\r\n\x1A");          idx_email_GPRS++;             RapAddr=0;  RapAddr_p=0; }

		  if(idx_email_GPRS==26)  //Tresc mail`a
		  {
			       if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')) j=OdczytZdarzenSmtpGSM(&adr_flash_2_read);
			  else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')) j=OdczytRejTempSmtp(1);
			 // RapAddr_p = RapAddr;
			  //if(RaportStanu(&RapAddr,0)==1) j=2;     buf_rx2[_Size_BufWifi+(RapAddr-RapAddr_p)]='\x1A';   buf_rx2[_Size_BufWifi+(RapAddr-RapAddr_p)+1]=0;  //dbg3("\r\n"); dbg3(&buf_rx2[_Size_BufWifi]);
			  if(j==2){ idx_email_GPRS++; dbg3("\r\nKONIEC Wysylania Danych ");  }
		      ResumptionReceiveDataFromGSM();   dbg(&buf_rx2[_Size_BufWifi]); /*dbg3("\r\n"); dbg3(&buf_rx2[_Size_BufWifi]);*/  if(j!=2) dbg3("\r\nWysylam Dane...");

		     //  dbg3("\r\nSSSSS: ");  HAL_Delay(2000); dbg3(buf_rx);
		  }

		  if(idx_email_GPRS==28){ ResumptionReceiveDataFromGSM();   dbg("\r\n.\r\n\x1A");         idx_email_GPRS++;  }
		  if(idx_email_GPRS==30){ ResumptionReceiveDataFromGSM();   dbg("QUIT\r\n\x1A");          idx_email_GPRS++;   }
	  }
	  else
	  {
	     dbg3("\r\nGSM:  Wysylam SMS o tresci:\r\n"); //dbg3(buf_sms);   dbg3("\r\n\r\n");
	     ResumptionReceiveDataFromGSM();
	     if((wysylka_sms>0)&&(wysylka_sms<10)){
		      for(i=0;i<160;i++) buf_sms[i]=b_sms[160*(wysylka_sms-1)+i];   /*i_sms[wysylka_sms-1]=0;*/   }
	     send_buf_sms();  HAL_Delay(500);
	  }

  }
  if(strstr(buf_rx,"\r\n+MIPSEND:"))
  {
	       if(idx_email_GPRS==19){  HAL_Delay(300);  GprsSend("AT+MIPSEND=1\x0D");   }
	  else if(idx_email_GPRS==21){  HAL_Delay(300);  GprsSend("AT+MIPSEND=1\x0D");   }
	  else if(idx_email_GPRS==23){  HAL_Delay(300);  GprsSend("AT+MIPSEND=1\x0D");   }
	  else if(idx_email_GPRS==200){  HAL_Delay(300);  GprsSend("AT+MIPSEND=1\x0D");   }
	  else if(idx_email_GPRS==202){  HAL_Delay(300);  GprsSend("AT+MIPSEND=1\x0D");   }
	  else if(idx_email_GPRS==25){  HAL_Delay(300);  GprsSend("AT+MIPSEND=1\x0D");   }
	  else if((idx_email_GPRS==26)||(idx_email_GPRS==27)){ if(idx_email_GPRS==26){ idx_email_GPRS--;  }  HAL_Delay(300);  GprsSend("AT+MIPSEND=1\x0D");   }


  }
  if(strstr(buf_rx,"\r\n+MIPCALL:"))
  {
	  if(idx_email_GPRS==1)
      {
		  GprsSend("AT+GTSET=\"IPRFMT\",1\x0D");
	  }
  }
  if(strstr(buf_rx,"\r\nOK"))  //To musi byc na koncu wszystkich komend
  {

	  if(idx_email_GPRS>0)
	  {
	     if(idx_email_GPRS==1)
	     {
             HAL_Delay(500);
             if(strstr(buf_rx,"\r\n+MIPCALL:"))  GprsSend("AT+GTSET=\"IPRFMT\",1\x0D");
		     ResumptionReceiveDataFromGSM();
	     }
	     else if(idx_email_GPRS==2){ sprintf(buf_p,"AT+MIPOPEN=1,1201,\"%s\",%s,0\x0D",Const.s_Email[0].ip,Const.s_Email[0].port);  GprsSend(buf_p);  }

	  }
	  else
	  {

	     HAL_Delay(600);
	     ResumptionReceiveDataFromGSM();

  #ifdef _Neoway
	           if(nr_kom==__ATE0)     { nr_kom2=0;  nr_kom=__cpin_pyta;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  ATE0 ");  delay_funkc[5]=1; }
	   	  else if(nr_kom==__cpin_wpis){ nr_kom2=0;  nr_kom=__creg_pyta;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  PIN ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__cmgf_wpis){  nr_kom2=0; nr_kom=__cnmi_wpis;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGF ");       delay_funkc[5]=1;}
	   	  else if(nr_kom==__cnmi_wpis){  nr_kom2=0; nr_kom=__clip_wpis;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CNMI ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__clip_wpis){  nr_kom2=0; nr_kom=__cops_pyta;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CLIP ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__cnum)     {  nr_kom2=0; nr_kom=__csmp_wpis;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CNUM ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__csmp_wpis){  nr_kom2=0; nr_kom=__cscs;     wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CSMP ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__cscs)     {  nr_kom2=0; nr_kom=__cmgd_1;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CSCS ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__cmgd_1)   {  nr_kom2=0; nr_kom=__cmgd_2;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGD1 ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__cmgd_2)   {  nr_kom2=0; nr_kom=__cmgd_3;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGD2 ");  delay_funkc[5]=1;}
	   	  else if(nr_kom==__cmgd_3)   {  nr_kom2=0; nr_kom=__csq;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGD3 "); delay_funkc[5]=1; }
	   	  else if(nr_kom==__csq)      {  nr_kom2=0; nr_kom=100;   dbg3("\r\nGSM:  KONIEC_INIT ");  start=1;  licz_rst=0;  delay_funkc[5]=0;  delay_funkc[9]=1;  }
  #endif

  #ifdef _Fibocom
       if(nr_kom==__ATE0)     { nr_kom2=0;  nr_kom=__cpin_pyta;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  ATE0 ");  delay_funkc[5]=1; }
	  else if(nr_kom==__cpin_wpis){ nr_kom2=0;  nr_kom=__creg_pyta;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  PIN ");  delay_funkc[5]=1;}
	  else if(nr_kom==__cmgf_wpis){  nr_kom2=0; nr_kom=__cnmi_wpis;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGF ");       delay_funkc[5]=1;}
	  else if(nr_kom==__cnmi_wpis){  nr_kom2=0; nr_kom=__clip_wpis;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CNMI ");  delay_funkc[5]=1;}
	  else if(nr_kom==__clip_wpis){  nr_kom2=0; nr_kom=__cops_pyta;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CLIP ");  delay_funkc[5]=1;}
	  else if(nr_kom==__cnum)     {  nr_kom2=0; nr_kom=__csmp_wpis;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CNUM ");  delay_funkc[5]=1;}
	  else if(nr_kom==__csmp_wpis){  nr_kom2=0; nr_kom=__cmgd_1;     wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CSMP ");  delay_funkc[5]=1;}
	  else if(nr_kom==__cmgd_1)   {  nr_kom2=0; nr_kom=__cmgd_2;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGD1 ");  delay_funkc[5]=1;}
	  else if(nr_kom==__cmgd_2)   {  nr_kom2=0; nr_kom=__cmgd_3;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGD2 ");  delay_funkc[5]=1;}
	  else if(nr_kom==__cmgd_3)   {  nr_kom2=0; nr_kom=__cmgd_4;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGD3 ");  delay_funkc[5]=1;}
	  else if(nr_kom==__cmgd_4)   {  nr_kom2=0; nr_kom=__csq;  wpis_komend(nr_kom);  HAL_Delay(100); dbg(at_send); dbg3("\r\nGSM:  CMGD4 "); delay_funkc[5]=1; }
	  else if(nr_kom==__csq)      {  nr_kom2=0; nr_kom=100;   dbg3("\r\nGSM:  KONIEC_INIT ");  start=1;  licz_rst=0;  delay_funkc[5]=0;  delay_funkc[9]=1;  }
 #endif

	  }
  }

//jezeli SMS wyslemy do ukladu ktory ³aczy sie po GPRS to ten SMS nie dojdzie  podobnie jesli dzwonimy do ukladu to abonent jest zajety
// Blokada wysylania smsów podczas GPRS
  //Blokada GPRS  jesli w czasie wysylania SMS uruchamiamy GPRS

  if(strstr(buf_rx,"220 ")){                   if(idx_email_GPRS==3) GprsSend("AT+MIPSEND=1\x0D");  }
  else if(strstr(buf_rx,"250-")){              if(idx_email_GPRS==5) GprsSend("AT+MIPSEND=1\x0D");  }
  else if(strstr(buf_rx,"334 VXNlcm5hbWU6")){  if(idx_email_GPRS==7) GprsSend("AT+MIPSEND=1\x0D");  }
  else if(strstr(buf_rx,"334 UGFzc3dvcmQ6")){  if(idx_email_GPRS==9) GprsSend("AT+MIPSEND=1\x0D");  }
  else if(strstr(buf_rx,"235 "))            {  if(idx_email_GPRS==11) GprsSend("AT+MIPSEND=1\x0D");  }
  else if(strstr(buf_rx,"250 "))            {  if((idx_email_GPRS==13)||(idx_email_GPRS==15)||(idx_email_GPRS==29)) GprsSend("AT+MIPSEND=1\x0D");  }
  else if(strstr(buf_rx,"354 "))            {  if(idx_email_GPRS==17) GprsSend("AT+MIPSEND=1\x0D");  }
  else if(strstr(buf_rx,"221 "))            {  if(idx_email_GPRS==31){ GprsSend("AT+MIPCALL=0\x0D"); idx_email_GPRS=0; delay_funkc[26]=0;        if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')){ Const.s_Rap[0].start_zd= adr_flash_2_read;  delay_funkc[6]=1;/*zapisz flash*/}     if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){ Const.s_Rap[0].start_po= adr_flash_read;  delay_funkc[6]=1;/*zapisz flash*/}          }  }

  else if(strstr(buf_rx,"535 "))   //obsluga b³êdów  np:   535 5.7.8 Error: authentication failed: UGFzc3dvcmQ6
  {
      HAL_Delay(300);
	  dbg3("\r\n"); dbg3(buf_rx);
      GprsSend("AT+MIPCALL=0\x0D"); idx_email_GPRS=0; delay_funkc[26]=0;
  }






}


  //######################  -- WiFi---- ####################################################
 /*
AT+GMR
AT version:1.2.0.0(Jul  1 2016 20:04:45)
SDK version:1.5.4.1(39cb9a32)
Ai-Thinker Technology Co. Ltd.
Dec  2 2016 14:21:16
OK
*/

/*
  if(licz_htt==5)  //TU dla esp01
  {
	  dbg3(&buf_rx2[0]);  dbg3("\r\nWysylam: AT+CIPSERVER=1,80 ");
	  ResumptionReceiveDataFromWifi();  SendDataToModemWifi("AT+CIPSERVER=1,80\x0D\x0A");
     licz_htt=20; HAL_Delay(1000);


  }
*/
    //AT+UART=460800,8,1,0,0
   //AT+UART=115200,8,1,0,0

if(licz_htt<10000)
{

  if(email==1)   //wifi
  {
		HAL_Delay(1000);  if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')){
			debug_smtp_esp=1;  ZapisSesjiSMTP(&buf_rx2[0], strlen(&buf_rx2[0]), &adr_flash_SesjaSMTP);
		}
		//dbg3(&buf_rx2[0]);
		if(ptr=strstr(buf_rx2,"GET /"))  //+IPD,0,431:GET /TME.txt HTTP/1.1
		{
			/*port_smtp_p= *(ptr-6)&0x0f;
			sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=%d\x0D\x0A",port_smtp_p);
			SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
			dbg3("\r\nAA: "); dbg3(&buf_rx2[_Size_BufWifi]);*/
			port_smtp_p=100;
		}
	    if(strstr(buf_rx2,":220 "))
	 	{
	 	   ResumptionReceiveDataFromWifi();
	 	   sprintf(buf_p,"AT+CIPSEND=%d,10\x0D\x0A",port_smtp);    SendDataToModemWifi(buf_p);   //EHLO ..
	 	   email=2;
	 	   HAL_Delay(200);
	 	  // dbg3("\r\nSSSSSSSSSSSSSSSSSSSSSSSSS");
	 	}

  }

  //dbg3(&buf_rx2[0]);
  if(licz_htt==0)
  { if(email==0){
    if(ptr=strstr_hex(&buf_rx2[0],"àà"))   //wifi
    // if(ptr=strstr(&buf_rx2[0],"\r\nready"))   //wifi
    {

		///if(ptr=strstr_hex(&buf_rx2[0],"\r\nready"))
		//{
		   dbg3(ptr); dbg3("\r\nWIFI:  ATE0 ");  delay_funkc[1]=0;
		   ResumptionReceiveDataFromWifi();
	       SendDataToModemWifi("ATE0\x0D\x0A");
	       licz_htt=1;   delay_funkc[21]=0;  HAL_Delay(1000);
	       // licz_htt=0;
		//}
	 }}

  }

if(email==0){
  if(ptr=strstr(&buf_rx2[0],"+CIPDOMAIN:"))
  {
	  HAL_Delay(100);
	  dbg3(ptr);
	  ResumptionReceiveDataFromWifi();
  }
  if(ptr=strstr(&buf_rx2[0],"+CIPSTAMAC:"))  //+CIPSTAMAC:"a0:20:a6:32:c2:76"
  {
	  HAL_Delay(100);
	  Const.s_Lan[0].eth[0]=*(ptr+12);
	  Const.s_Lan[0].eth[1]=*(ptr+13);
	  Const.s_Lan[0].eth[2]=*(ptr+15);
	  Const.s_Lan[0].eth[3]=*(ptr+16);
	  Const.s_Lan[0].eth[4]=*(ptr+18);
	  Const.s_Lan[0].eth[5]=*(ptr+19);
	  Const.s_Lan[0].eth[6]=*(ptr+21);
	  Const.s_Lan[0].eth[7]=*(ptr+22);
	  Const.s_Lan[0].eth[8]=*(ptr+24);
	  Const.s_Lan[0].eth[9]=*(ptr+25);
	  Const.s_Lan[0].eth[10]=*(ptr+27);
	  Const.s_Lan[0].eth[11]=*(ptr+28);


	  //sprintf(buf_p,"\r\nMAC: %02x:%02x:%02x:%02x:%02x:%02x",ETHADDRESS[0],ETHADDRESS[1],ETHADDRESS[2],ETHADDRESS[3],ETHADDRESS[4],ETHADDRESS[5]);
	 // dbg3(buf_p);
	 // NRF_Start();  WgrajMacENC(); netstackInit(IPADDRESS, NETMASK, GATEWAY);   NRF_Stop();
	  delay_funkc[15]=1;  //send arp request

	  ResumptionReceiveDataFromWifi();
  }
  if(ptr=strstr(&buf_rx2[0],"+CWLAP:"))
  {
	  HAL_Delay(1000);
	  dbg3(ptr);
	    if(strlen(buf_rx2)>1000) buf_rx2[999]=0;
	    sprintf(&buf_rx2[_Size_BufWifi+3000],&buf_rx2[0]);
	    wybor_bitowy|=0x0001;
	  ResumptionReceiveDataFromWifi();
  }
  if(ptr=strstr(&buf_rx2[0],"+CIFSR:"))
  {
	  if(licz_htt==5)
	  {   if((ekran_bit&0x01)>0){ ekran_bit&=~0x01;  DrawImageStart(); YlcdWpis=41; }
		  if(ptr=strstr(&buf_rx2[0],"+CIFSR:STAIP"))
		  {
			  HAL_Delay(200);
			  i=0;
			  do{ buf_p[i]=*(ptr+14+i); }while(*(ptr+14+i++)!='"');  buf_p[i-1]=0;
#ifndef _LCD_TFT
	sprintf(Const.s_Lan[0].ip, buf_p);
    StringFont5x7(buf_p,strlen(buf_p));  NewLineFont5x7(strlen(buf_p)-1,0);
#else
    sprintf(Const.s_Lan[0].ip, buf_p);
    sprintf(&buf_p[100],"Wifi: %s",buf_p);
	lcd_tft(&buf_p[100],ST7735_WHITE);
    sprintf(&buf_p[100],"Wifi: %s",Const.s_Wifi[0].ssid);
    lcd_tft(&buf_p[100],ST7735_YELLOW);


	          if(ptr=strstr(&buf_rx2[0],"+CIFSR:APIP"))
			  {
				  HAL_Delay(200);
				  i=0;
				  do{ buf_p[i]=*(ptr+13+i); }while(*(ptr+13+i++)!='"');  buf_p[i-1]=0; // dbg3("\r\nZZZZ "); dbg3(buf_p);

	              sprintf(&buf_p[100],"Wifi: %s  AP",buf_p);
		          lcd_tft(&buf_p[100],ST7735_WHITE);
			  }
	          HAL_IWDG_Refresh(&hiwdg);
	          HAL_Delay(5000);
	          HAL_IWDG_Refresh(&hiwdg);

#endif
			   //dbg3("\r\nAAAAAAAAAAAAA  ");
			 /* buf_p[50+0]=  100*(buf_p[0]&0x0f)+10*(buf_p[1]&0x0f)+(buf_p[2]&0x0f);
			  buf_p[50+0]=  100*(buf_p[0]&0x0f)+10*(buf_p[1]&0x0f)+(buf_p[2]&0x0f);
			  buf_p[50+0]=  100*(buf_p[0]&0x0f)+10*(buf_p[1]&0x0f)+(buf_p[2]&0x0f);
			  buf_p[50+0]=  100*(buf_p[0]&0x0f)+10*(buf_p[1]&0x0f)+(buf_p[2]&0x0f);*/
		  }
		  else if(ptr=strstr(&buf_rx2[0],"+CIFSR:APIP"))
		  {
			  HAL_Delay(200);
			  i=0;
			  do{ buf_p[i]=*(ptr+13+i); }while(*(ptr+13+i++)!='"');  buf_p[i-1]=0; // dbg3("\r\nZZZZ "); dbg3(buf_p);

#ifndef _LCD_TFT
    sprintf(Const.s_Lan[0].ip, buf_p);
	StringFont5x7(buf_p,strlen(buf_p));  NewLineFont5x7(strlen(buf_p)-1,0);
#else
    sprintf(Const.s_Lan[0].ip, buf_p);
	sprintf(&buf_p[100],"Wifi: %s  AP",buf_p);
	lcd_tft(&buf_p[100],ST7735_WHITE);
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(3000);
    HAL_IWDG_Refresh(&hiwdg);
#endif


		  }
		  dbg3("\r\nWIFI:  ");
		  HAL_Delay(100);
		  dbg3(&buf_rx2[0]);
		  sprintf(buf_p,"AT+CIPSERVER=1,%s\x0D\x0A",Const.s_Lan[0].port);  dbg3("\r\nWIFI:  ");  dbg3(buf_p);
		  ResumptionReceiveDataFromWifi();  SendDataToModemWifi(buf_p);  wybor_bitowy|=0x8000;
	     licz_htt=21;
	     HAL_Delay(1000);
  	     delay_funkc[8]=1;   delay_funkc[34]=0;

	  }
	  else
	  {
		  HAL_Delay(100);
		  dbg3(&buf_rx2[0]);
		  ResumptionReceiveDataFromWifi();
	  }

  }
  //if(ptr=strstr(&buf_rx2[0],"WIFI DISCONNECT")){   dbg3("\r\nWIFI DISCONNECT"); ResumptionReceiveDataFromWifi();  }
//  if(ptr=strstr(&buf_rx2[0],"WIFI CONNECTED")) {   dbg3("\r\nWIFI CONNECTED");  ResumptionReceiveDataFromWifi();  }
  //if(ptr=strstr(&buf_rx2[0],"WIFI GOT IP"))    {   dbg3("\r\nWIFI GOT IP");      ResumptionReceiveDataFromWifi(); }

  if(ptr=strstr(&buf_rx2[0],"busy"))
  {
	  HAL_Delay(100);
	  dbg3(ptr);
	  li_WifiCipsend[0]=1;
	  ResumptionReceiveDataFromWifi();
  }
 /* if(strstr(buf_rx2,"\r\nWIFI GOT IP"))
  {
	  if(licz_htt==4)
	  {
		  dbg3("\r\n__WIFI GOT IP_____Wysylam: AT+CIFSR ");
		  ResumptionReceiveDataFromWifi();  SendDataToModemWifi("AT+CIFSR\x0D\x0A");
		  licz_htt=5;  HAL_Delay(2000);
	  }
  }*/
  if((strstr(buf_rx2,"\r\nOK"))||(strstr(buf_rx2,"no change")))
  {

    if(delay_funkc[31]>0)
    {
	    delay_funkc[31]=0;  dbg3("OK ");   ResumptionReceiveDataFromWifi();
    }
	else
    {

	  if(licz_htt==69)
  	  {  dbg3("\r\nWIFI:   ");

  	     ResumptionReceiveDataFromWifi();
  	     //SendDataToModemWifi("AT+CIPAP=\"192.168.5.1\", \"192.168.5.1\", \"255.255.255.0\"\x0D\x0A");
  	     sprintf(buf_p,"AT+CIPSTA_CUR=\"%s\",\"%s\",\"%s\"\x0D\x0A",Const.s_Lan[0].ip, Const.s_Lan[0].br, Const.s_Lan[0].mask);
  	   	 SendDataToModemWifi(buf_p);  dbg3(buf_p);
  	     licz_htt=2;  HAL_Delay(1000);
  	  }
	  else if(licz_htt==68)
  	  {
		  dbg3("\r\nWIFI:  ");
  	     ResumptionReceiveDataFromWifi();
  	             if((Const.s_Lan[0].param&0x0C)==0x08){ dbg3("\r\nA1  ");sprintf(&buf_rx2[_Size_BufWifi],"AT+CWSAP_CUR=\"%s\",\"%s\",5,3\x0D\x0A",Const.s_Wifi[0].AP_ssid, Const.s_Wifi[0].AP_haslo);}
  	        else if((Const.s_Lan[0].param&0x0C)==0x0C){  dbg3("\r\nA2  ");sprintf(&buf_rx2[_Size_BufWifi],"AT+CWSAP_CUR=\"%s\",\"%s\",5,3\x0D\x0A",Const.s_Wifi[0].AP_ssid, Const.s_Wifi[0].AP_haslo);}
  	        else if((Const.s_Lan[0].param&0x0C)==0x04){  dbg3("\r\nA3  ");sprintf(&buf_rx2[_Size_BufWifi],"AT\x0D\x0A");}

  	    	     if((Const.s_Lan[0].param&0x01)>0) licz_htt=2;
  	    	     else                              licz_htt=69;

  	      omin_eettg:
  	     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);

  	     omin_eettg2:
		 asm("nop");
  	     //HAL_Delay(1000);
  	     //dbg3(buf_rx2);
  	  }
	  else if(licz_htt==67)
  	  {  dbg3("\r\nWIFI:  ");

  	     ResumptionReceiveDataFromWifi();
  	   if((Const.s_Lan[0].param&0x0C)==0x08) sprintf(&buf_rx2[_Size_BufWifi],"AT\x0D\x0A");
  	   else
  	   {  sprintf(&buf_rx2[_Size_BufWifi],"AT\x0D\x0A");
  		  /*sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSTAMAC_CUR=\"  :  :  :  :  :  \"\x0D\x0A");
  		 buf_rx2[_Size_BufWifi+18]=Const.s_Lan[0].eth[0];
  		 buf_rx2[_Size_BufWifi+19]=Const.s_Lan[0].eth[1];
  		 buf_rx2[_Size_BufWifi+21]=Const.s_Lan[0].eth[2];
  		 buf_rx2[_Size_BufWifi+22]=Const.s_Lan[0].eth[3];
  		 buf_rx2[_Size_BufWifi+24]=Const.s_Lan[0].eth[4];
  		 buf_rx2[_Size_BufWifi+25]=Const.s_Lan[0].eth[5];
  		 buf_rx2[_Size_BufWifi+27]=Const.s_Lan[0].eth[6];
  		 buf_rx2[_Size_BufWifi+28]=Const.s_Lan[0].eth[7];
  		 buf_rx2[_Size_BufWifi+30]=Const.s_Lan[0].eth[8];
  		 buf_rx2[_Size_BufWifi+31]=Const.s_Lan[0].eth[9];
  		 buf_rx2[_Size_BufWifi+33]=Const.s_Lan[0].eth[10];
  		 buf_rx2[_Size_BufWifi+34]=Const.s_Lan[0].eth[11];*/
  	   }
  		dbg3(&buf_rx2[_Size_BufWifi]);
  		  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
  	     licz_htt=68;  HAL_Delay(1000);
  	  }
	  else if(licz_htt==99)
  	  {  dbg3("\r\n___OK___ Wysylam:  AT+CWDHCP_CUR=1,0 ");

  	     ResumptionReceiveDataFromWifi();
  	     SendDataToModemWifi("AT+CWDHCP_CUR=1,0\x0D\x0A");
  	     licz_htt=69;  HAL_Delay(1000);
  	  }
	  else if(licz_htt==70)
  	  {  dbg3("\r\n___OK___ Wysylam: AT+CWLAPOPT=1,23 ");
  	     ResumptionReceiveDataFromWifi(); SendDataToModemWifi("AT+CWLAPOPT=1,23\x0D\x0A");
  	     licz_htt=68;  HAL_Delay(1000);
  	  }
	  else if(licz_htt==1)
  	  {  dbg3("\r\nWIFI:  CWMODE ");
  	     ResumptionReceiveDataFromWifi();
	          if((Const.s_Lan[0].param&0x0C)==0x0C){ licz_htt=67;  SendDataToModemWifi("AT+CWMODE=3\x0D\x0A"); }  //SoftAP+Station mode
	     else if((Const.s_Lan[0].param&0x0C)==0x04){ licz_htt=67;  SendDataToModemWifi("AT+CWMODE=1\x0D\x0A");}  //Station mode
	     else if((Const.s_Lan[0].param&0x0C)==0x08){ licz_htt=67;  SendDataToModemWifi("AT+CWMODE=2\x0D\x0A");}  //SoftAP mode
  	       HAL_Delay(1000);
  	  }
  	  else if(licz_htt==2)
  	  {  dbg3("\r\nWIFI:  CIPMUX");
  	     ResumptionReceiveDataFromWifi();  SendDataToModemWifi("AT+CIPMUX=1\x0D\x0A");
  	     licz_htt=3;



  	     HAL_Delay(1000);
  	  }
  	  else if(licz_htt==3)
  	  {
  		 dbg3("\r\nWIFI:  ");
  		 ResumptionReceiveDataFromWifi();
  		 if((Const.s_Lan[0].param&0x0C)==0x08) sprintf(&buf_rx2[_Size_BufWifi],"AT+CWJAP_CUR=\"aaaaaaaaaaaaaaa\",\"bbbbbbbbbbbbbbb\"\x0D\x0A");
  		 else                                  sprintf(&buf_rx2[_Size_BufWifi],"AT+CWJAP_CUR=\"%s\",\"%s\"\x0D\x0A",Const.s_Wifi[0].ssid, Const.s_Wifi[0].haslo);
  		//sprintf(&buf_rx2[_Size_BufWifi],"AT\x0D\x0A",Const.s_Wifi[0].ssid, Const.s_Wifi[0].haslo);
  	     dbg3(&buf_rx2[_Size_BufWifi]);
  	     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
  	    // SendDataToModemWifi("AT+CWJAP_CUR=\"NETGEAR\",\"123456789abc\"\x0D\x0A");
  	    // SendDataToModemWifi("AT+CWJAP_CUR=\"Internet Domowy-0F7280\",\"niewiemjakiehaslo1234567890\"\x0D\x0A");
  	    // SendDataToModemWifi("AT+CWJAP_CUR=\"PLAY INTERNET 4G LTE-0477\",\"LAJE1YDNJ5M\"\x0D\x0A");
  	     licz_htt=4;  HAL_Delay(1000);
  	     delay_funkc[4]=1;
  	  }
  	  else if(licz_htt==4)
  	  {  dbg3("\r\nWIFI:  CIFSR");
  	     ResumptionReceiveDataFromWifi();
  	           licz_htt=5;  SendDataToModemWifi("AT+CIFSR\x0D\x0A");
  	       HAL_Delay(2000);
  	     delay_funkc[4]=0;
  	  }
  	  else if(licz_htt==50)
  	  {  dbg3("\r\n__CLOSED__");
  	     licz_htt=20;
  	  }
  	  else if(licz_htt==45)
  	  {  dbg3("\r\n__OK_AP__");
  	     licz_htt=20;
  	  }
  	  else if(licz_htt==66)
  	  {  dbg3("\r\n__OK_UART");
  	     licz_htt=20;
  	  }
  	  else if(licz_htt==7000)
  	  {  ResumptionReceiveDataFromWifi();
  		  SendDataToModemWifi("AT+CIFSR\x0D\x0A");
  	     licz_htt=20;
  	  }
  	  else if(licz_htt==7001)
  	  {   ResumptionReceiveDataFromWifi();
  		  SendDataToModemWifi("AT+CIPSERVER=0\x0D\x0A");   dbg3("\r\nAT+CIPSERVER=0 ");
  		  delay_funkc[3]=1;
  		  HAL_Delay(500);
  	     licz_htt=7002;
  	  }
  	  else if(licz_htt==7002)
  	  {   ResumptionReceiveDataFromWifi();
  	      sprintf(buf_p,"AT+CIPSERVER=1,%s\x0D\x0A",Const.s_Lan[0].port);
  	      SendDataToModemWifi(buf_p);     dbg3(buf_p);
  	      delay_funkc[3]=0;
  	      HAL_Delay(500);
  	      licz_htt=20;
  	  }
  	  else if(licz_htt==7003)
	  {
	   	     ResumptionReceiveDataFromWifi();
	   	     //SendDataToModemWifi("AT+CIPAP=\"192.168.5.1\", \"192.168.5.1\", \"255.255.255.0\"\x0D\x0A");
	   	     sprintf(buf_p,"AT+CIPSTA_CUR=\"%s\",\"%s\",\"%s\"\x0D\x0A",Const.s_Lan[0].ip, Const.s_Lan[0].br, Const.s_Lan[0].mask);
	   	   	 SendDataToModemWifi(buf_p);   dbg3(buf_p);
	   	     licz_htt=7001;
	   	     HAL_Delay(500);
	  }
	  else if(licz_htt==7004)
  	  {
         ResumptionReceiveDataFromWifi();
  	     SendDataToModemWifi("AT+CWDHCP_CUR=1,1\x0D\x0A");  dbg3("AT+CWDHCP_CUR=1,1\r\n");
  	     licz_htt=7001;  HAL_Delay(500);
  	  }
	  else if(licz_htt==21)
  	  {
         ResumptionReceiveDataFromWifi();
         sprintf(buf_p,"Port: %s",Const.s_Lan[0].port);
#ifndef _LCD_TFT
	StringFont5x7(buf_p,strlen(buf_p));  NewLineFont5x7(strlen(buf_p),0);
#else
	sprintf(buf_p,"WiFi: Port %s",Const.s_Lan[0].port);
	lcd_tft(buf_p,ST7735_WHITE);
#endif
  	     licz_htt=20;  HAL_Delay(500);
  	     wybor_bitowy|=0x0080; //zacznij mrugac
  	  }





    }
  }
  if(ptr=strstr(&buf_rx2[0],"ERROR"))
  {
	  if(licz_htt==45){ dbg3("\r\n__ERROR_AP__");  licz_htt=20; }

	  if((licz_htt==2)||(licz_htt==69))
	  {
		     ResumptionReceiveDataFromWifi();   dbg3("\r\nA4  ");
		     sprintf(&buf_rx2[_Size_BufWifi],"AT+CWSAP_CUR=\"%s\",\"%s\",5,3\x0D\x0A",Wifi_AP_ssid,Wifi_AP_haslo);
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
		     if((Const.s_Lan[0].param&0x01)>0) licz_htt=2;
		     else                               licz_htt=69;
	  }


  }
}

  if(strstr(buf_rx2,"\r\n>")&&(email>0))  //>
  {
	    if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')){
	    	debug_smtp_esp=1;   ZapisSesjiSMTP(&buf_rx2[0], strlen(&buf_rx2[0]), &adr_flash_SesjaSMTP);
	    }
		ResumptionReceiveDataFromWifi();

		    if(email==2){  SendDataToModemWifi("EHLO Mar\r\n");    if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o')) RapAddr=0;    if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')) RapAddr= Const.s_Rap[0].start_zd;        if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){ RapAddr=0;  adr_flash_read= Const.s_Rap[0].start_po;   }    }
	   else if(email==3){   SendDataToModemWifi("AUTH LOGIN\r\n"); }
	   else if(email==4)
	   {
		  /* if((wybor_bitowy&0x2000)>0) base64_encode("mirek.sekula@poczta.fm", buf_p, strlen("mirek.sekula@poczta.fm"));
		   else */                       base64_encode(Const.s_Email[0].nad, buf_p, strlen(Const.s_Email[0].nad));
		   sprintf(&buf_rx2[_Size_BufWifi],"%s\r\n",buf_p);  dbg3("\r\n111:  "); dbg3(buf_p);  dbg3("  ");
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==5)
	   {
		   /* if((wybor_bitowy&0x2000)>0) base64_encode("mirek123456789", buf_p, strlen("mirek123456789"));
		   else */                       base64_encode(Const.s_Email[0].has, buf_p, strlen(Const.s_Email[0].has));
		   sprintf(&buf_rx2[_Size_BufWifi],"%s\r\n",buf_p);  dbg3("\r\n222:  "); dbg3(buf_p);  dbg3("  ");
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==6)
	   {
		   /* if((wybor_bitowy&0x2000)>0) sprintf(&buf_rx2[_Size_BufWifi],"MAIL From: <mirek.sekula@poczta.fm>\x0D\x0A");
		   else  */                      sprintf(&buf_rx2[_Size_BufWifi],"MAIL From: <%s>\x0D\x0A",Const.s_Email[0].nad);
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==7)
	   {
	          if(wysylka_mail==1) sprintf(buf_p,Const.s_Email[0].odb1);
	     else if(wysylka_mail==2) sprintf(buf_p,Const.s_Email[0].odb2);
	     else if(wysylka_mail==3) sprintf(buf_p,Const.s_Email[0].odb3);
	     else if(wysylka_mail==4) sprintf(buf_p,Const.s_Email[0].odb4);
	     else if(wysylka_mail==5) sprintf(buf_p,Const.s_Email[0].odb5);
	     else if(wysylka_mail==6) sprintf(buf_p,Const.s_Email[0].odb6);
	     else if(wysylka_mail==7) sprintf(buf_p,Const.s_Email[0].odb7);
	     else if(wysylka_mail==8) sprintf(buf_p,Const.s_Email[0].odb8);
	     else                     sprintf(buf_p,Const.s_Email[0].odb1);

		   sprintf(&buf_rx2[_Size_BufWifi],"RCPT To: <%s>\x0D\x0A",buf_p);
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	        email=7;
	   }
	   else if(email==8)
	   {
		     SendDataToModemWifi("DATA\x0D\x0A");
	   }
	   else if(email==9)
	   {
		   /*if((wybor_bitowy&0x2000)>0) sprintf(&buf_rx2[_Size_BufWifi],"From: <mirek.sekula@poczta.fm>\x0D\x0A");
		   else     */                   sprintf(&buf_rx2[_Size_BufWifi],"From: <%s>\x0D\x0A",Const.s_Email[0].nad);
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==10)
	   {
		   sprintf(&buf_rx2[_Size_BufWifi],"To: <%s>\x0D\x0A",Const.s_Email[0].odb1);
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==11)
	   {
		   sprintf(&buf_rx2[_Size_BufWifi],"Subject: %s\x0D\x0A",email_tytul);
		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==12)
	   {
	   		 sprintf(&buf_rx2[_Size_BufWifi],"Content-Type: text/plain\x0D\x0A");
	   		SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
	   		if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')) email=200;
	   }
	   else if(email==200)
	   {
		   sprintf(&buf_rx2[_Size_BufWifi],"Content-Disposition: attachment; filename=POMIARY.txt\x0D\x0A");
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
		   if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')) email=12;
	   }
	   else if(email==13)
	   {
		   sprintf(&buf_rx2[_Size_BufWifi],"\x0D\x0A\x0D\x0A");
		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==14)
	   {
		   if((wybor2_bitowy&0x0004)>0){  wybor2_bitowy&=~0x0004;
		         sprintf(&buf_rx2[_Size_BufWifi],"Tresc testowa\x0D\x0A");
		   }
		   else
		   {
			   if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o')){

				   if(RaportStanu(&RapAddr_p,0)==0) email--;
			   }
			   else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')){

				   if(OdczytZdarzenSmtp(&RapAddr_p)==1) email--; // sprintf(buf_p,"\r\nHHH2222: %d  %d ",RapAddr_p, strlen(&buf_rx2[_Size_BufWifi]) ); dbg3(buf_p);
				   delay_funkc[12]=1;
			   }
			   else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){

				    adr_flash_read=RapAddr_p;   if(OdczytRejTempSmtpWifi(&op)==1) email--;    delay_funkc[12]=1;
			   }
			   else
			       sprintf(&buf_rx2[_Size_BufWifi],"%s\x0D\x0A",&b_mail[_Dlg_buf_Mail*(wysylka_mail-1)]);   //&b_mail[_Dlg_buf_Mail*(wysylka_mail-1)]
		   }

		   SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); /* dbg3("\r\n"); dbg3_roz(&buf_rx2[_Size_BufWifi]);*/   dbg3("\r\nTRESC... ");   delay_funkc[29]=1;
	   }
	   else if(email==15)
	   {
		   sprintf(&buf_rx2[_Size_BufWifi],".\x0D\x0A");
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
	   }
	   else if(email==16)
	   {
		   sprintf(&buf_rx2[_Size_BufWifi],"QUIT\x0D\x0A");
		     SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);  dbg3(&buf_rx2[_Size_BufWifi]);
	   }

  }

if(email==0){
  if(ptr=strstr(buf_rx2,"STATUS:"))
  {
  	// if((*(ptr+7)!='2')||(*(ptr+7)!='4'))
  	  *(ptr+8)=0;
  	  dbg3(ptr);

  	   ResumptionReceiveDataFromWifi();
  }
  if(ptr=strstr(buf_rx2,"+CWSAP"))
  {
	  HAL_Delay(100);
	  dbg3(ptr);

	   ResumptionReceiveDataFromWifi();
  }
}
 // if(strstr(buf_rx2,"CLOSED")){  dbg3("__...0,CLOSED__"); licz_htt=20;  li_WifiCipsend[1]=0; ResumptionReceiveDataFromWifi(); }

  if(strstr(buf_rx2,"\r\nSEND OK")&&(email>0))
  {
	  delay_funkc[30]=0;  // zawieszenie   mail ESP podczas powtorki AT+CIPSTART.... i nic wiecej
		HAL_Delay(200);
		//dbg3(&buf_rx2[0]);
		if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')){
			  debug_smtp_esp=1;  ZapisSesjiSMTP(&buf_rx2[0], strlen(&buf_rx2[0]), &adr_flash_SesjaSMTP);
		}
		licz_delay_ekran=0;  ekran_tymczasowy=3;

		ptr2= &buf_rx2[0];
		powtorziii:
			  if(ptr=strstr(ptr2,"\r\n+IPD,"))
			  {  port_smtp_p= *(ptr+7)&0x0f;
				  if(ptr=strstr(ptr,":")){
				   if((*(ptr+1)>0x20)&&(*(ptr+1)<0x80)){
				      snprintf(info_email,60,(ptr+1));
				      if(strstr(info_email,"450 ")||strstr(info_email,"535 "))
				      {
				    	        ResumptionReceiveDataFromWifi();
				    	 	 	   dbg3("\r\nMAIL BLAD !!!  ");  delay_funkc[17]=1;
				    	 	 	   email=0;
				    	 	 	   wybor_bitowy&=~0x2000;  //zerujemy powtorke
				    	 	 	   wybor2_bitowy&=~0x0004; //zerujemy zadanie test_email

				    	 	 	  //sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=0\x0D\x0A");  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); HAL_Delay(30);
				    	 	 	  //sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=1\x0D\x0A");  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); HAL_Delay(30);
				    	 	 	 // sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=2\x0D\x0A");  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); HAL_Delay(30);

				    	 	 	   delay_funkc[12]=0;  goto omindddddd;
				      }
				      if((info_email[0]=='G')&&(info_email[1]=='E')&&(info_email[2]=='T'))  //+IPD,0,431:GET /TME.txt HTTP/1.1
				      {
					 		 /*sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=%d\x0D\x0A",port_smtp_p);
					 		  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
					 		  dbg3("\r\nAA: "); dbg3(&buf_rx2[_Size_BufWifi]);*/
					 		  port_smtp_p=100;
				    	      info_email[0]=0;
				      }
				      else{ port_smtp_p=100;  ptr2=ptr; goto powtorziii; }
				   }
				   else port_smtp_p=100;
			     }
				 else port_smtp_p=100;
			  }
			  else port_smtp_p=100;


	 	  if(email==9)
	 	  {
		 	   ResumptionReceiveDataFromWifi();
	 		   sprintf(&buf_rx2[_Size_BufWifi+100],"To: <%s>\x0D\x0A",Const.s_Email[0].odb1);
	 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
	 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
		 	   email=10;
	 	  }
	 	  else if(email==10)
	 	  {
		 	   ResumptionReceiveDataFromWifi();
	 		   sprintf(&buf_rx2[_Size_BufWifi+100],"Subject: %s\x0D\x0A",email_tytul);
	 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
	 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
		 	   email=11;
	 	  }
	 	  else if(email==11)
	 	  {
		 	   ResumptionReceiveDataFromWifi();
	 		   sprintf(&buf_rx2[_Size_BufWifi+100],"Content-Type: text/plain\x0D\x0A");
	 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
	 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
                email=12;
	 	  }
	 	  else if(email==200)
	 	  {

	 		 ResumptionReceiveDataFromWifi();

	 			 sprintf(&buf_rx2[_Size_BufWifi+100],"Content-Disposition: attachment; filename=POMIARY.txt\x0D\x0A");
	 				 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
	 				 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);


	 	  }
	 	  else if(email==12)
	 	  {

	 		 ResumptionReceiveDataFromWifi();

	 			    sprintf(&buf_rx2[_Size_BufWifi+100],"\x0D\x0A\x0D\x0A");
	 				 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
	 				 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	 				 		    email=13;


	 	  }
	 	  else if(email==13)
	 	  {
		 	   ResumptionReceiveDataFromWifi();
	 		   if((wybor2_bitowy&0x0004)>0)
	 		   {
	 			   sprintf(&buf_rx2[_Size_BufWifi+600],"Tresc testowa\x0D\x0A");
	 			   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+600]));
	 		   }
	 		   else
	 		   {
	 			   if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o')){
	 				  RapAddr_p=RapAddr;  RaportStanu(&RapAddr,0);
	 			       sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,RapAddr-RapAddr_p);  //dbg3("\r\nGGGGGGGGGG: "); dbg3(&buf_rx2[_Size_BufWifi]);
	 			   }
	 			   else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')){
		 				  RapAddr_p=RapAddr;  OdczytZdarzenSmtp(&RapAddr); // sprintf(buf_p,"\r\nHHH: %d  %d ",RapAddr-RapAddr_p, strlen(&buf_rx2[_Size_BufWifi]) ); dbg3(buf_p);
		 			       sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,RapAddr-RapAddr_p);  //dbg3("\r\nGGGGGGGGGG: "); dbg3(&buf_rx2[_Size_BufWifi]);
		 		   }
	 			   else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){
	 				      RapAddr_p=adr_flash_read;  OdczytRejTempSmtpWifi(&op);
		 			       sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,op);   dbg3(&buf_rx2[_Size_BufWifi]);
		 		   }
	 			   else {
	 				   sprintf(&buf_rx2[_Size_BufWifi+600],"%s\x0D\x0A",&b_mail[_Dlg_buf_Mail*(wysylka_mail-1)]);   //&b_mail[_Dlg_buf_Mail*(wysylka_mail-1)]
	 				   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+600]));
	 			   }
	 		   }
	 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
		 	   email=14;
	 	  }
	 	  else if(email==14)
	 	  {
		 	   ResumptionReceiveDataFromWifi();
	 		   sprintf(&buf_rx2[_Size_BufWifi+100],".\x0D\x0A");
	 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
	 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
		 	   email=15;
	 	  }





	 	if(strstr(buf_rx2,"250"))
	 	{
	 		if(email==6)
	 		{
	 		   ResumptionReceiveDataFromWifi();

		             if(wysylka_mail==1) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb1);
		        else if(wysylka_mail==2) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb2);
		        else if(wysylka_mail==3) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb3);
		        else if(wysylka_mail==4) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb4);
		        else if(wysylka_mail==5) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb5);
		        else if(wysylka_mail==6) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb6);
		        else if(wysylka_mail==7) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb7);
		        else if(wysylka_mail==8) sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb8);
		        else                     sprintf(&buf_rx2[_Size_BufWifi+100],"RCPT To: <%s>\x0D\x0A",Const.s_Email[0].odb1);

	 		        dbg3("\r\n"); dbg3(&buf_rx2[_Size_BufWifi+100]);

	  		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
	  		   SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	 	 	   email=7;

	 		}
	 		else if(email==7)
	 		{
	 	 	   ResumptionReceiveDataFromWifi();
	  		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,6\x0D\x0A",port_smtp);
	  		   SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	 	 	   email=8;
	 		}
	 		else if(email==2)
	 		{
	 		   ResumptionReceiveDataFromWifi();
	 	       sprintf(buf_p,"AT+CIPSEND=%d,12\x0D\x0A",port_smtp);   SendDataToModemWifi(buf_p);
	 	       email=3;
	 		}
	 		else if(email==15)
	 		{
		 	   ResumptionReceiveDataFromWifi();
		  	   sprintf(&buf_rx2[_Size_BufWifi+100],"QUIT\x0D\x0A");
		  	   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
		  	  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	 	       email=16;
	 		}
	 	}
	 	else if(strstr(buf_rx2,"334 "))
	 	{
	 	   ResumptionReceiveDataFromWifi();
	 	   if(email==3)
	 	   {
	 		  /* if((wybor_bitowy&0x2000)>0) base64_encode("mirek.sekula@poczta.fm", &buf_rx2[_Size_BufWifi], strlen("mirek.sekula@poczta.fm"));
	 		   else  */                      base64_encode(Const.s_Email[0].nad, &buf_rx2[_Size_BufWifi], strlen(Const.s_Email[0].nad));
	 		   sprintf(buf_p,"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi])+2);
	 		  // dbg3("\r\n"); dbg3(&buf_rx2[_Size_BufWifi]);   dbg3("   ");
	 	   }
	 	   if(email==4)
	 	   {
	 		   /*if((wybor_bitowy&0x2000)>0) base64_encode("mirek123456789", &buf_rx2[_Size_BufWifi], strlen("mirek123456789"));
	 		   else   */                     base64_encode(Const.s_Email[0].has, &buf_rx2[_Size_BufWifi], strlen(Const.s_Email[0].has));
	 		   sprintf(buf_p,"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi])+2);
	 		   // dbg3("\r\n"); dbg3(&buf_rx2[_Size_BufWifi]);   dbg3("   ");
	 	   }
	 	    SendDataToModemWifi(buf_p);  //base64 name
	 	   email++;
	 	}
	 	else if(strstr(buf_rx2,"235 "))
	 	{
	 	   ResumptionReceiveDataFromWifi();
	 	  /* if((wybor_bitowy&0x2000)>0) sprintf(&buf_rx2[_Size_BufWifi+100],"MAIL From: <mirek.sekula@poczta.fm>\x0D\x0A");
	 	   else   */                     sprintf(&buf_rx2[_Size_BufWifi+100],"MAIL From: <%s>\x0D\x0A",Const.s_Email[0].nad);
 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
 		   SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	 	   email=6;
	 	}
	 	else if(strstr(buf_rx2,"354 "))
	 	{
	 	   ResumptionReceiveDataFromWifi();
	 	  /* if((wybor_bitowy&0x2000)>0) sprintf(&buf_rx2[_Size_BufWifi+100],"From: <mirek.sekula@poczta.fm>\x0D\x0A");
	 	   else    */                    sprintf(&buf_rx2[_Size_BufWifi+100],"From: <%s>\x0D\x0A",Const.s_Email[0].nad);
 		   sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPSEND=%d,%d\x0D\x0A",port_smtp,strlen(&buf_rx2[_Size_BufWifi+100]));
 		    SendDataToModemWifi(&buf_rx2[_Size_BufWifi]);
	 	   email=9;
	 	}
	 	else if(strstr(buf_rx2,"221 "))
	 	{
	 	   ResumptionReceiveDataFromWifi();     delay_funkc[29]=0;
	 	   dbg3("\r\nMAIL wyslano  ");  delay_funkc[17]=1;     if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){  Const.s_Rap[0].start_po= adr_flash_read;  delay_funkc[6]=1;/*zapisz flash*/  }
	 	   email=0;                                            if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')){  Const.s_Rap[0].start_zd= RapAddr;         delay_funkc[6]=1;/*zapisz flash*/  }
	 	   wybor_bitowy&=~0x2000;  //zerujemy powtorke
	 	   delay_funkc[32]=1;   //Odblokuj 'Wyslij_zdarzenia'

	 	  //sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=1\x0D\x0A");  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); HAL_Delay(30);
	 	//  sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=2\x0D\x0A");  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); HAL_Delay(30);
	 	  //sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=3\x0D\x0A");  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); HAL_Delay(30);
	 	 // sprintf(&buf_rx2[_Size_BufWifi],"AT+CIPCLOSE=4\x0D\x0A");  SendDataToModemWifi(&buf_rx2[_Size_BufWifi]); HAL_Delay(30);

	 	   delay_funkc[12]=0;  goto omindddddd;
	 	}

	  dbg3("__SEND OK__");    delay_funkc[29]=0;

      omindddddd:
	  asm("nop");

  }
  if(ptr=strstr(buf_rx2,"\r\n+IPD,"))
  {
	 if(email==0)
	 {
	  	HAL_Delay(100);  //dbg3(&buf_rx2[0]);
	  	delay_funkc[31]=0;    //Test wifi komend¹ AT  czy ok



#ifdef _CLOUD

	  	if(ptr3=strstr(buf_rx2,"\r\nElektronika RM"))  //clo
	  	{
	  		dbg3("\r\nDANE:\r\n"); //dbg3(buf_rx2);

	  		cload_activ=0;

	  		if(ptr3=strstr(ptr3,"--"))
	  		{
	  			ni=0;
	  			if(ptr3[2]=='1')
	  			{
	  				dbg3("*1*");

	  				 if((Const.s_WY[ni/16].val&(1<<(ni-16*(ni/16))))==0)
	  			     {	 Const.s_WY[ni/16].val|=(1<<(ni-16*(ni/16)));
	  			     	 GPIO_PK(ni);
	  			         sprintf(buf_PK, "<font color=\"#eee\">%s</font>",Const.s_PK[ni].nazwa_on);
	  			         ZapisZdarzenia(_ZdaPKwww,_PKon,ni);
	  			#ifdef _LoRa
	  			  if(lifeLora[t_wy[ni]-1]>0) Lora_Send_Przek(t_wy[ni],1);
	  			#endif
	  			     }
	  			}
	  			else
	  			{
	  				dbg3("*0*");

	  			     if((Const.s_WY[ni/16].val&(1<<(ni-16*(ni/16))))>0)
	  			     {	 Const.s_WY[ni/16].val&=~(1<<(ni-16*(ni/16)));
	  			     	 GPIO_PK(ni);
	  			         sprintf(buf_PK, "<font color=\"#000\">%s</font>",Const.s_PK[ni].nazwa_of);
	  			         ZapisZdarzenia(_ZdaPKwww,_PKoff,ni);
	  			#ifdef _LoRa
	  			  if(lifeLora[t_wy[ni]-1]>0) Lora_Send_Przek(t_wy[ni],0);
	  			#endif
	  			     }
	  			}
	  		}

	  		ZakonczPolaczenieTCP();
	  		ResumptionReceiveDataFromWifi();

	  		goto omin_ipd;
	  	}

#endif


	  	if((WhichPage!=6)&&(WhichPage!=8)) scroll=0;



  	      wskk=0;  // dla 'busy'


          //sprintf(buf_p,"\r\nRRRRR: %c",HttpPort);  dbg3(buf_p);


  	  if(ptr1=strstr(buf_rx2,"GET / HTTP/"))
  	  {
		  i=0;
  		  porrr:
  		  if(*(ptr1-i++)=='\n'); else goto porrr;
  		  ptr=(ptr1-i); ptr1=strstr(ptr,"\r\n+IPD,");
  		  HttpPort=ptr1[7]; // sprintf(buf_p,"\r\nRRRRR: %c",HttpPort);  dbg3(buf_p);


  		     WhichPage=15;


  		  wybor_bitowy|=0x0200;

  		 // HttpAuthEsp(buf_rx2);

  		 // #ifdef _DebugEnable
  		  //   dbg3(&buf_rx2[0]);
  		 // #endif
  		   //MainPanel();
  		 ResumptionReceiveDataFromWifi();
  		 HttpPageStartSend();

  	  }
  	  else if(ptr1=strstr(buf_rx2,"GET /lpc.cgi"))
  	  {//dbg3_roz(&buf_rx2[0]);
		  i=0;
  		  porrr1:
  		  if(*(ptr1-i++)=='\n'); else goto porrr1;
  		  ptr=(ptr1-i); ptr1=strstr(ptr,"\r\n+IPD,");
  		  HttpPort=ptr1[7]; // sprintf(buf_p,"\r\nRRRRR: %c",HttpPort);  dbg3(buf_p);
  		   // HAL_Delay(3000);
          i=0;
  		  poworz_cgi:
  		      if(ptr1=strstr(buf_rx2,"n_109_zapisz"))
  		      {
  		    	 // *(ptr1+13)=0;
  		    	 // dbg3("\r\nn_109_zapisz");

  		      }
  		 else if(strstr(buf_rx2,"n_czas_zapisz"));
  		 else if(strstr(buf_rx2,"testemail")) delay_funkc[14]=1;
  		 else if(strstr(buf_rx2,"n_112_zapisz"))  //Zielony 'Adresuj'
  		 {

  		 }
  		 else if(ptr1=strstr(buf_rx2,"pprzek"))  //ster www przekaznikami
  		 {
             /* j=10*(ptr1[6]&0x0f)+(ptr1[7]&0x0f);
              if((Const.s_WY[0].val&(1<<j))>0) Const.s_WY[0].val&=~(1<<j);
              else                             Const.s_WY[0].val|=(1<<j);
              GPIO_PK(j);
              ZapiszFlash();
  			 sprintf(buf_p,"\r\nPPRZEK   %d  ",j); dbg3(buf_p); HAL_Delay(2000);*/
  		 }
  		 else if(strstr(buf_rx2,"wykres1_"));
  		 else if(strstr(buf_rx2,"wykres2"));
  		 else if(strstr(buf_rx2,"rstDPT"));
  		 else if(strstr(buf_rx2,"rstDPN"));
  		 else if(strstr(buf_rx2,"rstRZT"));
  		 else if(strstr(buf_rx2,"rstRZN"));
  		 else if(strstr(buf_rx2,"wykrprzedz"));
  		 else if(ptr=strstr(buf_rx2,"n_115_zapisz"))  //slider PWM
  		 {
  			if(ptr1=strstr(buf_rx2,"n_pwm"));
  			for(i=0;i<_Size__s_pwm;i++)
  			{
  			  if(ptr=strstr(ptr1,"n_pwm"))
  			  {
  				 j=(ptr[5]&0x0f);  j--;
  				 Const.s_PWM[j].duty = odczyt_liczby_cgi(ptr+7);   sprintf(buf_p,"\r\nJJ: %d",Const.s_PWM[j].duty); dbg3(buf_p); //HAL_Delay(3000);
  				 /*     if(j==0) PWM_1(Const.s_PWM[j].freq, Const.s_PWM[j].duty);
  			     else if(j==1) PWM_2(Const.s_PWM[j].freq, Const.s_PWM[j].duty);
  			     else if(j==2) PWM_3(Const.s_PWM[j].freq, Const.s_PWM[j].duty);
  			     else if(j==3) PWM_4(Const.s_PWM[j].freq, Const.s_PWM[j].duty);
  			     else if(j==4) PWM_5(Const.s_PWM[j].freq, Const.s_PWM[j].duty);
  			     else if(j==5) PWM_7(Const.s_PWM[j].freq, Const.s_PWM[j].duty);*/
  			  }
  			  else break;
  			  ptr1=ptr+2;
  			}
  			//ZapiszFlash();
  			FlashWsadPrg( &Const.s_PWM[0].freq, ((2+1)*_Size__s_pwm) );
  		 }
  		 else
  		 {
  			if(WhichPage!=1669)
  			{
  	  			 if(strstr(buf_rx2,"n_110_zapisz")){ delay_funkc[1]=1;  goto omin_htp_cgi; }
  	  			 HAL_Delay(1000);
  	  			 if(i>10) goto omin_htp_cgi;
  	  			 i++;
  	  			 dbg3("cgi.");
  	  			 goto poworz_cgi;

  			}
  		 }

           // #ifdef _DebugEnable
		      // dbg3_roz(&buf_rx2[0]);
		   // #endif



  		    if(WhichPage==1669)
  		    {
  		    	char *pttr1, *pttr2;    //dbg3_roz(&buf_rx2[0]);
  		    	pttr2= buf_rx2;
  		    	do
  		    	{
            	   pttr1=pttr2;
            	   pttr2 = HttpLogicParser(pttr1);

  		    	}while(pttr2!=pttr1);

  		        dbg3("\r\nZapis...");
  		         Flash_in_progr_char_na_32(_AdrFlashToRAM, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
  		        dbg3("ok ");
  		      FunctionLogicOperation();

  		    }
  		    else Http_cgi(buf_rx2);
  		    omin_htp_cgi:
			ResumptionReceiveDataFromWifi();
			if(HttpRefreshSend(HttpWhichPage())==1)  InitUartWifi();

  	  }
  	/*  else if(strstr(buf_rx2,"GET /jquery"))
  	  {
  		  dbg3("\r\n##################@@@@@@@@@@@");
  		  fff2();
  	  }*/
 //####################### -- Trans  Slave-Master ###################################
   	 /* else if(ptr1=strstr(buf_rx2,"GET /TRANS_1"))
   	  {
   		  HAL_Delay(2000);
   		  WskazPortHttp(ptr1);

   		  i=OdbierzDane(&buf_rx2[0]);   dbg3("\r\nDANE:\r\n"); dbg3(buf_rx2);  dbg3("\r\n");
   		  if(i==0) dbg3("Hash OK ");  else dbg3("Hash ERROR !!! ");
   		  ResumptionReceiveDataFromWifi();
   		  WyslijKomend("GET /TRANS_2");

 		    for(i=0;i<3999;i++) buf_rx2[_Size_BufWifi+i]='B';  buf_rx2[_Size_BufWifi+i]=0;
 		    buf_rx2[_Size_BufWifi+0]='1';
 		    buf_rx2[_Size_BufWifi+1999]='2';
 		    buf_rx2[_Size_BufWifi+3998]='3';
 		  if(WyslijDane(&buf_rx2[_Size_BufWifi],3999)==0);
   		//if(WyslijDane("\x01\x02\x03\x04\x05",5)==0);

   		  ZakonczPolaczenieTCP();
   		  ResumptionReceiveDataFromWifi();
   	  }
   	  else if(ptr1=strstr(buf_rx2,"GET /TRANS_2"))
   	  {
   		 HAL_Delay(500);
   		  dbg3("\r\nKURDE MOL !!!!!!");
   		  i=OdbierzDane(&buf_rx2[0]);   dbg3("\r\nDANE:\r\n");  dbg3(buf_rx2);
  		  if(i==0) dbg3("Hash OK ");  else dbg3("Hash ERROR !!! ");

   		 //sprintf(buf_p,"%02x %02x %02x %02x %02x",buf_rx2[12],buf_rx2[13],buf_rx2[14],buf_rx2[15],buf_rx2[16]);
         //dbg3(buf_p);

   		 dbg3("\r\n");
   		  ResumptionReceiveDataFromWifi();
   	  }*/






 //######################################################################################

  	  else if(ptr1=strstr(buf_rx2,"GET /"))
  	  {
		  i=0;
  		  porrr2:
  		  if(*(ptr1-i++)=='\n'); else goto porrr2;
  		  ptr=(ptr1-i); ptr1=strstr(ptr,"\r\n+IPD,");
  		  HttpPort=ptr1[7]; // sprintf(buf_p,"\r\nRRRRR: %c",HttpPort);  dbg3(buf_p);

  		 // #ifdef _DebugEnable
  		  //   dbg3(&buf_rx2[0]);
  		 // #endif

  		 WhichPage_p = WhichPage;
  		 WhichPage = WhichPageForGET(buf_rx2);    //sprintf(buf_p,"\r\nGGGGG: %d  ",WhichPage ); dbg3(buf_p);
  		 zmiana_ekranu();
  		 wybor_bitowy|=0x0200;

  		if(strstr(buf_rx2,"GET /TME.txt"));
  		else if(strstr(buf_rx2,"GET /SensLora.txt"));
  		else if(strstr(buf_rx2,"GET /Sensagh.txt"));
  		else if(strstr(buf_rx2,"GET /plik.txt"));
  		else if(strstr(buf_rx2,"GET /sesjaSMTP.txt"));
  		else if(strstr(buf_rx2,"GET /liczgodz.txt"));
  		else if(strstr(buf_rx2,"GET /liczdob.txt"));
  		else if(strstr(buf_rx2,"GET /licztyg.txt"));
  		else if(strstr(buf_rx2,"GET /liczmies.txt"));
  		else if(strstr(buf_rx2,"GET /TempWilg.txt"));
  		else if(strstr(buf_rx2,"GET /favicon.ico"));
  		//else   HttpAuthEsp(buf_rx2);


	     if(WhichPage==900)  //IRprg
		 {
			WhichPage=WhichPage_p;
			i= WyszukujPortElement(t_ir, _Size_ir, t_wy[WhichPage_p-500-1]);
           i=2*i;
			 sprintf(buf_p,"\r\nPrg IR:%d  ",i+1);  dbg3(buf_p);
			 while(do_buf_kod_pilota()==0);
			 Flash_in_progr_16( i*FLASH_PAGE_SIZE+_AdrFlashToIR, &buf_irda[0], (int)buf_irda[0]+1);
			 sprintf(buf_p," %d  ",buf_irda[0]); dbg3(buf_p);
		     dbg3("OK  ");

		 }
	     else if(WhichPage==901)  //IRprg
		 {
			WhichPage=WhichPage_p;
			i= WyszukujPortElement(t_ir, _Size_ir, t_wy[WhichPage_p-500-1]);
           i=2*i+1;
			 sprintf(buf_p,"\r\nPrg IR:%d  ",i+1);  dbg3(buf_p);
			 while(do_buf_kod_pilota()==0);
			 Flash_in_progr_16( i*FLASH_PAGE_SIZE+_AdrFlashToIR, &buf_irda[0], (int)buf_irda[0]+1);
			 sprintf(buf_p," %d  ",buf_irda[0]); dbg3(buf_p);
		     dbg3("OK  ");

		 }

  		   if(WhichPage==5)
  		   {
  			  SearchWifiFunction();
  			  delay_funkc[2]=1;
  		   }
  		  // else if(WhichPage==6) wybor_bitowy|=0x0200;
  		   //else if(WhichPage==15) wybor_bitowy|=0x0200;
  		   else if(WhichPage==16){  HttpDynTME(); /*dbg3(buf_TME);*/ }
  		   else if(WhichPage==77)
  		   {  if(ptr1=strstr(buf_rx2,"GET /SensLora.txt"))
  		      {
                unsigned short ppm;
                ptr1+=18;  j=0;
                for(i=0;i<_IleLor;i++)
                {
               	   ppm= wybierz_cyfry(ptr1);
               	   if(ppm!=Const.s_Map[0].x[i]) j=1;
               	   Const.s_Map[0].x[i]=ppm;
               	   while(1){ ptr1++; if(*ptr1=='_'){ ptr1++; break; } }

               	   ppm= wybierz_cyfry(ptr1);
               	   if(ppm!=Const.s_Map[0].y[i]) j=1;
               	   Const.s_Map[0].y[i]=ppm;
                  while(1){ ptr1++; if(*ptr1=='_'){ ptr1++; break; } }
                }
                if(j==1) ZapiszFlash();

  		       // if((wybor2_bitowy&0x0040)>0){ wybor2_bitowy&=~0x0040; sprintf(buf_TME, "+13.40+22.51-30.20+43.40+52.51-10.20+23.40+32.51-10.20+23.40+32.51-10.20"); }
  		      //  else                        { wybor2_bitowy|=0x0040;  sprintf(buf_TME, "+24.51+33.60-09.11+24.51+33.60-09.11+24.51+33.60-09.91+24.51+45.60-09.71"); }

                char *pkh=&buf_TME[0];  int tt,ww;
                for(int iv=0;iv<12;iv++)
			    {

		    	     if(iv==0) i=5;
		    	else if(iv==1) i=6;
		    	else if(iv==2) i=7;
		    	else if(iv==3) i=8;
		    	else if(iv==4) i=9;//17;
		    	else if(iv==5) i=10;//18;
		    	else if(iv==6) i=28;
		    	else if(iv==7) i=21;
		    	else if(iv==8) i=22;//15;
		    	else if(iv==9) i=23;
		    	else if(iv==10) i=24;
		    	else if(iv==11) i=33;

			    	     i--;

			    		 if(Const.s_GPIO[i].val==2)  //DS
			    		 {
			    			 j= WyszukajNrCzujkiDlaPortu(i+1);
			    			 if(j!=-1)
			    			 {
				    			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';
				    			 if((Var.value[j]>0x063f)&&(Var.value[j]<0x7FFF)){ buf_p[50+4]=' '; buf_p[50+5]=0; }
				    		 	  buf_p[60]='-';
				    		 	  buf_p[61]='-';
				    		 	  buf_p[62]='%';
				    		 	  buf_p[63]=0;
				    		 	  if(lifeLora[i-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
				    		     sprintf(pkh,"1%s%c%c%s0", &buf_p[50], KolorTemp(j), buf_p[70] ,&buf_p[60]);  pkh+=strlen(pkh);
			    			 }
			    		 }
			    		 else if(Const.s_GPIO[i].val==9)  //DHT
			       	     {
			    			 j= WyszukajNrCzujkiDlaPortu(i+1);
			    			 if(j!=-1)
			    			 {
				    			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';  tt=KolorTemp(j);
				       	         j= WyszukajNrCzujkiDlaPortu_2(i+1);
				       	         if(j!=-1)
				       	         {
				       	    	     wartTemp_odwrotnie(&buf_p[60],Var.value[j]); if(buf_p[60+1]=='0') buf_p[60+1]=' ';  ww=KolorTemp(j);
				       	         }
				    		 	 if(lifeLora[i-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
				       	         sprintf(pkh,"2%s%c%c%c%c",&buf_p[50],tt, buf_p[70],buf_p[61],buf_p[62]);
				       	         a=strlen(pkh); pkh[a++]='%'; pkh[a++]=ww; pkh[a]=0;   pkh+=strlen(pkh);
			    			 }
			       	     }
			    		 else if(Const.s_GPIO[i].val==3)  //Pt
			       	     {
			    			 j= WyszukujPortElement(t_pt,_Size_pt, i+1);
			    			 if(j!=-1)
			    			 {
				       	         if(Const.s_ADC_pt[j].val==0x7FFF){ sprintf(pkh,"3  --- 0 --0"); pkh+=strlen(pkh); }
				       	         else
				       	         {
				       	        	 sprintf(buf_p,"          ");
				       	        	 if((int16_t)Const.s_ADC_pt[j].val<0)
				       	        	 {
				       	        		 buf_p[0]='-';
				       	        		 sprintf(&buf_p[1],"%d",(int16_t)Const.s_ADC_pt[j].val);   buf_p[1]=' '; buf_p[1+strlen(&buf_p[1])]=' ';
				       	        	 }
				       	        	 else
				       	        	 {
				       	        		 buf_p[0]='+';
				       	        		 sprintf(&buf_p[2],"%d",(int16_t)Const.s_ADC_pt[j].val);   buf_p[1]=' '; buf_p[2+strlen(&buf_p[2])]=' ';
				       	        	 }
				       	        	 tt=KolorTemp( WyszukajNrCzujkiDlaPortu(i+1));
				       	        	 buf_p[5]=tt;
				       	        	 buf_p[6]=0;
				       	        	 sprintf(pkh,"3%s --- ",buf_p);  pkh+=strlen(pkh);
				       	         }

			    			 }
			       	     }
			    		 else if(Const.s_GPIO[i].val==4)  //WE
			    		 {
			    			 j= WyszukujPortElement(t_we,_Size_we, i+1);
							 if(j!=-1)
							 {
				    			 if((Var.we[j/16]&(1<<(j-16*(j/16))))>0){ if(lifeLora[i-1]>0) sprintf(pkh,"4-----3 ---0"); else sprintf(pkh,"4-----1 ---0");  }
				    			 else                                   { if(lifeLora[i-1]>0) sprintf(pkh,"4-----2 ---0"); else sprintf(pkh,"4-----0 ---0");  }
				    			 pkh+=strlen(pkh);
							 }

			    		 }
			    		 else if(Const.s_GPIO[i].val==0)  //PK
			    		 {
			    			 j= WyszukujPortElement(t_wy,_Size_wy, i+1);
							 if(j!=-1)
							 {
								 if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){ if(lifeLora[i-1]>0) sprintf(pkh,"5-----3 ---0"); else sprintf(pkh,"5-----1 ---0");  }
				    			 else                                           { if(lifeLora[i-1]>0) sprintf(pkh,"5-----2 ---0"); else sprintf(pkh,"5-----0 ---0");  }
				    			 pkh+=strlen(pkh);
							 }

			    		 }
			    		 else
			    		 {
			    	 	    buf_p[60]='-';
			    		    buf_p[61]='-';
			    		    buf_p[62]='%';
			    		    buf_p[63]=0;
			    		    sprintf(pkh,"0 --.-0 %s0",&buf_p[60]);   pkh+=strlen(pkh);
			    		 }



			    }


  		      }
  		     /* else if(ptr1=strstr(buf_rx2,"GET /Sensagh.txt"))
		      {
  		    	  j=0;
  		    	  for(i=0;i<_Ile_Czujek;i++)
  		    	  {
  		    		 if(lifeLora[i]==0) ni=0;
  		    	     else if(lifeLora[i]==_Czas_Zycia_Czujek+100) ni=2;
  		    	     else               ni=1;

  		    	     sprintf(&buf_TME[j], "%d%d%d%d ", (Lora[i]>>0)&0x01, (Lora[i]>>1)&0x01, (Lora[i]>>2)&0x01, ni );
  		    	     j += strlen(&buf_TME[j]);
  		    	  }
  		    	  dbg3(" Sensagh.txt");
		      }*/


  		   }
  		   else if(WhichPage==17)
  		   {   if(ptr1=strstr(buf_rx2,"GET /mobile/pk"))
  			   {
  			       HttpMobilePK(  100*(0x0f&(*(ptr1+14)))+10*(0x0f&(*(ptr1+15)))+(0x0f&(*(ptr1+16)))   -1);

  			   }
  		     /*  else if(ptr1=strstr(buf_rx2,"GET /mobile/aghpk"))
  			   {
  		    	   ni =  100*(0x0f&(*(ptr1+17)))+10*(0x0f&(*(ptr1+18)))+(0x0f&(*(ptr1+19)));
  		    	   sprintf(buf_PK, "<img src='data:image/png;base64,'>");

                   sprintf(buf_p,"\r\nAGH PK %d ",ni); dbg3(buf_p);

                   i_pwr[ni-1]=4;  Lora_Send_PK(ni);   //max power TX
                   Lora[ni-1]=0;
                   ZapisZdarzenia(_ZdaLoRa,_LoObiektOdw,ni);
                   lifeLora[ni-1]=0;

                   if(ni==5)
                   {
                       Lora[22-1]=0;
                       lifeLora[22-1]=0;

                       Lora[23-1]=0;
                       lifeLora[23-1]=0;
                   }

  			   }*/
  		      // sprintf(buf_p,"\r\nHHH: %c %d", *(ptr1+14), ((*(ptr1+14))&0x0f)-1 ); dbg3(buf_p);
  		   }
  		   else
  		   {
  			  delay_funkc[2]=0;
  		   }

  		   SprawdzZadanieGET();

  		 /*  for(int ip=0;ip<_Size__s_gpio;ip++)
  		 	  {
  		 		   pokaz_temp(ip+1);
  		 		   pokaz_ADC(i);
  		 		   ServiceWE(i);
  		 	  }*/

  		   ResumptionReceiveDataFromWifi();
  		   HttpPageStartSend();

  	  }



  	//  for(i=0;i<4002;i++){ buf_rx2[i]=0; buf_tx2[i]=0;  }

   }
	 omin_ipd:
	 asm("nop");
  }


}

//#endif



    //######################  -- Debug ---- ####################################################



      if(strstr(buf_rx3,"uart wifi"))   //dbg
      {
         	 dbg3("\r\nRST UART wifi ");
         	InitUartWifi();
           	  ResumptionReceiveDataFromDebug();

      }

      if(strstr(buf_rx3,"uart gsm"))   //dbg
      {
         	 dbg3("\r\nRST UART gsm ");
         	InitUartGSM();
           	  ResumptionReceiveDataFromDebug();

      }
      if(strstr(buf_rx3,"cclk"))   //dbg
      {
    	  sprintf(buf_p,"AT+CCLK?\x0D");   // AT+CCLK=\"08/06/17,14:50:00\"\x0D
    	  dbg(buf_p);
    	  dbg3("\r\ncclk\r\n");
           	  ResumptionReceiveDataFromDebug();

      }
      if(strstr(buf_rx3,"clts"))   //dbg
      {
    	  sprintf(buf_p,"AT+CLTS=1\x0D");   // synchronizacka czasu z siecie GSM
    	  dbg(buf_p); HAL_Delay(2000);
    	  dbg3(buf_rx); ResumptionReceiveDataFromGSM();
           	  ResumptionReceiveDataFromDebug();

      }

   /*   if(strstr(buf_rx3,"qq"))   //test
      {
          dbg3("\r\nQQQ");

  #define PID_PARAM_KP		1  //skok o sta³¹ wartosc KP gdy zliczanie w gore i w dol
  #define PID_PARAM_KI		1     //co ile sie zwieksza   tj. szybkosc zliczania gora dol
  #define PID_PARAM_KD		10  //chwilowy skok wartosci o KD w momencie zliczania gora dol

          q31_t  pid_error, duty, TEMP_WANT, TEMP_CURRENT;

            TEMP_WANT=30;

            TEMP_CURRENT= 230;


        	arm_pid_instance_q31 PID;


        	PID.Kp = PID_PARAM_KP;
        	PID.Ki = PID_PARAM_KI;
        	PID.Kd = PID_PARAM_KD;

        	arm_pid_init_q31(&PID, 1);


         //   TEMP_CURRENT=34;
          //  pid_error = TEMP_CURRENT - TEMP_WANT;
           // duty = arm_pid_f32(&PID, pid_error);
            // sprintf(buf_p, "\r\nExpected:   %d C\r\nActual:     %d C\r\nError:      %d C\r\nDuty cycle: %d  ", (int)TEMP_WANT, (int)TEMP_CURRENT, (int)pid_error, (int)duty);
            // dbg3(buf_p);
        	pid_error=0;   for(int i=0;i<_Size_buf_rx3;i++) buf_rx3[i]=0;
         while(1)
         {

       	   duty=arm_pid_q31(&PID, pid_error);  sprintf(buf_p, "\r\nDuty: %d",(int)duty); dbg3(buf_p);


        	   for(int i=0;i<_Size_buf_rx3;i++)
        	   {
        		   if(buf_rx3[i]=='a'){      pid_error++;  sprintf(buf_p,"  %d ",pid_error); dbg3(buf_p);  HAL_Delay(200); }
        		   else if(buf_rx3[i]=='b'){ pid_error--; sprintf(buf_p,"  %d ",pid_error); dbg3(buf_p);   HAL_Delay(200);  }
        		  // HAL_Delay(100);
        		   buf_rx3[i]=0;
        	   }

       	  HAL_Delay(50);
         }


      }*/
      if(strstr(buf_rx3,"init eth"))   //test
      {
    	  init_ENC();

    	  ResumptionReceiveDataFromDebug();
      }

      /*    if(strstr(buf_rx3,"cs1"))   //MAIL wybor
      {
 		 PobierzCzas(buf_p);
 		 sprintf(buf_p,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+08\"\x0D",buf_p[0],buf_p[1],buf_p[2], buf_p[4],buf_p[5],buf_p[6]);   // +CCLK: "18/02/26,09:42:01+04"
 		  dbg(buf_p);dbg3(buf_p);
 		  HAL_Delay(300);
 		  dbg3(buf_rx);

    	  ResumptionReceiveDataFromDebug();
      }

      if(strstr(buf_rx3,"cs2"))   //MAIL wybor
      {
 		 PobierzCzas(buf_p);
 		 sprintf(buf_p,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d-08\"\x0D",buf_p[0],buf_p[1],buf_p[2], buf_p[4],buf_p[5],buf_p[6]);   // +CCLK: "18/02/26,09:42:01+04"
 		  dbg(buf_p);dbg3(buf_p);
 		  HAL_Delay(300);
 		  dbg3(buf_rx);

    	  ResumptionReceiveDataFromDebug();
      }

    if(strstr(buf_rx3,"ccx"))
      {
          sprintf(buf_p,"AT+CCLK?\x0D");   // AT+CCLK=\"08/06/17,14:50:00\"\x0D
           dbg(buf_p);  dbg3("\r\nCCLK ");

    	  ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"dzwon"))
      {
   	        if((start==1)&&(wysylka_sms==0)&&(li_sms==0)&&(dzwon==0)&&(idx_email_GPRS==0)){
   	    	   dzwon=1;
   	    	   delay_funkc[25]=1;  //gdyby dzwon nie wrocilo do 0
   	    	   dbg("atd+48665040953\x0D");
   	        }
   	      HAL_Delay(1000);
    	  dbg3(buf_rx);

    	  ResumptionReceiveDataFromDebug();
      }*/
      if(strstr(buf_rx3,"gprs"))
      {
    	  if((start==1)&&(wysylka_sms==0)&&(li_sms==0)&&(dzwon==0)&&(idx_email_GPRS==0)){
    	    ResumptionReceiveDataFromGSM();
    	    sprintf(buf_p,"AT+MIPCALL=1,\"%s\",\"%s\",\"%s\"\x0D",Const.s_GPRS[0].apn, Const.s_GPRS[0].usr, Const.s_GPRS[0].has);
    	    dbg(buf_p);     delay_funkc[26]=1; idx_email_GPRS=1;   ekran_tymczasowy=3;
    	    dbg3("\r\nGPRS");  info_email[0]=0;
            #ifdef _LCD_TFT
	           DrawInfoE("Wysylam E-mail");
            #endif
    	  }

    	  ResumptionReceiveDataFromDebug();
      }
    /*  if(strstr(buf_rx3,"test"))
      {
    	  RapAddr=0;  dbg3("\r\n\r\n");
    	  while(RaportStanu(&RapAddr,0)==0) dbg3_roz(&buf_rx2[_Size_BufWifi]);
    	  dbg3_roz(&buf_rx2[_Size_BufWifi]);

    	  dbg3("\r\n\r\n");

    	  ResumptionReceiveDataFromDebug();
      }*/


      if(strstr(buf_rx3,"bb"))   //MAIL wybor
      {                                                     //czas_i
    	  PobierzCzas(buf_p);
    	  j = 60*buf_p[4]+buf_p[5];  //godz  min
    	  sprintf(&buf_p[100],"\r\nAA: %d  %d  %d %d  %d ",j, Const.s_Rot[0].intrv, NrRot, RotCzasNext, RotDoby );   dbg3(&buf_p[100]);

    	  ResumptionReceiveDataFromDebug();
      }
           if(strstr(buf_rx3,"esp1"))   //MAIL wybor
           {                                                     //czas_i
        	   dbg3("\r\nesp1");  ZapisZdarzenia(_ZdaTest,_Esp1,0);
         	  ResumptionReceiveDataFromDebug();
           }
           if(strstr(buf_rx3,"esp2"))   //MAIL wybor
           {                                                     //czas_i
        	   dbg3("\r\nesp2");  ZapisZdarzenia(_ZdaTest,_Esp2,0);
         	  ResumptionReceiveDataFromDebug();
           }
      if(strstr(buf_rx3,"aa"))   //MAIL wybor
      {

   	   for(i=0;i<_Size__s_Czujki_info;i++)
	        {
	      	  sprintf(buf_p,"\r\nhh %d: %02d ",i,Const.s_Czujki[i].addr_DS[8]);
	          dbg3(buf_p);
	        }

    	  // RaportStanu();
    	   // sprintf(buf_p,"\r\nAAAA: %d \r\n\r\n",strlen(&buf_rx2[_Size_BufWifi])); dbg3(buf_p);    dbg3(&buf_rx2[_Size_BufWifi]);

    	   sprintf(buf_p,"\r\nGG: %d %d %d ",dzwon,koniec_dzwon,param_dzwon  ); dbg3(buf_p);


  		     date.year=2019;
      		 date.month=  1;
      		 date.day=     1;

      		 date.hours=      1;
      		 date.minutes=    0;
      		 date.seconds=    47;

      		 i=-8;

      	     time=convertDateToUnixTime();
      	     time+=(15*60*i);
     	         convertUnixTimeToDate(time);
     	        sprintf(buf_p,"\r\nTime: %d-%02d-%02d  %02d:%02d:%02d   %d   ",date.year, date.month, date.day,    date.hours, date.minutes, date.seconds,    date.dayOfWeek); dbg3(buf_p);

    	   ResumptionReceiveDataFromDebug();



    	  // sprintf(buf_p,"\r\nKURWA  %d ",Const.s_Lan[0].param);  dbg3(buf_p);

      }

     /* if(strstr(buf_rx3,"ss1"))   //MAIL wybor
      {
       	  dbg3("\r\nSMS ");  buf_sms[0]=0;
       	  sprawdz_SMS("\r\nPompa pracuje");
       	  dbg3("\r\n");  dbg3(buf_sms);  dbg3("\r\n");
       	   ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"ss2"))   //MAIL wybor
      {
       	  dbg3("\r\nSMS ");  buf_sms[0]=0;
       	  sprawdz_SMS("\r\nPompa awaria");
       	  dbg3("\r\n");  dbg3(buf_sms);  dbg3("\r\n");
       	   ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"ss3"))   //MAIL wybor
      {
       	  dbg3("\r\nSMS ");  buf_sms[0]=0;
       	  sprawdz_SMS("\r\npompa awaria");
       	  dbg3("\r\n");  dbg3(buf_sms);  dbg3("\r\n");
       	   ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"ss4"))   //MAIL wybor
      {
       	  dbg3("\r\nSMS ");  buf_sms[0]=0;
       	  sprawdz_SMS("\r\nWyjscie Przekaznikowe 45 Stan ON");
       	  dbg3("\r\n");  dbg3(buf_sms);  dbg3("\r\n");
       	   ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"ss5"))   //MAIL wybor
      {
       	  dbg3("\r\nSMS ");  buf_sms[0]=0;
       	  sprawdz_SMS("\r\nWyjscie Przekaznikowe 45 Stan OFF");
       	  dbg3("\r\n");  dbg3(buf_sms);  dbg3("\r\n");
       	   ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"ss6"))   //MAIL wybor
      {
       	  dbg3("\r\nSMS ");  buf_sms[0]=0;
       	  sprawdz_SMS("\r\n111 Rh12");
       	  dbg3("\r\n");  dbg3(buf_sms);  dbg3("\r\n");
       	   ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"ss7"))   //MAIL wybor
      {
       	  dbg3("\r\nSMS ");  buf_sms[0]=0;
       	  sprawdz_SMS("\r\n111 Z32");
       	  dbg3("\r\n");  dbg3(buf_sms);  dbg3("\r\n");
       	   ResumptionReceiveDataFromDebug();
      }*/



	  if(strstr(buf_rx3,"modem"))
      {
		  dbg3("\r\nRST GSM");
		  wybor_bitowy|=0x0008;  //rst GSM
    	  dbg3("\r\nOK  ");

    	  ResumptionReceiveDataFromDebug();
      }

	  if(strstr(buf_rx3,"rst"))
      {

		  ResetFlash();
			dbg3("\r\nZapis...");
			Flash_in_progr_char_na_32(_AdrFlashToRAM, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
			dbg3("ok ");

			ResumptionReceiveDataFromDebug();
      }
	  if(strstr(buf_rx3,"arp"))
      {
		  arpPrintTable();  delay_funkc[15]=1;
		  ResumptionReceiveDataFromDebug();
      }
	  if(strstr(buf_rx3,"hh1"))
      {
		  dbg3("\r\nhttp1 ");
		  http_wstrz=1;
		  ResumptionReceiveDataFromDebug();

      }
	  if(strstr(buf_rx3,"hh0"))
      {
		  dbg3("\r\nhttp0 ");
		  http_wstrz=0;
		  ResumptionReceiveDataFromDebug();

      }
	  if(strstr(buf_rx3,"http"))
      {
		  dbg3("\r\n");
		       for(i=0; i<HTTP_task_LEN; i++)
	           {

				 sprintf(buf_p,"%d: %d %d  %d  %d  -- %d \r\n",i, HTTP[i].vit, HTTP[i].port, HTTP[i].seq, HTTP[i].ack, replay_packet[i]    );
				 dbg3(buf_p);

			   }

		       sprintf(buf_p,"%d   ",http_wstrz);  dbg3(buf_p);


		  ResumptionReceiveDataFromDebug();
      }

      if(strstr(buf_rx3,"00"))
      {
    	  wysylka_mail=1;
    	  if(Wyslij_email_ESP()==1);
		  else                       dbg3("\r\nNO mail\r\n");

        ResumptionReceiveDataFromDebug();

      }
      if(strstr(buf_rx3,"11"))
      {
    	  dbg3("\r\n11 ");
    	 // sprintf(buf_p,"AT+CIPSERVER=1,%s\x0D\x0A",Const.s_Lan[0].port);  HAL_Delay(200); dbg3(buf_rx2);  ResumptionReceiveDataFromWifi();
    	  delay_funkc[1]=1;
    	  ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"AA"))
      {
    	  dbg3("\r\nAA ");
    	 // sprintf(buf_p,"AT+CIPSERVER=1,%s\x0D\x0A",Const.s_Lan[0].port);  HAL_Delay(200); dbg3(buf_rx2);  ResumptionReceiveDataFromWifi();
    	 	 SendDataToModemWifi("AT+CIPSTATUS\x0D\x0A");   HAL_Delay(200); dbg3(buf_rx2);  ResumptionReceiveDataFromWifi();
    	  ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"33"))
      {
    	  dbg3("\r\n33 ");
    	  RstWifi();   ResetWifiAT=0;
    	    ResumptionReceiveDataFromDebug();
      }
      if(strstr(buf_rx3,"44"))
      {
    	  dbg3("\r\nwhile(1)");
    	  //dbg("at+csq\x0D");
           // sprintf(buf_p,"\r\nddd: %d", strlen(HttpLoadFile)); dbg3(buf_p);

    	 // dbg3("\r\nWWDG");
    	 // if (HAL_WWDG_Start(&hwwdg) != HAL_OK);

    	  hiwdg.Instance = IWDG;
    	  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
    	  hiwdg.Init.Reload = 4095;
    	  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    	  {
    	    _Error_Handler(__FILE__, __LINE__);
    	  }


    	  while(1);

    	 //HAL_UART_Receive_DMA(&huart1, Received, 1);
    	 //it=0;
    	  ResumptionReceiveDataFromDebug();
      }











}

void FunctionDelayService()
{
  if(delay_funkc[0]>0)
  {
	 if(delay_funkc[0]==4);
	 else delay_funkc[0]++;
  }

  if(delay_funkc[1]>0)
  {
 	 if(delay_funkc[1]==6);
 	 else delay_funkc[1]++;
  }

  if(delay_funkc[2]>0)
  {
 	 if(delay_funkc[2]==4);
 	 else delay_funkc[2]++;
  }

  if(delay_funkc[3]>0)
  {
 	 if(delay_funkc[3]==4);
 	 else delay_funkc[3]++;
  }

  if(delay_funkc[4]>0){  if(delay_funkc[4]==20);  else delay_funkc[4]++;  }
  if(delay_funkc[5]>0){  if(delay_funkc[5]==4);  else delay_funkc[5]++;  }
  if(delay_funkc[6]>0){  if(delay_funkc[6]==4);  else delay_funkc[6]++;  }
  if(delay_funkc[7]>0){  if(delay_funkc[7]==4);  else delay_funkc[7]++;  }
  if(delay_funkc[8]>0){  if(delay_funkc[8]==7);  else delay_funkc[8]++;  }
  if(delay_funkc[9]>0){  if(delay_funkc[9]==20);  else delay_funkc[9]++;  }  //sms do siebie ZGR
  if(delay_funkc[10]>0){ if(delay_funkc[10]==3);  else delay_funkc[10]++; }
  if(delay_funkc[11]>0){ if(delay_funkc[11]==7);  else delay_funkc[11]++; }
  if(delay_funkc[12]>0){ if(delay_funkc[12]==20);  else delay_funkc[12]++; }  //email stop
  if(delay_funkc[13]>0){ if(delay_funkc[13]==10);  else delay_funkc[13]++; }  //email ENC stop (przekroczony czas wysylki email)
  if(delay_funkc[14]>0){ if(delay_funkc[14]==10);  else delay_funkc[14]++; }  //email test
  if(delay_funkc[15]>0){ if(delay_funkc[15]==4);  else delay_funkc[15]++; }  //send arp request
  if(delay_funkc[16]>0){ if(delay_funkc[16]==5);  else delay_funkc[16]++; }  //mac ESP do ENC i na strone
  if(delay_funkc[17]>0){ if(delay_funkc[17]==2);  else delay_funkc[17]++; }  //ciag email ENC lub ESP
  if(delay_funkc[18]>0){ if(delay_funkc[18]==2);  else delay_funkc[18]++; }  //Zapocz¹tkowanie wysylania Emaila przez ESP
  if(delay_funkc[19]>0){ if(delay_funkc[19]==6);  else delay_funkc[19]++; }  //zmiana init_lora przez Http
  if(delay_funkc[20]>0){ if(delay_funkc[20]==10);  else delay_funkc[20]++; }  //jesli nie dostaniemy od GSM  "AT command ready"  to ...
  if(delay_funkc[21]>0){ if(delay_funkc[21]==5);  else delay_funkc[21]++; }  //jesli do 5 sekund nie dostaniemy sygnalu od esp to wylaczamy go
  if(delay_funkc[22]>0){ if(delay_funkc[22]==3);  else delay_funkc[22]++; }  //przy kazdym polaczeniu HTTP:    "if(http_wstrz==0) all_HTTP_clear();"
  if(delay_funkc[23]>0){ if(delay_funkc[23]==8);  else delay_funkc[23]++; }  //sprawdzanie czy odebrano odp. od 'arp request' od bramy
  if(delay_funkc[24]>0){ if(delay_funkc[24]==4);  else delay_funkc[24]++; }  //wsztrzymanie wyswietlania TFT jesli Http idzie
  if(delay_funkc[25]>0){ if(delay_funkc[25]==30);  else delay_funkc[25]++; }  //flaga dzwon na 0 gdy przekroczy czas 60s
  if(delay_funkc[26]>0){ if(delay_funkc[26]==35);  else delay_funkc[26]++; }  //zerowanie flagi 'idx_email_GPRS' a zatem zamykanie polaczenia GPRS jesli sam sie nie zamknie
  if(delay_funkc[27]>0){ if(delay_funkc[27]==10);  else delay_funkc[27]++; }   // opoznienia miedzy Dzwonieniami
  if(delay_funkc[28]>0){ if(delay_funkc[28]==3);  else delay_funkc[28]++; }   // retr dla email
  if(delay_funkc[29]>0){ if(delay_funkc[29]==5);  else delay_funkc[29]++; }   // zawieszenie sie podczas mail ESP
  if(delay_funkc[30]>0){ if(delay_funkc[30]==20);  else delay_funkc[30]++; }   // zawieszenie   mail ESP podczas powtorki AT+CIPSTART.... i nic wiecej
  if(delay_funkc[31]>0){ if(delay_funkc[31]==5);  else delay_funkc[31]++; }   //Test wifi komend¹ AT  czy ok
  if(delay_funkc[32]>0){ if(delay_funkc[32]==20);  else delay_funkc[32]++; }   //Odblokuj 'Wyslij_zdarzenia'
  if(delay_funkc[33]>0){ if(delay_funkc[33]==5);  else delay_funkc[33]++; }   //RST wifi bo nie odpowiada na AT
  if(delay_funkc[34]>0){ if(delay_funkc[34]==120);  else delay_funkc[34]++; }   //
  if(delay_funkc[35]>0){ if(delay_funkc[35]==10);  else delay_funkc[35]++; }   //


}

void Reset_Modemu(void)
{

	dbg3("\r\nRST GSM...");

	ResumptionReceiveDataFromGSM();

#ifdef _Neoway
	TM_GPIO_SetPinAsOutput__();  //EMERGOFF
	 HAL_Delay(4000);

	  TM_GPIO_SetPinAsInput__();  //EMERGOFF

	  HAL_Delay(1000);

	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	  HAL_Delay(500);
#endif

#ifdef _Fibocom
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	  HAL_Delay(5000);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
#endif


	  licz_cpin=0;
	  delay_funkc[5]=0;
		nr_kom=0;
		nr_kom2=0;
		li_sms=0;
		start=0;
		licz_start=0;

		delay_funkc[20]=1;  // jesli nie dostaniemy od GSM  "AT command ready"  to ...

     dbg3("OK ");

}



void SysTick__modem_GSM()      //wykonuje sie co 1 sekunde  NIE WYKONYWAC FUNKCJI w przerwaniu !!!
{

  int i;

  for(i=0; i<_Size_port; i++)
  {
	  if( (lifeLora[i]>0) && (lifeLora[i]<(_Czas_Zycia_Czujek+1)) ) lifeLora[i]--;
  }




  if(wsk_1s==0) wsk_1s=1;

  if(wsk_2s<2) wsk_2s++;

  if(li_sms>0) li_sms--;
                                                                                                                 //Start na rozpoczecie pomiarów  //send arp request   //koniec powtarzania komend GSM
  if((licz_brak_Sim>5)&&(licz_brak_Sim<50)){ wybor_bitowy|=0x0020; dbg3("\r\nGSM OFF !!!"); licz_brak_Sim=100;  wybor_bitowy|=0x0400;             delay_funkc[15]=1;   delay_funkc[5]=0; } //GSM OFF

  if(idx_email_GPRS==0){
  if(email==0){
  if(start==1)
  { if(li_sms==0)
    {
	   licz_g++;
	   if((licz_g%30)==0){   gsm_flag=1;  wybor_bitowy|=0x0004; }  //csq send
    }
    if(licz_rst>5) wybor_bitowy|=0x0008;  //rst GSM

  }}}



//-----------------------------------------------
  if(start==1)
  {
	  licz_start=0;
  }
  else
  {
   	 if((wybor_bitowy&0x0020)==0)  //GSM ON
   	 {
	    licz_start++;
	    if(licz_start>250)
	    {
		   wybor_bitowy|=0x0008;  //rst GSM
	    }
   	 }
  }
 //-----------------------------------------------

 //if((wybor_bitowy&0x0040)>0)  //jezeli zaloguje sie i otrzyma sms: 'ZGR'
 if(start==1)
 { if((email==0)&&(li_sms==0)){
   if(idx_email_GPRS==0){
     if(licz_cclk>3000)
     {
	    licz_cclk=0;
	    PobierzCzas(buf_p);
	    gsm_flag=1;
	     sprintf(buf_p,"AT+CCLK?\x0D");   // AT+CCLK=\"08/06/17,14:50:00\"\x0D
	    dbg(buf_p);
      }
      else licz_cclk++;
   }}
 }

 if((wybor_bitowy&0x0020)>0)  //GSM OFF
 {  if((email==0)&&(li_sms==0)){
	    if(licz_cclk>3000)
	    {
		  licz_cclk=0;
		  PobierzCzas(buf_p);
		  gsm_flag=1;
		  sprintf(buf_p,"AT+CCLK?\x0D");   // AT+CCLK=\"08/06/17,14:50:00\"\x0D
		  dbg(buf_p);

	    }
	    else licz_cclk++;
    }
 }


  for(i=0; i<ARP_TABLE_SIZE; i++)
  {
     if(ArpTable[i].time)
     {
	     if(ArpTable[i].time!=0xFF) ArpTable[i].time--;
	 }
	 else
	 {
	     ArpTable[i].ethaddr.addr[0] = 0;
		 ArpTable[i].ethaddr.addr[1] = 0;
		 ArpTable[i].ethaddr.addr[2] = 0;
		 ArpTable[i].ethaddr.addr[3] = 0;
		 ArpTable[i].ethaddr.addr[4] = 0;
		 ArpTable[i].ethaddr.addr[5] = 0;
		 ArpTable[i].ipaddr = 0;
	 }
  }

  for(i=0; i<HTTP_task_LEN; i++)
  {
     if(HTTP[i].vit)
     {
		HTTP[i].vit--;
	 }
  }



}
