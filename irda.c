/*
 * irda.c
 *
 *  Created on: 23.08.2018
 *      Author: Elektronika RM
 */

#include "stm32f1xx_hal.h"

#include "irda.h"
#include "modem_uart.h"
#include "gpio.h"
#include "zdarzenia.h"


void MX_TIM6_Init_IRDA(void)
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

void TsopGpioKonfig()
{
	SetPinAsInput(TSOP_GPIO, TSOP_GPIO_nr);
}

void czysc_bufory(void)
{
     int i;

	 for(i=0;i<_Dlug_buf_ir;i++)
	 {
	      buf_irda[i]=0;
	 }
}


int do_buf_kod_pilota(void)  //irda
{
    unsigned short j,i,m;    unsigned long	ttime_out=0;  char kil;

    MX_TIM6_Init_IRDA();


    start:
	czysc_bufory();

	__HAL_TIM_SET_COUNTER(&htim6,0);
	kil=0;
	while(TM_GPIO_GetInputPinValue(TSOP_GPIO,TSOP_GPIO_nr)==0)   // czekanie na opadajace zbocze
    {
		if(__HAL_TIM_GET_COUNTER(&htim6)>50000)
		{
		    if(kil>0){  init_timer_delay();   return 0;   }
		    else
		    {
		    	kil++;
		    	__HAL_TIM_SET_COUNTER(&htim6,0);
		    }
        }
    }
	while(TM_GPIO_GetInputPinValue(TSOP_GPIO,TSOP_GPIO_nr)==1)
    {
		if(__HAL_TIM_GET_COUNTER(&htim6)>50000)
		{
			if(kil>0){  init_timer_delay();  return 0;   }
			else
			{
				kil++;
				__HAL_TIM_SET_COUNTER(&htim6,0);
			}
		}

    }



	__HAL_TIM_SET_COUNTER(&htim6,0);
	j=1;

	for(i=0;i<(_Dlug_buf_ir/2);i++)
	{
       //while((PINA&0x08)==0x00)
	    kil=0;
	    while(TM_GPIO_GetInputPinValue(TSOP_GPIO,TSOP_GPIO_nr)==0)
	   {
           if(__HAL_TIM_GET_COUNTER(&htim6)>50000)
		   {
			   //czysc_bufory();

			   //lcdxy(0,23); piszdlcd('Y');
			   if(kil==0)
			   {
				   __HAL_TIM_SET_COUNTER(&htim6,0);
			      kil=1;
			   }
			   else goto start1;





		   }
	   }


	   if(kil==1) buf_irda[j++]=50000;
	   else       buf_irda[j++]=__HAL_TIM_GET_COUNTER(&htim6);
	   __HAL_TIM_SET_COUNTER(&htim6,0);



	    kil=0;
	    while(TM_GPIO_GetInputPinValue(TSOP_GPIO,TSOP_GPIO_nr)==1)
	   {
           if(__HAL_TIM_GET_COUNTER(&htim6)>50000)
		   {
               //czysc_bufory();
               //lcdxy(1,23); piszdlcd('Y');
               if(kil==0)
			   {
            	   __HAL_TIM_SET_COUNTER(&htim6,0);
			      kil=1;
			   }
			   else goto start1;
		   }
	   }


	   if(kil==1) buf_irda[j++]=50000;
	   else       buf_irda[j++]=__HAL_TIM_GET_COUNTER(&htim6);
	   __HAL_TIM_SET_COUNTER(&htim6,0);

    }

	start1:  dbg3("t");

    if(j<19) goto start;




       m=0;
	   while(buf_irda[1+m]!=0x0000)
	   {
            m++;
	   }
       buf_irda[0]=m;

       init_timer_delay();
       return 1;

}


void wyslij_z_buf_na_podczer(int port)
{
  unsigned short j,i,m,h;   unsigned char p;

   GPIO_TypeDef* GPIOx;
   uint16_t GPIO_Pin;

   NrPortuNaPortZewn(port, &GPIOx, &GPIO_Pin);


  __HAL_TIM_SET_COUNTER(&htim6,0);


	j=1;  m=1;

	   for(i=0;i<buf_irda[0];i++)
	   {
		  __HAL_TIM_SET_COUNTER(&htim6,0);
		  // asm("wdr");
          while(__HAL_TIM_GET_COUNTER(&htim6)<buf_irda[m])
          {   for(h=0;h<(14*8);h++) asm("nop");
              TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin);
    	      for(h=0;h<(14*8);h++) asm("nop");
    	      TM_GPIO_SetPinLow(GPIOx, GPIO_Pin);  //dbg(".");
          }
          m++; i++;
          __HAL_TIM_SET_COUNTER(&htim6,0);
		  //asm("wdr");
          while(__HAL_TIM_GET_COUNTER(&htim6)<buf_irda[m])
          {
	   	      asm("nop");
          }
	      m++;
       }



   asm("nop");

}



