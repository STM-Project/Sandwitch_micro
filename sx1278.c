/*
 * sx1278.c
 *
 *  Created on: 16 gru 2017
 *      Author: Admin
 */

#include "stm32f1xx_hal.h"
#include "SX1278.h"
#include "modem_uart.h"
#include "agh.h"
#include "zdarzenia.h"
#include "define.h"
#include "gpio.h"

//////////////////////////////////
// logic
//////////////////////////////////




  SX1278_hw_t SX1278_hw;
  SX1278_t SX1278;

void Lora_Konfig(pwr)
{

	  power[0]=SX1278_POWER_0DBM;
	  power[1]=SX1278_POWER_8DBM;
	  power[2]=SX1278_POWER_11DBM;
	  power[3]=SX1278_POWER_14DBM;
	  power[4]=SX1278_POWER_17DBM;


	  Const.s_Lora[0].freq=SX1278_436MHZ;
	  Const.s_Lora[0].power=power[4];  //MAX POWER
	  Const.s_Lora[0].spread=SX1278_LORA_SF_12;
	  Const.s_Lora[0].bandwidth=SX1278_LORA_BW_500KHZ;
	  Const.s_Lora[0].packetlen=1;   //packet len = 10

		//initialize LoRa module
		SX1278_hw.dio0.port = GPIOB;
		SX1278_hw.dio0.pin = GPIO_PIN_7;
		SX1278_hw.nss.port = GPIOD;
		SX1278_hw.nss.pin = GPIO_PIN_3;
		SX1278_hw.reset.port = GPIOE;
		SX1278_hw.reset.pin = GPIO_PIN_14;
}

int  LoRaTxEntryRX(int pwr)
{
	int i,j;

	 Lora_Konfig(pwr);  //max power TX
	 SX1278_hw_Reset(&SX1278);
	 SX1278_begin(&SX1278, Const.s_Lora[0].freq, Const.s_Lora[0].power, Const.s_Lora[0].spread, Const.s_Lora[0].bandwidth, lora_lenTrans());

	 for(i=0;i<3;i++){  if(SX1278_LoRaEntryTx(&SX1278, lora_lenTrans(), 1000)==0)                    HAL_Delay(50); else break;  }
	 for(i=0;i<3;i++){  if(SX1278_LoRaTxPacket(&SX1278, (uint8_t *) buf_p, lora_lenTrans(),1000)==0) HAL_Delay(50); else break;  }
	 for(i=0;i<3;i++){  if(SX1278_LoRaEntryRx(&SX1278, lora_lenTrans(), 1000)==0)                    HAL_Delay(50); else break;  }
}

void Lora_start()
{
  int i,j;    SX1278.hw = &SX1278_hw;

	dbg3("\r\nConfiguring LoRa module ");

	  Lora_Konfig(1);
	  SX1278_hw_Reset(&SX1278);
	  SX1278_begin(&SX1278, Const.s_Lora[0].freq, Const.s_Lora[0].power, Const.s_Lora[0].spread, Const.s_Lora[0].bandwidth, lora_lenTrans());

	 dbg3("\r\nDone configuring LoRaModule  ");
	 sprintf(buf_p,"LR_RegPaConfig: %02x ",SX1278_SPIRead(&SX1278, LR_RegPaConfig)); dbg3(buf_p);

	 for(i=0;i<3;i++){  if(SX1278_LoRaEntryRx(&SX1278, lora_lenTrans(), 1000)==0)                    HAL_Delay(50); else break;  }
	 if(i==3)  dbg3("  NO !!! ");
	 else      dbg3("  OK recv");


	for(i=0;i<_Ile_Czujek;i++) Lora[i]=0;

	start_alarm=0;

}

void Lora_rst()
{
    Lora_Konfig(1);
    SX1278_hw_Reset(&SX1278);
    SX1278_begin(&SX1278, Const.s_Lora[0].freq, Const.s_Lora[0].power, Const.s_Lora[0].spread, Const.s_Lora[0].bandwidth, lora_lenTrans());
    SX1278_LoRaEntryRx(&SX1278, lora_lenTrans(), 10000);
}

void Lora_Send_Life(int nrCzujki)
{

	char wee;

	KOD_STALY=_KOD_STALY;

	          buf_p[0]=_Staly(1);  //Staly kod
			  buf_p[1]=_Staly(0);


			 // Hash_Lora(KOD[nr_we-1], &buf_p[100]);
			  buf_p[2]=nrCzujki;  //Zmienny kod
			  buf_p[3]=buf_p[100+1];
			  buf_p[4]=buf_p[100+2];
			  buf_p[5]=nrCzujki;

			  buf_p[6]=0x98;   //dla: zywotnosc czujki
			  buf_p[7]=nrCzujki;

			  wee=0;

			  if((TM_GPIO_GetInputPinValue(GPIOA, GPIO_PIN_15)==0)) wee|=0x01;
			  if((TM_GPIO_GetInputPinValue(GPIOC, GPIO_PIN_11)==0)) wee|=0x02;
			  if((TM_GPIO_GetInputPinValue(GPIOC, GPIO_PIN_10)==0)) wee|=0x04;


			 // buf_p[8]=wee;
			  buf_p[8]=4; //i_pwr[nrCzujki-1];    //LORA_PWR   MAX power
			  buf_p[9]=0xB4;    //LORA_PWR


			  LoRaTxEntryRX(4);

			  sprintf(buf_p,"\r\nSEND life nr:%d  pwr:%d  ",nrCzujki, i_pwr[nrCzujki-1]); dbg3(buf_p);

}

