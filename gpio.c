/*
 * gpio.c
 *
 *  Created on: 5 maj 2017
 *      Author: VA
 */

#include "stm32f1xx_hal.h"
#include "gpio.h"
#include <modem_uart.h>
#include "zdarzenia.h"

/*void Set_Port_PK(char port)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;

     if(port==1){ GPIOx=GPIOA;  GPIO_Pin=GPIO_PIN_4; }
else if(port==2){ GPIOx=GPIOA;  GPIO_Pin=GPIO_PIN_5; }
else if(port==3){ GPIOx=GPIOA;  GPIO_Pin=GPIO_PIN_6; }
else if(port==4){ GPIOx=GPIOC;  GPIO_Pin=GPIO_PIN_4; }
else if(port==5){ GPIOx=GPIOC;  GPIO_Pin=GPIO_PIN_5; }
else if(port==6){ GPIOx=GPIOA;  GPIO_Pin=GPIO_PIN_12; }
else if(port==7){ GPIOx=GPIOD;  GPIO_Pin=GPIO_PIN_2; }


	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_RESET);
}*/

void Set_output(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_output_0(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_RESET);
}

void Set_output_1(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_SET);
}

void Set_input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,GPIO_PIN_RESET);
}

void GPIO_3V3(int nr)
{
   if(nr==1)
   {
	   P1_OC__0;
	   P1_ster__0;
	   P1_ogoln__1;
   }
   else if(nr==2)
   {
	   P2_OC__0;
	   P2_ster__0;
	   P2_ogoln__1;
   }
   else if(nr==3)
   {
	   P3_OC__0;
	   P3_ster__0;
	   P3_ogoln__1;
   }
   else if(nr==4)
   {
	   P4_OC__0;
	   P4_ster__0;
	   P4_ogoln__1;
   }
   else if(nr==5)
   {
	   P5_OC__0;
	   P5_ster__0;
	   P5_ogoln__1;
   }
   else if(nr==6)
   {
	   P6_OC__0;
	   P6_ster__0;
	   P6_ogoln__1;
   }
   else if(nr==7)
   {
	   P7_OC__0;
	   P7_ster__0;
	   P7_ogoln__1;
   }

}

void GPIO_ogoln(int nr)
{
   if(nr==1)
   {
	   P1_OC__0;
	   P1_ster__0;
	   P1_ogoln__I;
   }
   else if(nr==2)
   {
	   P2_OC__0;
	   P2_ster__0;
	   P2_ogoln__I;
   }
   else if(nr==3)
   {
	   P3_OC__0;
	   P3_ster__0;
	   P3_ogoln__I;
   }
   else if(nr==4)
   {
	   P4_OC__0;
	   P4_ster__0;
	   P4_ogoln__I;
   }
   else if(nr==5)
   {
	   P5_OC__0;
	   P5_ster__0;
	   P5_ogoln__I;
   }
   else if(nr==6)
   {
	   P6_OC__0;
	   P6_ster__0;
	   P6_ogoln__I;
   }
   else if(nr==7)
   {
	   P7_OC__0;
	   P7_ster__0;
	   P7_ogoln__I;
   }

}

void GPIO_OC(int nr)
{
   if(nr==1)
   {
	   P1_ogoln__0;
	   P1_ster__1;
   }
   else if(nr==2)
   {
	   P2_ogoln__0;
	   P2_ster__1;
   }
   else if(nr==3)
   {
	   P3_ogoln__0;
	   P3_ster__1;
   }
   else if(nr==4)
   {
	   P4_ogoln__0;
	   P4_ster__1;
   }
   else if(nr==5)
   {
	   P5_ogoln__0;
	   P5_ster__1;
   }
   else if(nr==6)
   {
	   P6_ogoln__0;
	   P6_ster__1;
   }
   else if(nr==7)
   {
	   P7_ogoln__0;
	   P7_ster__1;
   }

}

