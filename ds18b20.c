/*
 * ds18b20.c
 *
 *  Created on: 23 lut 2017
 *      Author: VA
 */


#include "stm32f1xx_hal.h"
#include <modem_uart.h>
#include "zdarzenia.h"


#define TM_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))
#define TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)


void TM_GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
  	uint8_t i;
  	i=0; while(GPIO_Pin>>=1) i++;
  	if(i<8)  GPIOx->CRL = (GPIOx->CRL & ~(0x0f << (4 * i))) | (0x04 << (4 * i));
  	else     GPIOx->CRH = (GPIOx->CRH & ~(0x0f << (4 * (i-8)))) | (0x04 << (4 * (i-8)));
}
void TM_GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	uint8_t i;
	i=0; while(GPIO_Pin>>=1) i++;
	if(i<8)  GPIOx->CRL = (GPIOx->CRL & ~(0x0f << (4 * i))) | (0x02 << (4 * i));
	else     GPIOx->CRH = (GPIOx->CRH & ~(0x0f << (4 * (i-8)))) | (0x02 << (4 * (i-8)));

}


void zero_1wire(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	TM_GPIO_SetPinAsOutput(GPIOx, GPIO_Pin);
	TM_GPIO_SetPinLow(GPIOx, GPIO_Pin);
}

void High_Z_1wire(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	TM_GPIO_SetPinAsInput(GPIOx, GPIO_Pin);
}

int reset_1wire (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
   zero_1wire(GPIOx, GPIO_Pin);
   czekaj_us(500);
   High_Z_1wire(GPIOx, GPIO_Pin);
   czekaj_us(50);
   if(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)==GPIO_PIN_RESET)
   {   czekaj_us(150);
       if(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)==GPIO_PIN_SET) return 0;
	   else return 1;

   }
   else
   {  czekaj_us(150);
      return 1;
   }
}

void write_one_1wire (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	zero_1wire(GPIOx, GPIO_Pin);
	High_Z_1wire(GPIOx, GPIO_Pin);
   czekaj_us(260+90);
}
void write_zero_1wire (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
   zero_1wire(GPIOx, GPIO_Pin);
   czekaj_us(70);    //(60-120)
   High_Z_1wire(GPIOx, GPIO_Pin);
   czekaj_us(260);
}

unsigned char read_1wire (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{

	zero_1wire(GPIOx, GPIO_Pin);
    High_Z_1wire(GPIOx, GPIO_Pin);   czekaj_us(20);  //TO OPOZNIENIE DLA 48MHZ a nie dla 8MHZ !!!

   if(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)==GPIO_PIN_RESET)
   {  czekaj_us(260);
      return 0x00;
   }
   else if(TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)==GPIO_PIN_SET)
   {  czekaj_us(260);
      return 0x01;
   }
}

void send_1wire (unsigned char hhx, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    signed short tt,ff;
	int ggd=0,j;
    for(j=0; j < 8; j++)
	{
	    ggd = (hhx >> j) & 0x01;
		if(ggd==1) write_one_1wire(GPIOx, GPIO_Pin);
		else write_zero_1wire(GPIOx, GPIO_Pin);
	}
}

void liczRC(unsigned char bajt, unsigned char *CRC_)
{
 unsigned char zp1,zp2,ii;    //zmienne pomocnicze

 zp1=bajt;
 for(ii=0;ii<8;ii++)
 {
  bajt^=*CRC_;           //wskaŸnik *CRC wyznacza aktualnie wyliczony CRC
  zp2=bajt&0x01;        //wydzielenie bitu do obliczeñ
  bajt=*CRC_;
  if(zp2)
  {
   bajt^=0x18;
  }
  bajt=((unsigned char)(bajt)>>1)+0x80*zp2; //konwersja uch jest potrzebna do
                                            //prawid³owego wykonania przesuniêcia
  *CRC_=bajt;
  zp1=(bajt=zp1>>1);
 }
}

unsigned char receiver_1wire (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	unsigned char gga=0;  int j;
	unsigned char ggd=0;
    for(j=0; j < 8; j++)
	{
	   ggd = read_1wire(GPIOx, GPIO_Pin);
	   gga = gga | (ggd << j);
	   ggd=0;
	}
	return gga;
}