int RotacjaOnCzyOFF()
{
	int i,j;
	for(i=0;i<_Size_wy;i++)
	{
		 if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==8))    //PK || IR
		 {
			 for(j=0;j<8;j++){
			      if(((Const.s_Rot[0].ma[i]>>j)&0x01)>0) return 1;   }
		 }
	}
    return 0;
}

void Rotacja_PK()
{
   int i,j;

	        for(i=0;i<_Size_wy;i++)
    		{
    			 if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==8))    //PK || IR
    			 {

    			      if(((Const.s_Rot[0].ma[i]>>NrRot)&0x01)>0){   Const.s_WY[i/16].val|=(1<<(i-16*(i/16)));   GPIO_PK(i);  }
    			      else                                      {   Const.s_WY[i/16].val&=~(1<<(i-16*(i/16)));   GPIO_PK(i);  }
    			 }
    		}
	        ZapisZdarzenia(_ZdaRot,_RotKon,0);

}

void Rotacja_PK_Alarm_on()
{
	   int i,j;

	 for(i=0;i<_Size_wy;i++)
	 {
	       if(Const.s_GPIO[t_wy[i]-1].val==11)    //Termostat
	       {
	    	   if(t_wy[i]==__Port_PK_6){
	    	     Const.s_WY[i/16].val|=(1<<(i-16*(i/16)));   GPIO_PK(i);
	             return;
	    	   }
	       }
	 }
}
void Rotacja_PK_Alarm_off()
{
	   int i,j;

	 for(i=0;i<_Size_wy;i++)
	 {
	     if(Const.s_GPIO[t_wy[i]-1].val==11)    //Termostat
	     {
	          if(t_wy[i]==__Port_PK_6){
	    	     Const.s_WY[i/16].val&=~(1<<(i-16*(i/16)));   GPIO_PK(i);
	             return;
	          }
	     }
	 }
}

void Rotacja_All_PK_on()
{
   int i,j;

	        for(i=0;i<_Size_wy;i++)
    		{
    			 if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==8))    //PK || IR
    			 {
                     Const.s_WY[i/16].val|=(1<<(i-16*(i/16)));   GPIO_PK(i);
    			 }
    		}

}

void Rotacja_All_PK_off()
{
   int i,j;

	        for(i=0;i<_Size_wy;i++)
    		{
    			 if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==8))    //PK || IR
    			 {
                      Const.s_WY[i/16].val&=~(1<<(i-16*(i/16)));   GPIO_PK(i);
    			 }
    		}

}

void Redudancja0_po_RST()
{
  int czas,czas_i,i,j;   char k;   dbg3("\r\nRedudancja0_po_RST  ");
  wstrz=0;

  PobierzCzas(buf_p);
  czas = 60*buf_p[4]+buf_p[5];  //godz  min
  czas_i= Const.s_Rot[0].intrv;

  NrRot = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR6);

  Rotacja_PK();
  if(NrRot==(8-1)) NrRot=0; else NrRot++;  //kolejne cykle rotacji  , jest ich az max 8


  if(Const.s_Rot[0].dob>0)
  {
	  RotDoby = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR7);
  }
  else
  {
      RotCzasNext = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);

      // sprintf(buf_p,"\r\nAA: %d  %d  %d %d  %d ",czas, czas_i, NrRot, RotCzasNext, RotDoby );   dbg3(buf_p);

      if(RotCzasNext<czas)
      {
         do
	     {
    	     RotCzasNext+=czas_i;

	     }while(RotCzasNext<czas);


	     if(RotCzasNext>(60*23+59)) RotCzasNext=RotCzasNext-60*24;
	   //  write_I2C(0x6a, _ADDR_eeprom__Re+0  , czas_p>>8, 1);  czekaj(30);
        // write_I2C(0x6a, _ADDR_eeprom__Re+1  , czas_p>>0, 1);  czekaj(30);
      }
      if(RotCzasNext>czas)
      {
         do
	     {
    	     RotCzasNext-=czas_i;

	     }while(RotCzasNext>czas);

         RotCzasNext+=czas_i;
	    // write_I2C(0x6a, _ADDR_eeprom__Re+0  , czas_p>>8, 1);  czekaj(30);
        // write_I2C(0x6a, _ADDR_eeprom__Re+1  , czas_p>>0, 1);  czekaj(30);
      }
  }

}