void GPIO_OC_PK(int port, int val )
{
     if(port==1){   if(val==0) P1_OC__0;  else P1_OC__1;   }
else if(port==2){   if(val==0) P2_OC__0;  else P2_OC__1;   }
else if(port==3){   if(val==0) P3_OC__0;  else P3_OC__1;   }
else if(port==4){   if(val==0) P4_OC__0;  else P4_OC__1;   }
else if(port==5){   if(val==0) P5_OC__0;  else P5_OC__1;   }
else if(port==6){   if(val==0) P6_OC__0;  else P6_OC__1;   }
else if(port==7){   if(val==0) P7_OC__0;  else P7_OC__1;   }
}

void GPIO_nap(int nr)
{
   if(nr==1)
   {
	   P1_OC__0;
	   P1_ogoln__0;
	   P1_ster__1;
   }
   else if(nr==2)
   {
	   P2_OC__0;
	   P2_ogoln__0;
	   P2_ster__1;
   }
   else if(nr==3)
   {
	   P3_OC__0;
	   P3_ogoln__0;
	   P3_ster__1;
   }
   else if(nr==4)
   {
	   P4_OC__0;
	   P4_ogoln__0;
	   P4_ster__1;
   }
   else if(nr==5)
   {
	   P5_OC__0;
	   P5_ogoln__0;
	   P5_ster__1;
   }
   else if(nr==6)
   {
	   P6_OC__0;
	   P6_ogoln__0;
	   P6_ster__1;
   }
   else if(nr==7)
   {
	   P7_OC__0;
	   P7_ogoln__0;
	   P7_ster__1;
   }

}

void ObslugaTimerRaport()
{
	unsigned short k;  int i,j,dzien;

	dzien=oblicz_dzien();
	k= 60*buf_p[4]+buf_p[5];


		      if((Const.s_Rap[0].godzON1==k)||(Const.s_Rap[0].godzON1==k-1)||(Const.s_Rap[0].godzON1==k-2)||(Const.s_Rap[0].godzON1==k-3))
	          {

		    	  if(start==1){
		    	  for(i=0;i<_Ilosc_Tel;i++)
		    	  {
		    	     if(Var.RapSmsMa[i]==0)
		    	     {  if(Const.s_Rap[0].sms[i]>0){

		    		       Raport_SMS();   send_SMS(Const.s_Tel[0].tel1+17*i);
		    		       Var.RapSmsMa[i]=1;
		    		       break;
		    	     }}
		    	  }}

                  if((email_enc==0)&&(email==0)){  if(li_sms==0){
		    	  for(i=0;i<_Ilosc_Mail;i++)
		    	  {
		    	     if(Var.RapMailMa[i]==0)
		    	     {  if(Const.s_Rap[0].mail[i]>0){

		    	    	   wysylka_mail=i+1;  wyslij_email_raport();
		    		       Var.RapMailMa[i]=1;
		    		       break;
		    	     }}
		    	  }}}


	          }
		      else
		      {
		    	  for(i=0;i<_Ilosc_Tel;i++) Var.RapSmsMa[i]=0;
		    	  for(i=0;i<_Ilosc_Mail;i++) Var.RapMailMa[i]=0;
		      }

}