void Lora_Send_PK(int nrCzujki)
{

	char wee;

	KOD_STALY=_KOD_STALY;

	          buf_p[0]=_Staly(1);  //Staly kod
			  buf_p[1]=_Staly(0);


			 // Hash_Lora(KOD[nr_we-1], &buf_p[100]);
			  buf_p[2]=nrCzujki;  //Zmienny kod
			  buf_p[3]=buf_p[100+1];
			  buf_p[4]=buf_p[100+2];
			  buf_p[5]=nrCzujki;

			  buf_p[6]=0x39;   //dla: PK czujki
			  buf_p[7]=nrCzujki;

			 // buf_p[8]=wee;
			  buf_p[8]=i_pwr[nrCzujki-1];    //LORA_PWR
			  buf_p[9]=0x67;    //LORA_PWR


			  LoRaTxEntryRX(i_pwr[nrCzujki-1]);

			  sprintf(buf_p,"\r\nSEND PK:%d  pwr:%d  ",nrCzujki, i_pwr[nrCzujki-1]); dbg3(buf_p);

}

void Lora_Send_AlarmWePotw(int nrCzujki)
{
	char wee;

	KOD_STALY=_KOD_STALY;

	          buf_p[0]=_Staly(1);  //Staly kod
			  buf_p[1]=_Staly(0);

			  buf_p[2]=nrCzujki;  //Zmienny kod
			  buf_p[3]=buf_p[100+1];
			  buf_p[4]=buf_p[100+2];
			  buf_p[5]=nrCzujki;

			  buf_p[6]=0x13;
			  buf_p[7]=nrCzujki;

			  wee=0;


			  buf_p[8]=wee;
			  //buf_p[9]=i_pwr;    //LORA_PWR
			  buf_p[9]=0xC7;


			  LoRaTxEntryRX(4);


			  sprintf(buf_p,"\r\nSEND ACK nr czujki: %d   ", nrCzujki ); dbg3(buf_p);
}

void Hash_Lora2(char aa,char bb,char cc,char dd,   char *out)
{
	char buf1[4],buf2[20],a,b,c;

	buf1[0]=aa;
	buf1[1]=cc;
	buf1[2]=dd;
	buf1[3]=bb;
	hash_MD5(buf1, 4, &buf2);

	buf1[0]=buf2[9];
	buf1[1]=buf2[3];
	buf1[2]=buf2[11];
	buf1[3]=buf2[7];
	hash_MD5(buf1, 4, &buf2);

	buf1[0]=buf2[0];
	buf1[1]=buf2[15];
	buf1[2]=buf2[7];
	buf1[3]=buf2[14];
	hash_MD5(buf1, 4, &buf2);

	buf1[0]=buf2[8];
	buf1[1]=buf2[13];
	buf1[2]=buf2[2];
	buf1[3]=buf2[6];
	hash_MD5(buf1, 4, &buf2);

	out[0]=buf2[5];
	out[1]=buf2[10];
	out[2]=buf2[4];
	out[3]=buf2[3];

}


