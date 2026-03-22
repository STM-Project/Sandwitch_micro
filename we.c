/*
 * we.c
 *
 *  Created on: 18 maj 2017
 *      Author: VA
 */
#include "stm32f1xx_hal.h"
#include "modem_uart.h"
#include "we.h"
#include "agh.h"
#include "zdarzenia.h"
#include "SX1278.h"

void ServiceWE(int i)
{
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	int portWy=t_we[i]-1;

#ifdef _LoRa
    if(Const.s_GPIO[portWy].val==4)
    {
        NrPortuNaPortZewn(portWy+1, &GPIOx, &GPIO_Pin);

        if((TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)==GPIO_PIN_RESET) != ((Var.we_p[portWy/16]&(1<<(portWy-((portWy/16)*16))))==0) )
        	lifeLora[portWy]=0;

    }
 #endif



	if(lifeLora[t_we[i]-1]==0)
	{
       if(Const.s_GPIO[t_we[i]-1].val==4)
       {
           NrPortuNaPortZewn(t_we[i], &GPIOx, &GPIO_Pin);

           if(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)==GPIO_PIN_RESET){   Var.we[i/16] &= ~(1<<(i-((i/16)*16)));   Var.we_p[portWy/16] &= ~(1<<(portWy-((portWy/16)*16)));     Var.logicWe&=~(1<<(t_we[i]-1)); }
           else                                                         {   Var.we[i/16] |= (1<<(i-((i/16)*16)));    Var.we_p[portWy/16] |= (1<<(portWy-((portWy/16)*16)));      Var.logicWe|=(1<<(t_we[i]-1));  }
       }
	}
}

void ServiceEOL(int i)
{

}