void ObslugaTimerRejZda()
{
	unsigned short k;  int i,j,dzien;

	dzien=oblicz_dzien();
	k= 60*buf_p[4]+buf_p[5];


		      if((Const.s_Rap[0].godzina==k)||(Const.s_Rap[0].godzina==k-1)||(Const.s_Rap[0].godzina==k-2)||(Const.s_Rap[0].godzina==k-3))
	          {

		    	     if(Const.s_Rap[0].czas==0)  //codziennie
		    	     {
		    	    	 if(Var.RejZda==0)
		    	    	 {
		    	    		 Var.RejZda=1;

	    	    		     if(Const.s_Rap[0].siec==0)  //LAN ########################################
	    	    		     {
	    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
	    	    		    	 {
	    	    		    		 if(Const.s_Rap[0].start_zd < adr_flash_2){
	    	    		    		    sprintf(email_tytul,"RejZda");
	    	    		    		    Wyslij_email_ENC();
	    	    		    		 }
	    	    		    	 }
	    	    		    	 else if(Const.s_Rap[0].plik==1)       //z Pomiarami
	    	    		    	 {
	    	    		    		 if(Const.s_Rap[0].start_po < adr_flash){
	    	    		    		     sprintf(email_tytul,"Pomiar");
	    	    		    		     Wyslij_email_ENC();
	    	    		    	     }
	    	    		    	 }
	    	    		     }
	    	    		     else if(Const.s_Rap[0].siec==1)  //WiFi  ######################################
	    	    		     {
	    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
	    	    		    	 {if((wybor_bitowy&0x8000)>0){
	    	    		    		 if(Const.s_Rap[0].start_zd < adr_flash_2){
	    	    		    		    sprintf(email_tytul,"RejZda");
	    	    		    		    Wyslij_email_ESP();
	    	    		    		 }
	    	    		    	 }}
	    	    		    	 else if(Const.s_Rap[0].plik==1)      //z Pomiarami
	    	    		    	 {if((wybor_bitowy&0x8000)>0){
	    	    		    		 if(Const.s_Rap[0].start_po < adr_flash){
	    	    		    		    sprintf(email_tytul,"Pomiar");
	    	    		    		    Wyslij_email_ESP();
	    	    		    		 }
	    	    		    	 }}

	    	    		     }
	    	    		     else if(Const.s_Rap[0].siec==2)    //GSM  #######################################
	    	    		     {
	    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
	    	    		    	 {if(start==1){
	    	    		    		 if(Const.s_Rap[0].start_zd < adr_flash_2){
	    	    		    		    sprintf(email_tytul,"RejZda");
	    	    		    		    Wyslij_email_GPRS();
	    	    		    		 }
	    	    		    	 }}
	    	    		    	 else if(Const.s_Rap[0].plik==1)      //z Pomiarami
	    	    		    	 {if(start==1){
	    	    		    		 if(Const.s_Rap[0].start_po < adr_flash){
	    	    		    		    sprintf(email_tytul,"Pomiar");
	    	    		    		    Wyslij_email_GPRS();
	    	    		    		 }
	    	    		    	 }}

	    	    		     }
		    	    	 }

		    	     }
		    	     else  //raz w tygodniu
		    	     {
		    		     if(Const.s_Rap[0].czas==dzien)
		    		     {
			    	    	 if(Var.RejZda==0)
			    	    	 {
			    	    		 Var.RejZda=1;

		    	    		     if(Const.s_Rap[0].siec==0)  //LAN ########################################
		    	    		     {
		    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
		    	    		    	 {
		    	    		    		 if(Const.s_Rap[0].start_zd < adr_flash_2){
		    	    		    		    sprintf(email_tytul,"RejZda");
		    	    		    		    Wyslij_email_ENC();
		    	    		    		 }
		    	    		    	 }
		    	    		    	 else if(Const.s_Rap[0].plik==1)    //z Pomiarami
		    	    		    	 {
		    	    		    		 if(Const.s_Rap[0].start_po < adr_flash){
		    	    		    		     sprintf(email_tytul,"Pomiar");
		    	    		    		     Wyslij_email_ENC();
		    	    		    	     }
		    	    		    	 }
		    	    		     }
		    	    		     else if(Const.s_Rap[0].siec==1)  //WiFi  ######################################
		    	    		     {
		    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
		    	    		    	 {if((wybor_bitowy&0x8000)>0){
		    	    		    		 if(Const.s_Rap[0].start_zd < adr_flash_2){
		    	    		    		    sprintf(email_tytul,"RejZda");
		    	    		    		    Wyslij_email_ESP();
		    	    		    		 }
		    	    		    	 }}
		    	    		    	 else if(Const.s_Rap[0].plik==1)    //z Pomiarami
		    	    		    	 {if((wybor_bitowy&0x8000)>0){
		    	    		    		 if(Const.s_Rap[0].start_po < adr_flash){
		    	    		    		    sprintf(email_tytul,"Pomiar");
		    	    		    		    Wyslij_email_ESP();
		    	    		    		 }
		    	    		    	 }}

		    	    		     }
		    	    		     else if(Const.s_Rap[0].siec==2)    //GSM  #######################################
		    	    		     {
		    	    		    	 if(Const.s_Rap[0].plik==0)     //ze Zdarzeniami
		    	    		    	 {if(start==1){
		    	    		    		 if(Const.s_Rap[0].start_zd < adr_flash_2){
		    	    		    		    sprintf(email_tytul,"RejZda");
		    	    		    		    Wyslij_email_GPRS();
		    	    		    		 }
		    	    		    	 }}
		    	    		    	 else if(Const.s_Rap[0].plik==1)    //z Pomiarami
		    	    		    	 {if(start==1){
		    	    		    		 if(Const.s_Rap[0].start_po < adr_flash){
		    	    		    		    sprintf(email_tytul,"Pomiar");
		    	    		    		    Wyslij_email_GPRS();
		    	    		    		 }
		    	    		    	 }}

		    	    		     }


			    	    	 }
		    		     }
		    	     }


	          }
		      else
		      {
                  Var.RejZda=0;
		      }

}