void Odbior_LoRa()   //Rejestr zdarzen: brak zasiegu dla czujnika,   PK dla alarmu,
{  int i,x,is,j,wyspk;  int tte;

GPIO_TypeDef* GPIOx;
uint16_t GPIO_Pin;

   alg[0]=4;
   alg[1]=6;
   alg[2]=9;
   alg[3]=13;
   alg[4]=16;

   wyspk=0;

       i = SX1278_LoRaRxPacket(&SX1278);
	     if(i > 0)
	     { wyspk=1;
	    	 for(x=0;x<10;x++) buf_p[x]=0;
	    	 SX1278_read(&SX1278, (uint8_t *) buf_p, i);
	    	 if((buf_p[0]==Const.s_Kod[0].val2[0])&&(buf_p[1]==Const.s_Kod[0].val2[1])&&(buf_p[2]==Const.s_Kod[0].val2[2])&&(buf_p[3]==Const.s_Kod[0].val2[3])&&(buf_p[4]==Const.s_Kod[0].val2[4])&&(buf_p[5]==Const.s_Kod[0].val2[5])&&(buf_p[6]==Const.s_Kod[0].val2[6])&&(buf_p[7]==Const.s_Kod[0].val2[7]))
	    	 {    dbg3("\r\nLORA !!!!!!!!!!!!!!!!!");


	    	    if(buf_p[9]==1)// temp DS18B20
	    	    {
	    	    	Hash_Lora2(buf_p[8],buf_p[10],buf_p[11],buf_p[12], &buf_p[100]);
	    	    	if((buf_p[13]==buf_p[100])&&(buf_p[14]==buf_p[101])&&(buf_p[15]==buf_p[102])&&(buf_p[16]==buf_p[103]))
	    	    	{
	    	    	   j=WyszukajNrCzujkiDlaPortu(buf_p[8]);        lifeLora[buf_p[8]-1]=_Czas_Zycia_Czujek;
	    	    	   if(j!=-1){
	    	    	      tte= ((buf_p[10]<<8)|buf_p[11]);
	    	    	      if(tte!=0x7FFF) Var.value[j]= tte + Const.s_Czujki[j].korekcja;  else Var.value[j]= tte;
	    	    	   }
	    	    	}
	    	    }
	    	    else  if(buf_p[9]==2)// temp wilg  DHT22
	    	    {
	    	    	Hash_Lora2(buf_p[8],buf_p[10],buf_p[11],buf_p[12], &buf_p[100]);
	    	    	if((buf_p[13]==buf_p[100])&&(buf_p[14]==buf_p[101])&&(buf_p[15]==buf_p[102])&&(buf_p[16]==buf_p[103]))
	    	    	{
	    	    	   j=WyszukajNrCzujkiDlaPortu(buf_p[8]);        lifeLora[buf_p[8]-1]=_Czas_Zycia_Czujek;
	    	    	   if(j!=-1){
	    	    	      tte= ((buf_p[10]<<8)|buf_p[11]);
	    	    	      if(tte!=0x7FFF) Var.value[j]= tte + Const.s_Czujki[j].korekcja;   else Var.value[j]= tte;
	    	    	      j=WyszukajNrCzujkiDlaPortu_2(buf_p[8]);
	    	    	      if(j!=-1){   if(tte!=0x7FFF) Var.value[j]= buf_p[12]<<4 + Const.s_Czujki[j].korekcja;   else Var.value[j]= tte;    }
	    	    	   }
	    	    	}
	    	    }
	    	    else  if(buf_p[9]==5)// WE
	    	    {
	    	    	j= WyszukujPortElement(t_we,_Size_we,buf_p[8]);


	    	 if(j!=-1)
	    	 {

	    	    	int portWy=buf_p[8]-1;

	    	    	lifeLora[portWy]=_Czas_Zycia_Czujek;


	    	        if(Const.s_GPIO[portWy].val==4)
	    	        {
	    	            NrPortuNaPortZewn(portWy+1, &GPIOx, &GPIO_Pin);

	    	            if(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)==GPIO_PIN_RESET){   Var.we_p[portWy/16] &= ~(1<<(portWy-((portWy/16)*16)));  }
	    	            else                                                         {   Var.we_p[portWy/16] |= (1<<(portWy-((portWy/16)*16)));    }
	    	        }


	    	    	  if(buf_p[10]==0)
	    	    	  {
	    	    		  Var.we[j/16] &= ~(1<<(j-((j/16)*16)));    Var.logicWe&=~(1<<portWy);   dbg3("\r\nAAAAAAAAAAA");
	    	    	  }
	    	    	  else
	    	    	  {
	    	    		  Var.we[j/16] |= (1<<(j-((j/16)*16)));     Var.logicWe|=(1<<portWy);    dbg3("\r\nBBBBBBBBBBB");
	    	    	  }
	    	 }



	    	    }
	    	    else  if(buf_p[9]==4)// PK
	    	    {

	                j= WyszukujPortElement(t_wy,_Size_wy,buf_p[8]);    lifeLora[buf_p[8]-1]=_Czas_Zycia_Czujek;
	    	    	  if(buf_p[10]==0)
	    	    	  {
	    	    		  Const.s_WY[j/16].val&=~(1<<(j-16*(j/16)));
	    	    		  GPIO_PK(j);  wyspk=0;  Lora_wstrz_synch=0;
	    	    	  }
	    	    	  else
	    	    	  {
	    	    		  Const.s_WY[j/16].val|=(1<<(j-16*(j/16)));
	    	    		  GPIO_PK(j);  wyspk=0;  Lora_wstrz_synch=0;
	    	    	  }
	    	    }





	    		 /*for(x=0;x<(_Ile_Czujek);x++)   //w DS5 jest czujnik z nr 5,22,23
	    		 {
		    		 if((buf_p[2]==x+1)&&(buf_p[5]==x+1)&&(buf_p[7]==x+1)&&(buf_p[6]==0xAA)&&(buf_p[9]==0x55))
		    		 {
		    			  is=sms_wsk[_Size_sms_wsk-1];
		    			  if((buf_p[8]&0x01)==0){ dbg3("\r\nLora AGH: we1=0 "); -1]=is;   }
		    			  else                  { dbg3("\r\nLora AGH: we1=1 ");   if((Lora[x]&0x01)==0){ if(is<(_Size_sms_wsk-3)){ sms_wsk[is++]='L';  sms_wsk[is++]=x+1; sms_wsk[is++]=1|0x80; sms_wsk[_Size_sms_wsk-1]=is; } else dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! ");  ZapisZdarzenia(_ZdaLoRa,_LoAlarm2, x+1); }  Lora[x]|=0x01;   j=0; if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))==0){ Const.s_WY[j/16].val|=(1<<(j-16*(j/16))); HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) );  } start_alarm=1;  }
		    			  if((buf_p[8]&0x02)==0){ dbg3("\r\nLora AGH: we2=0 "); }
		    			  else                  { dbg3("\r\nLora AGH: we2=1 ");   if((Lora[x]&0x02)==0){ if(is<(_Size_sms_wsk-3)){ sms_wsk[is++]='L';  sms_wsk[is++]=x+1; sms_wsk[is++]=2|0x80; sms_wsk[_Size_sms_wsk-1]=is; } else dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! ");  ZapisZdarzenia(_ZdaLoRa,_LoAlarm1, x+1); }  Lora[x]|=0x02;   j=1; if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))==0){ Const.s_WY[j/16].val|=(1<<(j-16*(j/16))); HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);  FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) );  } start_alarm=1;  }
		    			  if((buf_p[8]&0x04)==0){ dbg3("\r\nLora AGH: we3=0 ");  }
		    			  else                  { dbg3("\r\nLora AGH: we3=1 ");   if((Lora[x]&0x04)==0){ if(is<(_Size_sms_wsk-3)){ sms_wsk[is++]='L';  sms_wsk[is++]=x+1; sms_wsk[is++]=3|0x80; sms_wsk[_Size_sms_wsk-1]=is; } else dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! ");  ZapisZdarzenia(_ZdaLoRa,_LoNapad,  x+1); }  Lora[x]|=0x04;   j=2; if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))==0){ Const.s_WY[j/16].val|=(1<<(j-16*(j/16))); HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);  FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) );  } start_alarm=1;  }
		    			 // lifeLora[x]=_Czas_Zycia_Czujek;
		    			  Lora_Send_AlarmWePotw(x+1);

		    		 }
		    		 else  if((buf_p[2]==x+1)&&(buf_p[5]==x+1)&&(buf_p[7]==x+1)&&(buf_p[6]==0x82)&&(buf_p[9]==0x4A))
		    		 {
		    			 sprintf(buf_p,"\r\nPrzekaznik:%d ON/OFF  ",x+1); dbg3(buf_p);
		    			 lifeLora[x]=_Czas_Zycia_Czujek;

		    		 }
		    		 else  if((buf_p[2]==x+1)&&(buf_p[5]==x+1)&&(buf_p[7]==x+1)&&(buf_p[6]==0xFD)&&(buf_p[9]==0x39))
		    		 {
		    			   dbg3("\r\nOdpowiedzial ze zyje :)  HURA !!! ");     start_odpytyw[x]=1;
		    			   lifeLora[x]=_Czas_Zycia_Czujek;
		    			   tim_retr_lora=0; tim_retr_lora_end=0;

		    			   if(SendLife_p<(_Ile_Czujek-1))
		    			   {
		    				   SendLife_p++;   if((SendLife_p+1)==15) SendLife_p++;
		    				   sprintf(buf_p,"\r\nOdpytywanie lora:%d  ",SendLife_p+1); dbg3(buf_p);
		    				   i_pwr[SendLife_p]=0;
		    				   tim_retr_lora=1;  tim_retr_lora_end=0;
		    				   HAL_Delay(1000);  Lora_Send_Life(SendLife_p+1);
		    			   }
		    			   else
		    			   {
		    				   SendLife_p=0;
		    			   }
		    		 }
	    		 }*/




	    	 }

	     }

	     if(wyspk==1)
	     {
           if(Lora_wstrz_synch>0)
           {
        	     Lora_Send_Przek(Lora_wstrz_synch, Lora_wstrz_synch2);          Lora_wstrz_synch=0;  dbg3("\r\nRetransmisja LoRa_PK\r\n");
           }
	     }



	   /*  if(czujki_timer>(_Czas_Zycia_Czujek/4))
	     {
		     dbg3("\r\nOkresowe odpytywanie i dostosowanie mocy sygnalu ");
		     sprintf(buf_p,"\r\nOdpytywanie lora:%d  ",SendLife_p+1); dbg3(buf_p);
		     czujki_timer=0;
		     i_pwr[0]=0;
		     tim_retr_lora=1; tim_retr_lora_end=0;
		     Lora_Send_Life(SendLife_p+1);
		  }

          if(tim_retr_lora>0)
		  {
		  	       if(tim_retr_lora > (alg[tim_retr_lora_end]+0))
		  	       {
		  		      if(tim_retr_lora_end<_Ile_Retrans)
		  		      {
		  			      if(i_pwr[SendLife_p]<(_Ile_power-1)) i_pwr[SendLife_p]++;
		  			      Lora_Send_Life(SendLife_p+1);
		  			      tim_retr_lora=1;
		  			      tim_retr_lora_end++;
		  			      sprintf(buf_p,"\r\nL: %d  retr %d   alg %d    PWR: %d ",SendLife_p+1, tim_retr_lora_end, alg[tim_retr_lora_end], i_pwr[SendLife_p]); dbg3(buf_p);

		  			      if(tim_retr_lora_end==_Ile_Retrans)
		  			      {
		  				      tim_retr_lora=0;
		  				      tim_retr_lora_end=0;
		  				      dbg3("\r\nBRAK ZASIEGU !!!  NIE ODPOWIDA !!!");
		  				      if(start_odpytyw[SendLife_p]==0){ lifeLora[SendLife_p]=0;  start_odpytyw[SendLife_p]=1; }
		  				      ZapisZdarzenia(_ZdaLoRa,_LoZasBrak,SendLife_p+1);

			    			   if(SendLife_p<(_Ile_Czujek-1))
			    			   {
			    				   SendLife_p++;   if((SendLife_p+1)==15) SendLife_p++;
			    				   i_pwr[SendLife_p]=0;
			    				   tim_retr_lora=1;  tim_retr_lora_end=0;
			    				   Lora_Send_Life(SendLife_p+1);
			    			   }
			    			   else
			    			   {
			    				   SendLife_p=0;
			    			   }
		  			      }
		  		      }
		  	       }
		  }*/




       //  AGH_stan_alarmu();



}