unsigned char odczyt_temp(signed short *ptr_temp, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    char zp,h,g;    int xx;   unsigned char buf_od_tmp[10],buf_p[40];

    if(reset_1wire(GPIOx, GPIO_Pin)==0);
	else
	{
	   *ptr_temp=0x7fff;  //dbg3("\r\n1");
	   return 1;
	}
	send_1wire(0xCC,GPIOx, GPIO_Pin);
	send_1wire(0xBE,GPIOx, GPIO_Pin);
	for(xx=0;xx<9;xx++)   buf_od_tmp[xx] = receiver_1wire(GPIOx, GPIO_Pin);
    h = buf_od_tmp[0];
    g = buf_od_tmp[1];
	zp=0;
    for(xx=0;xx<8;xx++) liczRC(buf_od_tmp[xx],&zp);
	buf_od_tmp[0] = zp;
	if(buf_od_tmp[0]==buf_od_tmp[8])
	{
	    *ptr_temp = (g << 8) & 0xFF00;
		*ptr_temp = *ptr_temp | (h & 0x00FF);
		if(*ptr_temp==0x0550){ *ptr_temp=0x7fff;   return 2;}
		return 3;
    }
	else
	{
       *ptr_temp=0x7fff; // dbg3("\r\n4");

     //	sprintf(buf_p,"d: %x %x %x %x %x %x %x %x  ",buf_od_tmp[0],buf_od_tmp[1],buf_od_tmp[2],buf_od_tmp[3],buf_od_tmp[4],buf_od_tmp[5],buf_od_tmp[6],buf_od_tmp[7]);
     //  dbg3(buf_p);

	   return 4;
	}
}

unsigned char DS18B20_pobierz_adres(char *addr, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)   // 28   "eb 53 e2 0 0 0"(adres)   5a(crc)
{
    char zp,h,g;    int xx;

    if(reset_1wire(GPIOx, GPIO_Pin)==0);
	else
	{
	   return 1;
	}
	//send_1wire(0xCC);
	send_1wire(0x33,GPIOx, GPIO_Pin);
	for(xx=0;xx<8;xx++)   addr[xx] = receiver_1wire(GPIOx, GPIO_Pin);
	zp=0;
    for(xx=0;xx<7;xx++) liczRC(addr[xx],&zp);
	if(zp==addr[7])
	{
		return 2;
    }
	else
	{
	   return 4;
	}
}

unsigned char DS18B20_conv_T(char *addr, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)   // 28   "eb 53 e2 0 0 0"(adres)   5a(crc)
{
    char zp,h,g;    int xx;

    if(reset_1wire(GPIOx, GPIO_Pin)==0);
	else
	{
	   return 1;
	}
	send_1wire(0x55,GPIOx, GPIO_Pin);
	for(xx=0;xx<8;xx++) send_1wire(addr[xx],GPIOx, GPIO_Pin);
	send_1wire(0x44,GPIOx, GPIO_Pin);

}

unsigned char DS18B20_odczyt_T(char *addr,signed short *ptr_temp, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)   // 28   "eb 53 e2 0 0 0"(adres)   5a(crc)
{
    char zp,h,g;    int xx; unsigned char buf_od_tmp[10];

    if(reset_1wire(GPIOx, GPIO_Pin)==0);
	else
	{
	  *ptr_temp=0x7fff;  //dbg3("\r\n1");
	   return 1;
	}
    send_1wire(0x55,GPIOx, GPIO_Pin);
    for(xx=0;xx<8;xx++) send_1wire(addr[xx],GPIOx, GPIO_Pin);
    send_1wire(0xBE,GPIOx, GPIO_Pin);

    for(xx=0;xx<9;xx++)   buf_od_tmp[xx] = receiver_1wire(GPIOx, GPIO_Pin);
        h = buf_od_tmp[0];
        g = buf_od_tmp[1];
    	zp=0;
        for(xx=0;xx<8;xx++) liczRC(buf_od_tmp[xx],&zp);
    	buf_od_tmp[0] = zp;
    	if(buf_od_tmp[0]==buf_od_tmp[8])
    	{
    	    *ptr_temp = (g << 8) & 0xFF00;
    		*ptr_temp = *ptr_temp | (h & 0x00FF);
    		if(*ptr_temp==0x0550){ *ptr_temp=0x7fff;  return 2;}
    		return 3;
        }
    	else
    	{
           *ptr_temp=0x7fff;  //dbg3("\r\n4");
    	   return 4;
    	}
}