void ObslugaTimer()
{
	unsigned long k;  int i,j,dzien;

	dzien=oblicz_dzien();
	k= 3600*buf_p[4]+60*buf_p[5]+buf_p[6];

	for(i=0;i<_Size_port;i++)
	{
	  //if((Const.s_Tim[i].akt&0x80)>0)
	  if(Const.s_GPIO[i].val==10)
	  {


		  for(j=0;j<_Size_wy;j++)
		  {
			  if((i+1)==t_wy[j]) break;
		  }
		  if(j==_Size_wy) goto end_wy111;




		 if(dzien>0)
	     {
		   if(((Const.s_Tim[j].akt>>(dzien-1))&0x01)>0)
		   {
		      if((Const.s_Tim[j].godzON1==k)||(Const.s_Tim[j].godzON2==k)||(Const.s_Tim[j].godzON3==k)||(Const.s_Tim[j].godzON4==k))
	          {
		    	  Const.s_WY[j/16].val|=(1<<(j-16*(j/16)));
		    	  if((Var.tim_wsk_on[j/16]&(1<<(j-16*(j/16))))==0){ ZapisZdarzenia(_ZdaTimer,_TimON,j); Var.tim_wsk_on[j/16]|=(1<<(j-16*(j/16)));      Var.logicTi|=(1<<(t_wy[j]-20-1));    FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) );   GPIO_PK(j); }

	          }
		      else Var.tim_wsk_on[j/16]&=~(1<<(j-16*(j/16)));

	          if((Const.s_Tim[j].godzOF1==k)||(Const.s_Tim[j].godzOF2==k)||(Const.s_Tim[j].godzOF3==k)||(Const.s_Tim[j].godzOF4==k))
	          {
	        	  Const.s_WY[j/16].val&=~(1<<(j-16*(j/16)));
	        	  if((Var.tim_wsk_of[j/16]&(1<<(j-16*(j/16))))==0){ ZapisZdarzenia(_ZdaTimer,_TimOFF,j); Var.tim_wsk_of[j/16]|=(1<<(j-16*(j/16)));     Var.logicTi&=~(1<<(t_wy[j]-20-1));   FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) );  GPIO_PK(j);  }

	          }
	          else Var.tim_wsk_of[j/16]&=~(1<<(j-16*(j/16)));
		   }
	     }
	   }
	  end_wy111:
	  asm("nop");
	}


	if(dzien==7)  //wysylanie sms`a do siebie w celu zsynchronizacji czasu
	{
		if(k==60*23)
	    {
			if((wybor_bitowy&0x0100)==0)
			{
				wybor_bitowy|=0x0100;
				delay_funkc[9]=1;
			}
	    }
		else wybor_bitowy&=~0x0100;
	}


}

void Led(int i)  //KED_GSM
{
	   /*  if(i==1) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	else if(i==0) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);*/
}
void Mrug(int czas)
{
	/*HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_Delay(czas);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);*/
}

void GPIO_PK_start(int p)
{
    int rr;
	GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;

  if((Const.s_GPIO[t_wy[p]-1].val==0)||(Const.s_GPIO[t_wy[p]-1].val==10)||(Const.s_GPIO[t_wy[p]-1].val==11))  //PK , Term , Tim
  {

	  NrPortuNaPortZewn(t_wy[p], &GPIOx, &GPIO_Pin);

	  if((Const.s_WY[p/16].val&(1<<(p-16*(p/16))))>0){  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);  }
	  else                                           {  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET); }

  }

}