void Lora_Send_Synchr()
{

	char wee;  char ppwr;


      buf_p[0]=Const.s_Kod[0].val2[0];   //nr kodu
	  buf_p[1]=Const.s_Kod[0].val2[1];
	  buf_p[2]=Const.s_Kod[0].val2[2];
	  buf_p[3]=Const.s_Kod[0].val2[3];
	  buf_p[4]=Const.s_Kod[0].val2[4];
	  buf_p[5]=Const.s_Kod[0].val2[5];
	  buf_p[6]=Const.s_Kod[0].val2[6];
	  buf_p[7]=Const.s_Kod[0].val2[7];

		  buf_p[8]=0x55;
		  buf_p[9]=0x55;
		  buf_p[10]=0x55;
		  buf_p[11]=0x55;
		  buf_p[12]=0x55;
		  buf_p[13]=0x55;
		  buf_p[14]=0x55;
		  buf_p[15]=0x55;
		  buf_p[16]=0x55;
		  buf_p[17]=0x55;
		  buf_p[18]=0x55;
		  buf_p[19]=0x55;
		  buf_p[20]=0x55;

		    LoRaTxEntryRX(ppwr);


}

void Lora_Send_Przek(int nr_port, int zalwyl)
{

	char wee;  char ppwr;


      buf_p[0]=Const.s_Kod[0].val2[0];   //nr kodu
	  buf_p[1]=Const.s_Kod[0].val2[1];
	  buf_p[2]=Const.s_Kod[0].val2[2];
	  buf_p[3]=Const.s_Kod[0].val2[3];
	  buf_p[4]=Const.s_Kod[0].val2[4];
	  buf_p[5]=Const.s_Kod[0].val2[5];
	  buf_p[6]=Const.s_Kod[0].val2[6];
	  buf_p[7]=Const.s_Kod[0].val2[7];

		  buf_p[8]=nr_port;
		  buf_p[9]=4;          //  _Typ_Czujki    4 -> PK
		  buf_p[10]=zalwyl;    //0 -> off     1 -> on
		  buf_p[11]=0x55;
		  buf_p[12]=0x55;
		  buf_p[13]=0x55;
		  buf_p[14]=0x55;
		  buf_p[15]=0x55;
		  buf_p[16]=0x55;
		  buf_p[17]=0x55;
		  buf_p[18]=0x55;
		  buf_p[19]=0x55;
		  buf_p[20]=0x55;

		    LoRaTxEntryRX(ppwr);


}