void wartTemp (unsigned char *ptr, signed short *tempp)
{
    unsigned char i,c;

		i = 10*(ptr[1]&0x0f)+(ptr[2]&0x0f);

		     if(ptr[4]=='0')  c=0x00;
		else if(ptr[4]=='1')  c=0x01;
		else if(ptr[4]=='2')  c=0x03;
		else if(ptr[4]=='3')  c=0x04;
		else if(ptr[4]=='4')  c=0x06;
		else if(ptr[4]=='5')  c=0x07;
		else if(ptr[4]=='6')  c=0x09;
		else if(ptr[4]=='7')  c=0x0a;
		else if(ptr[4]=='8')  c=0x0c;
		else if(ptr[4]=='9')  c=0x0e;

		*tempp=(i<<4)&0x0FF0;
		*tempp=*tempp|c;
		if((i==0)&&(c==0));
		else
		{
          if(ptr[0]=='-')
		  {   *tempp = ~*tempp;
		      *tempp++;
          }
		}
}

void wartTemp_odwrotnie (unsigned char *ptr, signed short tempp)
{
        if((tempp & 0x8000) == 0x0000)
		{
		     if((tempp&0x000f)==0x0000)  *(ptr+4)='0';
		else if((tempp&0x000f)==0x0001)  *(ptr+4)='1';
		else if((tempp&0x000f)==0x0002)  *(ptr+4)='1';
		else if((tempp&0x000f)==0x0003)  *(ptr+4)='2';
		else if((tempp&0x000f)==0x0004)  *(ptr+4)='3';
		else if((tempp&0x000f)==0x0005)  *(ptr+4)='3';
		else if((tempp&0x000f)==0x0006)  *(ptr+4)='4';
		else if((tempp&0x000f)==0x0007)  *(ptr+4)='5';
		else if((tempp&0x000f)==0x0008)  *(ptr+4)='5';
		else if((tempp&0x000f)==0x0009)  *(ptr+4)='6';
		else if((tempp&0x000f)==0x000a)  *(ptr+4)='7';
		else if((tempp&0x000f)==0x000b)  *(ptr+4)='7';
		else if((tempp&0x000f)==0x000c)  *(ptr+4)='8';
		else if((tempp&0x000f)==0x000d)  *(ptr+4)='8';
		else if((tempp&0x000f)==0x000e)  *(ptr+4)='9';
		else if((tempp&0x000f)==0x000f)  *(ptr+4)='9';

		}
		else
		{
             if((tempp&0x000f)==0x000f)  *(ptr+4)='0';
		else if((tempp&0x000f)==0x000e)  *(ptr+4)='1';
		else if((tempp&0x000f)==0x000d)  *(ptr+4)='1';
		else if((tempp&0x000f)==0x000c)  *(ptr+4)='2';
		else if((tempp&0x000f)==0x000b)  *(ptr+4)='3';
		else if((tempp&0x000f)==0x000a)  *(ptr+4)='3';
		else if((tempp&0x000f)==0x0009)  *(ptr+4)='4';
		else if((tempp&0x000f)==0x0008)  *(ptr+4)='5';
		else if((tempp&0x000f)==0x0007)  *(ptr+4)='5';
		else if((tempp&0x000f)==0x0006)  *(ptr+4)='6';
		else if((tempp&0x000f)==0x0005)  *(ptr+4)='7';
		else if((tempp&0x000f)==0x0004)  *(ptr+4)='7';
		else if((tempp&0x000f)==0x0003)  *(ptr+4)='8';
		else if((tempp&0x000f)==0x0002)  *(ptr+4)='8';
		else if((tempp&0x000f)==0x0001)  *(ptr+4)='9';
		else if((tempp&0x000f)==0x0000){ *(ptr+4)='9'; tempp+=0x0001; }

		}

		if((tempp & 0x8000) == 0x0000)  *ptr='+';
		else if((tempp & 0x8000) == 0x8000)
		{
            tempp = ~tempp;
            tempp++;
			*ptr='-';
		}
        if(tempp<0x0640)                       { sprintf(ptr+1,"%02d",(tempp >> 4) & 0x00FF);    *(ptr+3)='.';  *(ptr+5)=0;}
		else if((tempp>0x063f)&&(tempp<0x3E80)){ sprintf(ptr+1,"%d",(tempp >> 4) & 0x07FF);   *(ptr+4)=0;  }
		else if(tempp==0x7fff)                  { sprintf(ptr+0," --.-");                       *(ptr+5)=0;  }
        else                                     { sprintf(ptr+0," --.-");                       *(ptr+5)=0;  }
}