void Rotacja()
{
    int czas_i,czas_a,czas_st;   dbg3("r"); // sprintf(buf_p,"R:%d ",Const.s_Rot[0].RotDoby);  dbg3(buf_p);

	dzien=oblicz_dzien();

	czas_a= 60*buf_p[4]+buf_p[5];  //godz  min
	if(Const.s_Rot[0].dob==0)  czas_i= Const.s_Rot[0].intrv;
	else                       czas_i= 24*60*Const.s_Rot[0].dob;
    czas_st= Const.s_Rot[0].sta;



if((Const.s_GPIO[TabPort[0]-1].val==2)||(Const.s_GPIO[TabPort[0]-1].val==9))
{
  for(i=0;i<_Size__s_Czujki_info;i++)
  {   if(Const.s_Czujki[i].addr_DS[8]==TabPort[0]) break;
  }
  if(Var.value[i]!=0x7fff)
  {
    if(Var.value[i] > (Const.s_Rot[0].max + Const.s_Rot[0].hist/2))
    {
    	if(wstrz==0)
    	{
    		Rotacja_All_PK_on();
    		Rotacja_PK_Alarm_on();     ZapisZdarzenia(_ZdaRot,_AllPkOn,0);
    	}
    	wstrz=1;
    }
    else if( (Var.value[i] < (Const.s_Rot[0].max-Const.s_Rot[0].hist/2)) && (Var.value[i] > (Const.s_Rot[0].min+Const.s_Rot[0].hist/2)) )
    {
		if(wstrz==1)
		{
			Redudancja0_po_RST();
    		Rotacja_PK_Alarm_off();
		}
		wstrz=0;
    }
	else if(Var.value[i]< (Const.s_Rot[0].min-Const.s_Rot[0].hist/2))
	{
    	if(wstrz==0)
    	{
    		Rotacja_All_PK_off();
    		Rotacja_PK_Alarm_off();    ZapisZdarzenia(_ZdaRot,_AllPkOf,0);
    	}
		wstrz=1;
	}
  }
}





    if(Const.s_Rot[0].dob>0)
    {
    	if(czas_a==czas_st)
    	{
    		if((wybor2_bitowy&0x0100)==0)
    		{
    			wybor2_bitowy|=0x0100;
    			RotDoby++;
    		   // if(RotDoby==(czas_i/(24*60)))
    			if(RotDoby==Const.s_Rot[0].dob)
    		    {
    		    	RotDoby=0;
    		    	dbg3("\r\nRotacja Doby ");

    	    	   // RotCzasNext=czas_a+czas_i;  if(RotCzasNext>(23*60+59)) RotCzasNext=RotCzasNext-24*60;
    	    	    //HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, RotCzasNext);
    	    	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, NrRot);
    	    	    dbg3("\r\nRotacja ");
    	    	    if(wstrz==0) Rotacja_PK();
    	    		if(NrRot==(8-1)) NrRot=0; else NrRot++;  //kolejne cykle rotacji  , jest ich az max 8


    		    }
    		    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, RotDoby);   //FlashWsadPrg( &Const.s_Rot[0].ma[0], _Size_wy+1+2*2+2*2+2+1 );
    		}
    	}
    	else
    	{
    		wybor2_bitowy&=~0x0100;
    	}

    }
    else
    {
    	if(czas_a==RotCzasNext)
        {
    	    RotCzasNext=czas_a+czas_i;  if(RotCzasNext>(23*60+59)) RotCzasNext=RotCzasNext-24*60;
    	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, RotCzasNext);
    	    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, NrRot);
    	    dbg3("\r\nRotacja ");
    	    if(wstrz==0) Rotacja_PK();
    		if(NrRot==(8-1)) NrRot=0; else NrRot++;  //kolejne cykle rotacji  , jest ich az max 8


        }
    }


}