void WejsciaSMS()
{
   int i,j,is,ix;

   for(i=0;i<_Size_port;i++)
   {
	   if((Const.s_GPIO[i].val==4)||(Const.s_GPIO[i].val==13))
	   {


			  for(j=0;j<_Size_we;j++)
			  {
				  if((i+1)==t_we[j]) break;
			  }
		      if(j==_Size_we) goto end_we111;

		      if(Const.s_GPIO[i].val==4) goto dffdghrrf;


		  if(((Var.eol[j/16]>>(j-16*(j/16)))&0x0001)>0)  //EOL uszkodzenie lini
		  {
			  if((Var.wsk_we[j]&0x03)!=3)
			  {
				   Var.wsk_we[j]&=~0x03; Var.wsk_we[j]|=0x03;
				   Var.delay_we[j]=1;
				   sprintf(buf_p,"\r\nUszkodzenie EOL %d ",j+1); dbg3(buf_p);
			  }
		  }
		  else
		  {
			  dffdghrrf:
			 if(((Var.we[j/16]>>(j-16*(j/16)))&0x0001)>0)
             {
        	    if(((Var.wsk_we[j]&0x03)==0)||((Var.wsk_we[j]&0x03)==2)||((Var.wsk_we[j]&0x03)==3))  //rozwarcie
        	    {
        	    	if((Var.wsk_we[j]&0x03)==0);
        	    	else{  if(Var.delay_we[j]==0) Var.delay_we[j]=1; else Var.delay_we[j]=0;  }
        	    	//if(Var.delay_we[i]==0) Var.delay_we[i]=1; else Var.delay_we[i]=0;
        	    	Var.wsk_we[j]&=~0x03; Var.wsk_we[j]|=0x01;
        	    	sprintf(buf_p,"\r\nRozwarcie %d ",j+1); dbg3(buf_p);

        	    }

             }
             else if(((Var.we[j/16]>>(j-16*(j/16)))&0x0001)==0)
             {
        	    if(((Var.wsk_we[j]&0x03)==0)||((Var.wsk_we[j]&0x03)==1)||((Var.wsk_we[j]&0x03)==3))   //zwarcie
        	    {
        	    	if((Var.wsk_we[j]&0x03)==0);
        	    	else{   if(Var.delay_we[j]==0) Var.delay_we[j]=1; else Var.delay_we[j]=0;  }
        	    	//if(Var.delay_we[i]==0) Var.delay_we[i]=1; else Var.delay_we[i]=0;
        	    	Var.wsk_we[j]&=~0x03; Var.wsk_we[j]|=0x02;
        	    	sprintf(buf_p,"\r\nZwarcie %d ",j+1); dbg3(buf_p);

        	    }

             }
		  }
	   }
	   end_we111:
	   asm("nop");
   }

   is=sms_wsk[_Size_sms_wsk-1];
   for(i=0;i<_Size_we;i++)
   {
  	  if(Var.delay_we[i]>0)
  	  {
  		 if(Var.delay_we[i]>Const.s_WE[i].czas_reak)
  		 {
  			 if((Var.wsk_we[i]&0x03)==1)  //rozwarcie
  			 {
  				 if(((Const.s_WE[i].param&0x03)==0x00)||((Const.s_WE[i].param&0x03)==0x02))
  				 {
  				    sms_wsk[is++]='W';  if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! "); return; }
  				    sms_wsk[is++]=t_we[i];  //port
  				    sms_wsk[is++]=1;    ZapisZdarzenia(_ZdaWeZwRo,_WeRoz,i);   dbg3(" W1 SMS ");   Var.logicWe|=(1<<(t_we[i]-1));

  				    #ifdef _LoRa
  				    	Lora_Send_WE(t_we[i],1);
  				    #endif
  				 }

				 /*if(Const.s_WE[i].pk>0)
				 {   ix=Const.s_WE[i].pk-1;
					 if(Const.s_GPIO[ix].val==0)
					 {
						 for(j=0;j<_Size_wy;j++){ if((ix+1)==t_wy[j]) break; }
						 Const.s_WY[j/16].val&=~(1<<(j-16*(j/16)));  GPIO_PK(ix);   sprintf(buf_p,"  PK %d ON ",ix+1); dbg3(buf_p);
					 }
				 }*/

  			 }
  			 else if((Var.wsk_we[i]&0x03)==2)   //zwarcie
  			 {
  				 if(((Const.s_WE[i].param&0x03)==0x00)||((Const.s_WE[i].param&0x03)==0x01))
  				 {
  				    sms_wsk[is++]='W';  if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! "); return; }
  				    sms_wsk[is++]=t_we[i];   //port
  				    sms_wsk[is++]=2;    ZapisZdarzenia(_ZdaWeZwRo,_WeZwa,i);   dbg3(" W2 SMS ");  Var.logicWe&=~(1<<(t_we[i]-1));

					#ifdef _LoRa
  				    	Lora_Send_WE(t_we[i],0);
					#endif
  				 }

				 if(Const.s_WE[i].pk>0)
				 {	 ix=Const.s_WE[i].pk-1;
					 if(Const.s_GPIO[ix].val==0)
					 {
					      for(j=0;j<_Size_wy;j++){ if((ix+1)==t_wy[j]) break; }
					      Const.s_WY[j/16].val|=(1<<(j-16*(j/16)));  GPIO_PK(j); ZapisZdarzenia(_ZdaWEPK,_WEPKON,j);  sprintf(buf_p,"  PK %d ON ",ix+1); dbg3(buf_p);
					      Var.delay_wepk[i]=1;
					 }
				 }

  			 }
  			 else if((Var.wsk_we[i]&0x03)==3)   //uszkodzenie EOL
  			 {

  				 sms_wsk[is++]='W';  if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! "); return; }
  				 sms_wsk[is++]=t_we[i];   //port
  				 sms_wsk[is++]=3;    ZapisZdarzenia(_ZdaWeZwRo,_WeUsz,i);   dbg3(" W3 SMS ");


				 if(Const.s_WE[i].pk>0)
				 {	 ix=Const.s_WE[i].pk-1;
					 if(Const.s_GPIO[ix].val==0)
					 {
					      for(j=0;j<_Size_wy;j++){ if((ix+1)==t_wy[j]) break; }
					      Const.s_WY[j/16].val|=(1<<(j-16*(j/16)));  GPIO_PK(j); ZapisZdarzenia(_ZdaWEPK,_WEPKON,j);  sprintf(buf_p,"  PK %d ON ",ix+1); dbg3(buf_p);
					      Var.delay_wepk[i]=1;
					 }
				 }

  			 }
  			 Var.delay_we[i]=0;
  		 }
  		 else Var.delay_we[i]++;
  	  }
   }
   if(is>(_Size_sms_wsk-3)); else sms_wsk[_Size_sms_wsk-1]=is;




   for(i=0;i<_Size_we;i++)
   {
      if(Var.delay_wepk[i]>0)
      {
     	 if((Var.wsk_we[i]&0x03)==2)     //zwarcie
     	 {
     		 Var.delay_wepk[i]=1;
     	 }
     	 else if((Var.wsk_we[i]&0x03)==1)  //rozwarcie
     	 {
     		 Var.delay_wepk[i]++;
     		 if(Var.delay_wepk[i]>Const.s_WE[i].czas_przyt)
     		 {
     			 ix=Const.s_WE[i].pk-1;
     			 if(Const.s_GPIO[ix].val==0)
     			 {
     			  	  for(j=0;j<_Size_wy;j++){ if((ix+1)==t_wy[j]) break; }
     			  	  Const.s_WY[j/16].val&=~(1<<(j-16*(j/16)));  GPIO_PK(j);   ZapisZdarzenia(_ZdaWEPK,_WEPKOFF,j);  sprintf(buf_p,"  PK %d OFF ",ix+1); dbg3(buf_p);
     			 }
     			 Var.delay_wepk[i]=0;
     		 }
     	 }

      }
   }






}