void korekta_lcd(unsigned char *t)
{
     if(*(t+1)=='0')  //+02.3
     {
    	 t[1]=t[0];
    	 t[0]=' ';
     }
}

void wartTemp_Pt (unsigned char *ptr, signed short *tempp)
{
    unsigned short i,c;

		i = 100*(ptr[1]&0x0f) + 10*(ptr[2]&0x0f) + (ptr[3]&0x0f);

		*tempp=i;
		if(i==0);
		else
		{
          if(ptr[0]=='-')
		  {   *tempp = ~*tempp;
		      *tempp=*tempp+1;
          }
		}
}

void wartTemp_odwrotnie_Pt_http (unsigned char *ptr, signed short tempp)
{
	if(tempp==0x7fff){ sprintf(ptr," --- ");  *(ptr+5)=0; return; }
	if((tempp & 0x8000) == 0x0000)  *ptr='+';
	else if((tempp & 0x8000) == 0x8000)
	{
        tempp = ~tempp;
        tempp++;
		*ptr='-';
	}
    sprintf(ptr+1,"%03d ",tempp);  *(ptr+5)=0;

}

void korekta_Pt_lcd(unsigned char *t)
{
     if(*(t+1)=='0')
     {
    	 if(*(t+2)=='0')  //+001
    	 {
    		 t[2]=t[0];
    		 t[0]=' ';
    		 t[1]=' ';
    	 }
    	 else      //+021
    	 {
    		 t[1]=t[0];
    		 t[0]=' ';
    	 }
     }
}


void wartTemp_odwrotnie_Pt (unsigned char *ptr, signed short tempp)
{
	char znak;
	if(tempp==0x7fff){ sprintf(ptr," --- ");  *(ptr+5)=0; return; }
	if((tempp & 0x8000) == 0x0000)  znak='+';
	else if((tempp & 0x8000) == 0x8000)
	{
        tempp = ~tempp;
        tempp++;
        znak='-';
	}
    sprintf(ptr,"%03d ",tempp);    *(ptr+5)=0;

//korekta:     "+000 "   ->   "  +0 "
    //         "+001 "   ->   "  +1 "
    //         "+011 "   ->   " +11 "
    //         "+111 "   ->   "+111 "

    if(*(ptr+0)=='0')
    {  if(*(ptr+1)=='0') sprintf(ptr,"  %c%d ",znak,tempp);
       else              sprintf(ptr," %c%d ",znak,tempp);
    }
    else sprintf(ptr,"%c%d ",znak,tempp);
}


void start_DS18B20(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if(reset_1wire(GPIOx,GPIO_Pin)==0);
	send_1wire(0xCC, GPIOx,GPIO_Pin);
	send_1wire(0x44, GPIOx,GPIO_Pin);
}


/*
 * if(reset_1wire()==0);
	    send_1wire(0x55);
    	wyslij_kod_DS(DS_1);
        send_1wire(0x44);
	    if(reset_1wire()==0);
        send_1wire(0x55);
	    wyslij_kod_DS(DS_1);
        send_1wire(0xBE);
     	h = receiver_1wire();
	    g = receiver_1wire();
		if(reset_1wire()==0);
	    tempp1 = (g << 8) & 0xFF00;
		tempp1 = tempp1 | (h & 0x00FF);
		tempp_1 = tempp1;
		if((tempp1 & 0xF000) == 0x0000)
		{
            cccc='+';
		}
		else if((tempp1 & 0xF000) == 0xF000)
		{   tempp1 = ~tempp1;
			tempp1++;
			cccc='-';
		}
		sprintf(Temperatura, "%c%d%d.%d",cccc,((tempp1 >> 4) & 0x00FF)/10,((tempp1 >> 4) & 0x00FF)%10,((tempp1 & 0x000F)*6)/10);
 */