void Lora_Send_WE(int nr_port, int zalwyl)
{

	char wee;  char ppwr;


      buf_p[0]=Const.s_Kod[0].val2[0];   //nr kodu
	  buf_p[1]=Const.s_Kod[0].val2[1];
	  buf_p[2]=Const.s_Kod[0].val2[2];
	  buf_p[3]=Const.s_Kod[0].val2[3];
	  buf_p[4]=Const.s_Kod[0].val2[4];
	  buf_p[5]=Const.s_Kod[0].val2[5];
	  buf_p[6]=Const.s_Kod[0].val2[6];
	  buf_p[7]=Const.s_Kod[0].val2[7];

		  buf_p[8]=nr_port;
		  buf_p[9]=5;          //  _Typ_Czujki    5 -> WE
		  buf_p[10]=zalwyl;   //0 -> wyl     1 -> zal
		  buf_p[11]=0x55;
		  buf_p[12]=0x55;
		  buf_p[13]=0x55;
		  buf_p[14]=0x55;
		  buf_p[15]=0x55;
		  buf_p[16]=0x55;
		  buf_p[17]=0x55;
		  buf_p[18]=0x55;
		  buf_p[19]=0x55;
		  buf_p[20]=0x55;

		    LoRaTxEntryRX(ppwr);


}