void sprawdz_prog_temp(void)
{

 int iy,i,j,is,aa;    iy=0;


 for(i=0;i<_Size__s_Czujki_info;i++)
 {
  if(Const.s_Czujki[i].addr_DS[8]>0)
  {
	  //sprintf(buf_p," 0x%x ",Var.value[i] ); dbg3(buf_p);
	if(Var.value[i]!=0x7fff)
	{ // if((Var.value[i]<0x000007E0)&&(Var.value[i]>0xFFFFFC80))
	  // if(Var.value[i]>0xFFFFFC80)
	  // {

		//sprintf(buf_p,"  T: %d     %d   ",Var.value[i],Const.s_Czujki[i].prog_max1); dbg3(buf_p);

		//if(Const.s_GPIO[   Const.s_Czujki[i].addr_DS[8]  ].val==3)

       //----------------------------- Prog 1 ------------------------------------
		  if(Var.value[i] > (Const.s_Czujki[i].prog_max1 + Const.s_Czujki[i].hist/2))
		  {
			if(((Var.wsk_czujki[i]&0x0f)==0)||((Var.wsk_czujki[i]&0x0f)==2))
			{
				Var.wsk_czujki[i]&=~0x0f; Var.wsk_czujki[i]|=0x01;
			     if(Var.delay_czujki[i]==0) Var.delay_czujki[i]=1;
			     else
			     {	if((Var.wsk_czujki_t[i]&0x0f)==0x03) Var.delay_czujki[i]=0;
			        else                                 Var.delay_czujki[i]=1;
			     }
			}

		  }
		  else if( (Var.value[i] < (Const.s_Czujki[i].prog_max1-Const.s_Czujki[i].hist/2)) && (Var.value[i] > (Const.s_Czujki[i].prog_min1+Const.s_Czujki[i].hist/2)) )
		  {
			if(((Var.wsk_czujki[i]&0x0f)==1)||((Var.wsk_czujki[i]&0x0f)==2))
			{
				Var.wsk_czujki[i]&=~0x0f;
			     if(Var.delay_czujki[i]==0) Var.delay_czujki[i]=1;
			     else
			     {	if((Var.wsk_czujki_t[i]&0x0f)==0x02) Var.delay_czujki[i]=0;
			        else                                 Var.delay_czujki[i]=1;
			     }
			}
		  }
		  else if(Var.value[i]< (Const.s_Czujki[i].prog_min1-Const.s_Czujki[i].hist/2))
		  {
			if(((Var.wsk_czujki[i]&0x0f)==0)||((Var.wsk_czujki[i]&0x0f)==1))
			{
			    Var.wsk_czujki[i]&=~0x0f;  Var.wsk_czujki[i]|=0x02;
			     if(Var.delay_czujki[i]==0) Var.delay_czujki[i]=1;
			     else
			     {	if((Var.wsk_czujki_t[i]&0x0f)==0x01) Var.delay_czujki[i]=0;
			        else                                 Var.delay_czujki[i]=1;
			     }
			}
		  }

     if((Const.s_Czujki[i].prog_max1==Const.s_Czujki[i].prog_max2)&&(Const.s_Czujki[i].prog_min1==Const.s_Czujki[i].prog_min2));
     else
     {
	   //----------------------------- Prog 2 ------------------------------------

		  if(Var.value[i] > (Const.s_Czujki[i].prog_max2 + Const.s_Czujki[i].hist/2))
		  {
			if((((Var.wsk_czujki[i]>>4)&0x0f)==0)||(((Var.wsk_czujki[i]>>4)&0x0f)==2))
			{
				Var.wsk_czujki[i]&=~0xf0;  Var.wsk_czujki[i]|=0x10;
			     if(Var.delay_czujki2[i]==0) Var.delay_czujki2[i]=1;
			     else
			     {	if((Var.wsk_czujki_t[i]&0xf0)==0x30) Var.delay_czujki2[i]=0;
			        else                                 Var.delay_czujki2[i]=1;
			     }
			}

		  }
		  else if( (Var.value[i] < (Const.s_Czujki[i].prog_max2-Const.s_Czujki[i].hist/2)) && (Var.value[i] > (Const.s_Czujki[i].prog_min2+Const.s_Czujki[i].hist/2)) )
		  {
			if((((Var.wsk_czujki[i]>>4)&0x0f)==1)||(((Var.wsk_czujki[i]>>4)&0x0f)==2))
			{
				Var.wsk_czujki[i]&=~0xf0;
			     if(Var.delay_czujki2[i]==0) Var.delay_czujki2[i]=1;
			     else
			     {	if((Var.wsk_czujki_t[i]&0xf0)==0x20) Var.delay_czujki2[i]=0;
			        else                                 Var.delay_czujki2[i]=1;
			     }
			}
		  }
		  else if(Var.value[i]< (Const.s_Czujki[i].prog_min2-Const.s_Czujki[i].hist/2))
		  {
			if((((Var.wsk_czujki[i]>>4)&0x0f)==0)||(((Var.wsk_czujki[i]>>4)&0x0f)==1))
			{
			     Var.wsk_czujki[i]&=~0xf0;  Var.wsk_czujki[i]|=0x20;
			     if(Var.delay_czujki2[i]==0) Var.delay_czujki2[i]=1;
			     else
			     {	if((Var.wsk_czujki_t[i]&0xf0)==0x10) Var.delay_czujki2[i]=0;
			        else                                 Var.delay_czujki2[i]=1;
			     }
			}
		  }

     }




   }
  }//}
 }

 aa=0;

 is=sms_wsk[_Size_sms_wsk-1];
 for(i=0;i<_Size__s_Czujki_info;i++)
 { if(Const.s_Czujki[i].addr_DS[8]>0)
   {


	 if(Const.s_GPIO[ Const.s_Czujki[i].addr_DS[8]-1 ].val==9) aa++;   //operacja dla Logic wyodrebnienie DHT tem i wilg dla logicOper
	 else aa=0;



	  if(Var.delay_czujki[i]>0)
	  {
		 if(Var.delay_czujki[i]>Const.s_Czujki[i].czas_reak)
		 {
			 if((Var.wsk_czujki[i]&0x01)>0)   //powyzej
			 {
				 if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! "); }
				 else
				 {
				    sms_wsk[is++]='T';    ZapisZdarzenia(_ZdaTempWilg,_TempPowPmax1,i);  dbg3("  Prg1> ");        if(aa==2) Var.logicTemp|=(((uint64_t)1<<(Const.s_Czujki[i].addr_DS[8]-1))<<32);  else Var.logicTemp|=(1<<(Const.s_Czujki[i].addr_DS[8]-1));
				    sms_wsk[is++]=i+1;
				    sms_wsk[is++]=3;
				 }
				 Var.wsk_czujki_t[i]&=~0x03;  Var.wsk_czujki_t[i]|=0x03;
			 }
			 else if((Var.wsk_czujki[i]&0x0f)==0)   //w normie
			 {
				 if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! ");  }
				 else
				 {
				    sms_wsk[is++]='T';   ZapisZdarzenia(_ZdaTempWilg,_TempNorm1,i);   dbg3("  Prg1= ");           if(aa==2) Var.logicTemp&=~(((uint64_t)1<<(Const.s_Czujki[i].addr_DS[8]-1))<<32);  else Var.logicTemp&=~(1<<(Const.s_Czujki[i].addr_DS[8]-1));
				    sms_wsk[is++]=i+1;
				    sms_wsk[is++]=2;
				 }
				 Var.wsk_czujki_t[i]&=~0x03;  Var.wsk_czujki_t[i]|=0x02;
			 }
			 else if((Var.wsk_czujki[i]&0x02)>0)   //ponizej
			 {
				 if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! "); }
				 else
				 {
				    sms_wsk[is++]='T';   ZapisZdarzenia(_ZdaTempWilg,_TempPonPmin1,i);   dbg3(" Prg1< ");       if(aa==2) Var.logicTemp|=(((uint64_t)1<<(Const.s_Czujki[i].addr_DS[8]-1))<<32);  else Var.logicTemp|=(1<<(Const.s_Czujki[i].addr_DS[8]-1));
				    sms_wsk[is++]=i+1;
				    sms_wsk[is++]=1;
				 }
				 Var.wsk_czujki_t[i]&=~0x03;  Var.wsk_czujki_t[i]|=0x01;
			 }
			 Var.delay_czujki[i]=0;
		 }
		 else Var.delay_czujki[i]++;
	  }
   }
 }
 if(is>(_Size_sms_wsk-3)); else sms_wsk[_Size_sms_wsk-1]=is;



 aa=0;

 is=sms_wsk[_Size_sms_wsk-1];
 for(i=0;i<_Size__s_Czujki_info;i++)
 {
	if(Const.s_Czujki[i].addr_DS[8]>0)
    {


		 if(Const.s_GPIO[ Const.s_Czujki[i].addr_DS[8]-1 ].val==9) aa++;   //operacja dla Logic wyodrebnienie DHT tem i wilg dla logicOper
		 else aa=0;



	  if(Var.delay_czujki2[i]>0)
	  {
		 if(Var.delay_czujki2[i]>Const.s_Czujki[i].czas_reak)
		 {
			 if((Var.wsk_czujki[i]&0x10)>0)   //powyzej
			 {
				 if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! "); }
				 else
				 {
				    sms_wsk[is++]='D';   ZapisZdarzenia(_ZdaTempWilg,_TempPowPmax2,i);  dbg3(" Prg2> ");      if(aa==2) Var.logicTemp|=(((uint64_t)1<<(Const.s_Czujki[i].addr_DS[8]-1))<<32);  else Var.logicTemp|=(1<<(Const.s_Czujki[i].addr_DS[8]-1));
				    sms_wsk[is++]=i+1;
				    sms_wsk[is++]=3;
				 }
				 Var.wsk_czujki_t[i]&=~0x30;  Var.wsk_czujki_t[i]|=0x30;
			 }
			 else if((Var.wsk_czujki[i]&0xf0)==0)   //w normie
			 {
				 if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! ");  }
				 else
				 {
				    sms_wsk[is++]='D';   ZapisZdarzenia(_ZdaTempWilg,_TempNorm2,i);   dbg3(" Prg2= ");       if(aa==2) Var.logicTemp&=~(((uint64_t)1<<(Const.s_Czujki[i].addr_DS[8]-1))<<32);  else Var.logicTemp&=~(1<<(Const.s_Czujki[i].addr_DS[8]-1));
				    sms_wsk[is++]=i+1;
				    sms_wsk[is++]=2;
				 }
				 Var.wsk_czujki_t[i]&=~0x30;  Var.wsk_czujki_t[i]|=0x20;
			 }
			 else if((Var.wsk_czujki[i]&0x20)>0)   //ponizej
			 {
				 if(is>(_Size_sms_wsk-3)){ sms_wsk[_Size_sms_wsk-1]=_Size_sms_wsk-1; dbg3("\r\nPRZEKROCZONO _Size_sms_wsk !!! "); }
				 else
				 {
				    sms_wsk[is++]='D';   ZapisZdarzenia(_ZdaTempWilg,_TempPonPmin2,i);  dbg3(" Prg2< ");     if(aa==2) Var.logicTemp|=(((uint64_t)1<<(Const.s_Czujki[i].addr_DS[8]-1))<<32);  else Var.logicTemp|=(1<<(Const.s_Czujki[i].addr_DS[8]-1));
				    sms_wsk[is++]=i+1;
				    sms_wsk[is++]=1;
				 }
				 Var.wsk_czujki_t[i]&=~0x30;  Var.wsk_czujki_t[i]|=0x10;
			 }
			 Var.delay_czujki2[i]=0;
		 }
		 else Var.delay_czujki2[i]++;
	  }
   }
 }
 if(is>(_Size_sms_wsk-3)); else sms_wsk[_Size_sms_wsk-1]=is;




}


void ObslugaTermostatu(void)
{

 int iy,i,j,is,nr_temp,jo,stop;    iy=0;


 for(i=0;i<_Size_port;i++)
 {
   stop=1;
   if(Const.s_GPIO[i].val==11)
   {

	  for(j=0;j<_Size_wy;j++)
	  {   if((i+1)==t_wy[j])
          {  iy= Const.s_Term[j].nr_temp;
             if(iy==0);
             else break;
          }
	  }
      if(j==_Size_wy) goto end_term111;


	  //sprintf(buf_p,"\r\niy: Temp port: %d   term port: %d  -- ",iy,i+1); dbg3(buf_p);
      for(jo=0;jo<(2*_Size_te);jo++)
	  {
	 	 if(Const.s_Czujki[jo].addr_DS[8]==(0x7F&iy))
	 	 {
	 		if((iy&0x80)>0)
	 		{
	 		   for(is=jo+1;is<(2*_Size_te);is++)
	  	       {
	  	    	  if(Const.s_Czujki[is].addr_DS[8]==(0x7F&iy)) break;
	  	       }
	 		   stop=0; nr_temp=is; break;
	 		}
	 		else
	 		{
	 		   stop=0; nr_temp=jo;     break;
	 		}

	 	 }
	  }

      if(stop==0)
      {	  //sprintf(buf_p,"f %d %d  ",nr_temp,Const.s_Czujki[nr_temp].addr_DS[8]); dbg3(buf_p);
    	  if(Const.s_Czujki[nr_temp].addr_DS[8]>0)
		   { dbg3("@");
				if(Var.value[nr_temp]!=0x7fff)
				{
					  if(Var.value[nr_temp] > (Const.s_Term[j].prog + Const.s_Term[j].hist/2))
					  {   // dbg3("F");
						   if(((Var.wsk_termost[j]&0x03)==0)||((Var.wsk_termost[j]&0x03)==2))
						   {//dbg3("M");
							   Var.wsk_termost[j]&=~0x03; Var.wsk_termost[j]|=0x01;   if(Var.delay_termost[j]==0) Var.delay_termost[j]=1; else Var.delay_termost[j]=0;
						   }

					  }
					  else if(Var.value[nr_temp] < (Const.s_Term[j].prog - Const.s_Term[j].hist/2))
					  {     //dbg3("G");
						   if(((Var.wsk_termost[j]&0x03)==0)||((Var.wsk_termost[j]&0x03)==1))
						   {//dbg3("A");
							   Var.wsk_termost[j]&=~0x03; Var.wsk_termost[j]|=0x02;   if(Var.delay_termost[j]==0) Var.delay_termost[j]=1; else Var.delay_termost[j]=0;
						   }

					  }
				}
		   }
      }
   }
   end_term111:
   asm("nop");
 }


 for(i=0;i<_Size_wy;i++)
 {
	  if(Var.delay_termost[i]>0)
	  {
		 if(Var.delay_termost[i]>Const.s_Term[i].czas_reak)
		 {
			 if((Var.wsk_termost[i]&0x03)==1)   //powyzej
			 {
				if(Const.s_GPIO[t_wy[i]-1].val==11)
				{
	     			 Const.s_WY[i/16].val|=(1<<(i-16*(i/16)));   ZapisZdarzenia(_ZdaTermost,_TermON,i);  GPIO_PK(i);   sprintf(buf_p,"  PK %d ON ",t_wy[i]); dbg3(buf_p);
	     			 sprintf(buf_p,"\r\nTerm:%d Powyzej",i+1); dbg3(buf_p);

				}

			 }
			 else if((Var.wsk_termost[i]&0x03)==2)   //ponizej
			 {
				 if(Const.s_GPIO[t_wy[i]-1].val==11)
				 {
					 Const.s_WY[i/16].val&=~(1<<(i-16*(i/16)));   ZapisZdarzenia(_ZdaTermost,_TermOFF,i);   GPIO_PK(i);   sprintf(buf_p,"  PK %d OFF ",t_wy[i]); dbg3(buf_p);
					 sprintf(buf_p,"\r\nTerm:%d ponizej",i+1); dbg3(buf_p);
				 }

			 }
			 Var.delay_termost[i]=0;
		 }
		 else Var.delay_termost[i]++;
	  }
 }



}

void SysTick__prg_temp(void)
{


}