int lora_lenTrans()
{
	     if(Const.s_Lora[0].packetlen==0) return 2;
	else if(Const.s_Lora[0].packetlen==1) return 20;
	else if(Const.s_Lora[0].packetlen==2) return 50;
	else if(Const.s_Lora[0].packetlen==3) return 100;
	else if(Const.s_Lora[0].packetlen==4) return 250;
}

uint8_t TM_SPI_Send_sx(uint8_t data)
{
	unsigned char ttt,i,j;
    ttt=0;

	        for(i=0;i<8;i++)
			{
		       if((data&0x80)==0x80)  TM_GPIO_SetPinHigh(GPIOB, GPIO_PIN_4);    //SDI
		       else TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_4);    //SDI

			   data = data << 1;

			   TM_GPIO_SetPinHigh(GPIOB, GPIO_PIN_6);    //SCK

			   ttt = ttt << 1;

			   if(TM_GPIO_GetInputPinValue(GPIOB, GPIO_PIN_5)==GPIO_PIN_SET) ttt |= 0x01;   //SDO
			   else ttt &= ~0x01;

			   TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_6);    //SCK

			}

	        return ttt;
}

 void SX1278_hw_init(SX1278_hw_t * hw) {
	SX1278_hw_SetNSS(hw, 1);
	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_SET);
}

 void SX1278_hw_SetNSS(SX1278_hw_t * hw, int value) {
	//dbg3("\r\nXXX1 ");

   // if(value==1) HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
   // else         HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, (value == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);

	//dbg3("\r\nZZZ2 ");
}

 void SX1278_hw_Reset(SX1278_hw_t * hw) {
	/*SX1278_hw_SetNSS(hw, 1);
	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_RESET);

	SX1278_hw_DelayMs(1000);

	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_SET);

	SX1278_hw_DelayMs(100);*/

	 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);  //NSS

	 HAL_Delay(10);
	 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET);  //RST
	 HAL_Delay(50);
	 HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
	 HAL_Delay(10);


}

 void SX1278_hw_SPICommand(SX1278_hw_t * hw, uint8_t cmd) {
	SX1278_hw_SetNSS(hw, 0);
	TM_SPI_Send_sx(cmd);
	//HAL_SPI_Transmit(hw->spi, &cmd, 1, 1000);
	//while (HAL_SPI_GetState(hw->spi) != HAL_SPI_STATE_READY);
}

 uint8_t SX1278_hw_SPIReadByte(SX1278_hw_t * hw) {
	uint8_t txByte = 0x00;
	uint8_t rxByte = 0x00;

	SX1278_hw_SetNSS(hw, 0);
	rxByte=TM_SPI_Send_sx(txByte);
	//HAL_SPI_TransmitReceive(hw->spi, &txByte, &rxByte, 1, 1000);
	//while (HAL_SPI_GetState(hw->spi) != HAL_SPI_STATE_READY);
	return rxByte;
}

 void SX1278_hw_DelayMs(uint32_t msec) {
	HAL_Delay(msec);
}

 int SX1278_hw_GetDIO0(SX1278_hw_t * hw) {
	return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET);
}

//////////////////////////////////
// logic
//////////////////////////////////

uint8_t SX1278_SPIRead(SX1278_t * module, uint8_t addr) {
	uint8_t tmp;
	SX1278_hw_SPICommand(module->hw, addr);
	tmp = SX1278_hw_SPIReadByte(module->hw);
	SX1278_hw_SetNSS(module->hw, 1);
	return tmp;
}

void SX1278_SPIWrite(SX1278_t * module, uint8_t addr, uint8_t cmd) {
	SX1278_hw_SetNSS(module->hw, 0);
	SX1278_hw_SPICommand(module->hw, addr | 0x80);
	SX1278_hw_SPICommand(module->hw, cmd);
	SX1278_hw_SetNSS(module->hw, 1);
}

void SX1278_SPIBurstRead(SX1278_t * module, uint8_t addr, uint8_t* rxBuf,
		uint8_t length) {
	uint8_t i;
	if (length <= 1) {
		return;
	} else {
		SX1278_hw_SetNSS(module->hw, 0);
		SX1278_hw_SPICommand(module->hw, addr);
		for (i = 0; i < length; i++) {
			*(rxBuf + i) = SX1278_hw_SPIReadByte(module->hw);
		}
		SX1278_hw_SetNSS(module->hw, 1);
	}
}

void SX1278_SPIBurstWrite(SX1278_t * module, uint8_t addr, uint8_t* txBuf,
		uint8_t length) {
	unsigned char i;
	if (length <= 1) {
		return;
	} else {
		SX1278_hw_SetNSS(module->hw, 0);
		SX1278_hw_SPICommand(module->hw, addr | 0x80);
		for (i = 0; i < length; i++) {
			SX1278_hw_SPICommand(module->hw, *(txBuf + i));
		}
		SX1278_hw_SetNSS(module->hw, 1);
	}
}

void SX1278_defaultConfig(SX1278_t * module) {
	SX1278_config(module, module->frequency, module->power, module->LoRa_Rate,
			module->LoRa_BW);
}

void SX1278_config(SX1278_t * module, uint8_t frequency, uint8_t power,
		uint8_t LoRa_Rate, uint8_t LoRa_BW) {
	SX1278_sleep(module); //Change modem mode Must in Sleep mode
	SX1278_hw_DelayMs(15);

	SX1278_entryLoRa(module);
	//SX1278_SPIWrite(module, 0x5904); //?? Change digital regulator form 1.6V to 1.47V: see errata note

	SX1278_SPIBurstWrite(module, LR_RegFrMsb,
			(uint8_t*) SX1278_Frequency[frequency], 3); //setting  frequency parameter

	//setting base parameter
	SX1278_SPIWrite(module, LR_RegPaConfig, SX1278_Power[power]); //Setting output power parameter


	SX1278_SPIWrite(module, LR_RegOcp, 0x0B);			//RegOcp,Close Ocp
	SX1278_SPIWrite(module, LR_RegLna, 0x23);		//RegLNA,High & LNA Enable
	if (SX1278_SpreadFactor[LoRa_Rate] == 6) {	//SFactor=6
		uint8_t tmp;
		SX1278_SPIWrite(module,
		LR_RegModemConfig1,
				((SX1278_LoRaBandwidth[LoRa_BW] << 4) + (SX1278_CR << 1) + 0x01)); //Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

		SX1278_SPIWrite(module,
		LR_RegModemConfig2,
				((SX1278_SpreadFactor[LoRa_Rate] << 4) + (SX1278_CRC << 2)
						+ 0x03));

		tmp = SX1278_SPIRead(module, 0x31);
		tmp &= 0xF8;
		tmp |= 0x05;
		SX1278_SPIWrite(module, 0x31, tmp);
		SX1278_SPIWrite(module, 0x37, 0x0C);
	} else {
		SX1278_SPIWrite(module,
		LR_RegModemConfig1,
				((SX1278_LoRaBandwidth[LoRa_BW] << 4) + (SX1278_CR << 1) + 0x00)); //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

		SX1278_SPIWrite(module,
		LR_RegModemConfig2,
				((SX1278_SpreadFactor[LoRa_Rate] << 4) + (SX1278_CRC << 2)
						+ 0x03)); //SFactor &  LNA gain set by the internal AGC loop
	}

	SX1278_SPIWrite(module, LR_RegSymbTimeoutLsb, 0xFF); //RegSymbTimeoutLsb Timeout = 0x3FF(Max)
	SX1278_SPIWrite(module, LR_RegPreambleMsb, 0x00); //RegPreambleMsb
	SX1278_SPIWrite(module, LR_RegPreambleLsb, 12); //RegPreambleLsb 8+4=12byte Preamble
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING2, 0x01); //RegDioMapping2 DIO5=00, DIO4=01
	module->readBytes = 0;
	SX1278_standby(module); //Entry standby mode
}

void SX1278_standby(SX1278_t * module) {
	SX1278_SPIWrite(module, LR_RegOpMode, 0x09);
	module->status = STANDBY;
}

void SX1278_sleep(SX1278_t * module) {
	SX1278_SPIWrite(module, LR_RegOpMode, 0x08);
	module->status = SLEEP;
}

void SX1278_entryLoRa(SX1278_t * module) {
	SX1278_SPIWrite(module, LR_RegOpMode, 0x88);
}

void SX1278_clearLoRaIrq(SX1278_t * module) {
	SX1278_SPIWrite(module, LR_RegIrqFlags, 0xFF);
}

int SX1278_LoRaEntryRx(SX1278_t * module, uint8_t length, uint32_t timeout) {
	uint8_t addr;

	module->packetLength = length;

	SX1278_defaultConfig(module);		//Setting base parameter
	SX1278_SPIWrite(module, REG_LR_PADAC, 0x84);	//Normal and RX
	SX1278_SPIWrite(module, LR_RegHopPeriod, 0xFF);	//No FHSS
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x01);//DIO=00,DIO1=00,DIO2=00, DIO3=01
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0x3F);//Open RxDone interrupt & Timeout
	SX1278_clearLoRaIrq(module);
	SX1278_SPIWrite(module, LR_RegPayloadLength, length);//Payload Length 21byte(this register must difine when the data long of one byte in SF is 6)
	addr = SX1278_SPIRead(module, LR_RegFifoRxBaseAddr); //Read RxBaseAddr
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr->FiFoAddrPtr
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8d);	//Mode//Low Frequency Mode
	//SX1278_SPIWrite(module, LR_RegOpMode,0x05);	//Continuous Rx Mode //High Frequency Mode
	module->readBytes = 0;

	while (1) {
		if ((SX1278_SPIRead(module, LR_RegModemStat) & 0x04) == 0x04) {	//Rx-on going RegModemStat
			module->status = RX;
			return 1;
		}
		if (--timeout == 0) {
			SX1278_hw_Reset(module->hw);
			SX1278_defaultConfig(module);
			return 0;
		}
		SX1278_hw_DelayMs(1);
	}
}

uint8_t SX1278_LoRaRxPacket(SX1278_t * module) {
	unsigned char addr;
	unsigned char packet_size;

	if (SX1278_hw_GetDIO0(module->hw)) {
		memset(module->rxBuffer, 0x00, SX1278_MAX_PACKET);

		addr = SX1278_SPIRead(module, LR_RegFifoRxCurrentaddr); //last packet addr
		SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr -> FiFoAddrPtr

		if (module->LoRa_Rate == SX1278_LORA_SF_6) { //When SpreadFactor is six,will used Implicit Header mode(Excluding internal packet length)
			packet_size = module->packetLength;
		} else {
			packet_size = SX1278_SPIRead(module, LR_RegRxNbBytes); //Number for received bytes
		}

		SX1278_SPIBurstRead(module, 0x00, module->rxBuffer, packet_size);
		module->readBytes = packet_size;
		SX1278_clearLoRaIrq(module);
	}
	return module->readBytes;
}

int SX1278_LoRaEntryTx(SX1278_t * module, uint8_t length, uint32_t timeout) {
	uint8_t addr;
	uint8_t temp;

	module->packetLength = length;

	SX1278_defaultConfig(module); //setting base parameter
	SX1278_SPIWrite(module, REG_LR_PADAC, 0x87);	//Tx for 20dBm
	SX1278_SPIWrite(module, LR_RegHopPeriod, 0x00); //RegHopPeriod NO FHSS
	SX1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x41); //DIO0=01, DIO1=00,DIO2=00, DIO3=01
	SX1278_clearLoRaIrq(module);
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0xF7); //Open TxDone interrupt
	SX1278_SPIWrite(module, LR_RegPayloadLength, length); //RegPayloadLength 21byte
	addr = SX1278_SPIRead(module, LR_RegFifoTxBaseAddr); //RegFiFoTxBaseAddr
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RegFifoAddrPtr

	while (1) {
		temp = SX1278_SPIRead(module, LR_RegPayloadLength);
		if (temp == length) {
			module->status = TX;
			return 1;
		}

		if (--timeout == 0) {
			SX1278_hw_Reset(module->hw);
			SX1278_defaultConfig(module);
			return 0;
		}
	}
}

int SX1278_LoRaTxPacket(SX1278_t * module, uint8_t* txBuffer, uint8_t length,
		uint32_t timeout) {
	SX1278_SPIBurstWrite(module, 0x00, txBuffer, length);
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8b);	//Tx Mode
	while (1) {
		if (SX1278_hw_GetDIO0(module->hw)) { //if(Get_NIRQ()) //Packet send over
			SX1278_SPIRead(module, LR_RegIrqFlags);
			SX1278_clearLoRaIrq(module); //Clear irq
			SX1278_standby(module); //Entry Standby mode
			return 1;
		}

		if (--timeout == 0) {
			SX1278_hw_Reset(module->hw);
			SX1278_defaultConfig(module);
			return 0;
		}
		SX1278_hw_DelayMs(1);
	}
}

void SX1278_begin(SX1278_t * module, uint8_t frequency, uint8_t power,
		uint8_t LoRa_Rate, uint8_t LoRa_BW, uint8_t packetLength) { // dbg3("\r\nBBBBBB");
	SX1278_hw_init(module->hw);
	module->frequency = frequency;
	module->power = power;
	module->LoRa_Rate = LoRa_Rate;
	module->LoRa_BW = LoRa_BW;
	module->packetLength = packetLength; // dbg3("\r\nAAAAAAAAAA");
	SX1278_defaultConfig(module);
}

int SX1278_transmit(SX1278_t * module, uint8_t* txBuf, uint8_t length,
		uint32_t timeout) {
	if (SX1278_LoRaEntryTx(module, length, timeout)) {
		return SX1278_LoRaTxPacket(module, txBuf, length, timeout);
	}
	return 0;
}

int SX1278_receive(SX1278_t * module, uint8_t length, uint32_t timeout) {
	return SX1278_LoRaEntryRx(module, length, timeout);
}

uint8_t SX1278_available(SX1278_t * module) {
	return SX1278_LoRaRxPacket(module);
}

uint8_t SX1278_read(SX1278_t * module, uint8_t* rxBuf, uint8_t length) {
	if (length != module->readBytes)
		length = module->readBytes;
	memcpy(rxBuf, module->rxBuffer, length);
	rxBuf[length] = '\0';
	module->readBytes = 0;
	return length;
}

uint8_t SX1278_RSSI_LoRa(SX1278_t * module) {
	uint32_t temp = 10;
	temp = SX1278_SPIRead(module, LR_RegRssiValue); //Read RegRssiValue, Rssi value
	temp = temp + 127 - 137; //127:Max RSSI, 137:RSSI offset
	return (uint8_t) temp;
}

uint8_t SX1278_RSSI(SX1278_t * module) {
	uint8_t temp = 0xff;
	temp = SX1278_SPIRead(module, 0x11);
	temp = 127 - (temp >> 1);	//127:Max RSSI
	return temp;
}
