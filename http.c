/*
 * http.c
 *
 *  Created on: 21 mar 2017
 *      Author: VA
 */
#include <modem_uart.h>
#include "stm32f1xx_hal.h"

#include "http.h"
#include "flash_in.h"
#include "gpio.h"
#include "wykres.h"
#include "zdarzenia.h"
#include "irda.h"
#include "lcd.h"

IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;

WWDG_HandleTypeDef hwwdg;

int http_itx;

int BufNotIdentity(char *b1, char *b2)
{
   int k,i;  k=strlen(b1);

   if(k==strlen(b2))
   {
	  for(i=0;i<k;i++)
	  {  if(b1[i]==b2[i]);
	     else return 1;
	  }
   }
   else return 1;

   return 0;
}

char *HttpMenuF()
{
	if((wybor_bitowy&0x0200)>0) return HttpMenuEsp;   //ESP
	else                        return HttpMenu;
}

char *HttpMenu2F()
{
	if((wybor_bitowy&0x0200)>0) return HttpMenu2Esp;   //ESP
	else                        return HttpMenu2;
}

void load_page(int page)
{
    int i,j;
	char a,b;
/*
1).  dla pierwszego wskaznika HttpPagesBuffer[0]  nie ma parsera HTTP
2).  #define _Size_WskFragmentPage    4000   -> TO MAXYMALNY ROZMIAR JEDNEGO WSKAZNIKA !!!!
3).  KAZDY BUTTON 'ZAPISZ' MUSI MIEC NAZWE 'n_109_zapisz' !!!!!!
 */


	   for(i=0;i<_Size__s_Czujki_info;i++) HttpCountOffset[i]=0;

#ifdef _Nasze
	  /* if(page==11)
	   {

		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenu;

		   a=0;  j=0;
		   for(i=0;i<_Size__s_Czujki_info;i++)
		   {
		     if(Const.s_Czujki[i].addr_DS[8]>0){ HttpPagesBuffer[j+2]=HttpCzujki;  HttpCountOffset[j]=a;  j++;  }
		     else a++;
		   }
		   if(j==0)
		   {
			   HttpPagesBuffer[j+2]=HttpEnd;
			   HttpPagesBuffer[j+3]=0;
		   }
		   else
		   {
		      HttpPagesBuffer[j+2]=Http_zapisz;
		      HttpPagesBuffer[j+3]=HttpEnd;
		      HttpPagesBuffer[j+4]=0;
		   }
	   }
	   else if(page==9)
	   {

		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenu;

		   a=0;  j=0;
		   for(i=0;i<_Size__s_gpio;i++)
		   {
			   if(Const.s_GPIO[i].val==4){ HttpPagesBuffer[j+2]=HttpWeCyfr;  HttpCountOffset[j]=a;  j++;  }
		       else a++;
		   }
		   if(j==0)
		   {
			   HttpPagesBuffer[j+2]=HttpEnd;
			   HttpPagesBuffer[j+3]=0;
		   }
		   else
		   {
		      HttpPagesBuffer[j+2]=Http_zapisz;
		      HttpPagesBuffer[j+3]=HttpEnd;
		      HttpPagesBuffer[j+4]=0;
		   }
	   }
	   else if(page==10)    //Termostaty
	   {

		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenu;

		   a=0;  j=0;
		   for(i=0;i<_Size__s_gpio;i++)
		   {
			   if(Const.s_GPIO[i].val==0){ HttpPagesBuffer[j+2]=HttpTermost;  HttpCountOffset[j]=a;  j++;  }
		       else a++;
		   }
		   if(j==0)
		   {
			   HttpPagesBuffer[j+2]=HttpTermost;  HttpCountOffset[j]=7;
			   HttpPagesBuffer[j+3]=Http_zapisz;
			   HttpPagesBuffer[j+4]=HttpEnd;
			   HttpPagesBuffer[j+5]=0;
		   }
		   else
		   {
			  HttpPagesBuffer[j+2]=HttpTermost;  HttpCountOffset[j]=7-j;
			  HttpPagesBuffer[j+3]=Http_zapisz;
		      HttpPagesBuffer[j+4]=HttpEnd;
		      HttpPagesBuffer[j+5]=0;
		   }
	   }
	   else if(page==12)    //Timery
	   {

		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenu;

		   a=0;  j=0;
		   for(i=0;i<_Size__s_gpio;i++)
		   {
			   if(Const.s_GPIO[i].val==0){ HttpPagesBuffer[j+2]=HttpTimer;  HttpCountOffset[j]=a;  j++;  }
		       else a++;
		   }
		   if(j==0)
		   {
			   HttpPagesBuffer[j+2]=HttpTimer;  HttpCountOffset[j]=7;
			   HttpPagesBuffer[j+3]=Http_zapisz;
			   HttpPagesBuffer[j+4]=HttpEnd;
			   HttpPagesBuffer[j+5]=0;
		   }
		   else
		   {
			  HttpPagesBuffer[j+2]=HttpTimer;  HttpCountOffset[j]=7-j;
		      HttpPagesBuffer[j+3]=Http_zapisz;
		      HttpPagesBuffer[j+4]=HttpEnd;
		      HttpPagesBuffer[j+5]=0;
		   }
	   }*/
	   if(page==2)
	   {
			HttpPagesBuffer[0]=HttpRefreshUpload;
		    HttpPagesBuffer[1]=0;
	   }
	   else if(page==999)    //HttpUpload - obraz wgrywanie;
	   {
		   if((wybor_bitowy&0x0200)>0)   //ESP
		   {
			    HttpPagesBuffer[0]="        ";
			    HttpPagesBuffer[1]=0;
		   }
		   else
		   {
		      HttpPagesBuffer[0]=HttpUpload_1;
			  HttpPagesBuffer[1]=HttpUpload_2;
			  HttpPagesBuffer[2]=HttpUpload_3;
		      HttpPagesBuffer[3]=0;
		   }
	   }
	   else if(page==78)   //LAN parametry
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpUstawIP;  HttpCountOffset[0]=1;
		    HttpPagesBuffer[3]=HttpStrWWW;
		    HttpPagesBuffer[4]=Http_zapisz;
		    HttpPagesBuffer[5]=HttpEnd;
		    HttpPagesBuffer[6]=0;
	   }
	   else if(page==79)   //Czas
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpCzas;
		    HttpPagesBuffer[3]=HttpEkr;
		    HttpPagesBuffer[4]=HttpEnd;
		    HttpPagesBuffer[5]=0;
	   }
	   else if(page==80)   //Rejestrator
	   {

		    HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpRejestr;
		    HttpPagesBuffer[3]=Http_zapisz;
		    HttpPagesBuffer[4]=HttpEnd;
		    HttpPagesBuffer[5]=0;

	   }
	   else if((page==81)||(page==10))   //Odczyt Zapisów w formacie plik.txt      TempWilg.txt
	   {
			HttpPagesBuffer[0]=HttpWsk_1;
			for(i=0;i<20;i++) HttpPagesBuffer[1+i]=HttpWsk_1;
		    HttpPagesBuffer[1+i+0]=0;
	   }
	   else if(page==83)   //Odczyt Zapisów w formcie plik.csv
	   {
			HttpPagesBuffer[0]=HttpWsk_1;
			for(i=0;i<20;i++) HttpPagesBuffer[1+i]=HttpWsk_1;
		    HttpPagesBuffer[1+i+0]=0;
	   }
	   else if(page==82)   //wybor_wykres
	   {
			HttpPagesBuffer[0]=Http_start;
			HttpPagesBuffer[1]=HttpWsk_1;
			HttpPagesBuffer[2]=HttpWsk_2;
		    HttpPagesBuffer[3]=0;
	   }
	   else if(page==4)  //Wifi parametry
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpUstawIP;
		    HttpPagesBuffer[3]=HttpUstawWifiAP;
		    HttpPagesBuffer[4]=HttpUstawWifiAP;
		    if((wybor_bitowy&0x0200)>0)  HttpPagesBuffer[5]=Http_zapisz_przeszuk;        //ESP
		    else                         HttpPagesBuffer[5]=Http_zapisz_przeszuk_ENC;    // ENC
		    HttpPagesBuffer[6]=HttpEnd;
		    HttpPagesBuffer[7]=0;
	   }
	   else if(page==5)
	   {
		    HttpPagesBuffer[0]=&buf_rx2[_Size_BufWifi];
		    HttpPagesBuffer[1]=0;
	   }
	   else if(page==8)
	   {
		    HttpPagesBuffer[0]=Http_favicon;
		    HttpPagesBuffer[1]=0;
	   }
	   else if(page==990)  //obraz
	   {

		   for(i=0;i<120;i++)
		   {
			   if((i%2)==0) HttpPagesBuffer[i]=HttpWsk_1;
			   else         HttpPagesBuffer[i]=HttpWsk_2;
		   }
		   HttpPagesBuffer[i]=0;

	   }
	   else if(page==1669)   //LOperacje logiczne
	   {
		   if(Const.OutNameLogic[0].name==0xFF)
		   {
				HttpPagesBuffer[0]=(char*)HttpStyle;
			    HttpPagesBuffer[1]=HttpMenuF();
			    HttpPagesBuffer[2]="<button class=\"n\"> Prosze wybrac przynajmniej jedno wyjscie przekaznikowe</button>";
			    HttpPagesBuffer[3]=(char*)HttpEnd;
			    HttpPagesBuffer[4]=0;

		   }
		   else
		   {
				HttpPagesBuffer[0]=(char*)HttpStyle;
			    HttpPagesBuffer[1]=HttpMenuF();
			    HttpPagesBuffer[2]=(char*)HttpScriptLogic_1;
			    HttpPagesBuffer[3]=(char*)HttpScriptMenu;
				HttpPagesBuffer[4]=(char*)HttpWsk_1;
			    HttpPagesBuffer[5]=(char*)HttpScriptLogic_2;
			    HttpPagesBuffer[6]=(char*)HttpScriptLogic_3;
			    HttpPagesBuffer[7]=(char*)HttpScriptLogic_4;
			    HttpPagesBuffer[8]=0;

		   }
	   }
	   else if(((page>991)&&(page<997))||(page==998))   //sesjaSMTP.txt  + etr godz....mies  +  sesjaLogGSM.txt
	   {
			HttpPagesBuffer[0]=HttpWsk_1;
			for(i=0;i<20;i++) HttpPagesBuffer[1+i]=HttpWsk_1;
		    HttpPagesBuffer[1+i+0]=0;
	   }
	/*   else if(page==6)
	   {

		    HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenu;
		    HttpPagesBuffer[2]=HttpMain_1;
		    HttpPagesBuffer[3]=HttpMain_2;
		    HttpPagesBuffer[4]=HttpMain_3;
		    HttpPagesBuffer[5]=HttpWsk_1;
		    HttpPagesBuffer[6]=HttpWsk_2;
		    HttpPagesBuffer[7]=HttpWsk_3;
		    HttpPagesBuffer[8]=HttpWsk_4;
		    HttpPagesBuffer[9]=HttpWsk_5;
		    HttpPagesBuffer[10]=HttpWsk_6;
		    HttpPagesBuffer[11]=HttpWsk_7;
		    HttpPagesBuffer[12]=HttpWsk_8;
		    HttpPagesBuffer[13]=HttpMainScript_1;
		    HttpPagesBuffer[14]=HttpMainScript_2;
		    HttpPagesBuffer[15]=0;
	   }*/
	   else if(page==15)   //3main
	   {

		    HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenu2F();
		    HttpPagesBuffer[2]=HttpWsk_3;
		    for(i=0;i<70;i++)
		    {
		    	HttpPagesBuffer[3+i++]=HttpWsk_1;
		    	HttpPagesBuffer[3+i]=HttpWsk_2;
		    }
		    HttpPagesBuffer[3+i]=0;

	   }
	   else if(page==6)  //main
	   {
		    HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenu2F();
		    for(i=0;i<60;i++)
		    {
		    	HttpPagesBuffer[2+i++]=HttpWsk_1;
		    	HttpPagesBuffer[2+i]=HttpWsk_2;
		    }
		    HttpPagesBuffer[2+i]=0;

	   }
	   else if(page==14)
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpUstawGSM;
		    HttpPagesBuffer[3]=Http_zapisz;
		    HttpPagesBuffer[4]=HttpKomendyGSM;
		    HttpPagesBuffer[5]=HttpEnd;
		    HttpPagesBuffer[6]=0;
	   }
	   else if((page==7)||(page==991))
	   {
			HttpPagesBuffer[0]="   ";
			HttpPagesBuffer[1]=HttpWsk_1;
		    HttpPagesBuffer[2]=0;
	   }
	   else if(page==16)   //TME.txt
	   {
			HttpPagesBuffer[0]=buf_TME;
		    HttpPagesBuffer[1]=0;
	   }
	   else if(page==77)   //GET /SensLora.txt
	   {
			HttpPagesBuffer[0]=buf_TME;
		    HttpPagesBuffer[1]=0;
	   }
	   else if(page==17)   // mobile/pk..
	   {
			HttpPagesBuffer[0]=buf_PK;
		    HttpPagesBuffer[1]=0;
	   }
	   else if(page==9)   // Set LoRa
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpLora;
		    HttpPagesBuffer[3]=Http_zapisz;
		    HttpPagesBuffer[4]=HttpEnd;
		    HttpPagesBuffer[5]=0;
	   }
	   else if((page>200)&&(page<400))
	   {

		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();

		   a=0;  j=0;  b=0;
		   for(i=0;i<_Size__s_Czujki_info;i++)
		   {
		     if(Const.s_Czujki[i].addr_DS[8]==(page-200))
		     {
		    	 HttpPagesBuffer[j+2]=HttpCzujki;
		    	  if(b==0){ HttpCountOffset[j]=i;   b=1; }
		    	  else      HttpCountOffset[j]=i-1;
		    	 j++;
		     }
		   }
		   if(j==0)
		   {
			   HttpPagesBuffer[j+2]=HttpEnd;
			   HttpPagesBuffer[j+3]=0;
		   }
		   else
		   {
		      HttpPagesBuffer[j+2]=Http_zapisz;
		      HttpPagesBuffer[j+3]=HttpEnd;
		      HttpPagesBuffer[j+4]=0;
		   }
	   }
	   else if((page>400)&&(page<500))
	   {
		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();

           HttpPagesBuffer[2]=HttpWeCyfr;   HttpCountOffset[0]=page-400-1;

           HttpPagesBuffer[3]=Http_zapisz;
		   HttpPagesBuffer[4]=HttpEnd;
		   HttpPagesBuffer[5]=0;

	   }
	   else if((page>1000)&&(page<1100))
	   {
		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();

           HttpPagesBuffer[2]=HttpWeEOL;   HttpCountOffset[0]=page-1000-1;

           HttpPagesBuffer[3]=Http_zapisz;
		   HttpPagesBuffer[4]=HttpEnd;
		   HttpPagesBuffer[5]=0;

	   }
	   else if((page>30)&&(page<35))
	   {
		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();

           HttpPagesBuffer[2]=HttpEtr;   HttpCountOffset[0]=page-30-1;

           HttpPagesBuffer[3]=HttpWsk_1;
           HttpPagesBuffer[4]=HttpWsk_2;
           HttpPagesBuffer[5]=HttpWsk_3;
           HttpPagesBuffer[6]=HttpWsk_4;
           HttpPagesBuffer[7]=HttpWsk_5;
           HttpPagesBuffer[8]=Http_zapisz;
		   HttpPagesBuffer[9]=HttpEnd;
		   HttpPagesBuffer[10]=0;

	   }
	   else if((page>500)&&(page<600))
	   {
		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();

           HttpPagesBuffer[2]=HttpWyCyfr;   HttpCountOffset[0]=page-500-1;

           HttpPagesBuffer[3]=Http_zapisz;
		   HttpPagesBuffer[4]=HttpEnd;
		   HttpPagesBuffer[5]=0;

	   }
	   else if((page>600)&&(page<700))
	   {
		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();

           HttpPagesBuffer[2]=HttpTimer;   HttpCountOffset[0]=page-600-1;

           HttpPagesBuffer[3]=HttpWyCyfr;
           HttpPagesBuffer[4]=Http_zapisz;
		   HttpPagesBuffer[5]=HttpEnd;
		   HttpPagesBuffer[6]=0;

	   }
	   else if((page>700)&&(page<800))
	   {
		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();

           HttpPagesBuffer[2]=HttpTermost;   HttpCountOffset[0]=page-700-1;

           HttpPagesBuffer[3]=HttpWyCyfr;
           HttpPagesBuffer[4]=Http_zapisz;
		   HttpPagesBuffer[5]=HttpEnd;
		   HttpPagesBuffer[6]=0;

	   }
	   else if(page==19)   //maski SMS
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpWsk_0;
			   HttpPagesBuffer[3]=HttpWsk_1;
			   HttpPagesBuffer[4]=HttpWsk_2;
			   HttpPagesBuffer[5]=HttpWsk_3;
			   HttpPagesBuffer[6]=HttpWsk_4;
			   HttpPagesBuffer[7]=HttpWsk_5;
			   HttpPagesBuffer[8]=HttpWsk_6;
			   HttpPagesBuffer[9]=HttpWsk_7;
			   HttpPagesBuffer[10]=HttpWsk_8;
			   HttpPagesBuffer[11]=HttpWsk_0;
			   HttpPagesBuffer[12]=HttpWsk_1;
			   HttpPagesBuffer[13]=HttpWsk_2;
		    HttpPagesBuffer[14]=Http_zapisz;
		    HttpPagesBuffer[15]=HttpEnd;
		    HttpPagesBuffer[16]=0;
	   }
	   else if(page==18)   //Maski E-mail
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpWsk_0;
			   HttpPagesBuffer[3]=HttpWsk_1;
			   HttpPagesBuffer[4]=HttpWsk_2;
			   HttpPagesBuffer[5]=HttpWsk_3;
			   HttpPagesBuffer[6]=HttpWsk_4;
			   HttpPagesBuffer[7]=HttpWsk_5;
			   HttpPagesBuffer[8]=HttpWsk_6;
			   HttpPagesBuffer[9]=HttpWsk_7;
			   HttpPagesBuffer[10]=HttpWsk_8;
			   HttpPagesBuffer[11]=HttpWsk_0;
			   HttpPagesBuffer[12]=HttpWsk_1;
			   HttpPagesBuffer[13]=HttpWsk_2;
			HttpPagesBuffer[14]=HttpUstawEmailNad;
		    HttpPagesBuffer[15]=HttpEnd;
		    HttpPagesBuffer[16]=0;
	   }
	   else if(page==3)   //Wykres
	   {
		   if(Var.wykres_nr==0xff)
		   {
			   HttpPagesBuffer[0]="brak danych do wykresu";
			   HttpPagesBuffer[1]=0;
		   }
		   else
		   {
		      HttpPagesBuffer[0]=Http_start;
		      HttpPagesBuffer[1]=HttpWsk_2;
		      HttpPagesBuffer[2]=HttpWsk_3;
		      for(i=0;i<20;i++) HttpPagesBuffer[3+i]=HttpWsk_4;
		      HttpPagesBuffer[3+i+0]=0;
		   }
	   }
	   else if(page==997)   //Wykres
	   {
		      HttpPagesBuffer[0]=Http_start;
		      HttpPagesBuffer[1]=HttpWsk_2;
		      HttpPagesBuffer[2]=HttpWsk_3;
		      for(i=0;i<20;i++) HttpPagesBuffer[3+i]=HttpWsk_4;
		      HttpPagesBuffer[3+i+0]=0;
	   }
	   else if(page==20)
	   {
		   HttpPagesBuffer[0]=HttpAuth;
		   HttpPagesBuffer[1]=0;
	   }
	   else if((page>800)&&(page<900))   //Table Translacji
	   {
		   HttpPagesBuffer[0]=HttpStyle;
		   HttpPagesBuffer[1]=HttpMenuF();
		   HttpPagesBuffer[2]=HttpStartTT;
		   for(i=0;i<20;i++) HttpPagesBuffer[3+i]=HttpWsk_1;
		   HttpPagesBuffer[3+i+0]=HttpWsk_2;
		   HttpPagesBuffer[3+i+1]=HttpEnd;
		   HttpPagesBuffer[3+i+2]=0;
	   }
	   else if(page==902)   //Rotacja
	   {
			HttpPagesBuffer[0]=HttpStyle;
		    HttpPagesBuffer[1]=HttpMenuF();
		    HttpPagesBuffer[2]=HttpWsk_1;
		    HttpPagesBuffer[3]=HttpWsk_2;
		    HttpPagesBuffer[4]=HttpWsk_3;
		    HttpPagesBuffer[5]=HttpWsk_4;
		    HttpPagesBuffer[6]=HttpWsk_5;
		    HttpPagesBuffer[7]=HttpWsk_6;
		    HttpPagesBuffer[8]=HttpEnd;
		    HttpPagesBuffer[9]=0;
	   }
	   else
	   {
		   // HttpPagesBuffer[0]=HttpStyleMain;
		    //HttpPagesBuffer[1]=&buf_rx2[_Size_BufWifi];
		    //HttpPagesBuffer[2]=0;


	   }






#endif
}

int WhichPageForGET(char *pt)
{
	int ret,m;  char *ptr;

	    if(ptr=strstr(pt,"GET /temp"))
	    {
	       m= 100*((*(ptr+9))&0x0f)+10*((*(ptr+10))&0x0f)+((*(ptr+11))&0x0f);
	       ret= 200+m;
	    }
	    else if(ptr=strstr(pt,"GET /we"))
	    {
	       m= 100*((*(ptr+7))&0x0f)+10*((*(ptr+8))&0x0f)+((*(ptr+9))&0x0f);
	       ret= 400+m;
	    }
	    else if(ptr=strstr(pt,"GET /wypk"))
	    {
	       m= 100*((*(ptr+9))&0x0f)+10*((*(ptr+10))&0x0f)+((*(ptr+11))&0x0f);
	       ret= 500+m;
	    }
	    else if(ptr=strstr(pt,"GET /tim"))
	    {
	       m= 100*((*(ptr+8))&0x0f)+10*((*(ptr+9))&0x0f)+((*(ptr+10))&0x0f);
	       ret= 600+m;
	    }
	    else if(ptr=strstr(pt,"GET /term"))
	    {
	       m= 100*((*(ptr+9))&0x0f)+10*((*(ptr+10))&0x0f)+((*(ptr+11))&0x0f);
	       ret= 700+m;
	    }
	    else if(ptr=strstr(pt,"GET /nap"))
	    {
	       m= 100*((*(ptr+8))&0x0f)+10*((*(ptr+9))&0x0f)+((*(ptr+10))&0x0f);
	       ret= 800+m;
	    }
	    else if(ptr=strstr(pt,"GET /etr"))
	    {
	       m= 100*((*(ptr+8))&0x0f)+10*((*(ptr+9))&0x0f)+((*(ptr+10))&0x0f);
	       ret= 30+m;
	    }
	    else if(ptr=strstr(pt,"GET /eol"))
	    {
	        m= 100*((*(ptr+8))&0x0f)+10*((*(ptr+9))&0x0f)+((*(ptr+10))&0x0f);
	       ret= 1000+m;
	    }
   else if(strstr(pt,"GET /maskimail")) ret=  18;
   else if(strstr(pt,"GET /maskisms")) ret=  19;
   else if(strstr(pt,"GET /upload")) ret=  2;
   else if(strstr(pt,"GET /load")) ret=  999;
   else if(strstr(pt,"GET /UserProgram")) ret=  1;
   else if(strstr(pt,"GET /wifi")) ret=  4;
   else if(strstr(pt,"GET /lan")) ret=  78;
   else if(strstr(pt,"GET /czas")) ret=  79;
   else if(strstr(pt,"GET /rej")) ret=  80;
   else if(strstr(pt,"GET /rstdp")) ret=  7;
   else if(strstr(pt,"GET /plik.txt")) ret=  81;
   else if(strstr(pt,"GET /TempWilg.txt")) ret=  10;
   else if(strstr(pt,"GET /pomiar")) ret=  82;
   else if(strstr(pt,"GET /plik.csv")) ret=  83;
   else if(strstr(pt,"GET /gsm")) ret=  14;
   else if(strstr(pt,"GET /lcd")) ret=  13;
   else if(strstr(pt,"GET /main")) ret=  6;
   else if(strstr(pt,"GET /3main")) ret=  15;
   else if(strstr(pt,"GET /searchwifi")) ret=  5;
   //else if(strstr(pt,"GET /gpio")) ret=  7;
   else if(strstr(pt,"GET /TME.txt")) ret=  16;
   else if(strstr(pt,"GET /SensLora.txt")) ret=  77;
   else if(strstr(pt,"GET /Sensagh.txt")) ret=  77;
   else if(strstr(pt,"GET /mobile/pk")) ret=  17;
   else if(strstr(pt,"GET /mobile/aghpk")) ret=  17;
   else if(strstr(pt,"GET /wykres1")) ret=  3;
   else if(strstr(pt,"GET /wykres2")) ret=  997;
   else if(strstr(pt,"GET /Auth")) ret=  20;
   else if(strstr(pt,"GET /favicon")) ret=  8;
   else if(strstr(pt,"GET /nrf")) ret=  9;
   else if(strstr(pt,"GET /obraz")) ret=  990;
   else if(strstr(pt,"GET /KRZ")) ret=  991;
   else if(strstr(pt,"GET /sesjaSMTP.txt")) ret=  992;
   else if(strstr(pt,"GET /sesjaLogGSM.txt")) ret=  998;
   else if(strstr(pt,"GET /liczgodz.txt")) ret=  993;
   else if(strstr(pt,"GET /liczdob.txt")) ret=  994;
   else if(strstr(pt,"GET /licztyg.txt")) ret=  995;
   else if(strstr(pt,"GET /liczmies.txt")) ret=  996;
   else if(strstr(pt,"GET /agh")) ret=  21;
   else if(strstr(pt,"GET /IRprga")) ret=  900;
   else if(strstr(pt,"GET /IRprgb")) ret=  901;
   else if(strstr(pt,"GET /Rot")) ret=  902;
   else if(strstr(pt,"GET /logic")) ret=  1669;
   else
   {
	     ret=  15;
   }
   return ret;
}

void WyborWpisowPanel(int nr, int i)
{
   char sss[15]; int kolejn;
   if(i==5) kolejn=1;
   if(i==6) kolejn=2;
   if(i==7) kolejn=3;
   if(i==8) kolejn=4;
   if(i==/*17*/9) kolejn=5;
   if(i==/*18*/10) kolejn=6;
   if(i==28) kolejn=7;
   if(i==21) kolejn=8;
   if(i==/*15*/22) kolejn=9;
   if(i==23) kolejn=10;
   if(i==24) kolejn=11;
   if(i==33) kolejn=12;
   if(kolejn<10) sprintf(sss,"&nbsp; %d",kolejn); else sprintf(sss,"%d",kolejn);

	    if(nr==1)  sprintf(buf_p,"<option value=\"00p%03d\"          >%s &nbsp;Przekaznik</option>\x0D\x0A",i,sss);
   else if(nr==2)  sprintf(buf_p,"<option value=\"02p%03d\"          >%s &nbsp;Temperatura</option>\x0D\x0A",i,sss);
   else if(nr==3)  sprintf(buf_p,"<option value=\"03p%03d\"          >%s &nbsp;Czujnik Pt1000</option>\x0D\x0A",i,sss);
   else if(nr==4)  sprintf(buf_p,"<option value=\"09p%03d\"          >%s &nbsp;Temperatura Wilgotnosc</option>\x0D\x0A",i,sss);
   else if(nr==5)  sprintf(buf_p,"<option value=\"10p%03d\"          >%s &nbsp;Timer</option>\x0D\x0A",i,sss);
   else if(nr==6)  sprintf(buf_p,"<option value=\"11p%03d\"          >%s &nbsp;Termostat</option>\x0D\x0A",i,sss);
   else if(nr==7)  sprintf(buf_p,"<option value=\"04p%03d\"          >%s &nbsp;Czujniki zwar-rozwar</option>\x0D\x0A",i,sss);
   else if(nr==8)  sprintf(buf_p,"<option value=\"01p%03d\"          >%s &nbsp;Sterowanie PWM</option>\x0D\x0A",i,sss);
   else if(nr==9)  sprintf(buf_p,"<option value=\"05p%03d\"          >%s &nbsp;Pomiar Napiecia 0-10V</option>\x0D\x0A",i,sss);
   else if(nr==10) sprintf(buf_p,"<option value=\"06p%03d\"          >%s &nbsp; -- -- -- </option>\x0D\x0A",i,sss);
   else if(nr==11) sprintf(buf_p,"<option value=\"07p%03d\"          >%s &nbsp;Licznik impulsow </option>\x0D\x0A",i,sss);
   else if(nr==12) sprintf(buf_p,"<option value=\"08p%03d\"          >%s &nbsp;Sterowanie IR </option>\x0D\x0A",i,sss);
   else if(nr==13) sprintf(buf_p,"<option value=\"13p%03d\"          >%s &nbsp;Wejscie EOL </option>\x0D\x0A",i,sss);

}

WpisPortElement()  //wpis
{
     //t_..[nr_czujki]= nr_portu

	odd= 1;   //Ile Portów na WWW
	ddo= 12;

/*
 *   Rotacja_PK_Alarm_on      ->  __Port_PK_6
 *   Rotacja temp alarmowa    ->    TabPort[0]
 *
 */

	TabPort[0]=__Port_Ogoln_5;   //wybierz osobliw¹ kolejnosc Portow na WWW
	TabPort[1]=__Port_Ogoln_6;
	TabPort[2]=__Port_Ogoln_7;
	TabPort[3]=__Port_Ogoln_8;
	TabPort[4]=__Port_Ogoln_9;  //__Port_Ogoln_Etr_1
	TabPort[5]=__Port_Ogoln_10;//__Port_Ogoln_Etr_2;
	TabPort[6]=__Port_PK_1;
	TabPort[7]=__Port_PWM_1;
	TabPort[8]=__Port_PWM_2;//__Port_Ogoln_Nap_1;
	TabPort[9]=__Port_PWM_3;
	TabPort[10]=__Port_OC_1;
	TabPort[11]=__Port_PK_6;


//	TabPort[0]=__Port_Ogoln_1;   //wybierz osobliw¹ kolejnosc Portow na WWW
//	TabPort[1]=__Port_Ogoln_2;
//	TabPort[2]=__Port_Ogoln_3;
//	TabPort[3]=__Port_Ogoln_4;
//	TabPort[4]=__Port_Ogoln_5;
//	TabPort[5]=__Port_Ogoln_6;
//	TabPort[6]=__Port_Ogoln_7;
//	TabPort[7]=__Port_Ogoln_8;
//	TabPort[8]=__Port_Ogoln_9;
//	TabPort[9]=__Port_Ogoln_10;
//	TabPort[10]=__Port_Ogoln_11;
//	TabPort[11]=__Port_Ogoln_12;
//	TabPort[12]=__Port_Ogoln_13;
//	TabPort[13]=__Port_Ogoln_14;
//	TabPort[14]=__Port_Ogoln_Nap_1;
//	TabPort[15]=__Port_Ogoln_Nap_2;
//	TabPort[16]=__Port_Ogoln_Etr_1;
//	TabPort[17]=__Port_Ogoln_Etr_2;
//	TabPort[18]=__Port_Ogoln_Etr_3;
//	TabPort[19]=__Port_Ogoln_Etr_4;
//	TabPort[20]=__Port_PWM_1;
//	TabPort[21]=__Port_PWM_2;
//	TabPort[22]=__Port_PWM_3;
//	TabPort[23]=__Port_OC_1;
//	TabPort[24]=__Port_OC_2;
//	TabPort[25]=__Port_OC_3;
//	TabPort[26]=__Port_OC_4;
//	TabPort[27]=__Port_PK_1;
//	TabPort[28]=__Port_PK_2;
//	TabPort[29]=__Port_PK_3;
//	TabPort[30]=__Port_PK_4;
//	TabPort[31]=__Port_PK_5;
//	TabPort[32]=__Port_PK_6;



  //'t_pt'  nalezy do      't_te'
  //'t_te'  pokrywa sie z  't_we'


    t_te[0]=__Port_Ogoln_1;  //przypisywanie funkcji do poszczegolnych Portów np:    "dla portu '__Port_Ogoln_1' funkcja 't_te'
    t_te[1]=__Port_Ogoln_2;
    t_te[2]=__Port_Ogoln_3;
    t_te[3]=__Port_Ogoln_4;
    t_te[4]=__Port_Ogoln_5;
    t_te[5]=__Port_Ogoln_6;
    t_te[6]=__Port_Ogoln_7;
    t_te[7]=__Port_Ogoln_8;
    t_te[8]=__Port_Ogoln_9;
    t_te[9]=__Port_Ogoln_10;
    t_te[10]=__Port_Ogoln_11;
    t_te[11]=__Port_Ogoln_12;
    t_te[12]=__Port_Ogoln_13;
    t_te[13]=__Port_Ogoln_14;
    t_te[14]=__Port_Ogoln_Etr_1;
    t_te[15]=__Port_Ogoln_Etr_2;
    t_te[16]=__Port_Ogoln_Etr_3;
    t_te[17]=__Port_Ogoln_Etr_4;



    t_we[0]=__Port_Ogoln_1;
    t_we[1]=__Port_Ogoln_2;
    t_we[2]=__Port_Ogoln_3;
    t_we[3]=__Port_Ogoln_4;
    t_we[4]=__Port_Ogoln_5;
    t_we[5]=__Port_Ogoln_6;
    t_we[6]=__Port_Ogoln_7;
    t_we[7]=__Port_Ogoln_8;
    t_we[8]=__Port_Ogoln_9;
    t_we[9]=__Port_Ogoln_10;
    t_we[10]=__Port_Ogoln_11;
    t_we[11]=__Port_Ogoln_12;
    t_we[12]=__Port_Ogoln_13;
    t_we[13]=__Port_Ogoln_14;
    t_we[14]=__Port_Ogoln_Etr_1;
    t_we[15]=__Port_Ogoln_Etr_2;
    t_we[16]=__Port_Ogoln_Etr_3;
    t_we[17]=__Port_Ogoln_Etr_4;

    t_eol[0]=__Port_Ogoln_1;      //t_oel musi sie zawierac w t_we !!!!
     t_eol[1]=__Port_Ogoln_2;
     t_eol[2]=__Port_Ogoln_3;
     t_eol[3]=__Port_Ogoln_4;
     t_eol[4]=__Port_Ogoln_5;
     t_eol[5]=__Port_Ogoln_6;
     t_eol[6]=__Port_Ogoln_7;
     t_eol[7]=__Port_Ogoln_8;
     t_eol[8]=__Port_Ogoln_9;
     t_eol[9]=__Port_Ogoln_10;
     t_eol[10]=__Port_Ogoln_11;
     t_eol[11]=__Port_Ogoln_12;
     t_eol[12]=__Port_Ogoln_13;
     t_eol[13]=__Port_Ogoln_14;



    t_wy[0]=__Port_PK_1;
    t_wy[1]=__Port_PK_2;
    t_wy[2]=__Port_PK_3;
    t_wy[3]=__Port_PK_4;
    t_wy[4]=__Port_PK_5;
    t_wy[5]=__Port_PK_6;
    t_wy[6]=__Port_OC_1;
    t_wy[7]=__Port_OC_2;
    t_wy[8]=__Port_OC_3;
    t_wy[9]=__Port_OC_4;
    t_wy[10]=__Port_PWM_1;
    t_wy[11]=__Port_PWM_2;
    t_wy[12]=__Port_PWM_3;



    t_pt[0]=__Port_Ogoln_1;
    t_pt[1]=__Port_Ogoln_2;
    t_pt[2]=__Port_Ogoln_3;
    t_pt[3]=__Port_Ogoln_4;
    t_pt[4]=__Port_Ogoln_5;
    t_pt[5]=__Port_Ogoln_6;
    t_pt[6]=__Port_Ogoln_7;
    t_pt[7]=__Port_Ogoln_8;
    t_pt[8]=__Port_Ogoln_9;
    t_pt[9]=__Port_Ogoln_10;
    t_pt[10]=__Port_Ogoln_11;
    t_pt[11]=__Port_Ogoln_12;
    t_pt[12]=__Port_Ogoln_13;
    t_pt[13]=__Port_Ogoln_14;
    //t_pt[14]=__Port_Ogoln_Nap_1;
    //t_pt[15]=__Port_Ogoln_Nap_2;


    t_pwm[0]=__Port_PWM_1;
    t_pwm[1]=__Port_PWM_2;
    t_pwm[2]=__Port_PWM_3;


    t_TT[0]=__Port_Ogoln_Nap_1;
    t_TT[1]=__Port_Ogoln_Nap_2;


    t_imp[0]=__Port_Ogoln_Etr_1;
    t_imp[1]=__Port_Ogoln_Etr_2;
    t_imp[2]=__Port_Ogoln_Etr_3;
    t_imp[3]=__Port_Ogoln_Etr_4;


    t_ir[0]=__Port_PWM_1;
    t_ir[1]=__Port_PWM_2;
    t_ir[2]=__Port_PWM_3;
    t_ir[3]=__Port_OC_1;
    t_ir[4]=__Port_OC_2;
    t_ir[5]=__Port_OC_3;
    t_ir[6]=__Port_OC_4;






}


int WyszukujPortElement(unsigned char *tab, int size, int port)
{
   int i;

	  for(i=0;i<size;i++)
	  {
          if(tab[i]==port) return i;
	  }
	  return -1;
}

int HttpNazwyPortow(int *pocz)
{
  int i,j,itx;   char *ptr,sss[15];  itx=0;

 if(*pocz==0)
 {
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<form method=\"get\" action=\"lpc.cgi/A117\" data-ajax=\"false\">\x0D\x0A\
 \x0D\x0A\
 <div class=\"corner-radius4\" style=\"margin-left: 0px; width: 340px; height:   (<<<<*)   \">\x0D\x0A\
 <div class=\"corner-radius3\" style=\"margin-left: 10px; width: 310px; height:  (>>>>*)   \">\x0D\x0A\
 <table><tr><td width=\"30px\"></td><td><font class=\"css-font-style\" style=\"font-family: Courier\"><b>Nazwa Portu Nr:</b></font></td></tr></table>\x0D\x0A");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
 }

 for(i=*pocz;i<ddo;i++)
 {
	 if(i==0) i=odd-1;
	 if(TabPort[i]<10) sprintf(sss,"&nbsp; %d",TabPort[i]); else sprintf(sss,"%d",TabPort[i]);
	 if((itx+230)>=_Size_WskFragmentPage){ *pocz=i; return itx; }
	 sprintf(&buf_rx2[_Size_BufWifi+itx],"<table><tr><td><font class=\"css_list\">%d.</font><input type=\"text\" class=\"textbox\" style=\"width:240px;\" name=\"n_s_Port_nazwa%d_000\" size=\"30\" maxlength=\"30\" value=\"\"                               ></td></tr></table>\x0D\x0A",i+1,TabPort[i]); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
 }

 if((itx+570)>=_Size_WskFragmentPage){ *pocz=ddo; return itx; }
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<table><tr><td width=\"80px\"></td><td><button type=\"submit\" name=\"n_109_zapisz\" class=\"css_button\" style=\"padding: 2px 17px;\" >Zapisz Nazwy</button></td></tr></table>\x0D\x0A\
</div></div>\x0D\x0A\
</form>\x0D\x0A\
<div class=\"corner-radius4\" style=\"margin-left: 0px; width: 340px; height:   (<<<<*)   \">\x0D\x0A\
<div class=\"corner-radius3\" style=\"margin-left: 10px; width: 310px; height:  (>>>>*)   \">\x0D\x0A\
<table><tr><td><font class=\"css-font-style\" style=\"font-family: Courier\"><b>Tryb Pracy Portu nr:</b></font><br></td></tr></table>\x0D\x0A\
\x0D\x0A");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


 *pocz=ddo+1;   return itx;

}

void HttpMain_22(int d1, int d2)
{
  int i,j,itx;   char *ptr;  itx=0;

 for(i=d1;i<(d2+1);i++)
 {
		sprintf(&buf_rx2[_Size_BufWifi+itx],"<table><tr><td><form name=\"wybor%03d\" method=\"get\"  action=\"lpc.cgi/A117\" data-ajax=\"false\">\x0D\x0A\
 <div id=\"mainselection\">\x0D\x0A\
 <select name=\"n_GPIO_val_%03d\" style=\"width: 290px\">\x0D\x0A",TabPort[i-1],TabPort[i-1]);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(WyszukujPortElement(t_wy,_Size_wy,TabPort[i-1])!=-1){   WyborWpisowPanel(1,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_te,_Size_te,TabPort[i-1])!=-1){   WyborWpisowPanel(2,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_pt,_Size_pt,TabPort[i-1])!=-1){   WyborWpisowPanel(3,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_te,_Size_te,TabPort[i-1])!=-1){   WyborWpisowPanel(4,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_wy,_Size_wy,TabPort[i-1])!=-1){   WyborWpisowPanel(5,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_wy,_Size_wy,TabPort[i-1])!=-1){   WyborWpisowPanel(6,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_we,_Size_we,TabPort[i-1])!=-1){   WyborWpisowPanel(7,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_pwm,_Size_pwm,TabPort[i-1])!=-1){ WyborWpisowPanel(8,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_TT,_Size_TT,TabPort[i-1])!=-1){   WyborWpisowPanel(9,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_imp,_Size_imp,TabPort[i-1])!=-1){ WyborWpisowPanel(11,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_ir,_Size_ir,TabPort[i-1])!=-1){   WyborWpisowPanel(12,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 if(WyszukujPortElement(t_eol,_Size_eol,TabPort[i-1])!=-1){ WyborWpisowPanel(13,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }
 WyborWpisowPanel(10,TabPort[i-1]); sprintf(&buf_rx2[_Size_BufWifi+itx],buf_p);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 sprintf(&buf_rx2[_Size_BufWifi+itx],"\
 </select></div>\x0D\x0A\
 </form></td></tr></table>\x0D\x0A\
 \x0D\x0A");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(i==ddo){ sprintf(&buf_rx2[_Size_BufWifi+itx],HttpMain_3); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]); }

 }




/*
for(i=d1;i<(d2+1);i++)
{
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<table><tr><td><form name=\"wybor%03d\" method=\"get\"  action=\"lpc.cgi/A117\" data-ajax=\"false\">\x0D\x0A\
 <div id=\"mainselection\">\x0D\x0A\
 <select name=\"n_GPIO_val_%03d\">\x0D\x0A\
 <option value=\"00p%03d\"          >%d &nbsp;Przekaznik</option>\x0D\x0A\
 <option value=\"02p%03d\"          >%d &nbsp;Temperatura</option>\x0D\x0A\
 <option value=\"03p%03d\"          >%d &nbsp;Czujnik Pt1000</option>\x0D\x0A\
 <option value=\"09p%03d\"          >%d &nbsp;Temperatura Wilgotnosc</option>\x0D\x0A\
 <option value=\"10p%03d\"          >%d &nbsp;Timer</option>\x0D\x0A\
 <option value=\"11p%03d\"          >%d &nbsp;Termostat</option>\x0D\x0A\
 <option value=\"04p%03d\"          >%d &nbsp;Czujniki zwar-rozwar</option>\x0D\x0A\
 <option value=\"01p%03d\"          >%d &nbsp;Sterowanie PWM</option>\x0D\x0A\
 <option value=\"05p%03d\"          >%d &nbsp;Pomiar Napiecia 0-10V</option>\x0D\x0A\
 <option value=\"06p%03d\"          >%d &nbsp; -- -- -- </option>\x0D\x0A\
 </select></div>\x0D\x0A\
 </form></td></tr></table>\x0D\x0A\
 \x0D\x0A",i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i);

 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(i==9){ sprintf(&buf_rx2[_Size_BufWifi+itx],HttpMain_3); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  }

}*/

if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO itx !!!");  sprintf(buf_p,"\r\nItx: %d   %d  ",itx,WhichFragmentPage); dbg3(buf_p);

}

int HttpMainScript(int *point)
{
  int i,j,itx,k,KO,n, pocz,kolej; k=0; KO=0;  char *ptr,c,sss[30],sss2[30];  itx=0;   buf_rx2[_Size_BufWifi]=0;

  kolej=*point/100;  pocz=0;    if((*point%100)!=0){ pocz= *point-(100*kolej)-(odd-1); sprintf(buf_p," POCZ=%d  ",pocz); dbg3(buf_p); }

       if(kolej==0) goto _poziom00;
  else if(kolej==1) goto _poziom01;
  else if(kolej==2) goto _poziom02;
  else if(kolej==3) goto _poziom03;
  else if(kolej==4) goto _poziom04;
  else if(kolej==5) goto _poziom05;
  else if(kolej==6) goto _poziom06;
  else if(kolej==7) goto _poziom07;
  else if(kolej==8) goto _poziom08;
  else if(kolej==9) goto _poziom09;
  else if(kolej==10) goto _poziom10;
  else if(kolej==11) goto _poziom11;
  else if(kolej==12) goto _poziom12;
  else if(kolej==13) goto _poziom13;
  else if(kolej==14) goto _poziom14;
  else if(kolej==15) goto _poziom15;
  else if(kolej==16) goto _poziom16;
  else if(kolej==17) goto _poziom17;
  else if(kolej==18) goto _poziom17a;
  else if(kolej==19) goto _poziom18;
  else if(kolej==20) goto _poziom19;
  else if(kolej==21) goto _poziom20;
  else if(kolej==22) goto _poziom20a;
  else if(kolej==23) goto _poziom21;



  _poziom00:
   dbg3("K0 "); kolej++;   if((itx+141)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }
  sprintf(&buf_rx2[_Size_BufWifi+itx],"</div></div>\r\n</div>\r\n</body>\r\n<script type=\"text/javascript\">\r\nwindow.onload = function(){\r\nLaduj();\r\npageScroll();\r\n };\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

  _poziom01:
  dbg3("K1 "); kolej++;
  for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
  {
	  if((itx+34)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
	  sprintf(&buf_rx2[_Size_BufWifi+itx],"var active_id%03d=\"00p%03d\";\r\n",TabPort[j-1],TabPort[j-1]);
	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  }
  pocz=0;

  _poziom02:
  dbg3("K2 "); kolej++;   if((itx+25)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }
  sprintf(&buf_rx2[_Size_BufWifi+itx],"function Laduj()\r\n{\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


  _poziom03:
  dbg3("K3 "); kolej++;
  for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
  {
	  if((itx+91)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
	  sprintf(&buf_rx2[_Size_BufWifi+itx],"var blok2 = document.getElementById(active_id%03d);\r\nblok2.style.display = \"block\";\r\n",TabPort[j-1]);
	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  }
  pocz=0;

  sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

  _poziom04:
  dbg3("K4 "); kolej++;
  for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
  {
	  if((itx+69)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
	  sprintf(&buf_rx2[_Size_BufWifi+itx],"document.forms['wybor%03d'].n_GPIO_val_%03d.onchange = Zmien%03d;\r\n",TabPort[j-1],TabPort[j-1],TabPort[j-1]);
	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  }
  pocz=0;

  sprintf(&buf_rx2[_Size_BufWifi+itx],"}\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


  _poziom05:
  dbg3("K5 "); kolej++;
  for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
  {
	  if((itx+365)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
	  sprintf(&buf_rx2[_Size_BufWifi+itx]," function Zmien%03d()\x0D\x0A\
 {\x0D\x0A\
 if (active_id%03d != \"\")\x0D\x0A\
 {\x0D\x0A\
 var blok2 = document.getElementById(active_id%03d);\x0D\x0A\
 blok2.style.display = \"none\";\x0D\x0A\
 }\x0D\x0A\
 var blok = document.getElementById(this.value);\x0D\x0A\
 blok.style.display = \"block\";\x0D\x0A\
 active_id%03d = this.value;\x0D\x0A\
 }\x0D\x0A",TabPort[j-1],TabPort[j-1],TabPort[j-1],TabPort[j-1]);
	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  }
  pocz=0;


  _poziom06:
  dbg3("K6 "); kolej++;  if((itx+115)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }
  sprintf(&buf_rx2[_Size_BufWifi+itx],"function zaladuj()\r\n{\r\nwindow.location.href=\"lpc.cgi/n_GPIO_val_%03d=\" + active_id%03d[0]+active_id%03d[1] +\r\n", TabPort[pocz+1+(odd-1)-1], TabPort[pocz+1+(odd-1)-1], TabPort[pocz+1+(odd-1)-1]);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


  _poziom07:
  dbg3("K7 "); kolej++;
  for(j=(pocz+2+(odd-1));j<(ddo+1);j++)
  {
	  if((itx+62)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-2)); return itx; }
	  sprintf(&buf_rx2[_Size_BufWifi+itx],"\"&n_GPIO_val_%03d=\" + active_id%03d[0]+active_id%03d[1]+\r\n",TabPort[j-1],TabPort[j-1],TabPort[j-1]);
	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  }
  pocz=0;


  _poziom08:
  dbg3("K8 "); kolej++;   if((itx+1000)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }
  sprintf(&buf_rx2[_Size_BufWifi+itx],"\"&n_109_zapisz=\";\x0D\x0A\
 }\x0D\x0A\
  </script>\x0D\x0A\
 \x0D\x0A\
 <script>\x0D\x0A\
 var nr_pwm; var val_pwm; var i,j,wst;  var txt=[]; \x0D\x0A\
 wst=0; \x0D\x0A\
 var now=0;void setInterval(function(){now++;if(now==2){wst=0;}},500);\x0D\x0A\ 
 function loadXMLDoc_PWM()\x0D\x0A\
 {var xmlhttp;v_dddd \x0D\x0A\
 if (window.XMLHttpRequest) xmlhttp=new XMLHttpRequest();\x0D\x0A\
 else xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");  \x0D\x0A\
 xmlhttp.open(\"GET\",\"/lpc.cgi/n_pwm\"+nr_pwm+\"=\"+val_pwm+\"&n_115_zapisz=\", true);\x0D\x0A\
 xmlhttp.send();\x0D\x0A\
 }\x0D\x0A\
 function loadXMLDoc_PK()\x0D\x0A\
 { var xmlhttp;v_dddd \x0D\x0A\
   if (window.XMLHttpRequest) xmlhttp=new XMLHttpRequest();\x0D\x0A\
   else xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");  \x0D\x0A\
   xmlhttp.onreadystatechange=function()\x0D\x0A\
   { if (xmlhttp.readyState==4 && xmlhttp.status==200){\x0D\x0A\		
	  txt= xmlhttp.responseText;\x0D\x0A\
	  if(txt[0]=='<'){\x0D\x0A");    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


 _poziom09:
 dbg3("K9 "); kolej++;
 for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
 {
	 i= WyszukujPortElement(t_wy,_Size_wy,TabPort[j-1]);
     if(i!=-1)
	 {
   	     if((itx+94)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
   	     sprintf(&buf_rx2[_Size_BufWifi+itx],"if(nr_pwm==%d) document.getElementById(\"tu_pk0%02d\").innerHTML=xmlhttp.responseText;\x0D\x0A",TabPort[j-1],TabPort[j-1]);
   	     itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	 }

 }
 pocz=0;

 _poziom10:
 dbg3("K10 ");  kolej++;  if((itx+15)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }
 sprintf(&buf_rx2[_Size_BufWifi+itx],"}\r\n}\r\n}\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 _poziom11:
 dbg3("K11 ");  kolej++;
 for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
 {
	 i= WyszukujPortElement(t_wy,_Size_wy,TabPort[j-1]);
     if(i!=-1)
	 {
   	     if((itx+66)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
   	     sprintf(&buf_rx2[_Size_BufWifi+itx],"if(nr_pwm==%d) xmlhttp.open(\"GET\",\"/mobile/pk%03d\", true);\r\n",TabPort[j-1],i+1);
     	 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	 }
 }
 pocz=0;

 _poziom12:
 dbg3("K12 "); kolej++;  if((itx+32)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }
 sprintf(&buf_rx2[_Size_BufWifi+itx],"xmlhttp.send();\x0D\x0A}\x0D\x0A");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


 _poziom13:
 dbg3("K13 "); kolej++;
 for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
 {
	 i= WyszukujPortElement(t_wy,_Size_wy,TabPort[j-1]);
     if(i!=-1)
	 {
   	     if((itx+79)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
   	     sprintf(&buf_rx2[_Size_BufWifi+itx],"function xpk%03d(){if(wst==0){wst=1;now=0;nr_pwm=%d;loadXMLDoc_PK(); }}\x0D\x0A",TabPort[j-1],TabPort[j-1]);
    	 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	 }
 }
 pocz=0;


 _poziom14:
 dbg3("K14 "); kolej++;
	 for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
	 {
		 i= WyszukujPortElement(t_pwm,_Size_pwm,TabPort[j-1]);
	     if(i!=-1)
		 {
		     if((itx+174)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
		     sprintf(&buf_rx2[_Size_BufWifi+itx],"function outputUpdate%03d(vol){document.querySelector('#volume%03d').value=vol;}function x%03d(val){if(wst==0){wst=1;now=0;nr_pwm=%d; val_pwm=val; loadXMLDoc_PWM();}}\x0D\x0A",TabPort[j-1],TabPort[j-1],TabPort[j-1],i+1);
	 	     itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 }
	 }
	  pocz=0;

	 _poziom15:
	 dbg3("K15 ");  kolej++;   if((itx+872)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }
	 sprintf(&buf_rx2[_Size_BufWifi+itx],"var myVar = setInterval(loadXMLDoc, v_reff     );\x0D\x0A\
 var txt=[]; \x0D\x0A\
 var txt1=[]; \x0D\x0A\
 var txt2=[]; \x0D\x0A\
 var txt3=[]; \x0D\x0A\
 var txt4=[]; \x0D\x0A\
 var txt5=[]; \x0D\x0A\
 var ccol=[]; \x0D\x0A\
 function loadXMLDoc()\x0D\x0A\
 {if(wst==0){wst=1;v_dddd now=0;\x0D\x0A\
   var xmlhttp;\x0D\x0A\
   if (window.XMLHttpRequest) xmlhttp=new XMLHttpRequest();\x0D\x0A\
   else xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");\x0D\x0A\
   xmlhttp.onreadystatechange=function()\x0D\x0A\
   { if (xmlhttp.readyState==4 && xmlhttp.status==200)\x0D\x0A\
     {  \x0D\x0A\		
		 txt= xmlhttp.responseText;\x0D\x0A\
		 txt3=txt[0]+txt[1]+txt[2]+txt[3]+txt[4]+txt[5]+txt[6]+txt[7]; document.getElementById(\"tu_zgr\").innerHTML=txt3;\x0D\x0A\
		 txt3=\"GSM: \"+txt[23]+txt[24]; document.getElementById(\"tu_gsm\").innerHTML=txt3;\x0D\x0A");     itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


_poziom16:
dbg3("K16 "); kolej++;
  sprintf(sss,"25");
  for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
  {
	  i= WyszukujPortElement(t_wy,_Size_wy,TabPort[j-1]);
	  if(i!=-1)
	  {
	      if((itx+321)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
	      sprintf(&buf_rx2[_Size_BufWifi+itx],"if((txt[%s+%d]=='0')||(txt[%s+%d]=='2')) txt3=\"<font color='#000'>v_pkstof%02d           </font>\"; else txt3=\"<font color='#eee'>v_pkston%02d           </font>\";  document.getElementById(\"tu_pk0%02d\").innerHTML=txt3; document.getElementById(\"tu_ti0%02d\").innerHTML=txt3; document.getElementById(\"tu_te0%02d\").innerHTML=txt3;   if((txt[%s+%d]=='2')||(txt[%s+%d]=='3')) document.getElementById(\"tu_lora%03dwy\").innerHTML=\"<div id='tu_ant'></div>\"; else document.getElementById(\"tu_lora%03dwy\").innerHTML=\" \";\x0D\x0A",sss,i,sss,i,i+1,i+1,TabPort[j-1],TabPort[j-1],TabPort[j-1],sss,i,sss,i,TabPort[j-1],TabPort[j-1]);
	      itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	  }
  }
  pocz=0;

	 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	 _poziom17:
	 dbg3("K17 "); kolej++;
	 sprintf(sss,"25+%d+1",_Size_wy);
	 for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
	 {
		  i= WyszukujPortElement(t_we,_Size_we,TabPort[j-1]);
		  if(i!=-1)
		  {
		      if((itx+380)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
		      if(WhichPage==15) sprintf(&buf_p[0],"d00"); else sprintf(&buf_p[0],"f88");
		      if(WhichPage==15) sprintf(&buf_p[10],"000"); else sprintf(&buf_p[10],"eee");
	          sprintf(&buf_rx2[_Size_BufWifi+itx],"if((txt[%s+%d]=='0')||(txt[%s+%d]=='2')) txt3=\"<font color='#%s'>v_weston%02d           </font>\"; else txt3=\"<font color='#%s'>v_westof%02d           </font>\";  document.getElementById(\"tu_we0%02d\").innerHTML=txt3; if((txt[%s+%d]=='2')||(txt[%s+%d]=='3')) document.getElementById(\"tu_lora%03dwe\").innerHTML=\"<div id='tu_ant'></div>\"; else document.getElementById(\"tu_lora%03dwe\").innerHTML=\" \";\x0D\x0A",sss,i,sss,i,&buf_p[0],i+1,&buf_p[10],i+1,TabPort[j-1],sss,i,sss,i,TabPort[j-1],TabPort[j-1]);
	          itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		  }
	 }
	  pocz=0;

	 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	 _poziom17a:
	 dbg3("K17a "); kolej++;
	 sprintf(sss,"25+%d+1+%d+1",_Size_wy,_Size_we);
	 for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
	 {
		  i= WyszukujPortElement(t_eol,_Size_eol,TabPort[j-1]);
		  if(i!=-1)
		  {
		      if((itx+380)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
		      if(WhichPage==15) sprintf(&buf_p[0],"d00"); else sprintf(&buf_p[0],"f88");
		      if(WhichPage==15) sprintf(&buf_p[10],"000"); else sprintf(&buf_p[10],"eee");
	          sprintf(&buf_rx2[_Size_BufWifi+itx],"if((txt[%s+%d]=='5')||(txt[%s+%d]=='6')) txt3=\"<font color='#%s'>v_westos%02d           </font>\"; else if((txt[%s+%d]=='0')||(txt[%s+%d]=='2')) txt3=\"<font color='#%s'>v_weston%02d           </font>\"; else txt3=\"<font color='#%s'>v_westof%02d           </font>\";  document.getElementById(\"tu_eol0%02d\").innerHTML=txt3; if((txt[%s+%d]=='2')||(txt[%s+%d]=='3')) document.getElementById(\"tu_lora%03deol\").innerHTML=\"<div id='tu_ant'></div>\"; else document.getElementById(\"tu_lora%03deol\").innerHTML=\" \";\x0D\x0A",sss,i,sss,i,&buf_p[0],i+1,sss,i,sss,i,&buf_p[0],i+1,&buf_p[10],i+1,TabPort[j-1],sss,i,sss,i,TabPort[j-1],TabPort[j-1]);
	          itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		  }
	 }
	  pocz=0;

	 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	 _poziom18:
	 dbg3("K18 ");  kolej++;
	 for(j=(pocz+1+(odd-1));j<(ddo+1);j++)
	 {

	i= WyszukujPortElement(t_te,_Size_te,TabPort[j-1]);
    if(i!=-1)
	{
		 if((itx+1250)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+(j-1)); return itx; }
		 sprintf(sss,"25+%d+1+%d+1+%d+1+%d",_Size_wy,_Size_we,_Size_eol,i*12);

		sprintf(&buf_rx2[_Size_BufWifi+itx],"\ 
 for(i=0;i<7;i++) txt4[i]=txt[%s+i];if(txt4[5]=='1') ccol=\"d00\";if(txt4[5]=='0') ccol=\"000\";if(txt4[5]=='2') ccol=\"00d\"; document.getElementById(\"tu_temp%03da\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt4[0]+\" \"+txt4[1]+txt4[2]+txt4[3]+txt4[4]+\" °C\"+\"</font>\";if(txt4[6]=='x') document.getElementById(\"tu_lora%03da\").innerHTML=\"<div id='tu_ant'></div>\"; else document.getElementById(\"tu_lora%03da\").innerHTML=\" \";\x0D\x0A\
 for(i=0;i<7;i++) txt4[i]=txt[%s+i];if(txt4[5]=='1') ccol=\"d00\";if(txt4[5]=='0') ccol=\"000\";if(txt4[5]=='2') ccol=\"00d\"; document.getElementById(\"tu_temp%03db\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt4[0]+\" \"+txt4[1]+txt4[2]+txt4[3]+txt4[4]+\" °C\"+\"</font>\";if(txt4[6]=='x') document.getElementById(\"tu_lora%03db\").innerHTML=\"<div id='tu_ant'></div>\"; else document.getElementById(\"tu_lora%03db\").innerHTML=\" \";\x0D\x0A\
 for(i=0;i<2;i++) txt4[i]=txt[%s+7+i];txt4[5]=txt[%s+7+3]; txt4[i++]=' ';txt4[i]=txt[%s+7+2]; if(txt4[5]=='1') ccol=\"d00\";if(txt4[5]=='0') ccol=\"000\";if(txt4[5]=='2') ccol=\"00d\"; document.getElementById(\"tu_temp%03dc\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt4[0]+txt4[1]+txt4[2]+txt4[3]+\"</font>\";\x0D\x0A\
 \x0D\x0A ",sss,TabPort[j-1],TabPort[j-1],TabPort[j-1],sss,TabPort[j-1],TabPort[j-1],TabPort[j-1],sss,sss,sss,TabPort[j-1]);    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}

	 }
	  pocz=0;


	 _poziom19:
	 dbg3("K19 ");  kolej++;
	 for(j=(pocz+0+(odd-1));j<ddo;j++)  //TT
	 {
		 i= WyszukujPortElement(t_TT,_Size_TT,TabPort[j]);
	     if(i!=-1)
		 {
		    if((itx+246)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+j); return itx; }
		    sprintf(sss, "25+%d+1+%d+1+%d+1+%d+1+%d+1+%d+1+%d",_Size_wy, _Size_we, _Size_eol, 12*_Size_te, 10*_Size_TT, 10*_Size_pt, i*6);  //nazwy jedn.
		    sprintf(sss2,"25+%d+1+%d+1+%d+1+%d+1+%d",_Size_wy, _Size_we, _Size_eol, 12*_Size_te, i*10);  //wartosci z ADC.
	        sprintf(&buf_rx2[_Size_BufWifi+itx],"for(i=0;i<5;i++) txt5[i]=txt[%s+i];for(i=0;i<9;i++) txt4[i]=txt[%s+i];document.getElementById(\"tu_adc%03d\").innerHTML=txt4[0]+txt4[1]+txt4[2]+txt4[3]+txt4[4]+txt4[5]+txt4[6]+txt4[7]+txt4[8]+\" \"+txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4];\x0D\x0A",sss,sss2,TabPort[j]);
	        itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 }
	  }
	  pocz=0;

	 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	 _poziom20:
	 dbg3("K20 "); kolej++;
	 for(j=(pocz+0+(odd-1));j<ddo;j++)  //Pt1000
	 {
		 i= WyszukujPortElement(t_pt,_Size_pt,TabPort[j]);
	     if(i!=-1)
		 {
		    if((itx+295)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+j); return itx; }
		    sprintf(sss,"25+%d+1+%d+1+%d+1+%d+1+%d+1+%d",_Size_wy, _Size_we, _Size_eol, 12*_Size_te, i*10, 10*_Size_TT);  //wartosci z ADC.
	        sprintf(&buf_rx2[_Size_BufWifi+itx],"for(i=0;i<10;i++) txt4[i]=txt[%s+i];if(txt4[9]=='1') ccol=\"d00\";if(txt4[9]=='0') ccol=\"000\";if(txt4[9]=='2') ccol=\"00d\";document.getElementById(\"tu_ptt%03d\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt4[0]+txt4[1]+txt4[2]+txt4[3]+txt4[4]+txt4[5]+txt4[6]+txt4[7]+txt4[8]+\" °C\";\x0D\x0A",sss,TabPort[j]);    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 }
     }
	  pocz=0;


		 _poziom20a:
		 dbg3("K20a "); kolej++;
		 for(j=(pocz+0+(odd-1));j<ddo;j++)  //Imp (Etr)
		 {
			 i= WyszukujPortElement(t_imp,_Size_imp,TabPort[j]);
		     if(i!=-1)
			 {
		    	    if((itx+450*_Imp_IleVal)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)+j); return itx; }
		    	 if(WhichPage==6)
			        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\nj=0;for(i=0;i<22;i++){ txt5[i]=txt[450+14+1+23*%d+4*18*%d+i]; if((txt5[i]=='0')&&(j==0))txt5[i]=' ';else j=1;}document.getElementById(\"tu_imp0_%03d\").innerHTML=txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4]+txt5[5]+txt5[6]+txt5[7]+txt5[8]+txt5[9]+txt5[10]+txt5[11]+txt5[12]+txt5[13]+txt5[14]+txt5[15]+\"<font color='#FFF' size='6'>\"+txt5[16]+txt5[17]+txt5[18]+txt5[19]+txt5[20]+\"</font>\";",i,i,TabPort[j]);
			     else
				    sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\nj=0;for(i=0;i<22;i++){ txt5[i]=txt[450+14+1+23*%d+4*18*%d+i]; if((txt5[i]=='0')&&(j==0))txt5[i]=' ';else j=1;}document.getElementById(\"tu_imp0_%03d\").innerHTML=txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4]+txt5[5]+txt5[6]+txt5[7]+txt5[8]+txt5[9]+txt5[10]+txt5[11]+txt5[12]+txt5[13]+txt5[14]+txt5[15]+txt5[16]+txt5[17]+txt5[18]+txt5[19]+txt5[20];",i,i,TabPort[j]);
			     itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			     if(WhichPage==6){
			        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\nj=0;for(i=0;i<17;i++){ txt5[i]=txt[450+14+1+23*%d+4*18*%d+18*0+23+i]; if((txt5[i]=='0')&&(j==0))txt5[i]=' ';else j=1;} if(txt5[15]=='1') ccol=\"f88\"; else ccol=\"eee\"; document.getElementById(\"tu_imp1_%03d\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4]+txt5[5]+txt5[6]+txt5[7]+txt5[8]+txt5[9]+txt5[10]+txt5[11]+txt5[12]+txt5[13]+\"</font>\";",i,i,TabPort[j]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\nj=0;for(i=0;i<17;i++){ txt5[i]=txt[450+14+1+23*%d+4*18*%d+18*1+23+i]; if((txt5[i]=='0')&&(j==0))txt5[i]=' ';else j=1;} if(txt5[15]=='1') ccol=\"f88\"; else ccol=\"eee\"; document.getElementById(\"tu_imp2_%03d\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4]+txt5[5]+txt5[6]+txt5[7]+txt5[8]+txt5[9]+txt5[10]+txt5[11]+txt5[12]+txt5[13]+\"</font>\";",i,i,TabPort[j]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\nj=0;for(i=0;i<17;i++){ txt5[i]=txt[450+14+1+23*%d+4*18*%d+18*2+23+i]; if((txt5[i]=='0')&&(j==0))txt5[i]=' ';else j=1;} if(txt5[15]=='1') ccol=\"f88\"; else ccol=\"eee\"; document.getElementById(\"tu_imp3_%03d\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4]+txt5[5]+txt5[6]+txt5[7]+txt5[8]+txt5[9]+txt5[10]+txt5[11]+txt5[12]+txt5[13]+\"</font>\";",i,i,TabPort[j]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			        sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\nj=0;for(i=0;i<17;i++){ txt5[i]=txt[450+14+1+23*%d+4*18*%d+18*3+23+i]; if((txt5[i]=='0')&&(j==0))txt5[i]=' ';else j=1;} if(txt5[15]=='1') ccol=\"f88\"; else ccol=\"eee\"; document.getElementById(\"tu_imp4_%03d\").innerHTML=\"<font color='#\"+ccol+\"'>\"+txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4]+txt5[5]+txt5[6]+txt5[7]+txt5[8]+txt5[9]+txt5[10]+txt5[11]+txt5[12]+txt5[13]+\"</font>\";",i,i,TabPort[j]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			     }
			 }
	     }
		 pocz=0;





         //sprintf(&buf_rx2[_Size_BufWifi+itx],"for(i=0;i<6;i++) txt5[i]=txt[25+13+1+18+1+216+1+20+1+140+2+12+7+i];document.getElementById(\"tu_imp018\").innerHTML=txt5[0]+txt5[1]+txt5[2]+txt5[3]+txt5[4];+txt5[5];");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	 _poziom21:
	 dbg3("K21 ");  kolej++;

if((wybor_bitowy&0x0200)>0)  //ESP
{
	 if((itx+164+436)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }

	 sprintf(&buf_rx2[_Size_BufWifi+itx],"\ 	
 }\x0D\x0A\
 }\x0D\x0A\
 xmlhttp.open(\"GET\",\"/TME.txt\", true);\x0D\x0A\
 xmlhttp.send();\x0D\x0A\
 }}\x0D\x0A\
 </script> \x0D\x0A\
 %s\x0D\x0A\
  ",HttpScriptMenu);

}
else
{
	 if((itx+164)>=_Size_WskFragmentPage){ *point=(100*(kolej-1)); return itx; }

	sprintf(&buf_rx2[_Size_BufWifi+itx],"\ 	
 }\x0D\x0A\
 }\x0D\x0A\
 xmlhttp.open(\"GET\",\"/TME.txt\", true);\x0D\x0A\
 xmlhttp.send();\x0D\x0A\
 }}\x0D\x0A\
 </script> \x0D\x0A\
  ");
}

 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


 sprintf(buf_p,"\r\nXXX: %d  %d ",itx, *point); dbg3(buf_p);

 return -1;
 //if(itx>4000) dbg3("\r\nPRZEKROCZONO itx !!!");  sprintf(buf_p,"\r\nItx: %d   %d  ",itx,WhichFragmentPage); dbg3(buf_p);



}


void HttpAntenaLora()
{
	int i,j,itx,kx,ap;  itx=0;
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<style>\r\n­iv { margin: 20px; }\r\n #tu_ant {\r\npadding: 15px 12px;\r\nmargin-top: 11px;\r\nborder-radius: 6px;\r\n\
		background:   url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABkAAAAcCAYAAACUJBTQAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAlwSFlzAAAOxAAADsQBlSsOGwAABsFJREFUSEvtVl2sVNUV/vb5n3POzL2DXFDEolRQxKqtQmpbQ6Vogj/EmCCmoQkvVvx56F+I7UsfeaFNSlJjK02QqkmLL6Y19aeNQdT6oKA4pEKsjbfQcJF75947d+bM+dl791tnuNHXvndy150ze6+91vrW+tbaRw1OPWAd5QDWhzIelAUcaCiUXKvAfxTuf0GscrnswCgu84BjqaPkjOhq7slvC+MoDLWB4xYKrq4oQ7hVDkeX1KcDEfOFb3k2Fc9rGDqvVAEDii1RMSBZM9SxRpwY6lrYokLiMwivaMKpXLheSOeyqQmAimC0bgRNdFlhUVqFyndRcjmj2Zy6mQQXBFCeyxWForIoyovIec41CWwmIN/9ri2cWXiRT5spytyiKjyUJkYZLMcnk9PofPgxPjo1ifNT8wwCGGtGGG9FWH3lBG77+vVYPtGgiwyRr1EM56GZkUYzBXIJeoFOjm23KmEq/ABTnxVI0+Vw3aV48aX38Jvn/47JczlmZys41A8CoskIn2mQKil4aI47WHNVjJ07N2DrnV/l2QGG+TQznWM8ipn+AZ0c32FtajHVLXHZyhvx0p9P4Bf7XsPp0z1UqYdhxUS4hE4HaeLCdxV0YaAZZTnPvHM/afkYzpe4+sYWnvjZd7Dh1hWI3RyYXUBU5CMnXTIjTFbgt0+/hV8/dQIDHgjGeHCuxM23XYpN316PdeuWYemyCHEjRNY3mLlQ4IP3zjKYKbz8pxM1r7y2D03Gff+xa7B712Y0h9NISkHywYPWtCewd9/LeGr/v+CMkXZDi1tuaWLvnq1YdXkD40tZQLKvn7EmYs5xkFdMmMt1m+DkP85h3/5X8O77Ug9mPlbYtfMm7HnoK2jqWVKcKLK5DG8dOY8oVTDM+U+f+CYOHXgc69YEiNwZZLNTjEYjjZcg9McRuDHarQTNgOQdnsHNN4zj8DMP4yePrkfkMQzaOPLqR2Qr00lRCye2WU+neOPNU3jymX/iwR2r8MD2OzAzPcUmyhGQcZ9MZjjeOY93jk1iQDq30wauvnIJtnxjLZaOGbQYeT7sIRqL8crrHex/8jR2fW8ttm25CqEdQvU622xcNIAwIHzDHmJRS82+CDC94OLAoaM4/OJZzMyxf0aNDsuapolC03Ow/e7LseeH9yEb/AftCYVBPiTado2iyubYbSROr3OvDfseFrICzYkmev0+WmmMf59TeOjHf0HnZB+GlJWPz1wYoT5FPiwNFPe+fG0TB5/ejbHWZ2Rfv15jPEwrFZWhk5P32LgkEk3PHAFKKMp5s28/++TZs0xDRWo3cP+2Dbhj89d4aIBS5zjy9nE8/8fTJImui33f3Zdi78/vRb/7KSbYrGwWZobzzWVfCVsqQgT7wPUUFLuO5cdNN4xBcW3jpkvwh+d24PFHrsH6tV3cuhG4fl0Pux++Fi8cvger1yT17Lx90xVI4hITaQj0B0DFBpfxQPtqoXOX9XsOvFaDUQ1QMKwkCsn3AGe6Bu1lbRTZNC7hftG36HbnsfK6FejPzkK7LrKMA3Posfkc+MUAfp7B80jzmDXmLNNkG51stf6chdcIkLNIUeKzJyzTRBLAZyEcIjK4cKHEoYMdvHFkGhs3jhHJFsTpgB0/QDtJYPolRw87kTVAQCfsdEmXrdNFFnhS0IJdToNlUWB+ro+QtWkwooJE8Dhlf/dchxPhDD7+NMPvnz2HAwePopWMU4eN2Z2jIUmPEEQz/RmMoHFkDtTZ5ANp67AeMofkHkhjnwziuOefUj4y4+DNY3P1iK+SGEPy5Oj7F9CnusMAwkjGJX+4JEFI+nMAD0ODSqaqNKN+53biIUyBWjsWyCIj5xy06JGzXrQMP/jR6/jbX7vYvHUJfvXLzbD9GYS8yFy5Cesak7hixmN0RCG9knkekdR3qGjIRTW6NrVIDVScOYyBWa0MGoGDtOkgDl3kg4JBisXFACUpn4vEJyhkrPC0OBnZE++GoglTRFBo3pLyCqDl+iXCvMc9EiNu8H2AwdHM6GydhtGzAFs0ygzSiaRlUYeohCEuayBSp5k5l6hch0zj6ZBIXJJikPG+l+O11cUoR8+S7UWjmhn8P5L/sSaf12hUNNbIspGsxztc3r3YsQmv4qChsXJVhMtWR7jiSxHzznnncKQIQUSoJ1eFvORZ+S2fEZM5hd/+lo0duSikWheZMnoVucgYeeZkFi5pjhz5SeMOx0fdH7UsqgrlF8/yu6BwhDk554uWJ76mMny2DB0adjwnmxjVJuA7mNwj3JOJWk8I4bQ7WqNufVbmHMXQhqaU5P8wdNBlg/4X8S+jJn4L8/oAAAAASUVORK5CYII=');\
        }\r\n</style>\r\n</html>");

	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

}


void HttpTabelaTranslacji(int nr, int ktr)
{
	int i,j,itx,kx,ap;  itx=0;

  if((nr%10)==0) ap=0; else ap=5;
  for(i=0;i<5;i++)
  {
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<font class=\"css_list\">%02d.%d</font>&nbsp;&nbsp;\x0D\x0A\
<input type=\"text\" class=\"textbox\" style=\"width: 15px\" style=\"font-size: 35px\" name=\"n_s_ttp%d_%03d\" size=\"1\" maxlength=\"1\" value=\" \">\x0D\x0A\
<input type=\"text\" class=\"textbox\" style=\"width: 45px\" name=\"n_s_ttd%d_%03d\" size=\"4\" maxlength=\"4\" value=\"    \">\x0D\x0A\
<font class=\"css_list\" style=\"padding: 0px;color:#e0e0e0;\" ><b>.</b></font>\x0D\x0A\
<input type=\"text\" class=\"textbox\" style=\"width: 25px\" name=\"n_s_ttm%d_%03d\" size=\"2\" maxlength=\"2\" value=\"  \">&nbsp;&nbsp;\x0D\x0A\
<font class=\"css_list\" style=\"padding: 0px;color:#e0e0e0;\" >%s</font><br>\x0D\x0A",nr/10,ap+i, ktr,nr+i, ktr,nr+i, ktr,nr+i, Const.s_TT[ktr].jm);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  }

  if(nr==(19*5)){
    sprintf(&buf_rx2[_Size_BufWifi+itx],"</div></div>");
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  }
  else{
	 if(nr>0){  if((nr%10)!=0){
	    sprintf(&buf_rx2[_Size_BufWifi+itx],"<br>");
	    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	 }}
  }

}
/*

for(c=0;c<MAX_SIGOUT;c++)
{
for(j=0;j<MAX_INITIAL_LOGIC_GATES;j++)
{
for(i=0;i<MAX_INPUT_INITIAL_LOGIC_GATE;i++)
{
	if(Const.Logic[c].InitGate[j].NameInputSignal[i]==nr_portu sie zgadza)     Var.Logic[c].InitGate[j].binaryInputSignal[nrGate]|= (1<<i);
}
}Var.Logic[c].IGate[j].binaryInputSignal
}
 */

void HttpLogicEntry(void)
{
	int i,j,c,itx,top, lg; char ttext[10];  itx=0;    lg=Const.OutNameLogic[0].name;   if(lg>=MAX_SIGOUT) dbg3("\r\nUWAGAA!!!!");


	sprintf(&buf_rx2[_Size_BufWifi+itx],"<script>\r\nvar txtB=[];var txtC=[];var txtD=[];var txtE=[];var txt=[];var ZdAkt=[];\r\nvar txtA=\"<select name=\\\"n_s_SWX \\\" ><option value='0'          >--- --- --- </option>");
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	for(i=0;i<_Size_port;i++) //nazwy sygnalow wejsciowych
	{	                        //DS                     //DHT                      //Pt                      //We                      //EOL                      //Li                       //Ti
		if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==9)||(Const.s_GPIO[i].val==3)||(Const.s_GPIO[i].val==4)||(Const.s_GPIO[i].val==13)||(Const.s_GPIO[i].val==7)||(Const.s_GPIO[i].val==10))
		{
			if((Const.s_GPIO[i].val==9))
			{
				 sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value='%d\'          >T: %s</option>",i+1, Const.s_GPIO[i].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
				 sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value='%d\'          >W: %s</option>",i+1+60, Const.s_GPIO[i].nazwa);
			}
			else sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value='%d\'          >%s</option>",i+1, Const.s_GPIO[i].nazwa);

			itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			if(itx>_Size_WskFragmentPage){ dbg3(Http_PRZEKROCZONO); buf_rx2[_Size_buf_rx2-1]=0; return; }
		}
	}

	sprintf(&buf_rx2[_Size_BufWifi+itx],"</select>\"\r\nvar txtWy=\"<select name=\\\"n_s_SigWy\\\" onchange='this.form.submit()' >");
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	for(i=20;i<_Size__s_gpio;i++)
	{
		if(Const.OutNameLogic[0].name==i-20) sprintf(ttext,"selected");
		else                                sprintf(ttext,"        ");
		                        //PK                          IR                       PWM
		if((Const.s_GPIO[i].val==0)||(Const.s_GPIO[i].val==1)||(Const.s_GPIO[i].val==8))
		{
			sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value='%d' %s >%s</option>",i-20,ttext,Const.s_GPIO[i].nazwa);
			itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			if(itx>_Size_WskFragmentPage){ dbg3(Http_PRZEKROCZONO); buf_rx2[_Size_buf_rx2-1]=0; return; }
		}
	}


	sprintf(&buf_rx2[_Size_BufWifi+itx],"</select>\"\r\nvar ZdAkt=\" \"\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);//WYYYY

	sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelLog1=[%d,%d,%d,%d];\r\n",Const.Logic[lg].InitGate[0].logicalOperation\
																	   	   ,Const.Logic[lg].InitGate[1].logicalOperation\
																		   ,Const.Logic[lg].InitGate[2].logicalOperation\
																		   ,Const.Logic[lg].InitGate[3].logicalOperation);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //bramki NAND AND XOR  I sekcji

	sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelLog2=[%d,%d];\r\n",Const.Logic[lg].AddGate[0].logicalOperation\
																	 ,Const.Logic[lg].AddGate[1].logicalOperation);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //bramki NAND AND XOR  II sekcji

	sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelLog3=[%d];\r\n",Const.Logic[lg].FinGate[0].logicalOperation);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //bramki NAND AND XOR  III sekcji

	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelLog1=[0,2,3,4];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //bramki NAND AND XOR  I sekcji
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelLog2=[3,0];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);		//bramki NAND AND XOR  II sekcji
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelLog3=[2];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);			//bramki NAND AND XOR  III sekcji

	for(i=0;i<MAX_INITIAL_LOGIC_GATES;i++)
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSel%c=[%d,%d,%d,%d,%d,%d,%d,%d];  ",'A'+i, Const.Logic[lg].InitGate[i].NameInputSignal[0]\
																								,Const.Logic[lg].InitGate[i].NameInputSignal[1]\
																								,Const.Logic[lg].InitGate[i].NameInputSignal[2]\
																								,Const.Logic[lg].InitGate[i].NameInputSignal[3]\
																								,Const.Logic[lg].InitGate[i].NameInputSignal[4]\
																								,Const.Logic[lg].InitGate[i].NameInputSignal[5]\
																								,Const.Logic[lg].InitGate[i].NameInputSignal[6]\
																								,Const.Logic[lg].InitGate[i].NameInputSignal[7]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   //sygnaly wejsciowe operatora A

		    sprintf(&buf_rx2[_Size_BufWifi+itx],"var Bin%c=[%d,%d,%d,%d,%d,%d,%d,%d]; var Out%c=%d\r\n",'A'+i, 0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>0)\
		    																					              ,0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>1)\
																											  ,0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>2)\
																											  ,0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>3)\
																											  ,0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>4)\
																											  ,0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>5)\
																											  ,0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>6)\
																											  ,0x01&(Var.Logic[lg].IGate[i].binaryInputSignal>>7),'A'+i, 0x01&(Var.Logic[lg].IGate[i].binaryOutputSignal));  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

		sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelI%c=[%d,%d,%d,%d,%d,%d,%d,%d];\r\n",'A'+i, Const.Logic[lg].InitGate[i].inputBuffer[0]\
																								 ,Const.Logic[lg].InitGate[i].inputBuffer[1]\
																								 ,Const.Logic[lg].InitGate[i].inputBuffer[2]\
																								 ,Const.Logic[lg].InitGate[i].inputBuffer[3]\
																								 ,Const.Logic[lg].InitGate[i].inputBuffer[4]\
																								 ,Const.Logic[lg].InitGate[i].inputBuffer[5]\
																								 ,Const.Logic[lg].InitGate[i].inputBuffer[6]\
																								 ,Const.Logic[lg].InitGate[i].inputBuffer[7]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   //sygnaly wejsciowe operatora A
	}


	for(i=0;i<MAX_ADDITIONAL_LOGIC_GATES;i++)
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSel%c=[%d,%d,%d,%d];\r\n",'E'+i,  Const.Logic[lg].AddGate[i].NameInputSignal[0]\
																					 ,Const.Logic[lg].AddGate[i].NameInputSignal[1]\
																					 ,Const.Logic[lg].AddGate[i].NameInputSignal[2]\
																					 ,Const.Logic[lg].AddGate[i].NameInputSignal[3]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   //sygnaly wejsciowe operatora A

		sprintf(&buf_rx2[_Size_BufWifi+itx],"var Bin%c=[%d,%d,%d,%d]; var Out%c=%d\r\n",'E'+i,  0x01&(Var.Logic[lg].AGate[i].binaryInputSignal>>0)\
	    																		               ,0x01&(Var.Logic[lg].AGate[i].binaryInputSignal>>1)\
	    																		               ,0x01&(Var.Logic[lg].AGate[i].binaryInputSignal>>2)\
	    																		               ,0x01&(Var.Logic[lg].AGate[i].binaryInputSignal>>3),'E'+i, 0x01&(Var.Logic[lg].AGate[i].binaryOutputSignal));  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

		sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelI%c=[%d,%d,%d,%d];\r\n",'E'+i, Const.Logic[lg].AddGate[i].inputBuffer[0]\
																					 ,Const.Logic[lg].AddGate[i].inputBuffer[1]\
																					 ,Const.Logic[lg].AddGate[i].inputBuffer[2]\
																					 ,Const.Logic[lg].AddGate[i].inputBuffer[3]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   //sygnaly wejsciowe operatora A
	}


	for(i=0;i<MAX_FINAL_LOGIC_GATES;i++)
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSel%c=[%d,%d];\r\n",'G'+i,  Const.Logic[lg].FinGate[i].NameInputSignal[0]\
																			   ,Const.Logic[lg].FinGate[i].NameInputSignal[1]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   //sygnaly wejsciowe operatora A

		sprintf(&buf_rx2[_Size_BufWifi+itx],"var Bin%c=[%d,%d]; var Out%c=%d\r\n",'G'+i,  0x01&(Var.Logic[lg].FGate[i].binaryInputSignal>>0)\
	    																                 ,0x01&(Var.Logic[lg].FGate[i].binaryInputSignal>>1),'G'+i, 0x01&(Var.Logic[lg].FGate[i].binaryOutputSignal));  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

		sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelI%c=[%d,%d];\r\n",'G'+i, Const.Logic[lg].FinGate[i].inputBuffer[0]\
																			   ,Const.Logic[lg].FinGate[i].inputBuffer[1]);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   //sygnaly wejsciowe operatora A
	}
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelIA=[1,2,0,1,0,0,2,1];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   //sygnaly invert operatora A
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelIB=[1,2,0,1,0,0,0,1];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //sygnaly invert operatora B
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelIC=[1,1,1,1,1,1,2,1];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //sygnaly invert operatora C
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelID=[0,2,0,1,0,0,0,1];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //sygnaly invert operatora D

	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelE=[0,0,4,4];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);    //sygnaly posr opeartora E
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelIE=[0,2,1,1];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //sygnaly invert opeartora E

	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelF=[4,3,2,0];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);    //sygnaly posr opeartora F
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelIF=[0,0,0,1];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //sygnaly invert opeartora F

	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelG=[0,2];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);    //sygnaly posr opeartora G
	//sprintf(&buf_rx2[_Size_BufWifi+itx],"var tabSelIG=[1,0];\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  //sygnaly invert opeartora G

	if(itx>_Size_WskFragmentPage){ dbg3(Http_PRZEKROCZONO); buf_rx2[_Size_buf_rx2-1]=0; return; }


}

void HttpRst(int ktr)
{
	int i,j,itx,kx,ap;  itx=0;

	if(ktr==1){
		 sprintf(&buf_p[0],"rstDPT");
		 sprintf(&buf_p[10],"rstDPN");
		 sprintf(&buf_p[100],"dane pomiarowe");
	}
	else if(ktr==2){
		 sprintf(&buf_p[0],"rstRZT");
		 sprintf(&buf_p[10],"rstRZN");
		 sprintf(&buf_p[100],"rejestr zdarzen");
	}

    sprintf(&buf_rx2[_Size_BufWifi+itx],"\
<!DOCTYPE html PUBLIC \"-//WAPFORUM//DTD XHTML Mobile 1.2//EN\"\"http://www.openmobilealliance.org/tech/DTD/xhtml-mobile12.dtd\">\r\n\
<html>\r\n\
<head>\r\n\
<meta name=\"handheldFriendly\" content=\"true\" />\r\n\
<link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAIAAACQkWg2AAAABGdBTUEAALGPC/xhBQAAACRJREFUOE9jZGj4z0AaAGogCZGmGuQckowf1UBkcI0GKxEBBQA55GigBl4jdAAAAABJRU5ErkJggg==\">\r\n\
   <style>.c{text-decoration:none;font-size: 19px;background-color: #c00333;border-radius: 10px;padding: 5px 13px 5px 13px;}</style>\r\n\
   <style>.b{text-decoration:none;font-size: 19px;background-color: #2220db;border-radius: 10px;padding: 5px 13px 5px 13px;}</style>\r\n\
<style>.n{border-width: 0px;color: #202020;font-size: 21px;background-color: #e0e0e0;border-radius: 10px;padding: 5px 13px 5px 13px;}</style>\r\n\
  <style>.x{border-radius: 30px;float:center;width: 520px;height: 220px;padding: 1px;border: solid 4px #808080;background-color: #505050;text-align: center;overflow: auto;font-size: 40px;}</style>\r\n\
 <style>.y{border-radius: 0px;float:center;width: 550px;height: 250px;text-align: center;overflow: auto;}</style>\r\n\
</head>\r\n\
<body bgcolor=\"000000\">\r\n\
<br><br>\r\n\
<center>\r\n\
<div class=\"y\"><div class=\"x\">\r\n\
<br><button class=\"n\"> Czy na pewno wymazac %s ? </button><br><br>\r\n\
<a href=\"lpc.cgi/%s\" rel=\"external\"  class=\"c\" style=\"font-size: 25px;\"><b><font color=\"#fff\"> T A K </font></a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\r\n\
<a href=\"lpc.cgi/%s\" rel=\"external\"  class=\"b\" style=\"font-size: 25px;\"><b><font color=\"#fff\"> N I E </font></a>\r\n\
</div></div>\r\n\
</center>\r\n\
</body>\r\n\
 </html>",&buf_p[100],&buf_p[0],&buf_p[10]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


}

void Http_TT_zapisz(int ktr)
{
	int i,j,itx,kx,ap;  itx=0;

    snprintf(&buf_rx2[_Size_BufWifi+itx],117,Http_zapisz);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

    sprintf(&buf_rx2[_Size_BufWifi+itx],"<font class=\"css_list\">Jednostka miary</font>&nbsp;&nbsp;\x0D\x0A<input type=\"text\" class=\"textbox\" style=\"width: 75px\" name=\"n_s_ttj%d_000\" size=\"5\" maxlength=\"5\" value=\"     \">\x0D\x0A<br><br>\x0D\x0A",ktr);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

    sprintf(&buf_rx2[_Size_BufWifi+itx],&Http_zapisz[117]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}

void HttpWyborWykres()
{
	int i,j,itx,kx,k;  itx=0;  char *buf[5],*ptr,*ptr1,*ptr2,*ptr3;   uint32_t addr;


itx=0;
for(i=0;i<(401);i++) buf_rx2[_Size_BufWifi+i]= HttpStyle[i+75];    itx=i;

sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>.x{border-radius: 30px;float:left;width: 370px;height: 220px;padding: 1px;border: solid 4px #808080;background-color: #eeeeee;text-align: center;overflow: auto;font-size: 40px;}</style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>.y{border-radius: 0px;float:left;width: 400px;height: 250px;text-align: center;overflow: auto;}</style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

if(ptr=strstr(HttpStyle,".css_button {"))
{
	if(ptr1=strstr(ptr,"</style>"))
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		snprintf(&buf_rx2[_Size_BufWifi+itx], ptr1-ptr, ptr); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		sprintf(&buf_rx2[_Size_BufWifi+itx],"</style>\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}
}
i=IloscCzujnikow();
if(adr_flash_start==0) addr=0;
else                   addr= adr_flash_start-(i+4);
AT25SF_ReadArray(4096+addr, buf_p,  4 );  //4 bo czas zapisywany jest w 4-rech bajtach
time=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
if(adr_flash>0){
  ZamianaCzasuZFlash(time);
  j=podaj_dzien(rok,mies,dzien);
       if(j==1) sprintf(&buf_p[90],"Po");
  else if(j==2) sprintf(&buf_p[90],"Wt");
  else if(j==3) sprintf(&buf_p[90],"Sr");
  else if(j==4) sprintf(&buf_p[90],"Cz");
  else if(j==5) sprintf(&buf_p[90],"Pi");
  else if(j==6) sprintf(&buf_p[90],"So");
  else if(j==7) sprintf(&buf_p[90],"Ni");
  else          sprintf(&buf_p[90],"--");
}
else{  rok=0; mies=0; dzien=0; godz=0, min=0; sec=0;   sprintf(&buf_p[90],"--");  }

sprintf(&buf_p[0],"+++");
sprintf(&buf_p[10],"---");

sprintf(&buf_rx2[_Size_BufWifi+itx],"<title>Wykres</title></head>\x0D\x0A\
 <body bgcolor=\"bbbbbb\"><br><div class=\"y\"><div class=\"x\"><font size=\"5\" color=\"black\"> Rozmiar zapisanych danych</font><br><font size=\"4\" color=\"black\"> z czujnikow temp i wilgot</font><br><font size=\"5\" color=\"blue\"> %d B</font><br><a href=\"rej\" return false><button type=\"button\" class=\"css_button\">Wroc</button></a></div></div>\r\n<div class=\"y\"><div class=\"x\"><font size=\"5\" color=\"black\"> Poczatek pomiarow </font><br><font size=\"5\" color=\"blue\"> 20%02d. %02d. %02d &nbsp;&nbsp;&nbsp; %02d:%02d:%02d &nbsp; %s </font><br>\
 ",adr_flash,rok,mies,dzien,godz,min,sec,&buf_p[90]);
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
k=0;
for(i=0;i<6;i++)
{
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykrprzedz%d\"; return false><button type=\"button\">%s</button></a> ",i+1,&buf_p[k+i]);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	if(i==2){ k=7; sprintf(&buf_rx2[_Size_BufWifi+itx],"&nbsp;&nbsp;&nbsp;");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]); }
}

i=IloscCzujnikow();
if(adr_flash_end==0) addr=0;
else                 addr=adr_flash_end-(i+4);
AT25SF_ReadArray(4096+addr, &buf_p[20],  4 );    //4 bo czas zapisywany jest w 4-rech bajtach
time=   0xff000000&(((uint32_t)buf_p[20])<<24) | 0x00ff0000&(((uint32_t)buf_p[21])<<16) | 0x0000ff00&(((uint32_t)buf_p[22])<<8) | 0x000000ff&((uint32_t)buf_p[23]);
if(adr_flash>0){
  ZamianaCzasuZFlash(time);
  j=podaj_dzien(rok,mies,dzien);
       if(j==1) sprintf(&buf_p[90],"Po");
  else if(j==2) sprintf(&buf_p[90],"Wt");
  else if(j==3) sprintf(&buf_p[90],"Sr");
  else if(j==4) sprintf(&buf_p[90],"Cz");
  else if(j==5) sprintf(&buf_p[90],"Pi");
  else if(j==6) sprintf(&buf_p[90],"So");
  else if(j==7) sprintf(&buf_p[90],"Ni");
  else          sprintf(&buf_p[90],"--");
}
else{  rok=0; mies=0; dzien=0; godz=0, min=0; sec=0;   sprintf(&buf_p[90],"--");  }

sprintf(&buf_rx2[_Size_BufWifi+itx],"<br><font size=\"4\" color=\"black\">Adres Poczatku pomiarow &nbsp; %d B</font></div></div>\r\n<div class=\"y\"><div class=\"x\"><font size=\"5\" color=\"black\">Koniec pomiarow </font><br><font size=\"5\" color=\"blue\"> 20%02d .%02d .%02d &nbsp;&nbsp;&nbsp; %02d:%02d:%02d &nbsp; %s </font><br>\
 ",adr_flash_start,rok,mies,dzien,godz,min,sec,&buf_p[90]);
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
k=0;
for(i=0;i<6;i++)
{
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykrprzedz%x\"; return false><button type=\"button\">%s</button></a> ",i+6+1,&buf_p[k+i]);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	if(i==2){ k=7; sprintf(&buf_rx2[_Size_BufWifi+itx],"&nbsp;&nbsp;&nbsp;");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);}
}

sprintf(&buf_rx2[_Size_BufWifi+itx],"<br><font size=\"4\" color=\"black\">Adres Konca pomiarow &nbsp; %d B</font></div></div>\r\n",adr_flash_end);
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);



//sprintf(buf_p,"\r\nHttpWyborWykres   %d  ",itx);  dbg3(buf_p);
if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWyborWykres itx !!!");
}



void HttpWykresStart_0()
{
	int i,j,itx,kx;  itx=0;   char *ptr,*ptr1,*ptr2,*ptr3;    buf_rx2[_Size_BufWifi]=0;

 itx=0;
 for(i=0;i<(401);i++) buf_rx2[_Size_BufWifi+i]= HttpStyle[i+75];    itx=i;

 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>.x{border-radius: 30px;float:left;width: 420px;height: 570px;padding: 1px;border: solid 4px #a0a0a0;background-color: #eeeeee;text-align: center;overflow: auto;font-size: 40px;}</style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>.y{border-radius: 0px;left:center;width: 450px;height: 600px;text-align: center;overflow: auto;}</style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(ptr=strstr(HttpStyle,".css_button {"))
 {
	if(ptr1=strstr(ptr,"</style>"))
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		snprintf(&buf_rx2[_Size_BufWifi+itx], ptr1-ptr, ptr); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		sprintf(&buf_rx2[_Size_BufWifi+itx],"</style>\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}
 }

 if(ptr=strstr(HttpStyle,"#mainselection select"))
 {
	if(ptr1=strstr(ptr,"</style>"))
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		snprintf(&buf_rx2[_Size_BufWifi+itx], ptr1-ptr, ptr); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		sprintf(&buf_rx2[_Size_BufWifi+itx],"</style>\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}
 }

 if(ptr=strstr(HttpStyle,".textboxB {"))
 {
	if(ptr1=strstr(ptr,"</style>"))
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<style>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		snprintf(&buf_rx2[_Size_BufWifi+itx], ptr1-ptr, ptr); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		sprintf(&buf_rx2[_Size_BufWifi+itx],"</style>\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}
 }

 sprintf(&buf_rx2[_Size_BufWifi+itx],"<title>Wykres</title></head>\x0D\x0A\
  <body onload=\"init()\"><div align=\"center\"><h2><a href=\"pomiar\" return false><button type=\"button\"><--</button></a> &nbsp; Archiwizacja Danych Pomiarowych</h2><font size=\"4\" color=\"blue\">%s</font><br><br><div id=\"tu_menu\"><center><font size=\"5\" color=\"blue\">Wgrywam dane ... prosze czekac</font></center></div><canvas id=\"can\" height=\"400\"\x0D\x0A\
  width=\"%d\">\x0D\x0A\
  </canvas></div>",Const.s_Czujki[Var.wykres_nr].nazwa, Const.s_Wykr[Var.wykres_nr].szerwykr);
 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 sprintf(&buf_rx2[_Size_BufWifi+itx],"<br><br><font size=\"4\" color=\"blue\">Wybor czujnika:</font> &nbsp;");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStart_0 itx !!!");

}

void HttpWykresStart_1()  //Tu MAX 40 wartosci temp  -  jesli wiecej to trzeba w¹tek   ten 'HttpWykresStart_1'  podzielic na kilka pakietów !!!!!!!
{
	int i,j,itx,kx;  itx=0;   char *ptr,*ptr1,*ptr2,*ptr3;    buf_rx2[_Size_BufWifi]=0;

  for(i=0;i<_Size__s_gpio;i++)
  {
	       j= WyszukajNrCzujkiDlaPortu(i+1);
		   if(j!=-1)
		   {
	    	  if(Const.s_GPIO[i].val==9)
	    	  {
	    		 sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykres1_%02d\"; return false><button type=\"button\">%s</button></a> &nbsp;",j,Const.s_Czujki[j].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	    	     j= WyszukajNrCzujkiDlaPortu_2(i+1);
	    	     if(j!=-1)
	    	     {
	    	    	 sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykres1_%02d\"; return false><button type=\"button\">%s</button></a> &nbsp;",j,Const.s_Czujki[j].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	    	     }
	    	  }
		      else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))
		      {
		    	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykres1_%02d\"; return false><button type=\"button\">%s</button></a> &nbsp;",j,Const.s_Czujki[j].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		      }
		   }
  }

  if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStart_1 itx !!!");
}



void HttpWykresStart_2()
{
	int i,j,itx,kx;  itx=0;   char *ptr,*ptr1,*ptr2,*ptr3;    buf_rx2[_Size_BufWifi]=0;

sprintf(&buf_rx2[_Size_BufWifi+itx],"<br><br><br><br><br><br><div class=\"y\"><div class=\"x\"><br><form method=\"get\" action=\"lpc.cgi/A117\" data-ajax=\"false\"><font size=\"5\" color=\"black\">Przedzial wyswietlanych wartosci </font><div id=\"mainselection\"><select style=\"background: #909090;\" name=\"n_s_Wykr_prgmax_0%02d\" >",Var.wykres_nr+1);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
for(i=0;i<11;i++)
{
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value=\"%d\"          >%d</option>",(1100-i*100),(1100-i*100));
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}
for(i=0;i<9;i++)
{
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value=\"%d\"          >%d</option>",(90-i*10),(90-i*10));
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}
sprintf(&buf_rx2[_Size_BufWifi+itx],"</select> &nbsp;&nbsp; <select style=\"background: #909090;\" name=\"n_s_Wykr_prgmin_0%02d\" >",Var.wykres_nr+1);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

    sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value=\"00\"          >-00</option>");
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
for(i=0;i<20;i++)
{
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value=\"%d\"          >-%d</option>",((i+1)*10),((i+1)*10));
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}

sprintf(&buf_rx2[_Size_BufWifi+itx],"</select></div><br><font size=\"5\" color=\"black\">Ilosc probek na jednostke czasu</font><br> <input class=\"textboxB\" style=\"width:40px; background-color:#909090;\" type=\"text\" name=\"n_s_Wykr_zaw_0%02d\" size=\"3\" maxlength=\"3\"   value=\"%d\"      > <font size=\"4\" color=\"black\">(1-500)</font><br><br>",Var.wykres_nr+1,Const.s_Wykr[Var.wykres_nr].zawezen);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
sprintf(&buf_rx2[_Size_BufWifi+itx],"<font size=\"5\" color=\"black\">Szerokosc Wykresu</font><br> <input class=\"textboxB\" style=\"width:60px; background-color:#909090;\" type=\"text\" name=\"n_s_Wykr_sze_0%02d\" size=\"5\" maxlength=\"5\"   value=\"%d\"      > <font size=\"4\" color=\"black\">(1000 - 30000)</font><br><br>",Var.wykres_nr+1,Const.s_Wykr[Var.wykres_nr].szerwykr);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

sprintf(&buf_rx2[_Size_BufWifi+itx],"<button  class=\"css_button\" type=\"submit\" name=\"n_109_zapisz\">Ustaw</button></form></font></div></div>");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStart_2 itx !!!");
//sprintf(buf_p,"\r\nHttpWykresStart_2   %d  ",itx);  dbg3(buf_p);
//sprintf(buf_p,"\r\nHttpWykresStart_1:   Var.wykres_nr: %d  %d    ---%d ",Var.wykres_nr, Const.s_Wykr[Var.wykres_nr].zawezen,itx);  dbg3(buf_p);

}

void HttpWykresStart_3()
{
	int i,j,itx,kx,p,cc;  itx=0;  char bug1[7],bug2[7],a;    buf_rx2[_Size_BufWifi]=0;

     p= Const.s_Czujki[Var.wykres_nr].addr_DS[8];
     if(Const.s_GPIO[p-1].val==3)  //Pt1000
     {
    	  wartTemp_odwrotnie_Pt(bug1,Const.s_Czujki[Var.wykres_nr].prog_max1);
    	  wartTemp_odwrotnie_Pt(bug2,Const.s_Czujki[Var.wykres_nr].prog_min1);
     }
     else
     {
    	  wartTemp_odwrotnie(bug1,Const.s_Czujki[Var.wykres_nr].prog_max1);
    	  wartTemp_odwrotnie(bug2,Const.s_Czujki[Var.wykres_nr].prog_min1);
     }
     if((bug1[1]=='0')&&((bug1[2]>0x2F)&&(bug1[2]<0x3A))) bug1[1]=' ';
     if((bug2[1]=='0')&&((bug2[2]>0x2F)&&(bug2[2]<0x3A))) bug2[1]=' ';

// if(Const.s_Wykr[Var.wykres_nr].prgmax>100) cc=60; else cc=20;
 cc=(Const.s_Wykr[Var.wykres_nr].prgmax+Const.s_Wykr[Var.wykres_nr].prgmin)/10;
 if(cc<10);
 else
 {
	 ergtythyht3:
	 if((cc%10)!=0){ cc--; goto ergtythyht3; }
 }
   sprintf(&buf_rx2[_Size_BufWifi+itx],"</body><script>var i,j,k,k1,k2,can,ctx,przes1,przes2,przes3,krtn, ilosc_pomiarow,rozciag_zawez,Const_width,Samples,minVal,maxVal,xScalar, yScalar,y,const_y;var prg_max, prg_min;\x0D\x0A\			
 Const_width=4000;Samples = 144;\x0D\x0A\
 prg_max =%s;\x0D\x0A\
 prg_min =%s;\x0D\x0A\
 ilosc_pomiarow = 144*3000;\x0D\x0A\
 rozciag_zawez = %d;\x0D\x0A\
 function Round(n,k){var factor=Math.pow(10,k);return Math.round(n*factor)/factor;}\x0D\x0A\
 function init(){maxVal = %d;minVal= -%d;var stepSize = %d;var colHead = 50;var rowHead = 60;var margin = 10;var header = \"[°C]\";\x0D\x0A\
 can = document.getElementById(\"can\");\x0D\x0A\
 ctx = can.getContext(\"2d\");\x0D\x0A\
 ctx.fillStyle = \"black\"\x0D\x0A\
 yScalar = (can.height - colHead - margin) / (maxVal-minVal);\x0D\x0A\
 xScalar = (Const_width - rowHead) / (rozciag_zawez * Samples + 1);\x0D\x0A\
 ctx.strokeStyle = \"rgba(128,128,255, 0.5)\";\x0D\x0A\
 ctx.beginPath();\x0D\x0A\
 ctx.font = \"14pt Helvetica\"\x0D\x0A\
 ctx.fillText(header, 0, colHead - margin);\x0D\x0A\
 ctx.font = \"12pt Helvetica\"\x0D\x0A\
 var count =  0;\x0D\x0A\
 for (scale = maxVal; scale >= minVal; scale -= stepSize) {\x0D\x0A\
 y = colHead + (yScalar * count * stepSize);\x0D\x0A\
 ctx.fillText(Round(scale, 4), margin,y + margin);\x0D\x0A\
 ctx.moveTo(rowHead, y);\x0D\x0A\
 ctx.lineTo(can.width, y);\x0D\x0A\
 count++;\x0D\x0A\
 }\x0D\x0A\ 
 var count =  0;\x0D\x0A\
 for (scale = maxVal; scale >= minVal; scale -= 1) {\x0D\x0A\
 y = colHead + (yScalar * count * 1);\x0D\x0A\
 if(scale==0){ \x0D\x0A\
 ctx.moveTo(rowHead, y);\x0D\x0A\
 ctx.lineTo(can.width, y);\x0D\x0A\
 ctx.moveTo(rowHead, y);\x0D\x0A\
 ctx.lineTo(can.width, y);\x0D\x0A\
 }\x0D\x0A\
 count++;\x0D\x0A\
 }\x0D\x0A\
 ctx.font = \"9pt Helvetica\"\x0D\x0A\
 j=0;k=0;\x0D\x0A\
 przes1=2;\x0D\x0A\
 for (i = przes1; i <= ilosc_pomiarow+przes1; i++) { \x0D\x0A\  
 var x = i * ((Const_width - rowHead) / (Samples + 1));\x0D\x0A\
 if((i-przes1)==(2*parseInt(k/2)))\x0D\x0A\
 {\x0D\x0A\
 if(dataValue[rozciag_zawez*6*j]!=undefined){\x0D\x0A\
 ctx.fillText(dataValue[rozciag_zawez*6*j], x, colHead - margin);\x0D\x0A\
 ctx.fillText(dataValue[rozciag_zawez*6*j+1], x, colHead - margin -20);}\x0D\x0A\
 j++;\x0D\x0A\
 }\x0D\x0A\
 k++;\x0D\x0A\
 }\x0D\x0A\
 for (i = przes1; i <= 8*144+przes1; i++) {\x0D\x0A\
 var x = i * ((Const_width - rowHead) / (Samples + 1));\x0D\x0A\
 ctx.moveTo(x, colHead);\x0D\x0A\
 ctx.lineTo(x, can.height - margin);\x0D\x0A\
 }\x0D\x0A\
 ctx.stroke();\x0D\x0A\
 ctx.font = \"9pt Helvetica\";\x0D\x0A\
 ctx.textBaseline = \"bottom\";\x0D\x0A\
 k1=0; k2=0;\x0D\x0A\	 
 przes2=2;\x0D\x0A\
 j=0;\x0D\x0A\
 for (i = przes2; i < ilosc_pomiarow+przes2; i++) {\x0D\x0A\
 y = can.height - dataValue[3*j+2] * yScalar;\x0D\x0A\
 if(dataValue[3*j+2]>0) const_y=60+k1*12+k2*12;\x0D\x0A\
 else const_y=30+k1*12+k2*12;\x0D\x0A\
 if(dataValue[3*j+2]!=9999){if(rozciag_zawez==1){\x0D\x0A\
    if(dataValue[3*j+2]>0) ctx.fillText(Math.abs(dataValue[3*j+2]), (xScalar) * (i )-1, (345*(maxVal-dataValue[3*j+2])/(maxVal-minVal)) +43);\x0D\x0A\
	else ctx.fillText(Math.abs(dataValue[3*j+2]), (xScalar) * (i )-1, (345*(maxVal-dataValue[3*j+2])/(maxVal-minVal)) +67);}}\x0D\x0A\
 j++;\x0D\x0A\
 }\x0D\x0A\
 ctx.translate(0, can.height - margin); \x0D\x0A\
 ctx.scale( xScalar, -1 * yScalar);\x0D\x0A\
 j=0;\x0D\x0A\
 przes3=0;\x0D\x0A\
 for (i = przes3; i < ilosc_pomiarow+przes3; i++) { \x0D\x0A\
 if(dataValue[3*j+2]>prg_max) ctx.fillStyle = \"#ff0066\";\x0D\x0A\
 else if(dataValue[3*j+2]<prg_min) ctx.fillStyle = \"#6633ff\";\x0D\x0A\
 else                          ctx.fillStyle = \"#66cc66\";\x0D\x0A\				
 if(dataValue[3*j+2]!=9999) ctx.fillRect(i + 2*rozciag_zawez, -minVal, 0.5, dataValue[3*j+2]);\x0D\x0A\
 j++;\x0D\x0A\
 }\x0D\x0A\
 }\x0D\x0A",bug1,bug2, Const.s_Wykr[Var.wykres_nr].zawezen, Const.s_Wykr[Var.wykres_nr].prgmax, Const.s_Wykr[Var.wykres_nr].prgmin,cc);
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStart_3 itx !!!");

//sprintf(buf_p,"\r\nHttpWykresStart_2   %d  ",itx);  dbg3(buf_p);
	//sprintf(buf_p,"\r\nHttpWykresStart_2:   Var.wykres_nr: %d  %d  ---%d ",Var.wykres_nr, Const.s_Wykr[Var.wykres_nr].zawezen,itx);  dbg3(buf_p);

}


void HttpWykresDane() // ["12:09","06.16",+10.5,"12:00","06.17",+11.5,.....
{
	int i,j,itx,kx,k;  itx=0;  char a;   buf_rx2[_Size_BufWifi]=0;

	if(WhichFragmentPage==5)
	{   adr_flash_read=adr_flash_start;  IteracjaFlashZewnOdczyt(&adr_flash);
	   if((adr_flash==0)||(adr_flash_start==adr_flash_end)){ dbg3("\r\nKoniec Wykres "); WhichFragmentPage=22; sprintf(&buf_rx2[_Size_BufWifi+itx],"            "); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  goto HttpWykresDane_bbb2; }
	   sprintf(&buf_rx2[_Size_BufWifi+itx],"var dataValue = [");    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


		k=0;
	    for(i=0;i<_Size__s_gpio;i++)
		{
			       j= WyszukajNrCzujkiDlaPortu(i+1);
				   if(j!=-1)
				   {
			    	  if(Const.s_GPIO[i].val==9)
			    	  {
				    	  if( Var.wykres_nr==j){ Var.offs= 2*k;  break;}  //temp z DHT
                          k++;

			    	     j= WyszukajNrCzujkiDlaPortu_2(i+1);
			    	     if(j!=-1)
			    	     {
			    	    	 if( Var.wykres_nr==j){ Var.offs= 2*k;   break;}    //Wilg z DHT
	                         k++;
			    	     }
			    	  }
				      else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))
				      {
				    	  if( Var.wykres_nr==j){ Var.offs= 2*k;   break;}    //temp z DS
                          k++;
				      }
				   }
		}
	  /*  if(k==0)
	    {  dbg3("\r\nKoniec Wykres aaa "); WhichFragmentPage=22;    sprintf(buf_p,"\r\nXXX: %d  %d  ", Var.wykres_nr, offs); dbg3(buf_p);

	       sprintf(&buf_rx2[_Size_BufWifi+itx],"            "); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	       goto HttpWykresDane_bbb2;
	    }*/

	    sprintf(buf_p,"\r\nXXX: %d  %d  ", Var.wykres_nr, Var.offs); dbg3(buf_p);

	}

	k=IloscCzujnikow(); k+=4;      //4 bo czas zapisywany jest w 4-rech bajtach
	HttpWykresDane_aaa:
	AT25SF_ReadArray(4096+adr_flash_read, buf_p, k );
    adr_flash_read+=k;

    time=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
    ZamianaCzasuZFlash(time);


    // offs= 2*Var.wykres_nr;// Tu nr Czujki !!!!!
       ttemp=   0xff00&(((uint16_t)buf_p[4+Var.offs+0])<<8) | 0x00ff&(((uint16_t)buf_p[4+Var.offs+1])<<0);



	 a=SprawdzKtoraCzujkaZapisu(Var.offs/2);
   	 if((a=='T')||(a=='D')){ if(ttemp==0x7FFF) sprintf(&buf_p[90],"9999");  else{ wartTemp_odwrotnie (&buf_p[90], ttemp); if(buf_p[90+1]=='0') buf_p[90+1]=' ';  }  }
   	 else if(a=='P'){ if(ttemp==0x7FFF) sprintf(&buf_p[90],"9999");  else sprintf(&buf_p[90],"%d",ttemp);   }



	sprintf(&buf_rx2[_Size_BufWifi+itx],"\"%02d:%02d\",\"%02d.%02d\",%s,",godz,min,dzien,mies,&buf_p[90]);    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	if(adr_flash_read==adr_flash_end){ dbg3("\r\nKoniec Wykres ");  WhichFragmentPage=22; goto HttpWykresDane_bbb;  }
	if(itx < (_Size_WskFragmentPage-100)) goto HttpWykresDane_aaa;  //Nie wiem dlaczego 500 a nie np 100 !!!!!!
	else
	{  sprintf(buf_p,"\r\nAAA:  %d  %d",adr_flash_read, adr_flash_end); dbg3(buf_p);
	   if(WhichFragmentPage>5) WhichFragmentPage--;
	   goto HttpWykresDane_bbb2;
	}

	HttpWykresDane_bbb:
	sprintf(&buf_rx2[_Size_BufWifi+itx-1],"];</script><script>var tyt=[];tyt[0]=\" \";document.getElementById(\"tu_menu\").innerHTML=tyt;</script></html>");    itx+=strlen(&buf_rx2[_Size_BufWifi+itx-1]);  itx--;
	HttpWykresDane_bbb2:
	buf_rx2[_Size_BufWifi+itx]=0;

	 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresDane itx !!!");

}





void HttpWyborWykres2()
{
	int i,j,itx,kx,k;  itx=0;  char *buf[5],*ptr,*ptr1,*ptr2,*ptr3;   uint32_t addr;


sprintf(&buf_rx2[_Size_BufWifi+itx],"</div></div>\r\n<div class=\"y\"><div class=\"x\"><font size=\"5\" color=\"black\"> Rodzaj Wizualizacji</font><br><font size=\"4\" color=\"black\"> pomiarow temp i wilgot</font><br><br><a href=\"wykres1\" return false><font size=\"5\" color=\"green\">Wykres</font></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"plik.txt\" return false><font size=\"5\" color=\"green\">Plik.txt</font></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"plik.csv\" return false><font size=\"5\" color=\"green\">Plik.csv</font></a></div></div>");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

j=0;
for(i=0;i<_Size_imp;i++){ if(Const.s_GPIO[t_imp[i]-1].val==7) j=1;  }

if(j==1){
 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n<div class=\"y\" style=\"height:350px\"><div class=\"x\"style=\"height:320px\"><font size=\"5\" color=\"black\"> Dane Licznikowe</font><br>");
 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
 sprintf(&buf_rx2[_Size_BufWifi+itx],"\
 <a href=\"liczgodz.txt\" return false><font size=\"5\" color=\"green\">Godzinne.txt</font></a><br>\
 <a href=\"liczdob.txt\" return false><font size=\"5\" color=\"green\">Dobowe.txt</font></a><br>\
 <a href=\"licztyg.txt\" return false><font size=\"5\" color=\"green\">Tygodniowe.txt</font></a><br>\
 <a href=\"liczmies.txt\" return false><font size=\"5\" color=\"green\">Miesieczne.txt</font></a><br>\
 <a href=\"wykres2\" return false><font size=\"5\" color=\"green\">Wykres</font></a></div></div>");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}



sprintf(&buf_rx2[_Size_BufWifi+itx],"</body></html>");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


//sprintf(buf_p,"\r\nHttpWyborWykres   %d  ",itx);  dbg3(buf_p);
if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWyborWykres2 itx !!!");
}



void HttpMaskiSmsEmail_startA(char smsmail)
{
	int i,j,itx,kx,h,h1,h2,h3,k;  itx=0;

	if(smsmail=='S') sprintf(buf_p,"Numery");
	else             sprintf(buf_p,"E-mail'e");

	if(smsmail=='S'){  sprintf(&buf_p[20],"<td width=\"25px\"></td><td><font size=\"4\" color=\"#9edf0f\">Dzwon</font></td><td width=\"3px\"></td><td><font size=\"4\" color=\"#9edf0f\">Raport</font></td>");   sprintf(&buf_p[200],"450");   sprintf(&buf_p[220],"420"); }
	else            {  sprintf(&buf_p[20],"<td width=\"26px\"></td><td><font size=\"4\" color=\"#9edf0f\">Raport</font></td>");   sprintf(&buf_p[200],"390");   sprintf(&buf_p[220],"350");  }

	sprintf(&buf_rx2[_Size_BufWifi+itx],"\
<div class=\"corner-radius2\" style=\"margin-left: 0px; width: %spx; height: 660px;\">\x0D\x0A\
<div class=\"corner-radius\" style=\"margin-left: 5px; width: %spx; height: 630px;\">\x0D\x0A\
<table><tr><td><font class=\"css-font-style\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s Uzytkownikow</font></td></tr></table>\x0D\x0A\
<table><tr><td><font size=\"4\" color=\"#ccc\" style=\"padding: 20px;\">Powiadamianie uzytkownikow</font></td></tr></table>\x0D\x0A\
<table><tr><td><font size=\"4\" color=\"#ccc\" style=\"padding: 20px;\">o zaistnialych zdarzeniach</font></td>%s</tr></table>\x0D\x0A\
<table>\x0D\x0A",&buf_p[200],&buf_p[220],buf_p,&buf_p[20]);
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

if(smsmail=='S'){ sprintf(buf_p,"nrtel");      kx=17; }
else            { sprintf(buf_p,"email_odb");  kx=40; }


for(i=0;i<_Ilosc_Tel/2;i++)
{
	if(smsmail=='S') sprintf(&buf_p[20],"<td width=\"10px\"></td><td width=\"40px\"><input type=\"checkbox\" id=\"chk_%03d\" name=\"n_s_xerrr%03d_000\" class=\"regular-checkbox checkbox\"          /><label for=\"chk_%03d\"></label></td>",i+1,i+1,i+1);
	else             sprintf(&buf_p[20]," ");
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<tr><td><font class=\"css_list\">%d.</font><input type=\"text\" class=\"textbox\" style=\"width:240px;\" name=\"n_s_%s%d_000\" size=\"%d\" maxlength=\"%d\" value=\"\"                                           ></td>%s",i+1,buf_p,i+1,kx,kx,&buf_p[20]);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	if(smsmail=='S') sprintf(&buf_p[20],"<td width=\"10px\"></td><td width=\"40px\"><input type=\"checkbox\" id=\"Dchk_%03d\" name=\"n_s_derrr%03d_000\" class=\"regular-checkbox checkbox\"          /><label for=\"Dchk_%03d\"></label></td>",i+1,i+1,i+1);
    else             sprintf(&buf_p[20],"<td width=\"10px\"></td><td width=\"40px\"><input type=\"checkbox\" id=\"Mchk_%03d\" name=\"n_s_merrr%03d_000\" class=\"regular-checkbox checkbox\"          /><label for=\"Mchk_%03d\"></label></td>",i+1,i+1,i+1);
	sprintf(&buf_rx2[_Size_BufWifi+itx],"%s</tr>\x0D\x0A",&buf_p[20]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

}

if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO startA itx !!!");


}

void HttpMaskiSmsEmail_startAB(char smsmail)
{
	int i,j,itx,kx,h,h1,h2,h3,k;  itx=0;

if(smsmail=='S'){ sprintf(buf_p,"nrtel");      kx=17; }
else            { sprintf(buf_p,"email_odb");  kx=40; }


for(i=_Ilosc_Tel/2;i<_Ilosc_Tel;i++)
{
	if(smsmail=='S') sprintf(&buf_p[20],"<td width=\"10px\"></td><td width=\"40px\"><input type=\"checkbox\" id=\"chk_%03d\" name=\"n_s_xerrr%03d_000\" class=\"regular-checkbox checkbox\"          /><label for=\"chk_%03d\"></label></td>",i+1,i+1,i+1);
	else             sprintf(&buf_p[20]," ");
	sprintf(&buf_rx2[_Size_BufWifi+itx],"<tr><td><font class=\"css_list\">%d.</font><input type=\"text\" class=\"textbox\" style=\"width:240px;\" name=\"n_s_%s%d_000\" size=\"%d\" maxlength=\"%d\" value=\"\"                                           ></td>%s",i+1,buf_p,i+1,kx,kx,&buf_p[20]);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	if(smsmail=='S') sprintf(&buf_p[20],"<td width=\"10px\"></td><td width=\"40px\"><input type=\"checkbox\" id=\"Dchk_%03d\" name=\"n_s_derrr%03d_000\" class=\"regular-checkbox checkbox\"          /><label for=\"Dchk_%03d\"></label></td>",i+1,i+1,i+1);
    else             sprintf(&buf_p[20],"<td width=\"10px\"></td><td width=\"40px\"><input type=\"checkbox\" id=\"Mchk_%03d\" name=\"n_s_merrr%03d_000\" class=\"regular-checkbox checkbox\"          /><label for=\"Mchk_%03d\"></label></td>",i+1,i+1,i+1);
	sprintf(&buf_rx2[_Size_BufWifi+itx],"%s</tr>\x0D\x0A",&buf_p[20]);
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}

if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO startAB itx !!!");


}

void HttpMaskiSmsEmail_startB(char smsmail)
{
	int i,j,itx,kx,h,h1,h2,h3,k;  itx=0;

if(smsmail=='S') sprintf(buf_p,"SMS");
else             sprintf(buf_p,"E-mail");

k=0;
for(i=odd-1; i<ddo; i++)
{
	h1=WyszukujPortElement(t_te,_Size_te,TabPort[i]);
	h2=WyszukujPortElement(t_we,_Size_we,TabPort[i]);
	h3=WyszukujPortElement(t_pt,_Size_pt,TabPort[i]);

	if(h1!=-1) goto omin234wsdd;
	if(h2!=-1) goto omin234wsdd;
	if(h3!=-1) goto omin234wsdd;

	goto  omin234ws111dd;
	omin234wsdd:
     k++;
    omin234ws111dd:
    asm("nop");
}


sprintf(&buf_rx2[_Size_BufWifi+itx],"</table>\x0D\x0A</div></div>\x0D\x0A\
<div class=\"corner-radius2\" style=\"margin-left: 0px; width: %dpx; height: 660px;\" >\x0D\x0A\
<div class=\"corner-radius\"  style=\"margin-left: 5px; width: %dpx; height: 630px;\" >\x0D\x0A\
<table><tr><td><font class=\"css-font-style\">&nbsp;Wybor portow dla powiadamiania %s</font></td></tr></table>\x0D\x0A\
<table><tr><td><font size=\"4\" color=\"#9edf0f\" style=\"padding: 0px;\"><b>Port:</b></font></td>\x0D\x0A", (2+k)*45+30, (2+k)*45 , buf_p);
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

for(i=odd-1; i<ddo; i++)
{
	h1=WyszukujPortElement(t_te,_Size_te,TabPort[i]);
	h2=WyszukujPortElement(t_we,_Size_we,TabPort[i]);
	h3=WyszukujPortElement(t_pt,_Size_pt,TabPort[i]);

	if(h1!=-1) goto omin234ws;
	if(h2!=-1) goto omin234ws;
	if(h3!=-1) goto omin234ws;

	goto  omin234ws111;
	omin234ws:

		sprintf(&buf_rx2[_Size_BufWifi+itx],"<td width=\"40px\"><font size=\"4\" color=\"#9edf0f\"><b>%d</b></font></td>\x0D\x0A",TabPort[i]);
		itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

    omin234ws111:
    asm("nop");
}

sprintf(&buf_rx2[_Size_BufWifi+itx],"</tr></table>\x0D\x0A");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO startB itx !!!");

}

void HttpMaskiSmsEmail_start_2()
{
	int i,j,itx,kx,h,h1,h2,h3;  itx=0;


	sprintf(&buf_rx2[_Size_BufWifi+itx],"<table><tr>\r\n<td width=\"40px\"><font size=\"4\" color=\"#ccc\">Typ:</font></td>\r\n");
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	for(i=odd-1; i<ddo; i++)
	{
	   	h1=WyszukujPortElement(t_te,_Size_te,TabPort[i]);
	   	h2=WyszukujPortElement(t_we,_Size_we,TabPort[i]);
	   	h3=WyszukujPortElement(t_pt,_Size_pt,TabPort[i]);

	   	if(h1!=-1) goto omin234wss;
	   	if(h2!=-1) goto omin234wss;
	   	if(h3!=-1) goto omin234wss;

	   	goto  omin234ws111s;
	   	omin234wss:

             if((Const.s_GPIO[TabPort[i]-1].val==2)||(Const.s_GPIO[TabPort[i]-1].val==9)||(Const.s_GPIO[TabPort[i]-1].val==3)) sprintf(buf_p,"Te");  // 't_te'  to tosamo co  't_we'
        else if((Const.s_GPIO[TabPort[i]-1].val==4)||(Const.s_GPIO[TabPort[i]-1].val==13))                                  sprintf(buf_p,"We");
        else                                                                                                                   sprintf(buf_p,"  ");
        sprintf(&buf_rx2[_Size_BufWifi+itx],"<td width=\"40px\"><font size=\"4\" color=\"#ccc\">%s</font></td>\r\n",buf_p);
        itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	    omin234ws111s:
	    asm("nop");
	}

	sprintf(&buf_rx2[_Size_BufWifi+itx],"</tr></table>\r\n<table>\r\n");

    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO start 2 itx !!!");

}

void HttpMaskiSmsEmail_wsad(int i, char smsmail)
{
	int j,itx,kx,h,h1,h2,h3;  itx=0;


    if(i==1);
    else  sprintf(&buf_rx2[_Size_BufWifi+itx],"</tr>\r\n");

	sprintf(&buf_rx2[_Size_BufWifi+itx],"<tr><td width=\"40px\"><font class=\"css_list\" style=\"padding: 0px\">%d.</font></td>\r\n",i);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	for(j=odd-1; j<ddo; j++)
	{
	   	h1=WyszukujPortElement(t_te,_Size_te,TabPort[j]);
	   	h2=WyszukujPortElement(t_we,_Size_we,TabPort[j]);
	   	h3=WyszukujPortElement(t_pt,_Size_pt,TabPort[j]);

	   	if(h1!=-1) goto omin234wssq;
	   	if(h2!=-1) goto omin234wssq;
	   	if(h3!=-1) goto omin234wssq;

	   	goto  omin234ws111sq;
	   	omin234wssq:

		if(smsmail=='S'){
		   if(((Const.s_Ma[i-1].sms[h/16]>>(h-16*(h/16)))&0x01)>0) sprintf(buf_p,"checked");
	       else                                                    sprintf(buf_p,"       ");
		}
		else{
		   if(((Const.s_Ma[i-1].mail[h/16]>>(h-16*(h/16)))&0x01)>0) sprintf(buf_p,"checked");
		   else                                                     sprintf(buf_p,"       ");
		}

	    if((Const.s_GPIO[TabPort[j]-1].val==2)||(Const.s_GPIO[TabPort[j]-1].val==9)||(Const.s_GPIO[TabPort[j]-1].val==4)||(Const.s_GPIO[TabPort[j]-1].val==13)||(Const.s_GPIO[TabPort[j]-1].val==3))
		     sprintf(&buf_rx2[_Size_BufWifi+itx],"<td width=\"40px\"><input type=\"checkbox\" id=\"wr_checkboxS%03d_%03d\" name=\"n_s_Ma%c_akt%03d_%03d\" class=\"regular-checkbox checkbox\" %s /><label for=\"wr_checkboxS%03d_%03d\"></label></td>\r\n",TabPort[j],i,smsmail,TabPort[j],i,buf_p,TabPort[j],i);

	    else
		     sprintf(&buf_rx2[_Size_BufWifi+itx],"<td width=\"40px\"></td>\r\n");
	    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	    omin234ws111sq:
	    asm("nop");

	}

	if(i==_Ilosc_Tel)
	{
		sprintf(&buf_rx2[_Size_BufWifi+itx],"</tr></table><table><tr><td width=\"30px\"></td><td><font size=\"4\" color=\"#ccc\" style=\"padding: 20px;\">Te - &nbsp;&nbsp;temperatura, wilgotnosc</font> <font size=\"4\" color=\"#ccc\" style=\"padding: 20px;\">We - &nbsp;&nbsp;wejscie cyfrowe</font></td></tr></table></div></div>\r\n");
		itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

		sprintf(&buf_rx2[_Size_BufWifi+itx],"\
<div class=\"corner-radius2\" style=\"margin-left: 0px; height: 660px;\">\x0D\x0A\
<div class=\"corner-radius\" style=\"margin-left: 10px; height: 630px;\">\x0D\x0A\
 <font class=\"css-font-style\">Ustawianie Raportów</font><br>\x0D\x0A\
 <font size=\"4\" color=\"#9edf0f\">Raport wysylany codziennie <br> o godz. <input type=\"text\" class=\"textbox\" style=\"width:50px;\" name=\"n_s_RR_godzON1_000\" size=\"5\" maxlength=\"5\" value=\"00:00\"  onkeypress=\"return maskujKlawisze(event, ':0123456789');\"></font>\x0D\x0A\
</div></div>\x0D\x0A");
	    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	}

	if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO wsad itx !!!");
}


void dopelnij_nazwy(char *var2, int size)
{
   int i,j;
	j=strlen(var2);  var2[j]='"'; var2[j+1]=0;  for(i=0;i<(size-j);i++) var2[j+1+i]=' ';  var2[j+1+i]=0;
}

char* HttpParser(char*buf,int nr)   //asp
{
	char *ptr,*ptr1,*ptr2,*ptr3,var[51],var2[51],bufff[10],m,m1,n,ile_option; int i,j,k,o,pp,po,wilgtemp,h;  char spa[9];
	ekran_bit|=0x02; delay_funkc[24]=1; //wstrzymuj wyswietlanie tft
	sprintf(&buf_rx2[_Size_BufWifi],buf);
	ptr2=&buf_rx2[_Size_BufWifi];      sprintf(bufff,"%03d",nr);

   do
   {    for(i=0;i<31;i++){ var[i]=0; var2[i]=0; }

       if(ptr=strstr(ptr2,"var active_id"))  //var active_id002="03b"
       {
             i= 100*(ptr[13]&0x0f)+10*(ptr[14]&0x0f)+(ptr[15]&0x0f); i--;
   	         sprintf(buf_p,"%02d",Const.s_GPIO[i].val);
	         *(ptr+18)=buf_p[0];
	         *(ptr+19)=buf_p[1];

	        // if(i==7){sprintf(buf_p,"\r\nASP: %d  ",Const.s_GPIO[7].val); dbg3(buf_p); }

   	     ptr1=ptr+5;
   	     goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_nrident"))
   	   {
    	    *(ptr+0)=hex_to_ASCII((Const.s_Kod[0].val2[0]>>4)&0x0f);
           	*(ptr+1)=hex_to_ASCII((Const.s_Kod[0].val2[0])&0x0f);
           	*(ptr+2)=hex_to_ASCII((Const.s_Kod[0].val2[1]>>4)&0x0f);
           	*(ptr+3)=hex_to_ASCII((Const.s_Kod[0].val2[1])&0x0f);
           	*(ptr+4)=hex_to_ASCII((Const.s_Kod[0].val2[2]>>4)&0x0f);
           	*(ptr+5)=hex_to_ASCII((Const.s_Kod[0].val2[2])&0x0f);
           	*(ptr+6)=hex_to_ASCII((Const.s_Kod[0].val2[3]>>4)&0x0f);
           	*(ptr+7)=hex_to_ASCII((Const.s_Kod[0].val2[3])&0x0f);
           	*(ptr+8)=hex_to_ASCII((Const.s_Kod[0].val2[4]>>4)&0x0f);
           	*(ptr+9)=hex_to_ASCII((Const.s_Kod[0].val2[4])&0x0f);
           	*(ptr+10)=hex_to_ASCII((Const.s_Kod[0].val2[5]>>4)&0x0f);
           	*(ptr+11)=hex_to_ASCII((Const.s_Kod[0].val2[5])&0x0f);
           	*(ptr+12)=hex_to_ASCII((Const.s_Kod[0].val2[6]>>4)&0x0f);
           	*(ptr+13)=hex_to_ASCII((Const.s_Kod[0].val2[6])&0x0f);
           	*(ptr+14)=hex_to_ASCII((Const.s_Kod[0].val2[7]>>4)&0x0f);
           	*(ptr+15)=hex_to_ASCII((Const.s_Kod[0].val2[7])&0x0f);
           	 ptr1=ptr+16+9;
      	     goto end_updatasoft;
   	   }
       else if(ptr=strstr(ptr2,"v_gdz"))
       {
    	   PobierzCzas(buf_p);
    	   sprintf(&buf_p[10],"%02d",buf_p[4]);
    	   *(ptr+0)=buf_p[10+0];
    	   *(ptr+1)=buf_p[10+1];
    	   *(ptr+2)=':';
    	   sprintf(&buf_p[10],"%02d",buf_p[5]);
    	   *(ptr+3)=buf_p[10+0];
    	   *(ptr+4)=buf_p[10+1];
     	  // dbg3("\r\nGodz !!!");
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_date"))
       {
    	   PobierzCzas(buf_p);
    	   sprintf(&buf_p[10],"%02d",buf_p[0]);
    	   *(ptr+0)=buf_p[10+0];
    	   *(ptr+1)=buf_p[10+1];
    	   *(ptr+2)='/';
    	   sprintf(&buf_p[10],"%02d",buf_p[1]);
    	   *(ptr+3)=buf_p[10+0];
    	   *(ptr+4)=buf_p[10+1];
    	   *(ptr+5)='/';
    	   sprintf(&buf_p[10],"%02d",buf_p[2]);
    	   *(ptr+6)=buf_p[10+0];
    	   *(ptr+7)=buf_p[10+1];
     	   //dbg3("\r\nData !!!");
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_resX"))
       {
    	   sprintf(buf_p,"%d          ",Const.s_Map[0].resX);
    	   *(ptr+0)=buf_p[0];
    	   *(ptr+1)=buf_p[1];
    	   *(ptr+2)=buf_p[2];
    	   *(ptr+3)=buf_p[3];
    	   *(ptr+4)=buf_p[4];
    	   *(ptr+5)=buf_p[5];
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_resY"))
       {
    	   sprintf(buf_p,"%d          ",Const.s_Map[0].resY);
    	   *(ptr+0)=buf_p[0];
    	   *(ptr+1)=buf_p[1];
    	   *(ptr+2)=buf_p[2];
    	   *(ptr+3)=buf_p[3];
    	   *(ptr+4)=buf_p[4];
    	   *(ptr+5)=buf_p[5];
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_ilelor"))
       {
    	   sprintf(buf_p,"%d;          ",_IleLor);
    	   *(ptr+0)=buf_p[0];
    	   *(ptr+1)=buf_p[1];
    	   *(ptr+2)=buf_p[2];
    	   *(ptr+3)=buf_p[3];
    	   *(ptr+4)=buf_p[4];
    	   *(ptr+5)=buf_p[5];
    	   *(ptr+6)=buf_p[6];
    	   *(ptr+7)=buf_p[7];
    	   *(ptr+8)=buf_p[8];
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_obrazPort"))
       {
    	   j= 10*(*(ptr+11)&0x0f) + *(ptr+12)&0x0f;
    	   	   	if(j==0) j=5;
    	   else if(j==1) j=6;
    	   else if(j==2) j=7;
    	   else if(j==3) j=8;
    	   else if(j==4) j=9;
    	   else if(j==5) j=10;
    	   else if(j==6) j=28;
    	   else if(j==7) j=21;
    	   else if(j==8) j=22;
    	   else if(j==9) j=23;
    	   else if(j==10) j=24;
    	   else if(j==11) j=33;
    	   for(i=0;i<strlen(Const.s_GPIO[j-1].nazwa);i++) *(ptr+i)=Const.s_GPIO[j-1].nazwa[i];
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_obrazZw"))
       {
    	   j= WyszukujPortElement(t_we,_Size_we, (*(ptr+9)&0x0f)+5);
    	   if(j!=-1)
    	   {
        	   for(i=0;i<strlen(Const.s_WE[j].nazwa_zw);i++) *(ptr+i)=Const.s_WE[j].nazwa_zw[i];
        	   for(j=i;j<21;j++) *(ptr+j)=' ';
               ptr1=ptr+5;
    	   }
           goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_obrazRo"))
       {
    	   j= WyszukujPortElement(t_we,_Size_we, (*(ptr+9)&0x0f)+5);
    	   if(j!=-1)
    	   {
        	   for(i=0;i<strlen(Const.s_WE[j].nazwa_ro);i++) *(ptr+i)=Const.s_WE[j].nazwa_ro[i];
        	   for(j=i;j<21;j++) *(ptr+j)=' ';
               ptr1=ptr+5;
    	   }
           goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_obrazPn"))
       {
    	  int nrPn=0;
    	       if((*(ptr+9)&0x0f)==0) nrPn=28;
    	  else if((*(ptr+9)&0x0f)==1) nrPn=21;
    	  else if((*(ptr+9)&0x0f)==2) nrPn=22;
    	  else if((*(ptr+9)&0x0f)==3) nrPn=23;
    	  else if((*(ptr+9)&0x0f)==4) nrPn=24;
    	  else if((*(ptr+9)&0x0f)==5) nrPn=33;

    	   j= WyszukujPortElement(t_wy,_Size_wy, nrPn);
    	   if(j!=-1)
    	   {
        	   for(i=0;i<strlen(Const.s_PK[j].nazwa_on);i++) *(ptr+i)=Const.s_PK[j].nazwa_on[i];
        	   for(j=i;j<21;j++) *(ptr+j)=' ';
               ptr1=ptr+5;
    	   }
           goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_obrazPf"))
       {
     	  int nrPn=0;
     	       if((*(ptr+9)&0x0f)==0) nrPn=28;
     	  else if((*(ptr+9)&0x0f)==1) nrPn=21;
     	  else if((*(ptr+9)&0x0f)==2) nrPn=22;
     	  else if((*(ptr+9)&0x0f)==3) nrPn=23;
     	  else if((*(ptr+9)&0x0f)==4) nrPn=24;
     	  else if((*(ptr+9)&0x0f)==5) nrPn=33;

    	   j= WyszukujPortElement(t_wy,_Size_wy, nrPn);
    	   if(j!=-1)
    	   {
        	   for(i=0;i<strlen(Const.s_PK[j].nazwa_of);i++) *(ptr+i)=Const.s_PK[j].nazwa_of[i];
        	   for(j=i;j<21;j++) *(ptr+j)=' ';
               ptr1=ptr+5;
    	   }
           goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_SizeImg"))
       {
    	   sprintf(buf_p,"%d          ",_Obraz_Size);
    	   *(ptr+0)=buf_p[0];
    	   *(ptr+1)=buf_p[1];
    	   *(ptr+2)=buf_p[2];
    	   *(ptr+3)=buf_p[3];
    	   *(ptr+4)=buf_p[4];
    	   *(ptr+5)=buf_p[5];
    	   *(ptr+6)=buf_p[6];
    	   *(ptr+7)=buf_p[7];
    	   *(ptr+8)=buf_p[8];
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_zasgsm"))
       {
    	   *(ptr+0)=zasieg_gsm[0];
    	   *(ptr+1)=zasieg_gsm[1];
    	   *(ptr+2)='&';
    	   *(ptr+3)='n';
    	   *(ptr+4)='b';
    	   *(ptr+5)='s';
    	   *(ptr+6)='p';
    	   *(ptr+7)=';';
     	   //dbg3("\r\nZasieg GSM !!!");
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_czolinforej"))
       {
    	   sprintf(buf_p,"<button class=\"n\"><font color='#e00'>Rejestrator ON</font></button>");
    	   if((Const.s_Rej[0].param&0x01)>0){   for(i=0;i<67;i++) *(ptr+i)=buf_p[i];   }
    	   else                             {   for(i=0;i<67;i++) *(ptr+i)=' ';        }
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if((ptr=strstr(ptr2,"v_reff"))||(ptr=strstr(ptr2,"v_dddd")))
       {
    	   m1=0;
    	   if(ptr=strstr(ptr2,"v_reff")) m1++;
    	   if(ptr3=strstr(ptr2,"v_dddd")) m1++;
    	   if(m1==2){  if(ptr>ptr3){ ptr=ptr3; }  }
    	   else
    	   {
    		   if((ptr=strstr(ptr2,"v_reff"))||(ptr=strstr(ptr2,"v_dddd")));
    	   }

    	  if((wybor_bitowy&0x0200)==0){   //strony na ENC
     	      if(*(ptr+2)=='d'){  sprintf(buf_p,"wst=0;");                            for(i=0;i<6;i++) *(ptr+i)=buf_p[i]; }
     	      else             {  sprintf(buf_p,"%d     ",Const.s_Poz[0].refr*1000);  for(i=0;i<7;i++) *(ptr+i)=buf_p[i]; }

          }
          else{     //strony na ESP
        	if(*(ptr+2)=='d'){
     	         *(ptr+0)=' ';
     	         *(ptr+1)=' ';
     	         *(ptr+2)=' ';
     	         *(ptr+3)=' ';
         	     *(ptr+4)=' ';
         	     *(ptr+5)=' ';
        	}
        	else
        	{
       	      if(Const.s_Poz[0].refr>4)
       	      {
       	         sprintf(buf_p,"%d     ",Const.s_Poz[0].refr*1000);
       	         for(i=0;i<7;i++) *(ptr+i)=buf_p[i];
       	      }
       	      else
       	      {
        		*(ptr+0)='5';
       	        *(ptr+1)='0';
       	        *(ptr+2)='0';
       	        *(ptr+3)='0';
       	        *(ptr+4)=' ';
       	        *(ptr+5)=' ';
       	        *(ptr+6)=' ';
       	      }
        	}
          }
     	  ptr1=ptr+5;
     	  goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_pksto"))
       {
    	   m1=*(ptr+7);
    	   m= 10*((*(ptr+8))&0x0f)+((*(ptr+9))&0x0f);
    	   if(m1=='n')   //ON
    	   {  sprintf(&buf_p[50],Const.s_PK[m-1].nazwa_on);   for(j=0;j<20;j++){ if(buf_p[50+j]==0) break; }
    		  for(i=0;i<20;i++)
    		  {
    			  if(i<j) *(ptr+i)= buf_p[50+i];
    			  else *(ptr+i)=' ';
    		  }
    		  if(m==ddo)
    		  {
                  /* if(ptr=strstr(ptr,"(z*)"))
    			   {
    					    if(WhichPage==15){  *(ptr+0)='#'; *(ptr+1)='d'; *(ptr+2)='0'; *(ptr+3)='0';  }
    				   else if(WhichPage==6) {  *(ptr+0)='#'; *(ptr+1)='f'; *(ptr+2)='8'; *(ptr+3)='8';  }
    			   }*/
    		  }
    	   }
    	   else         //OFF
    	   {  sprintf(&buf_p[60],Const.s_PK[m-1].nazwa_of);  for(j=0;j<20;j++){ if(buf_p[60+j]==0) break; }
     		  for(i=0;i<20;i++)
     		  {
     			  if(i<j) *(ptr+i)= buf_p[60+i];
     			  else *(ptr+i)=' ';
     		  }
    	   }
     	  // dbg3(" Y");
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_westo"))
       {
    	   m1=*(ptr+7);
    	   m= 10*((*(ptr+8))&0x0f)+((*(ptr+9))&0x0f);
    	   if(m1=='n')   //ON
    	   {  sprintf(&buf_p[50],Const.s_WE[m-1].nazwa_zw);   for(j=0;j<20;j++){ if(buf_p[50+j]==0) break; }
    		  for(i=0;i<20;i++)
    		  {
    			  if(i<j) *(ptr+i)= buf_p[50+i];
    			  else *(ptr+i)=' ';
    		  }
    		 /* if(ptr=strstr(ptr,"(q*)"))
    		  {
    			       if(WhichPage==15){  *(ptr+0)='#'; *(ptr+1)='0'; *(ptr+2)='0'; *(ptr+3)='0';  }
    			  else if(WhichPage==6) {  *(ptr+0)='#'; *(ptr+1)='e'; *(ptr+2)='e'; *(ptr+3)='e';   }
    		  }*/

    	   }
    	   else if(m1=='f')        //OFF
    	   {  sprintf(&buf_p[60],Const.s_WE[m-1].nazwa_ro);  for(j=0;j<20;j++){ if(buf_p[60+j]==0) break; }
     		  for(i=0;i<20;i++)
     		  {
     			  if(i<j) *(ptr+i)= buf_p[60+i];
     			  else *(ptr+i)=' ';
     		  }
              /*if(ptr=strstr(ptr,"(z*)"))
			  {
					   if(WhichPage==15){  *(ptr+0)='#'; *(ptr+1)='d'; *(ptr+2)='0'; *(ptr+3)='0';  }
				  else if(WhichPage==6) {  *(ptr+0)='#'; *(ptr+1)='f'; *(ptr+2)='8'; *(ptr+3)='8';  }
			  }*/
    	   }
    	   else if(m1=='s')        //USZKodzenie
    	   {  sprintf(&buf_p[60],Const.s_WE[m-1].nazwa_uszk);  for(j=0;j<20;j++){ if(buf_p[60+j]==0) break; }
     		  for(i=0;i<20;i++)
     		  {
     			  if(i<j) *(ptr+i)= buf_p[60+i];
     			  else *(ptr+i)=' ';
     		  }
    	   }
     	  // dbg3(" W");
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"v_scroll"))
       {

    	   *(ptr+0)='0';
    	   *(ptr+1)=' ';
    	   *(ptr+2)=' ';
    	   *(ptr+3)=' ';
    	   *(ptr+4)=' ';
    	   *(ptr+5)=' ';
    	   *(ptr+6)=' ';
    	   *(ptr+7)=' ';
     	 //  dbg3("\r\n## v_scroll ##");
     	   ptr1=ptr+5;
     	   goto end_updatasoft;
       }
       else if(ptr=strstr(ptr2,"ccol=\"d00\""))
	   {
           if(WhichPage==15);
           else if(WhichPage==6)
           {
       	      *(ptr+6+0)='f';
       	      *(ptr+6+1)='8';
       	      *(ptr+6+2)='8';
           }
    	   ptr=ptr+5;

    	   if(ptr=strstr(ptr,"ccol=\"000\""))
    	   {
    		   if(WhichPage==15);
    		   else if(WhichPage==6)
    		   {
    		        *(ptr+6+0)='e';
    		        *(ptr+6+1)='e';
    		        *(ptr+6+2)='e';
    		    }
    	   }
    	   ptr=ptr+5;

    	   if(ptr=strstr(ptr,"ccol=\"00d\""))
    	   {
    		   if(WhichPage==15);
    		   else if(WhichPage==6)
    		   {
    		        *(ptr+6+0)='8';
    		        *(ptr+6+1)='c';
    		        *(ptr+6+2)='f';
    		    }
    	   }
    	   ptr=ptr+5;

      	   ptr1=ptr+5;
      	   goto end_updatasoft;
	   }
       else if(ptr=strstr(ptr2,"Ustawienia sieci Wifi"))
	   {
           if(WhichPage==78)
           {
        	  *(ptr+17)='L';
        	  *(ptr+18)='A';
        	  *(ptr+19)='N';
        	  *(ptr+20)=' ';

           }
           else
           {
        	   if(WhichFragmentPage==4)
        	   {
             	  *(ptr+17)='A';
             	  *(ptr+18)='P';
             	  *(ptr+19)=' ';
             	  *(ptr+20)=' ';
        	   }
           }
      	   ptr1=ptr+5;
      	   goto end_updatasoft;
	   }






        if(ptr=strstr(ptr2,"updata "))
	    {         k=strlen("updata ");

            m=*(ptr+7)&0x0f;    if((m<0)||(m>8)) break; // sprintf(buf_p,"\r\nMM: %d",m); dbg3(buf_p);

	        if(((HttpInfoUploadBits>>2*(m-1))&0x0003)==1)
			{	  sprintf(var2,"Wgrany  ");   i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr+k+6+i)=var2[i];
		          sprintf(var2,"blue ");       i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr-14+i)=var2[i];
		          ptr1=ptr+k;
			}
	        else if(((HttpInfoUploadBits>>2*(m-1))&0x0003)==2)
			{	  sprintf(var2,"BLAD !! ");   i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr+k+6+i)=var2[i];
		          sprintf(var2,"red  ");       i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr-14+i)=var2[i];
		          ptr1=ptr+k;
			}
	        else if(((HttpInfoUploadBits>>2*(m-1))&0x0003)==0)
			{	  sprintf(var2,"______..");   i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr+k+6+i)=var2[i];
		          sprintf(var2,"white");       i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr-14+i)=var2[i];
		          ptr1=ptr+k;
	        }
            goto end_updatasoft;

	    }
        /*if(ptr=strstr(ptr2,"n_s_tt"))
        {


        	goto end_updatasoft;
        }*/







//------------------ Shemat dla Czujników -------------------------------

        if(ptr=strstr(ptr2,"Czujnik          0"))
	    {
  	       pp = Const.s_Czujki[nr-1].addr_DS[8];

  	       po=0;
  	       for(i=0;i<(2*7);i++)
  	       {
  	    	 if(Const.s_Czujki[i].addr_DS[8]==pp) po++;
  	       }
  	       if(po==1)  goto omin_ioi;

  	       po=0;
  	       for(i=nr;i<(2*_Size_te);i++)
  	       {
  	    	 if(Const.s_Czujki[i].addr_DS[8]==pp){ po=1; break; }
  	       }

  	       omin_ioi:
  	       if(po==1)
  	       {
    	         *(ptr+0)='T';
    	         *(ptr+1)='e';
    	         *(ptr+2)='m';
    	         *(ptr+3)='p';
    	         *(ptr+4)='e';
    	         *(ptr+5)='r';
    	         *(ptr+6)='a';
    	         *(ptr+7)='t';
    	         *(ptr+8)='u';
    	         *(ptr+9)='r';
    	         *(ptr+10)='a';
    	         *(ptr+11)=' ';
    	         *(ptr+12)='P';
    	         *(ptr+13)='o';
    	         *(ptr+14)='r';
    	         *(ptr+15)='t';    wilgtemp=1;

  	       }
  	       else
  	       {
  	           *(ptr+0)='W';
  	           *(ptr+1)='i';
  	           *(ptr+2)='l';
  	           *(ptr+3)='g';
  	           *(ptr+4)='o';
  	           *(ptr+5)='t';
  	           *(ptr+6)='n';
  	           *(ptr+7)='o';
  	           *(ptr+8)='s';
  	           *(ptr+9)='c';
  	           *(ptr+10)=' ';
  	           *(ptr+11)='P';
  	           *(ptr+12)='o';
  	           *(ptr+13)='r';
  	           *(ptr+14)='t';
  	           *(ptr+15)=' ';   wilgtemp=2;

  	       }
           if(pp<10)
           {
    	      *(ptr+17)=pp|0x30;
    	      *(ptr+18)=' ';
           }
           else
           {
     	      *(ptr+17)=(pp/10)|0x30;
     	      *(ptr+18)=(pp-10*(pp/10))|0x30;
           }
		   *(ptr+19)=' ';


			       if(ptr=strstr(ptr2,"v_xx"))
			       {
			    	   sprintf(buf_p,"%d",nr); i=strlen(buf_p);  buf_p[i+0]=' ';  buf_p[i+1]=' ';

			    	   *(ptr+0)='T';
			    	   *(ptr+1)=buf_p[0];
			    	   *(ptr+2)=buf_p[1];
			    	   *(ptr+3)=buf_p[2];
			     	   ptr1=ptr+5;
			     	   goto end_updatasoft;
			       }


	    }
        if(ptr=strstr(ptr2,"v_progrIR"))
        {
        	//dbg3("\r\nAWWWWWWWWQQQ ");
       	    if(Const.s_GPIO[ t_wy[nr-1]-1 ].val==8)
       	    {
       	    	if(ptr[9]=='a') sprintf(buf_p,"<a href=\"/IRprga\" class=\"shiny-button\"><strong>Programuj IR ON</strong></a>");
       	    	else            sprintf(buf_p,"<a href=\"/IRprgb\" class=\"shiny-button\"><strong>Programuj IR OFF</strong></a>");
       	    	for(i=0;i<strlen(buf_p);i++)  *(ptr+i)= buf_p[i];
       	    }
       	    else
       	    {
       	    	*(ptr+0)=' ';
       	    	*(ptr+1)=' ';
       	    	*(ptr+2)=' ';
       	    	*(ptr+3)=' ';
       	    	*(ptr+4)=' ';
       	    	*(ptr+5)=' ';
       	    	*(ptr+6)=' ';
       	    	*(ptr+7)=' ';
       	    	*(ptr+8)=' ';
       	    	*(ptr+9)=' ';
       	    }
        }
        if(ptr=strstr(ptr2,"Wejscie Cyfrowe 0"))
	    {         i=strlen("Wejscie Cyfrowe 0");

		    *(ptr+i-1+0)=bufff[0];
		    *(ptr+i-1+1)=bufff[1];
		    *(ptr+i-1+2)=bufff[2];
	    }
        if(ptr=strstr(ptr2,"Wejscie EOL 0"))
	    {         i=strlen("Wejscie EOL 0");

		    *(ptr+i-1+0)=bufff[0];
		    *(ptr+i-1+1)=bufff[1];
		    *(ptr+i-1+2)=bufff[2];
	    }
        if(ptr=strstr(ptr2,"Licznik Impulsow 0"))
	    {         i=strlen("Licznik Impulsow 0");

		    *(ptr+i-1+0)=bufff[0];
		    *(ptr+i-1+1)=bufff[1];
		    *(ptr+i-1+2)=bufff[2];
	    }
        if(ptr=strstr(ptr2,"Wyjscie Cyfrowe port 0"))
	    {         i=strlen("Wyjscie Cyfrowe port 0");

	        sprintf(buf_p,"%02d",t_wy[nr-1]);
	        *(ptr+i-1+0)=buf_p[0];
		    *(ptr+i-1+1)=buf_p[1];
	    }
        if(ptr=strstr(ptr2,"Timer dla Wyjscia 0"))
	    {         i=strlen("Timer dla Wyjscia 0");

	        *(ptr+i-1+0)=bufff[0];
	        *(ptr+i-1+1)=bufff[1];
	        *(ptr+i-1+2)=bufff[2];
	    }

        if(ptr=strstr(ptr2,"v_info_email_ENC"))
	    {
        	for(i=0;i<16;i++)  *(ptr+i)= ' ';
        	for(i=0;i<strlen(info_email);i++)  *(ptr+i)= info_email[i];
	    }


	    if(ptr=strstr(ptr2,"v_seltempwilg"))
	    {
	          k=0;
	          sprintf(spa,"        ");
	          if(Const.s_Term[nr-1].nr_temp==0) sprintf(spa,"selected");
	          sprintf(buf_p,"<option value=\"T00\" %s > ---------- </option>\x0D\x0A",spa);
	          for(j=0;j<53;j++) *(ptr+k*53+j)=buf_p[j];
	          k++;

	          for(pp=1;pp<_Size_port;pp++)
	          {
	        	po=0;
	        	for(i=0;i<(2*_Size_te);i++)
	            {
	        	  if(Const.s_Czujki[i].addr_DS[8]==pp)
	        	  {
	        		  sprintf(spa,"        ");
	        		  if(po==0)
	        		  {  po=1;
	        		     if(Const.s_Term[nr-1].nr_temp==pp) sprintf(spa,"selected");
	        		     sprintf(buf_p,"<option value=\"T%02d\" %s >Temp. port %d</option>\x0D\x0A",pp,spa,pp);
	        		     if(pp<10) m=53; else m=54;
	        		  }
	        		  else
	        		  {	 if(Const.s_Term[nr-1].nr_temp==(pp|0x80)) sprintf(spa,"selected");
	        			 sprintf(buf_p,"<option value=\"W%02d\" %s >Wilgot. port %d</option>\x0D\x0A",pp,spa,pp);
	        			 if(pp<10) m=55; else m=56;
	        		  }
	        		  for(j=0;j<m;j++) *(ptr+k*57+j)=buf_p[j];
	        		  k++;
	        	  }
	            }
	          }
	    }
	    if(ptr=strstr(ptr2,"v_selautowe"))
	    {
	        k=0;
	    	sprintf(spa,"        ");
	    	if(Const.s_WE[nr-1].pk==0) sprintf(spa,"selected");
	    	sprintf(buf_p,"<option value=\"P00\" %s > ---- </option>\x0D\x0A",spa);
	          for(j=0;j<47;j++) *(ptr+k*47+j)=buf_p[j];
	          k++;

	    	for(i=0;i<_Size_port;i++)
	    	{
	    		if(Const.s_GPIO[i].val==0)
	    		{
	    			sprintf(spa,"        ");
	    			if(Const.s_WE[nr-1].pk==i+1) sprintf(spa,"selected");
	    			sprintf(buf_p,"<option value=\"P%02d\" %s >port %d</option>\x0D\x0A",i+1,spa,i+1);
       		        if((i+1)<10) m=47; else m=48;
	      		      for(j=0;j<m;j++) *(ptr+k*49+j)=buf_p[j];
	      		      k++;
	    		}
	    	}
	    }
	    if(ptr=strstr(ptr2,"(<<<<*)"))
	    {
	         sprintf(buf_p,"%dpx;  ",174+(53*(ddo-(odd-1)))+10);
	         for(i=0;i<strlen(buf_p);i++) *(ptr+i)=buf_p[i];

	         if(ptr=strstr(ptr2,"(>>>>*)"))
	         {
	        	  sprintf(buf_p,"%dpx;  ",144+(53*(ddo-(odd-1)))+10);
	        	  for(i=0;i<strlen(buf_p);i++) *(ptr+i)=buf_p[i];
	         }
	    }




	    if(ptr=strstr(ptr2,"n_s_"))   //n_s_Rot_akt001_001"
		{
			i=0;
			do
			{
	           var[i]= *(ptr+2+i);
	           i++;
			}while(*(ptr+2+i)!='"');
			var[i]=0;

	      if(strstr(var,"s_tt"));
	      else if(strstr(var,"s_Wykr_zaw"));
	      else if(strstr(var,"s_Wykr_sze"));
	      else if(strstr(var,"s_Wykr_prgmax"));
	      else if(strstr(var,"s_Wykr_prgmin"));
	      else if(strstr(var,"s_WykrEtr_zaw"));
	      else if(strstr(var,"s_WykrEtr_sze"));
	      else if(strstr(var,"s_WykrEtr_prgmax"));
	      else if(strstr(var,"s_WykrEtr_prgmin"));
	      else if(strstr(var,"s_Rot_akt")||strstr(var,"s_xxROTxx"));
	      else if(strstr(var,"s_xerrr"));
	      else if(strstr(var,"s_derrr"));
	      else if(strstr(var,"s_merrr"));
	      else if(strstr(var,"s_SP"));
	      else if(strstr(var,"s_OL"));
	      else if(strstr(var,"s_SW"));
	      else if(strstr(var,"s_SigWy"));
	      else
	      {
			 *(ptr+2+i-3+0)=bufff[0];
			 *(ptr+2+i-3+1)=bufff[1];
			 *(ptr+2+i-3+2)=bufff[2];
	      }

//------------------------------- Ustaw LoRa ------------------------------------------------------
if(strstr(var,"s_Lora_")) //SELECT
{
	     if(strstr(var,"s_Lora_freq")){       ile_option=5;   m1= Const.s_Lora[nr-1].freq;  }
	else if(strstr(var,"s_Lora_power")){      ile_option=4;   m1= Const.s_Lora[nr-1].power;  }
	else if(strstr(var,"s_Lora_spread")){     ile_option=7;   m1= Const.s_Lora[nr-1].spread;  }
	else if(strstr(var,"s_Lora_bandwidth")){  ile_option=10;  m1= Const.s_Lora[nr-1].bandwidth;  }
	else if(strstr(var,"s_Lora_packetlen")){  ile_option=5;   m1= Const.s_Lora[nr-1].packetlen;  }

	for(m=0;m<ile_option;m++)
	{
	    if(ptr1=strstr(ptr,"value=\""))
	    {
	        if(m==m1) sprintf(var2,"selected");
	        else      sprintf(var2,"        ");
	      	j=strlen(var2);
	      	for(i=0;i<j;i++)  *(ptr1+7+3+i)=var2[i];
	    }
	    ptr=ptr1+7;
	}
	goto omin_spr;
}

//------------------------------- Ustaw Siec ------------------------------------------------------
if(strstr(var,"s_Rot_dob")) //SELECT
{
	ile_option=8;
	m1= Const.s_Rot[nr-1].dob;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_Rot_int"))
{  sprintf(var2,"%02d:%02d",Const.s_Rot[nr-1].intrv/60,  Const.s_Rot[nr-1].intrv - 60*(Const.s_Rot[nr-1].intrv/60) );
}
else if(strstr(var,"s_Rot_sta"))
{  sprintf(var2,"%02d:%02d",Const.s_Rot[nr-1].sta/60,  Const.s_Rot[nr-1].sta - 60*(Const.s_Rot[nr-1].sta/60) );
}
else if(strstr(var,"s_Rot_max")){ wartTemp_odwrotnie(var2,Const.s_Rot[nr-1].max); }
else if(strstr(var,"s_Rot_min")){ wartTemp_odwrotnie(var2,Const.s_Rot[nr-1].min); }
else if(strstr(var,"s_Rot_hist")){ wartTemp_odwrotnie(var2,Const.s_Rot[nr-1].hist); }
else if(strstr(var,"s_Rot_rea"))
{  sprintf(var2,"%02d:%02d",Const.s_Rot[nr-1].reak/60,  Const.s_Rot[nr-1].reak - 60*(Const.s_Rot[nr-1].reak/60) );
}

//------------------------------- Ustaw GPRS ------------------------------------------------------
else if(strstr(var,"s_GPRSapn"))     {  sprintf(var2,Const.s_GPRS[nr-1].apn);         dopelnij_nazwy(var2,30); }
else if(strstr(var,"s_GPRSusr"))     {  sprintf(var2,Const.s_GPRS[nr-1].usr);         dopelnij_nazwy(var2,30); }
else if(strstr(var,"s_GPRShas"))     {  sprintf(var2,Const.s_GPRS[nr-1].has);         dopelnij_nazwy(var2,30); }

//------------------------------- Ustaw Siec ------------------------------------------------------

else if(strstr(var,"s_Lan_mac"))     {  sprintf(var2,Const.s_Lan[nr-1].eth);         dopelnij_nazwy(var2,12); }
else if(strstr(var,"s_Lan_ip"))      {  sprintf(var2,Const.s_Lan[nr-1].ip);          dopelnij_nazwy(var2,15); }
else if(strstr(var,"s_Lan_mask"))    {  sprintf(var2,Const.s_Lan[nr-1].mask);        dopelnij_nazwy(var2,15); }
else if(strstr(var,"s_Lan_br"))      {  sprintf(var2,Const.s_Lan[nr-1].br);          dopelnij_nazwy(var2,15); }
else if(strstr(var,"s_Lan_port"))    {  sprintf(var2,Const.s_Lan[nr-1].port);        dopelnij_nazwy(var2,5);  HttpCountOffset[0]=0;  }
else if(strstr(var,"s_Lan_param")) //SELECT
{
	ile_option=2;
	m1= Const.s_Lan[nr-1].param&0x01;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_Wifi_ssid"))
{   if(nr==1) sprintf(var2,Const.s_Wifi[0].ssid);   else  sprintf(var2,Const.s_Wifi[0].AP_ssid);
    dopelnij_nazwy(var2,40);
}
else if(strstr(var,"s_Wifi_haslo"))
{  if(nr==1) sprintf(var2,Const.s_Wifi[0].haslo);   else  sprintf(var2,Const.s_Wifi[0].AP_haslo);
   dopelnij_nazwy(var2,40);
}
//else if(strstr(var,"s_Wifi_AP_ssid")){   sprintf(var2,Const.s_Wifi[nr-1].AP_ssid);       dopelnij_nazwy(var2,40); }
//else if(strstr(var,"s_Wifi_AP_haslo")){  sprintf(var2,Const.s_Wifi[nr-1].AP_haslo);      dopelnij_nazwy(var2,40); }

else if(strstr(var,"s_Wifi_par1")) //SELECT Wifi
{
	ile_option=2;
	     if(nr==1) m1= ((Const.s_Lan[0].param>>2)&0x01);
	else if(nr==2) m1= ((Const.s_Lan[0].param>>3)&0x01);
	else goto omin_spr;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_email_ip")){   sprintf(var2,Const.s_Email[nr-1].ip);       dopelnij_nazwy(var2,40);  }
else if(strstr(var,"s_email_nad")){   sprintf(var2,Const.s_Email[nr-1].nad);       dopelnij_nazwy(var2,40);  }
else if(strstr(var,"s_email_has")){   sprintf(var2,Const.s_Email[nr-1].has);       dopelnij_nazwy(var2,40);  }
else if(strstr(var,"s_email_port")){   sprintf(var2,Const.s_Email[nr-1].port);       dopelnij_nazwy(var2,40);  }
else if(strstr(var,"s_email_odb"))
{  ptr3=strstr(var,"s_email_odb");     k=(*(ptr3+11))&0x0f;  k--;
   sprintf(var2,Const.s_Email[nr-1].odb1+41*k);    dopelnij_nazwy(var2,40);
}
else if(strstr(var,"s_email_tryb")) //SELECT
{
	ile_option=2;
	m1= (Const.s_Lan[0].param>>4)&0x01;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}

/*else if(strstr(var,"s_email_odb1")){  sprintf(var2,Const.s_Email[nr-1].odb1);      dopelnij_nazwy(var2,40); }
else if(strstr(var,"s_email_odb2")){  sprintf(var2,Const.s_Email[nr-1].odb2);      dopelnij_nazwy(var2,40); }
else if(strstr(var,"s_email_odb3")){  sprintf(var2,Const.s_Email[nr-1].odb3);      dopelnij_nazwy(var2,40); }
else if(strstr(var,"s_email_odb4")){  sprintf(var2,Const.s_Email[nr-1].odb4);      dopelnij_nazwy(var2,40); }
*/

//------------------------------- Ustaw GSM ------------------------------------------------------
else if(strstr(var,"s_nrtel"))
{  ptr3=strstr(var,"s_nrtel");  k=(*(ptr3+7))&0x0f;
        sprintf(var2,Const.s_Tel[nr-1].tel0+17*k);       dopelnij_nazwy(var2,17);
}

//------------------------------- Ustaw Etr Impuls ------------------------------------------------------
else if(strstr(var,"s_Imp_licz")){ sprintf(var2,"%d",Const.s_Imp[nr-1].licz); dopelnij_nazwy(var2,5); }
else if(strstr(var,"s_Imp_wart")){ sprintf(var2,"%d",Const.s_Imp[nr-1].wart); dopelnij_nazwy(var2,5); }
else if(strstr(var,"s_Imp_jedn")){ sprintf(var2,"%s",Const.s_Imp[nr-1].jedn); dopelnij_nazwy(var2,5); }

else if(strstr(var,"s_pEtrgd1")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog1_dzies[1]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrgu1")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog1_ulam[1],var2); dopelnij_nazwy(var2,5); }
else if(strstr(var,"s_pEtrgd2")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog2_dzies[1]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrgu2")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog2_ulam[1],var2);  dopelnij_nazwy(var2,5); }

else if(strstr(var,"s_pEtrdd1")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog1_dzies[2]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrdu1")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog1_ulam[2],var2);  dopelnij_nazwy(var2,5); }
else if(strstr(var,"s_pEtrdd2")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog2_dzies[2]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrdu2")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog2_ulam[2],var2);  dopelnij_nazwy(var2,5); }

else if(strstr(var,"s_pEtrtd1")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog1_dzies[3]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrtu1")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog1_ulam[3],var2);  dopelnij_nazwy(var2,5); }
else if(strstr(var,"s_pEtrtd2")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog2_dzies[3]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrtu2")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog2_ulam[3],var2);  dopelnij_nazwy(var2,5); }

else if(strstr(var,"s_pEtrmd1")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog1_dzies[4]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrmu1")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog1_ulam[4],var2);  dopelnij_nazwy(var2,5); }
else if(strstr(var,"s_pEtrmd2")){ sprintf(var2,"%d",Const.s_Imp[nr-1].prog2_dzies[4]); dopelnij_nazwy(var2,7); }
else if(strstr(var,"s_pEtrmu2")){ FormatHttpImpUlam(nr-1,Const.s_Imp[nr-1].prog2_ulam[4],var2);  dopelnij_nazwy(var2,5); }

//------------------------------- Tabela TT  ------------------------------------------------------
else if(strstr(var,"s_tt"))   //n_s_ttd3_008
{  ptr3=strstr(var,"s_tt");
   i=  (*(ptr3+5))&0x0f;
   j= Cgi_liczba(ptr3);

        if(*(ptr3+4)=='d'){ sprintf(buf_p,"%04d",Const.s_TT[i].dd[j]&0x7FFF); sprintf(var2,buf_p);  }
   else if(*(ptr3+4)=='m'){ sprintf(buf_p,"%02d",Const.s_TT[i].mm[j]); sprintf(var2,buf_p);  }
   else if(*(ptr3+4)=='p')
   {
	  if((Const.s_TT[i].dd[j]&0x8000)==0) sprintf(var2,"+");
	  else                                sprintf(var2,"-");
   }
   else if(*(ptr3+4)=='j'){ sprintf(var2,Const.s_TT[i].jm); dopelnij_nazwy(var2,5); }

}

//--------------------------------- Rotacja  -------------------------------------   n_s_Rot_akt001_001
else if(strstr(var,"s_Rot_akt"))
{
	i= 100*(var[9]&0x0f)+10*(var[10]&0x0f)+(var[11]&0x0f);
	h= 100*(var[13]&0x0f)+10*(var[14]&0x0f)+(var[15]&0x0f);
	       if(ptr1=strstr(ptr,"regular-checkbox checkbox"))
		   {
			  if(((Const.s_Rot[0].ma[h-1]>>(i-1))&0x01)>0) sprintf(var2,"checked");
	          else                                         sprintf(var2,"       ");
			  j=strlen(var2);
			  for(i=0;i<j;i++)  *(ptr1+27+i)=var2[i];   //n_s_Rot_akt001_001" class="regular-checkbox checkbox" checked
		   }
		   ptr=ptr1+7;
		   goto omin_spr;
}

//--------------------------------- Maski SMS  -------------------------------------   n_s_MaS_akt001_001
else if(strstr(var,"s_MaS_akt"))
{
	i= 100*(var[9]&0x0f)+10*(var[10]&0x0f)+(var[11]&0x0f);
 	h=WyszukujPortElement(t_te,_Size_te,i);   //lub  t_we
	       if(ptr1=strstr(ptr,"regular-checkbox checkbox"))
		   {
			  if(((Const.s_Ma[nr-1].sms[h/16]>>(h-16*(h/16)))&0x01)>0) sprintf(var2,"checked");
	          else                                                     sprintf(var2,"       ");
			  j=strlen(var2);
			  for(i=0;i<j;i++)  *(ptr1+27+i)=var2[i];   //n_s_MaS_akt1_003" class="regular-checkbox checkbox" checked
		   }
		   ptr=ptr1+7;
		   goto omin_spr;
}

//--------------------------------- Dzwonienie Maski  -------------------------------------   n_s_MaS_akt001_001
else if(strstr(var,"s_xerrr"))
{
	i= 100*(var[7]&0x0f)+10*(var[8]&0x0f)+(var[9]&0x0f);
	       if(ptr1=strstr(ptr,"regular-checkbox checkbox"))
		   {
			  if(Const.s_Dzw[0].ma[i-1]>0) sprintf(var2,"checked");
	          else                         sprintf(var2,"       ");
			  j=strlen(var2);
			  for(i=0;i<j;i++)  *(ptr1+27+i)=var2[i];   //n_s_MaS_akt1_003" class="regular-checkbox checkbox" checked
		   }
		   ptr=ptr1+7;
		   goto omin_spr;
}
//--------------------------------- Raport SMS Maski  -------------------------------------
else if(strstr(var,"s_derrr"))
{
	i= 100*(var[7]&0x0f)+10*(var[8]&0x0f)+(var[9]&0x0f);
	       if(ptr1=strstr(ptr,"regular-checkbox checkbox"))
		   {
			  if(Const.s_Rap[0].sms[i-1]>0) sprintf(var2,"checked");
	          else                           sprintf(var2,"       ");
			  j=strlen(var2);
			  for(i=0;i<j;i++)  *(ptr1+27+i)=var2[i];   //n_s_MaS_akt1_003" class="regular-checkbox checkbox" checked
		   }
		   ptr=ptr1+7;
		   goto omin_spr;
}
//--------------------------------- Raport EMAIL Maski  -------------------------------------
else if(strstr(var,"s_merrr"))
{
	i= 100*(var[7]&0x0f)+10*(var[8]&0x0f)+(var[9]&0x0f);
	       if(ptr1=strstr(ptr,"regular-checkbox checkbox"))
		   {
			  if(Const.s_Rap[0].mail[i-1]>0) sprintf(var2,"checked");
	          else                           sprintf(var2,"       ");
			  j=strlen(var2);
			  for(i=0;i<j;i++)  *(ptr1+27+i)=var2[i];   //n_s_MaS_akt1_003" class="regular-checkbox checkbox" checked
		   }
		   ptr=ptr1+7;
		   goto omin_spr;
}
 //--------------------------------- Maski E-MAIL  -------------------------------------
     else if(strstr(var,"s_MaE_akt"))
     {
     	i= 100*(var[9]&0x0f)+10*(var[10]&0x0f)+(var[11]&0x0f);
     	h=WyszukujPortElement(t_te,_Size_te,i);   //lub  t_we
     	       if(ptr1=strstr(ptr,"regular-checkbox checkbox"))
     		   {
     			  if(((Const.s_Ma[nr-1].mail[h/16]>>(h-16*(h/16)))&0x01)>0) sprintf(var2,"checked");
     	          else                                                      sprintf(var2,"       ");
     			  j=strlen(var2);
     			  for(i=0;i<j;i++)  *(ptr1+27+i)=var2[i];
     		   }
     		   ptr=ptr1+7;
     		   goto omin_spr;
     }


//------------------------------- INfo GSM ------------------------------------------------------
else if(strstr(var,"s_M590Einfo")){     sprintf(var2,info_gsm);       dopelnij_nazwy(var2,30);  }
else if(strstr(var,"s_M590Ezasieg")){   sprintf(var2,zasieg_gsm);      }

//------------------------------- Odswiezanie strony ------------------------------------------------------
else if(strstr(var,"s_Poz_refr"))
{  sprintf(var2,"%02d:%02d",Const.s_Poz[nr-1].refr/60,  Const.s_Poz[nr-1].refr - 60*(Const.s_Poz[nr-1].refr/60) );
}

//------------------------------- Haslo do WWW ------------------------------------------------------
else if(strstr(var,"s_log")){  sprintf(var2,Const.s_Log[0].login);    dopelnij_nazwy(var2,30);}
else if(strstr(var,"s_has")){  sprintf(var2,Const.s_Log[0].haslo);    dopelnij_nazwy(var2,30);}

//------------------------------- Termostat ------------------------------------------------------
else if(strstr(var,"s_Term_prog")){ if(Const.s_GPIO[  Const.s_Term[nr-1].nr_temp-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Term[nr-1].prog); else wartTemp_odwrotnie(var2,Const.s_Term[nr-1].prog); }
else if(strstr(var,"s_Term_hist")){ if(Const.s_GPIO[  Const.s_Term[nr-1].nr_temp-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Term[nr-1].hist); else wartTemp_odwrotnie(var2,Const.s_Term[nr-1].hist); }
else if(strstr(var,"s_Term_czas_reak"))
{  sprintf(var2,"%02d:%02d",Const.s_Term[nr-1].czas_reak/60,  Const.s_Term[nr-1].czas_reak - 60*(Const.s_Term[nr-1].czas_reak/60) );
}
/*else if(strstr(var,"s_Term_akt")) //SELECT
{
	ile_option=2;
	m1= Const.s_Term[nr-1].akt&0x01;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}*/
else if(strstr(var,"s_Term_nr_wy"))
{
	if(ptr1=strstr(ptr,"s_Term_nr_wy"));
	i= 10*(bufff[1]&0x0f)+(bufff[2]&0x0f)-1;
	sprintf(buf_p,"%02d",t_wy[i]);
	*(ptr1+18)=buf_p[0];   *(ptr1+19)=buf_p[1];
	ptr=ptr1+7;
	goto omin_spr;
}
//else if(strstr(var,"s_Term_nr_temp"))  sprintf(var2,"%02d",Const.s_Term[nr-1].nr_temp);



//------------------------------- Czas ------------------------------------------------------
else if(strstr(var,"s_Czas_godz")){ if(delay_funkc[7]>0){  buf_p[0]=czas[0];buf_p[1]=czas[1];buf_p[2]=czas[2];buf_p[4]=czas[4];buf_p[5]=czas[5];buf_p[6]=czas[6]; } else  PobierzCzas(buf_p); sprintf(var2,"%02d",buf_p[4]); }
else if(strstr(var,"s_Czas_min")){  sprintf(var2,"%02d",buf_p[5]); }
else if(strstr(var,"s_Czas_sec")){  sprintf(var2,"%02d",buf_p[6]); }
else if(strstr(var,"s_Czas_rok")){  sprintf(var2,"%02d",buf_p[0]); }
else if(strstr(var,"s_Czas_mies")){  sprintf(var2,"%02d",buf_p[1]); }
else if(strstr(var,"s_Czas_dzien")){  sprintf(var2,"%02d",buf_p[2]); }
else if(strstr(var,"s_Czas_tyg")){ WyznaczDzienTygodnia(); sprintf(var2,&buf_p[90]); }



//------------------------------- Raport o godz ------------------------------------------------------
else if(strstr(var,"s_RR_godzON1")){ sprintf(var2,"%02d:%02d",Const.s_Rap[0].godzON1/60,  Const.s_Rap[0].godzON1 - 60*(Const.s_Rap[0].godzON1/60) ); }

//------------------------------- Timer ------------------------------------------------------
else if(strstr(var,"s_Tim_godzON1")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzON1/3600,  (Const.s_Tim[nr-1].godzON1 - 3600*(Const.s_Tim[nr-1].godzON1/3600))/60,      Const.s_Tim[nr-1].godzON1 -3600*(Const.s_Tim[nr-1].godzON1/3600) - 60*((Const.s_Tim[nr-1].godzON1-3600*(Const.s_Tim[nr-1].godzON1/3600))/60)         ); }
else if(strstr(var,"s_Tim_godzON2")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzON2/3600,  (Const.s_Tim[nr-1].godzON2 - 3600*(Const.s_Tim[nr-1].godzON2/3600))/60,      Const.s_Tim[nr-1].godzON2 -3600*(Const.s_Tim[nr-1].godzON2/3600) - 60*((Const.s_Tim[nr-1].godzON2-3600*(Const.s_Tim[nr-1].godzON2/3600))/60)          ); }
else if(strstr(var,"s_Tim_godzON3")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzON3/3600,  (Const.s_Tim[nr-1].godzON3 - 3600*(Const.s_Tim[nr-1].godzON3/3600))/60,      Const.s_Tim[nr-1].godzON3 -3600*(Const.s_Tim[nr-1].godzON3/3600) - 60*((Const.s_Tim[nr-1].godzON3-3600*(Const.s_Tim[nr-1].godzON3/3600))/60)          ); }
else if(strstr(var,"s_Tim_godzON4")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzON4/3600,  (Const.s_Tim[nr-1].godzON4 - 3600*(Const.s_Tim[nr-1].godzON4/3600))/60,      Const.s_Tim[nr-1].godzON4 -3600*(Const.s_Tim[nr-1].godzON4/3600) - 60*((Const.s_Tim[nr-1].godzON4-3600*(Const.s_Tim[nr-1].godzON4/3600))/60)          ); }

else if(strstr(var,"s_Tim_godzOF1")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzOF1/3600,  (Const.s_Tim[nr-1].godzOF1 - 3600*(Const.s_Tim[nr-1].godzOF1/3600))/60,      Const.s_Tim[nr-1].godzOF1 -3600*(Const.s_Tim[nr-1].godzOF1/3600) - 60*((Const.s_Tim[nr-1].godzOF1-3600*(Const.s_Tim[nr-1].godzOF1/3600))/60)          ); }
else if(strstr(var,"s_Tim_godzOF2")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzOF2/3600,  (Const.s_Tim[nr-1].godzOF2 - 3600*(Const.s_Tim[nr-1].godzOF2/3600))/60,      Const.s_Tim[nr-1].godzOF2 -3600*(Const.s_Tim[nr-1].godzOF2/3600) - 60*((Const.s_Tim[nr-1].godzOF2-3600*(Const.s_Tim[nr-1].godzOF2/3600))/60)          ); }
else if(strstr(var,"s_Tim_godzOF3")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzOF3/3600,  (Const.s_Tim[nr-1].godzOF3 - 3600*(Const.s_Tim[nr-1].godzOF3/3600))/60,      Const.s_Tim[nr-1].godzOF3 -3600*(Const.s_Tim[nr-1].godzOF3/3600) - 60*((Const.s_Tim[nr-1].godzOF3-3600*(Const.s_Tim[nr-1].godzOF3/3600))/60)          ); }
else if(strstr(var,"s_Tim_godzOF4")){ sprintf(var2,"%02d:%02d:%02d",Const.s_Tim[nr-1].godzOF4/3600,  (Const.s_Tim[nr-1].godzOF4 - 3600*(Const.s_Tim[nr-1].godzOF4/3600))/60,      Const.s_Tim[nr-1].godzOF4 -3600*(Const.s_Tim[nr-1].godzOF4/3600) - 60*((Const.s_Tim[nr-1].godzOF4-3600*(Const.s_Tim[nr-1].godzOF4/3600))/60)          ); }

else if(strstr(var,"s_Tim_akt1")||strstr(var,"s_Tim_akt2")||strstr(var,"s_Tim_akt3")||strstr(var,"s_Tim_akt4")||strstr(var,"s_Tim_akt5")||strstr(var,"s_Tim_akt6")||strstr(var,"s_Tim_akt7"))
{
	     if(strstr(var,"s_Tim_akt1")) i=0;
	else if(strstr(var,"s_Tim_akt2")) i=1;
	else if(strstr(var,"s_Tim_akt3")) i=2;
	else if(strstr(var,"s_Tim_akt4")) i=3;
	else if(strstr(var,"s_Tim_akt5")) i=4;
	else if(strstr(var,"s_Tim_akt6")) i=5;
	else if(strstr(var,"s_Tim_akt7")) i=6;

	       if(ptr1=strstr(ptr,"regular-checkbox checkbox"))
		   {
			  if(((Const.s_Tim[nr-1].akt>>i)&0x01)>0) sprintf(var2,"checked");
	          else                                    sprintf(var2,"       ");
			  j=strlen(var2);
			  for(i=0;i<j;i++)  *(ptr1+27+i)=var2[i];
		   }
		   ptr=ptr1+7;
		   goto omin_spr;
}

else if(strstr(var,"s_Tim_akt8")) //SELECT
{
	ile_option=2;
	m1= (Const.s_Tim[nr-1].akt>>7)&0x01;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
/*
else if(strstr(var,"n_s_checkbox1")||strstr(var,"n_s_checkbox2")||strstr(var,"n_s_checkbox3")||strstr(var,"n_s_checkbox4")||strstr(var,"n_s_checkbox5")||strstr(var,"n_s_checkbox6")||strstr(var,"n_s_checkbox7"))
{
	ptr=ptr1+7;
	goto omin_spr;
}*/
//------------------------------- Nazwa Portów ------------------------------------------------------
else if(strstr(var,"s_Port_nazwa"))
{
   ptr3=strstr(var,"s_Port_nazwa");  k=wybierz_cyfry(ptr3+12);   k--;
   sprintf(var2,Const.s_GPIO[k].nazwa);    dopelnij_nazwy(var2,30);
}

  /*           else if(strstr(var,"s_Port_nazwa1")){ sprintf(var2,Const.s_GPIO[0].nazwa);    dopelnij_nazwy(var2,30);}
             else if(strstr(var,"s_Port_nazwa2")){  sprintf(var2,Const.s_GPIO[1].nazwa);    dopelnij_nazwy(var2,30);}
             else if(strstr(var,"s_Port_nazwa3")){  sprintf(var2,Const.s_GPIO[2].nazwa);    dopelnij_nazwy(var2,30);}
             else if(strstr(var,"s_Port_nazwa4")){  sprintf(var2,Const.s_GPIO[3].nazwa);    dopelnij_nazwy(var2,30);}
             else if(strstr(var,"s_Port_nazwa5")){  sprintf(var2,Const.s_GPIO[4].nazwa);    dopelnij_nazwy(var2,30);}
             else if(strstr(var,"s_Port_nazwa6")){  sprintf(var2,Const.s_GPIO[5].nazwa);    dopelnij_nazwy(var2,30);}
             else if(strstr(var,"s_Port_nazwa7")){  sprintf(var2,Const.s_GPIO[6].nazwa);    dopelnij_nazwy(var2,30);}
             else if(strstr(var,"s_Port_nazwa8")){  sprintf(var2,Const.s_GPIO[7].nazwa);    dopelnij_nazwy(var2,30);}
*/

//------------------------------- Rejestrator ----------------------------------------------------------'
/*else if(strstr(var,"s_Rej_intr")){
   sprintf(var2,"%02d:%02d",Const.s_Rej[nr-1].intr/60,  Const.s_Rej[nr-1].intr - 60*(Const.s_Rej[nr-1].intr/60) );
}
else if(strstr(var,"s_Rej_akt")) //SELECT
{
	ile_option=2;
	m1= (Const.s_Rej[nr-1].param)&0x01;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}*/

else if(strstr(var,"s_Rej_"))
{ ptr1= strstr(var,"s_Rej_");
  if(*(ptr1+6)=='i'){  sprintf(var2,"%02d:%02d",Const.s_Rej[nr-1].intr/60,  Const.s_Rej[nr-1].intr - 60*(Const.s_Rej[nr-1].intr/60) );  }
  else
  {
	  if(*(ptr1+6)=='a') k=0; else k=1;

		ile_option=2;
		m1= (Const.s_Rej[nr-1].param>>k)&0x01;

	    for(m=0;m<ile_option;m++)
	    {
		   if(ptr1=strstr(ptr,"value=\""))
		   {
	          if(m==m1) sprintf(var2,"selected");
	          else      sprintf(var2,"        ");
			  j=strlen(var2);
			  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
		   }
		   ptr=ptr1+7;
	    }
	    goto omin_spr;
  }
}
//---------------- Wykres ----------------------
else if(strstr(var,"s_Wykr_prg")) //SELECT
{
	if(strstr(var,"s_Wykr_prgmax"))
	{
		     if(Const.s_Wykr[Var.wykres_nr].prgmax==10) m1=19;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==20) m1=18;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==30) m1=17;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==40) m1=16;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==50) m1=15;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==60) m1=14;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==70) m1=13;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==80) m1=12;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==90) m1=11;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==100) m1=10;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==200) m1=9;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==300) m1=8;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==400) m1=7;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==500) m1=6;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==600) m1=5;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==700) m1=4;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==800) m1=3;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==900) m1=2;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==1000) m1=1;
		else if(Const.s_Wykr[Var.wykres_nr].prgmax==1100) m1=0;

		ile_option=20;

	}
	else{ m1= (Const.s_Wykr[Var.wykres_nr].prgmin/10)-0;     ile_option=21;  }

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_Wykr_zaw"))
{
	sprintf(var2,"%d",Const.s_Wykr[Var.wykres_nr].zawezen ); //sprintf(buf_p,"\r\nXXXX: %d ", nr-1); dbg3(buf_p);
}
else if(strstr(var,"s_Wykr_sze"))
{
	sprintf(var2,"%d",Const.s_Wykr[Var.wykres_nr].szerwykr );
}

//---------------- Wykres Liczniki ----------------------
else if(strstr(var,"s_WykrEtr_prg")) //SELECT
{
	if(strstr(var,"s_WykrEtr_prgmax"))
	{
		    if(Const.s_WykrEtr[WykrNrEtr].prgmax==1) m1=20;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==10) m1=19;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==20) m1=18;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==30) m1=17;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==40) m1=16;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==50) m1=15;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==60) m1=14;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==70) m1=13;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==80) m1=12;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==90) m1=11;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==100) m1=10;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==200) m1=9;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==300) m1=8;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==400) m1=7;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==500) m1=6;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==600) m1=5;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==700) m1=4;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==800) m1=3;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==900) m1=2;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==1000) m1=1;
	   else if(Const.s_WykrEtr[WykrNrEtr].prgmax==1100){ m1=0;  }

		ile_option=21;

	}
	else{ m1=0;     ile_option=1;  }

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_WykrEtr_zaw"))
{
	sprintf(var2,"%d",Const.s_WykrEtr[WykrNrEtr].zawezen ); //sprintf(buf_p,"\r\nXXXX: %d ", nr-1); dbg3(buf_p);
}
else if(strstr(var,"s_WykrEtr_sze"))
{
	sprintf(var2,"%d",Const.s_WykrEtr[WykrNrEtr].szerwykr );
}

//--------------------------------- Raportowanie --------------------------------------------------------
else if(strstr(var,"s_RRapZawSel")) //SELECT
{
	ile_option=3;
	m1= Const.s_Rap[nr-1].plik&0x03;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+3+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_RRapSiecSel")) //SELECT
{
	ile_option=3;
	m1= Const.s_Rap[nr-1].siec&0x03;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+3+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_RRapTimSel")) //SELECT
{
	ile_option=8;
	m1= Const.s_Rap[nr-1].czas&0x07;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+3+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}
else if(strstr(var,"s_RRapTimGodz")){
	 sprintf(var2,"%02d:%02d",Const.s_Rap[0].godzina/60,  Const.s_Rap[0].godzina - 60*(Const.s_Rap[0].godzina/60) );
}

//------------------------------- Wejscia Cyfrowe ------------------------------------------------------
else if(strstr(var,"s_WE_nazwa_zw")){  sprintf(var2,Const.s_WE[nr-1].nazwa_zw);  dopelnij_nazwy(var2,20);}
else if(strstr(var,"s_WE_nazwa_ro")){  sprintf(var2,Const.s_WE[nr-1].nazwa_ro);  dopelnij_nazwy(var2,20);}
else if(strstr(var,"s_WE_nazwa_uszk")){  sprintf(var2,Const.s_WE[nr-1].nazwa_uszk);  dopelnij_nazwy(var2,20);}
else if(strstr(var,"s_WE_nazwa"))   {  sprintf(var2,Const.s_WE[nr-1].nazwa);     dopelnij_nazwy(var2,30);}
else if(strstr(var,"s_WE_czas_reak")){
   sprintf(var2,"%02d:%02d",Const.s_WE[nr-1].czas_reak/60,  Const.s_WE[nr-1].czas_reak - 60*(Const.s_WE[nr-1].czas_reak/60) );
}
else if(strstr(var,"s_WE_ilosc_alarm")) sprintf(var2,"%02d",Const.s_WE[nr-1].ilosc_alarm);
//else if(strstr(var,"s_WE_przek")) sprintf(var2,"%02d",Const.s_WE[nr-1].pk);
else if(strstr(var,"s_WE_czas_przyt")){
	sprintf(var2,"%02d:%02d",Const.s_WE[nr-1].czas_przyt/60,  Const.s_WE[nr-1].czas_przyt - 60*(Const.s_WE[nr-1].czas_przyt/60) );
}
else if(strstr(var,"s_WE_param1")) //SELECT
{
	ile_option=3;
	m1= Const.s_WE[nr-1].param&0x03;

    for(m=0;m<ile_option;m++)
    {
	   if(ptr1=strstr(ptr,"value=\""))
	   {
          if(m==m1) sprintf(var2,"selected");
          else      sprintf(var2,"        ");
		  j=strlen(var2);
		  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
	   }
	   ptr=ptr1+7;
    }
    goto omin_spr;
}

//------------------------------- Wyjscia Cyfrowe ------------------------------------------------------
     else if(strstr(var,"s_PK_nazwa_on")){  sprintf(var2,Const.s_PK[nr-1].nazwa_on);  dopelnij_nazwy(var2,20);}
     else if(strstr(var,"s_PK_nazwa_of")){  sprintf(var2,Const.s_PK[nr-1].nazwa_of);  dopelnij_nazwy(var2,20);}
     else if(strstr(var,"s_PK_nazwa"))   {  sprintf(var2,Const.s_PK[nr-1].nazwa);     dopelnij_nazwy(var2,30);}



//------------------------------- Czujki temp ------------------------------------------------------

            else if(strstr(var,"s_Czujki_nazwa")){  sprintf(var2,Const.s_Czujki[nr-1].nazwa);    dopelnij_nazwy(var2,30);}
            else if(strstr(var,"s_Czujki_prog_max1")){ if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Czujki[nr-1].prog_max1); else wartTemp_odwrotnie(var2,Const.s_Czujki[nr-1].prog_max1); }
			else if(strstr(var,"s_Czujki_prog_max2")){ if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Czujki[nr-1].prog_max2); else wartTemp_odwrotnie(var2,Const.s_Czujki[nr-1].prog_max2); }
			else if(strstr(var,"s_Czujki_prog_min1")){ if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Czujki[nr-1].prog_min1); else wartTemp_odwrotnie(var2,Const.s_Czujki[nr-1].prog_min1); }
			else if(strstr(var,"s_Czujki_prog_min2")){ if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Czujki[nr-1].prog_min2); else wartTemp_odwrotnie(var2,Const.s_Czujki[nr-1].prog_min2); }
			else if(strstr(var,"s_Czujki_hist")){      if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Czujki[nr-1].hist);      else wartTemp_odwrotnie(var2,Const.s_Czujki[nr-1].hist);      }
			else if(strstr(var,"s_Czujki_korekcja")){  if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt_http(var2,Const.s_Czujki[nr-1].korekcja);  else wartTemp_odwrotnie(var2,Const.s_Czujki[nr-1].korekcja);  }

			else if(strstr(var,"s_Czujki_czas_reak"))
			{
				sprintf(var2,"%02d:%02d",Const.s_Czujki[nr-1].czas_reak/60,  Const.s_Czujki[nr-1].czas_reak - 60*(Const.s_Czujki[nr-1].czas_reak/60) );

			}
			else if(strstr(var,"s_Czujki_ilosc_alarm"))
			{
				sprintf(var2,"%02d",Const.s_Czujki[nr-1].ilosc_alarm);

			}
			else if(strstr(var,"s_Czujki_alarm_pk")) //SELECT
			{
				ile_option=2;
				m1= Const.s_Czujki[nr-1].alarm_pk&0x01;

                for(m=0;m<ile_option;m++)
                {
				   if(ptr1=strstr(ptr,"value=\""))
				   {
                      if(m==m1) sprintf(var2,"selected");
                      else      sprintf(var2,"        ");
					  j=strlen(var2);
					  for(i=0;i<j;i++)  *(ptr1+7+5+i)=var2[i];
				   }
				   ptr=ptr1+7;
                }
                goto omin_spr;
			}

			else{ ptr2=ptr+4;  goto omin_spr2; }




//------------------------ END ASP -------------------------------------------------

			if(ptr1=strstr(ptr,"value=\""))
			{
				i=0; j=strlen(var2);
			    for(i=0;i<j;i++)  *(ptr1+7+i)=var2[i];

			    if(ptr=strstr(ptr1+7+i,"lan_disabled"))
			    {
			    	if(WhichPage==78) sprintf(buf_p,"        "); else sprintf(buf_p,"disabled");
		        	if((Const.s_Lan[0].param&0x01)>0){  for(i=0;i<strlen(buf_p);i++) *(ptr+i)=buf_p[i];  for(j=0;j<4;j++) *(ptr+i+j)=' '; }
		        	else                             {  for(j=0;j<12;j++) *(ptr+j)=' ';  }
			    }

			}
		    omin_spr:
		    asm("nop");

		    if(ptr=strstr(ptr2,"(x@)"))
		    {
		   	   *(ptr+0)='(';
		   	   *(ptr+1)='%';
		   	   *(ptr+2)=')';
		   	   *(ptr+3)=' ';
		   	   //ptr1=ptr+5;
		   	   //goto end_updatasoft;
		    }
		    if(ptr=strstr(ptr2,"(q@)"))
		    {
               if(wilgtemp==1)
               {
            	   *(ptr+0)='°';
            	   *(ptr+1)='C';
            	   *(ptr+2)=' ';
            	   *(ptr+3)=' ';
               }
               else
               {
            	   *(ptr+0)='%';
            	   *(ptr+1)=' ';
            	   *(ptr+2)=' ';
            	   *(ptr+3)=' ';
               }
		    }
		    if(ptr=strstr(ptr2,"##**"))
		    {
		    	           if((WhichPage==78)||((WhichPage==80)&&((Const.s_Rej[0].param&0x01)==0)))
		    	           {
		    	        	  *(ptr+0)='<';
		    	        	  *(ptr+1)='!';
		    	        	  *(ptr+2)='-';
		    	        	  *(ptr+3)='-';
		    	           }
		    	           else
		    	           {
		    	         	  *(ptr+0)=' ';
		    	         	  *(ptr+1)=' ';
		    	         	  *(ptr+2)=' ';
		    	         	  *(ptr+3)=' ';
		    	           }
		    	      	   //ptr1=ptr+5;
		    	      	  // goto end_updatasoft;
		    }
		    if(ptr=strstr(ptr2,"**@"))
		    {
		    	           if((WhichPage==78)||((WhichPage==80)&&((Const.s_Rej[0].param&0x01)==0)))
		    	           {
		    	        	  *(ptr+0)='-';
		    	        	  *(ptr+1)='-';
		    	        	  *(ptr+2)='>';
		    	           }
		    	           else
		    	           {
		    	         	  *(ptr+0)=' ';
		    	         	  *(ptr+1)=' ';
		    	         	  *(ptr+2)=' ';
		    	           }
		    	      	 //  ptr1=ptr+5;
		    	      	  // goto end_updatasoft;
		    }


		}
	    else if(ptr=strstr(ptr2,"n_GPIO_val_")) //SELECT
	    {
	    	ile_option=13;
	    	i= 100*((*(ptr+11))&0x0f)+10*((*(ptr+12))&0x0f)+((*(ptr+13))&0x0f);     o=i;  m1= Const.s_GPIO[i-1].val;

	    	/*     if(strstr(ptr2,"n_GPIO_val_01")){ o=1; m1= Const.s_GPIO[0].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_02")){ o=2; m1= Const.s_GPIO[1].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_03")){ o=3; m1= Const.s_GPIO[2].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_04")){ o=4; m1= Const.s_GPIO[3].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_05")){ o=5; m1= Const.s_GPIO[4].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_06")){ o=6; m1= Const.s_GPIO[5].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_07")){ o=7; m1= Const.s_GPIO[6].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_08")){ o=8; m1= Const.s_GPIO[7].val;  }
	    	else if(strstr(ptr2,"n_GPIO_val_09")){ o=9; m1= Const.s_GPIO[8].val;  }*/

	        for(k=0;k<ile_option;k++)
	        {
	    	        if(k==0) m=0;    //pk                   //Kolejnosc w select musi byc tu zgodna !!!!
	    	   else if(k==1) m=2;    // Temp
	    	   else if(k==2) m=3;   //Pt
	    	   else if(k==3) m=9;    //Temperatura Wilgotnosc
	    	   else if(k==4) m=10;   //timer
	    	   else if(k==5) m=11;   //termostat
	    	   else if(k==6) m=4;    //WE
	    	   else if(k==7) m=1;   //ster. PWM
	    	   else if(k==8) m=5;  //(0-10)V
	    	   else if(k==9) m=7;  // licznik impulsow
	    	   else if(k==10) m=8; // Sterowanie IR
	    	   else if(k==11) m=13; // WE EOL
	    	   else if(k==12) m=6; // ----

	           if((m==9)||(m==2)){             if(WyszukujPortElement(t_te,_Size_te,o)==-1) goto ikswwrfs;   }
	           if((m==0)||(m==10)||(m==11)){   if(WyszukujPortElement(t_wy,_Size_wy,o)==-1) goto ikswwrfs;   }
	           if(m==8)                    {   if(WyszukujPortElement(t_ir,_Size_ir,o)==-1) goto ikswwrfs;   }
	           if(m==4)        			   {   if(WyszukujPortElement(t_we,_Size_we,o)==-1) goto ikswwrfs;   }
	           if(m==13)         		   {   if(WyszukujPortElement(t_eol,_Size_eol,o)==-1) goto ikswwrfs;   }
	           if(m==3)                    {   if(WyszukujPortElement(t_pt,_Size_pt,o)==-1) goto ikswwrfs;   }
	           if(m==5)                    {   if(WyszukujPortElement(t_TT,_Size_TT,o)==-1) goto ikswwrfs;   }
	           if(m==1)                    {   if(WyszukujPortElement(t_pwm,_Size_pwm,o)==-1) goto ikswwrfs;   }
	           if(m==7)                    {   if(WyszukujPortElement(t_imp,_Size_imp,o)==-1) goto ikswwrfs;   }

	           if(ptr1=strstr(ptr,"value=\""))   //value="00p001"
	    	   {
	              if(m==m1){ sprintf(var2,"selected");  }
	              else      sprintf(var2,"        ");
	    		  j=strlen(var2);
	    		  for(i=0;i<j;i++)  *(ptr1+7+8+i)=var2[i];   //if(m==6){ for(i=0;i<20;i++) buf_p[i]=*(ptr1+i);  } buf_p[i]=0; dbg3(buf_p);
	    	   }
	    	   ptr=ptr1+7;
	    	   ikswwrfs:
			   asm("nop");
	        }
	        //goto omin_spr;
	    }
	    else if(ptr=strstr(ptr2,"active_id"))
	    {
	    	dbg3("\r\naaaaa");
	    }
	    else if(ptr=strstr(ptr2,"stan_adresuj"))  //adresownie czujników
        {
            if((wybor_bitowy&0x0002)>0)
            {
            	sprintf(var2,"Zaprogramowano");   i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr+i)=var2[i];
            }
            else
            {
            	sprintf(var2,"              ");   i=0; j=strlen(var2);   for(i=0;i<j;i++) *(ptr+i)=var2[i];
            }
            wybor_bitowy&=~0x0002;

        }
	    else break;
 //------------------KONIEC Shematu dla Czujników -------------------------------

	    end_updatasoft:


	    ptr2=ptr1;
	    omin_spr2:
		asm("nop");

   }while(1);

	return &buf_rx2[_Size_BufWifi];


}

unsigned char korekcja_formatu_GET(unsigned char *in, int len)
{
   int i;  int lop;
   for(i=0;i<len;i++)
   {   lop=2;

	         if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='2')&&(*(in+i+7)=='6')&&(*(in+i+8)=='1')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='a';  i++; lop=11; }  //¹
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='2')&&(*(in+i+7)=='8')&&(*(in+i+8)=='1')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='e';  i++; lop=11; }  //ê
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='8')&&(*(in+i+8)=='0')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='z';  i++; lop=11; }  //¿
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='2')&&(*(in+i+7)=='6')&&(*(in+i+8)=='3')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='c';  i++; lop=11; }  //c
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='4')&&(*(in+i+8)=='7')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='s';  i++; lop=11; }  //œ
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='2')&&(*(in+i+8)=='2')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='l';  i++; lop=11; }  //³
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='2')&&(*(in+i+8)=='4')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='n';  i++; lop=11; }  //ñ

	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='2')&&(*(in+i+7)=='6')&&(*(in+i+8)=='0')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='A';  i++; lop=11; }  //¥
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='2')&&(*(in+i+7)=='8')&&(*(in+i+8)=='0')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='E';  i++; lop=11; }  //Ê
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='7')&&(*(in+i+8)=='9')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='Z';  i++; lop=11; }  //¯
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='2')&&(*(in+i+7)=='6')&&(*(in+i+8)=='2')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='C';  i++; lop=11; }  //Æ
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='4')&&(*(in+i+8)=='6')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='S';  i++; lop=11; }  //œ
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='2')&&(*(in+i+8)=='1')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='L';  i++; lop=11; }  //£
	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')&&(*(in+i+3)=='%')&&(*(in+i+4)=='2')&&(*(in+i+5)=='3')&&(*(in+i+6)=='3')&&(*(in+i+7)=='2')&&(*(in+i+8)=='3')&&(*(in+i+9)=='%')&&(*(in+i+10)=='3')&&(*(in+i+11)=='B')){  *(in+i)='N';  i++; lop=11; }  //Ñ

	    else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='B')){  *(in+i)='+';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='4')&&(*(in+i+2)=='0')){  *(in+i)='@';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='8')){  *(in+i)='(';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='9')){  *(in+i)=')';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='1')){  *(in+i)='!';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='3')){  *(in+i)='#';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='4')){  *(in+i)='$';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='5')){  *(in+i)='%';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='6')){  *(in+i)='&';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='3')&&(*(in+i+2)=='D')){  *(in+i)='=';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='F')){  *(in+i)='/';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='3')&&(*(in+i+2)=='F')){  *(in+i)='?';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='3')&&(*(in+i+2)=='B')){  *(in+i)=';';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='2')&&(*(in+i+2)=='C')){  *(in+i)=',';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='5')&&(*(in+i+2)=='B')){  *(in+i)='[';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='5')&&(*(in+i+2)=='D')){  *(in+i)=']';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='7')&&(*(in+i+2)=='C')){  *(in+i)='|';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='3')&&(*(in+i+2)=='C')){  *(in+i)='<';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='3')&&(*(in+i+2)=='E')){  *(in+i)='>';  i++; }
        else if((*(in+i)=='%')&&(*(in+i+1)=='3')&&(*(in+i+2)=='A')){  *(in+i)=':';  i++; }

	 //polskie znaki
        else if((*(in+i)=='%')&&(*(in+i+1)=='A')&&(*(in+i+2)=='1')){  *(in+i)='A';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='C')&&(*(in+i+2)=='6')){  *(in+i)='Æ';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='C')&&(*(in+i+2)=='A')){  *(in+i)='Ê';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='A')&&(*(in+i+2)=='3')){  *(in+i)='£';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='D')&&(*(in+i+2)=='1')){  *(in+i)='Ñ';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='D')&&(*(in+i+2)=='3')){  *(in+i)='Ó';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='A')&&(*(in+i+2)=='6')){  *(in+i)='S';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='A')&&(*(in+i+2)=='C')){  *(in+i)='Z';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='A')&&(*(in+i+2)=='F')){  *(in+i)='¯';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='B')&&(*(in+i+2)=='1')){  *(in+i)='a';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='E')&&(*(in+i+2)=='6')){  *(in+i)='æ';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='E')&&(*(in+i+2)=='A')){  *(in+i)='ê';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='B')&&(*(in+i+2)=='3')){  *(in+i)='³';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='F')&&(*(in+i+2)=='1')){  *(in+i)='ñ';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='F')&&(*(in+i+2)=='3')){  *(in+i)='ó';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='B')&&(*(in+i+2)=='6')){  *(in+i)='s';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='B')&&(*(in+i+2)=='C')){  *(in+i)='z';  i++; }
		else if((*(in+i)=='%')&&(*(in+i+1)=='B')&&(*(in+i+2)=='F')){  *(in+i)='¿';  i++; }


		else goto goto_kor_for_Get;


				while ( *(in+i+lop) != 0)
                {
                    *(in+i) = *(in+i+lop);
                    i++;
				}
				*(in+i) = 0;
				return 1;

	   goto_kor_for_Get:
	   asm("nop");
   }
   return 0;
}

unsigned char wzor_wpisu_temp(unsigned char *out)
{
    unsigned char k,sa,wy[10];  k=0;

    if(out[0]=='.'){ sprintf(out,"+99.0"); return 0; }

	if((out[0]=='+')||(out[0]=='-')){ k=1; sa=out[0]; }
	else{ if((out[0]>0x2F)&&(out[0]<0x3A)){ k=0; sa='+';} else{ sprintf(out,"+99.0"); return 0; }  }

    if((out[0+k]>0x2F)&&(out[0+k]<0x3A))
	{   if((out[1+k]>0x2F)&&(out[1+k]<0x3A))
	    {
            if(out[2+k]=='.')
			{
			    if((out[3+k]>0x2F)&&(out[3+k]<0x3A)) { wy[0]=sa; wy[1]=out[0+k]; wy[2]=out[1+k]; wy[3]='.'; wy[4]=out[3+k];  wy[5]=0; }
                else                                 { wy[0]=sa; wy[1]=out[0+k]; wy[2]=out[1+k]; wy[3]='.'; wy[4]='0';       wy[5]=0; }
			}
			else
			{
			    wy[0]=sa; wy[1]=out[0+k]; wy[2]=out[1+k]; wy[3]='.'; wy[4]='0';    wy[5]=0;
			}
		}
		else
		{
			if(out[1+k]=='.')
			{
			    if((out[2+k]>0x2F)&&(out[2+k]<0x3A)) { wy[0]=sa; wy[1]='0'; wy[2]=out[0+k]; wy[3]='.'; wy[4]=out[2+k]; wy[5]=0; }
                else                                 { wy[0]=sa; wy[1]='0'; wy[2]=out[0+k]; wy[3]='.'; wy[4]='0';      wy[5]=0; }
		    }
			else
			{
                wy[0]=sa; wy[1]='0'; wy[2]=out[0+k]; wy[3]='.'; wy[4]='0';    wy[5]=0;
			}
		}
	}
    else{ sprintf(out,"+99.0"); return 0; }
	for(k=0;k<6;k++) out[k]=wy[k];
    return 1;
}

unsigned char wzor_wpisu_temp_Pt(unsigned char *out)
{
    unsigned char k,sa,wy[10];  k=0;

    if(out[0]=='.'){ sprintf(out,"+9999"); return 0; }

	if((out[0]=='+')||(out[0]=='-')){ k=1; sa=out[0]; }
	else{ if((out[0]>0x2F)&&(out[0]<0x3A)){ k=0; sa='+';} else{ sprintf(out,"+9999"); return 0; }  }

    if((out[0+k]>0x2F)&&(out[0+k]<0x3A))
	{   if((out[1+k]>0x2F)&&(out[1+k]<0x3A))
	    {   if((out[2+k]>0x2F)&&(out[2+k]<0x3A))
	        {
	    	    wy[0]=sa; wy[1]=out[0+k];  wy[2]=out[1+k]; wy[3]=out[2+k];  wy[4]=' '; wy[5]=0;
	        }
	        else
	        {
	        	wy[0]=sa; wy[1]='0'; wy[2]=out[0+k]; wy[3]=out[1+k];  wy[4]=' '; wy[5]=0;
	        }
		}
		else
		{
             wy[0]=sa; wy[1]='0'; wy[2]='0'; wy[3]=out[0+k];  wy[4]=' '; wy[5]=0;
		}
	}
    else{ sprintf(out,"+9999"); return 0; }
	for(k=0;k<6;k++) out[k]=wy[k];
    return 1;
}

unsigned char wzor_wpisu_minsec(unsigned char *out)
{
    unsigned char k,sa,wy[10];  k=0;   char i,min,sec;

    i=0;  min=0; sec=0;

    if(strstr(out,":"))
    {
        while(out[i]!=':') i++;

            if(i==1) min=(out[0]&0x0f);
       else if(i>1)  min=10*(out[0]&0x0f)+(out[1]&0x0f);

            if(out[i+1]==0) sec=0;
       else if(out[i+2]==0) sec=(out[i+1]&0x0f);
       else if(out[i+3]==0) sec=10*(out[i+1]&0x0f)+(out[i+2]&0x0f);

       out[0]=min;  out[1]=sec;
    }
    else
    {
    	     if(out[0]==0){  min=0;  sec=0; }
    	else if(out[1]==0){  min=out[0]&0x0f;  sec=0; }
    	else if(out[2]==0){  min=10*(out[0]&0x0f)+(out[1]&0x0f);  sec=0; }
    	else              {  min=10*(out[0]&0x0f)+(out[1]&0x0f);  sec=0; }

    	out[0]=min;  out[1]=sec;
    }
}
unsigned char wzor_wpisu_godzminsec(unsigned char *out)
{
    unsigned char sa,wy[10];    char i,j,k,godz,min,sec;  char *po;

    i=0; j=0; k=0;  godz=0; min=0; sec=0;

    if(po=strstr(out,":"))
    {
    	if(strstr(po+1,":"))  //00:00:00
    	{
			while(out[i]!=':') i++;

			if(i==1){ godz=(out[k]&0x0f);  k++; }
			else if(i>1){  godz=10*(out[k]&0x0f)+(out[k+1]&0x0f);  k++;k++; }

			while(out[i+1]!=':'){ i++; j++; }
			k++;

			if(j==1){ min=(out[k]&0x0f);  k++; }
			else if(j>1){  min=10*(out[k]&0x0f)+(out[k+1]&0x0f);  k++;k++; }

			k++;

				 if(out[k+0]==0) sec=0;
			else if(out[k+1]==0) sec=(out[k]&0x0f);
			else if(out[k+2]==0) sec=10*(out[k]&0x0f)+(out[k+1]&0x0f);

			out[0]=godz; out[1]=min;  out[2]=sec;

    	}
    	else
    	{
    			while(out[i]!=':') i++;

    			godz=0;

    			if(i==1) min=(out[0]&0x0f);
    			else if(i>1)  min=10*(out[0]&0x0f)+(out[1]&0x0f);

    			if(out[i+1]==0) sec=0;
    			else if(out[i+2]==0) sec=(out[i+1]&0x0f);
    			else if(out[i+3]==0) sec=10*(out[i+1]&0x0f)+(out[i+2]&0x0f);

    			out[0]=godz; out[1]=min;  out[2]=sec;

    	}
    }
    else
    {
    	     if(out[0]==0){  godz=0;  min=0;  sec=0; }
    	else if(out[1]==0){  godz=0;  min=0;  sec=out[0]&0x0f; }
    	else if(out[2]==0){  godz=0;  min=0;  sec=10*(out[0]&0x0f)+(out[1]&0x0f); }
    	else              {  godz=0;  min=0;  sec=10*(out[0]&0x0f)+(out[1]&0x0f); }

    	out[0]=godz;  out[1]=min;  out[2]=sec;
    }
}

void Kasuj_Czujki_port(char port)
{
	int i,j;
	for(i=0;i<_Size__s_Czujki_info;i++)
	{
		if(Const.s_Czujki[i].addr_DS[8]==port)
		{
			Const.s_Czujki[i].addr_DS[8]=0;
			for(j=0;j<8;j++) Const.s_Czujki[i].addr_DS[j]=0;
		}
	}
}
void Wprowaz_Jedno_Temp(char port)
{
	int i,j;
	for(i=0;i<_Size__s_Czujki_info;i++)
	{
		if(Const.s_Czujki[i].addr_DS[8]==0) break;
	}
	if(i<_Size__s_Czujki_info)
	{
		Const.s_Czujki[i].addr_DS[8]=port;
		for(j=0;j<8;j++) Const.s_Czujki[i].addr_DS[j]=0;
	}
}



void Wprowaz_Temp_i_Wilg(char port)
{
	int i,j;

	for(i=0;i<_Size__s_Czujki_info;i++){ if(Const.s_Czujki[i].addr_DS[8]==0) break; }
	if(i<(_Size__s_Czujki_info-1))
	{
		Const.s_Czujki[i].addr_DS[8]=port;
		for(j=0;j<8;j++) Const.s_Czujki[i].addr_DS[j]=0;

		for(i=0;i<_Size__s_Czujki_info;i++){ if(Const.s_Czujki[i].addr_DS[8]==0) break; }

		Const.s_Czujki[i].addr_DS[8]=port;
		for(j=0;j<8;j++) Const.s_Czujki[i].addr_DS[j]=0;
	}
}
void GPIO_PK(int p)
{
    int rr;
	GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;

  if((Const.s_GPIO[t_wy[p]-1].val==0)||(Const.s_GPIO[t_wy[p]-1].val==10)||(Const.s_GPIO[t_wy[p]-1].val==11))  //PK , Term , Tim
  {
	  dbg3("  GPIO_PK");

	  NrPortuNaPortZewn(t_wy[p], &GPIOx, &GPIO_Pin);

	  if((Const.s_WY[p/16].val&(1<<(p-16*(p/16))))>0){  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);  }
	  else                                           {  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET); }

  }
  else if(Const.s_GPIO[t_wy[p]-1].val==8)     //IR
  {
	  dbg3("  GPIO_IR");

	  NrPortuNaPortZewn(t_wy[p], &GPIOx, &GPIO_Pin);

	  rr = WyszukujPortElement(t_ir, _Size_ir, t_wy[p]);
	  if(rr<_Size_ir){
	     if((Const.s_WY[p/16].val&(1<<(p-16*(p/16))))>0)
	     {
		    sprintf(buf_p,"\r\nIR zal %d  ",rr); dbg3(buf_p);
		    if(Flash_read_to_BufIrda(2*rr)==1) wyslij_z_buf_na_podczer(t_wy[p]);
	     }
	     else
	     {
		    sprintf(buf_p,"\r\nIR wyl %d  ",rr); dbg3(buf_p);
		    if(Flash_read_to_BufIrda(2*rr+1)==1) wyslij_z_buf_na_podczer(t_wy[p]);
	     }
	  }
  }

}

/*
 <option value=\"00p%03d\"          >%d &nbsp;Przekaznik</option>\x0D\x0A\
 <option value=\"02p%03d\"          >%d &nbsp;Temperatura</option>\x0D\x0A\
 <option value=\"03p%03d\"          >%d &nbsp;Czujnik Pt1000</option>\x0D\x0A\
 <option value=\"09p%03d\"          >%d &nbsp;Temperatura Wilgotnosc</option>\x0D\x0A\
 <option value=\"10p%03d\"          >%d &nbsp;Timer</option>\x0D\x0A\
 <option value=\"11p%03d\"          >%d &nbsp;Termostat</option>\x0D\x0A\
 <option value=\"04p%03d\"          >%d &nbsp;Czujniki zwar-rozwar</option>\x0D\x0A\
 <option value=\"01p%03d\"          >%d &nbsp;Sterowanie PWM</option>\x0D\x0A\
 <option value=\"05p%03d\"          >%d &nbsp;Pomiar Napiecia 0-10V</option>\x0D\x0A\
 <option value=\"06p%03d\"          >%d &nbsp; -- -- -- </option>\x0D\x0A\
 */

void Wybierz_Tryb_Portu(int port)  //tryb
{
	char p;    p=port-1;  int i;

	 GPIO_TypeDef* GPIOx;
	 uint16_t GPIO_Pin;
                          //DS                     //DHT                      //Pt                      //We
	if((Const.s_GPIO[p].val==2)||(Const.s_GPIO[p].val==9)||(Const.s_GPIO[p].val==3)||(Const.s_GPIO[p].val==4)||(Const.s_GPIO[p].val==13))
	{
		NrPortuNaPortZewn(port, &GPIOx, &GPIO_Pin);
		Set_input(GPIOx,GPIO_Pin);
	}
	else if(Const.s_GPIO[p].val==5)  //TT
	{
		NrPortuNaPortZewn(port, &GPIOx, &GPIO_Pin);
		Set_input(GPIOx,GPIO_Pin);
	}
	else if((Const.s_GPIO[p].val==0)||(Const.s_GPIO[p].val==10)||(Const.s_GPIO[p].val==11)||(Const.s_GPIO[p].val==8))  //PK  Ti  Te  IR
	{
		NrPortuNaPortZewn(port, &GPIOx, &GPIO_Pin);
		Set_output(GPIOx,GPIO_Pin);
		if((Const.s_GPIO[p].val==0)||(Const.s_GPIO[p].val==10)||(Const.s_GPIO[p].val==11))  //PK  Ti  Te
		{
			i=WyszukujPortElement(t_wy,_Size_wy,port);
			if(i!=-1) GPIO_PK_start(i);

		}

	}
	else if(Const.s_GPIO[p].val==1)   //Sterowanie PWM
	{
	        if(port==__Port_PWM_1){ if(Const.s_GPIO[__Port_Ogoln_Etr_4-1].val!=7) PWM_1(Const.s_PWM[0].freq, Const.s_PWM[0].duty); else Const.s_GPIO[__Port_PWM_1-1].val=6;  }
	   else if(port==__Port_PWM_2) PWM_2(Const.s_PWM[1].freq, Const.s_PWM[1].duty);
	   else if(port==__Port_PWM_3){ if(Const.s_GPIO[__Port_Ogoln_Etr_3-1].val!=7) PWM_3(Const.s_PWM[2].freq, Const.s_PWM[2].duty); else Const.s_GPIO[__Port_PWM_3-1].val=6;  }

	}
	else if(Const.s_GPIO[p].val==7)  //Licznik Impulsow
	{
		     if(port==__Port_Ogoln_Etr_1){ if((wybor2_bitowy&0x0010)>0){ Init_Tim1_Etr(); }  }
		else if(port==__Port_Ogoln_Etr_2){ if((wybor2_bitowy&0x0010)>0){ Init_Tim2_Etr(); }  }
		else if(port==__Port_Ogoln_Etr_3){ if(Const.s_GPIO[__Port_PWM_3-1].val!=1){ if((wybor2_bitowy&0x0010)>0){ Init_Tim3_Etr();  }  } else Const.s_GPIO[__Port_Ogoln_Etr_3-1].val=6;  }
		else if(port==__Port_Ogoln_Etr_4){ if(Const.s_GPIO[__Port_PWM_1-1].val!=1){ if((wybor2_bitowy&0x0010)>0){ Init_Tim4_Etr();  }  } else Const.s_GPIO[__Port_Ogoln_Etr_4-1].val=6;  }
	}
	else if(Const.s_GPIO[p].val==6)  //-----
	{
		NrPortuNaPortZewn(port, &GPIOx, &GPIO_Pin);
		Set_output_0(GPIOx,GPIO_Pin);
		i=WyszukujPortElement(t_wy,_Size_wy,port);    if(i!=-1) Const.s_WY[i/16].val&=~(1<<(i-16*(i/16)));


	}






}

int Cgi_liczba(char *ptr)
{
   char *pp;
   if(pp=strstr(ptr,"_0"))
   {
	  return (0x0f&(*(pp+1)))*100 + (0x0f&(*(pp+2)))*10 +(0x0f&(*(pp+3)))*1;
   }
   else return 0;
}

void TransPrgPtDS(signed short *param)
{
       if((*param>99)||(*param<-99))
       {
      	   if(*param>99)  *param=   (99<<4)|0x000F;
      	   if(*param<-99) *param= ((-99)<<4)&0xFFF0 -1;
       }
       else
       {
       	   *param =*param<<4;
           if(*param<0)
           {
           	  *param|=0x000F;
           	  *param-=16;
           }
       }
}
void TransPrgDSPt(signed short *param)
{
	*param =*param>>4;
	if(*param<0) *param+=1;
}

char* HttpLogicParser(char *httpBuff)
{
	char *ptr;
	int nrInGate, nrGate, nrSigInName, lg;    lg=Const.OutNameLogic[0].name;   if(lg>=MAX_SIGOUT) dbg3("\r\nUWAGAA!!!!");

	if((ptr=strstr(httpBuff,"n_s_")))
	{
		if((*(ptr+4)=='S')&&(*(ptr+5)=='W'))
		{
			ptr+=2;
			nrInGate= ((*(ptr+4))&0x0f)-1;
			nrGate= (*(ptr+5))-'a';
			if((*(ptr+8)>0x2F)&&(*(ptr+8)<0x3A)) nrSigInName = 10*((*(ptr+7))&0x0f)+((*(ptr+8))&0x0f);
			else             					 nrSigInName =     (*(ptr+7))&0x0f;
			Const.Logic[lg].InitGate[nrGate].NameInputSignal[nrInGate] = nrSigInName;
			return (ptr+5);

		}
		else if((*(ptr+4)=='O')&&(*(ptr+5)=='L'))
		{
			if((*(ptr+6)=='0'))
			{
				ptr+=2;
				nrGate= (*(ptr+5))-'a';
				if((*(ptr+8)>0x2F)&&(*(ptr+8)<0x3A)) nrSigInName = 10*((*(ptr+7))&0x0f)+((*(ptr+8))&0x0f);
				else             					 nrSigInName =     (*(ptr+7))&0x0f;

					if(nrGate<4) 						Const.Logic[lg].InitGate[nrGate].logicalOperation = nrSigInName;
					else if((nrGate>3)&&(nrGate<6))		Const.Logic[lg].AddGate[nrGate-4].logicalOperation = nrSigInName;
					else                                Const.Logic[lg].FinGate[0].logicalOperation = nrSigInName;
			    return (ptr+5);
			}
			else
			{
				ptr+=2;
				nrInGate= ((*(ptr+4))&0x0f)-1;
				nrGate= (*(ptr+5))-'a';
				if((*(ptr+8)>0x2F)&&(*(ptr+8)<0x3A)) nrSigInName = 10*((*(ptr+7))&0x0f)+((*(ptr+8))&0x0f);
				else             					 nrSigInName =     (*(ptr+7))&0x0f;

					if(nrGate<4) 						Const.Logic[lg].InitGate[nrGate].inputBuffer[nrInGate] = nrSigInName;
					else if((nrGate>3)&&(nrGate<6))		Const.Logic[lg].AddGate[nrGate-4].inputBuffer[nrInGate] = nrSigInName;
					else                                Const.Logic[lg].FinGate[0].inputBuffer[nrInGate] = nrSigInName;
			    return (ptr+5);

			}

		}
		else if((*(ptr+4)=='S')&&(*(ptr+5)=='P'))
		{
			ptr+=2;
			nrInGate= ((*(ptr+4))&0x0f)-1;
			nrGate= (*(ptr+5))-'e';
			if((*(ptr+8)>0x2F)&&(*(ptr+8)<0x3A)) nrSigInName = 10*((*(ptr+7))&0x0f)+((*(ptr+8))&0x0f);
			else             					 nrSigInName =     (*(ptr+7))&0x0f;

				if(nrGate<2) 						Const.Logic[lg].AddGate[nrGate].NameInputSignal[nrInGate] = nrSigInName;
				else                                Const.Logic[lg].FinGate[0].NameInputSignal[nrInGate] = nrSigInName;
		   return (ptr+5);
		}
		else if((*(ptr+4)=='S')&&(*(ptr+5)=='i')&&(*(ptr+6)=='g')&&(*(ptr+7)=='W')&&(*(ptr+8)=='y'))
		{
			if((*(ptr+11)>0x2F)&&(*(ptr+11)<0x3A)) nrSigInName = 10*((*(ptr+10))&0x0f)+((*(ptr+11))&0x0f);
			else             					   nrSigInName =     (*(ptr+10))&0x0f;
			Const.OutNameLogic[0].name = nrSigInName;
			sprintf(buf_p,"\r\ngggg: %d", Const.OutNameLogic[0].name); dbg3(buf_p);
			return (ptr+5);
		}
	}
	return httpBuff;
}

void HttpLogicParser_ENC(char *httpBuff, char *val)
{
	char *ptr;
	int nrInGate, nrGate, nrSigInName, lg;    lg=Const.OutNameLogic[0].name;   if(lg>=MAX_SIGOUT) dbg3("\r\nUWAGAA!!!!");
	if((ptr=strstr(httpBuff,"s_SW")))
	{
		nrInGate= ((*(ptr+4))&0x0f)-1;
		nrGate= (*(ptr+5))-'a';
	    if(val[1]==0) nrSigInName = val[0]&0x0f;
	    else          nrSigInName = 10*(val[0]&0x0f) + (val[1]&0x0f);
	    Const.Logic[lg].InitGate[nrGate].NameInputSignal[nrInGate] = nrSigInName;

	}
	else if((ptr=strstr(httpBuff,"s_OL1"))||(ptr=strstr(httpBuff,"s_OL2"))||(ptr=strstr(httpBuff,"s_OL3"))||(ptr=strstr(httpBuff,"s_OL4"))||(ptr=strstr(httpBuff,"s_OL5"))||(ptr=strstr(httpBuff,"s_OL6"))||(ptr=strstr(httpBuff,"s_OL7"))||(ptr=strstr(httpBuff,"s_OL8")))
	{
		nrInGate= ((*(ptr+4))&0x0f)-1;
		nrGate= (*(ptr+5))-'a';
	    if(val[1]==0) nrSigInName = val[0]&0x0f;
	    else          nrSigInName = 10*(val[0]&0x0f) + (val[1]&0x0f);

			if(nrGate<4) 						Const.Logic[lg].InitGate[nrGate].inputBuffer[nrInGate] = nrSigInName;
			else if((nrGate>3)&&(nrGate<6))		Const.Logic[lg].AddGate[nrGate-4].inputBuffer[nrInGate] = nrSigInName;
			else                                Const.Logic[lg].FinGate[0].inputBuffer[nrInGate] = nrSigInName;

	}
	else if((ptr=strstr(httpBuff,"s_OL0")))
	{
		nrGate= (*(ptr+5))-'a';
	    if(val[1]==0) nrSigInName = val[0]&0x0f;
	    else          nrSigInName = 10*(val[0]&0x0f) + (val[1]&0x0f);

			if(nrGate<4) 						Const.Logic[lg].InitGate[nrGate].logicalOperation = nrSigInName;
			else if((nrGate>3)&&(nrGate<6))		Const.Logic[lg].AddGate[nrGate-4].logicalOperation = nrSigInName;
			else                                Const.Logic[lg].FinGate[0].logicalOperation = nrSigInName;

	}
	else if((ptr=strstr(httpBuff,"s_SP")))
	{
		nrInGate= ((*(ptr+4))&0x0f)-1;
		nrGate= (*(ptr+5))-'e';
	    if(val[1]==0) nrSigInName = val[0]&0x0f;
	    else          nrSigInName = 10*(val[0]&0x0f) + (val[1]&0x0f);

			if(nrGate<2) 						Const.Logic[lg].AddGate[nrGate].NameInputSignal[nrInGate] = nrSigInName;
			else                                Const.Logic[lg].FinGate[0].NameInputSignal[nrInGate] = nrSigInName;
	}
	else if((ptr=strstr(httpBuff,"s_SigWy")))
	{
	    if(val[1]==0) nrSigInName = val[0]&0x0f;
	    else          nrSigInName = 10*(val[0]&0x0f) + (val[1]&0x0f);
		Const.OutNameLogic[0].name = nrSigInName;
		sprintf(buf_p,"\r\ngggg: %d", Const.OutNameLogic[0].name); dbg3(buf_p);
	}

}

char *CGI_Wpis2(char *ptr1, char *zm)
{
   char *ptr;  int j,i,k,n,r;     uint8_t rej,pom,m;  rej= (Const.s_Rej[0].param&0x01);

    if(ptr=strstr(ptr1,"n_GPIO_val_"))  //n_GPIO_val_001        //CGI.... ..&n_GPIO_val_008=06&n_GPIO_val_009=06&n_109_zapisz=
    {
    	  m= 100*(*(ptr+11)&0x0f)+10*(*(ptr+12)&0x0f)+(*(ptr+13)&0x0f);  m--;   pom=10*(ptr[15]&0x0f)+(ptr[16]&0x0f);

    	// sprintf(buf_p,"\r\nn_GPIO_val_%03d_%02d    %02d", m+1,pom,Var.gpio[m]);  dbg3(buf_p);

    	  if(rej==1)
    	  {  if((pom==2)||(pom==9)||(pom==3)||(pom==7)) goto enddfderrg;
    	     if(( Const.s_GPIO[m].val==2)||( Const.s_GPIO[m].val==9)||( Const.s_GPIO[m].val==3)||( Const.s_GPIO[m].val==7)) goto enddfderrg;
    	  }
    	  Const.s_GPIO[m].val= pom;



    	      if((Const.s_GPIO[m].val!=7)&&(Var.gpio[m]==7))
    	      {
    	    	       if((m+1)==__Port_Ogoln_Etr_1){ DeInit_Tim1_Etr(); ZerujparametryLicznikow(0); }
    	    	  else if((m+1)==__Port_Ogoln_Etr_2){ DeInit_Tim2_Etr(); ZerujparametryLicznikow(1); }
    	    	  else if((m+1)==__Port_Ogoln_Etr_3){ DeInit_Tim3_Etr(); ZerujparametryLicznikow(2); }
    	    	  else if((m+1)==__Port_Ogoln_Etr_4){ DeInit_Tim4_Etr(); ZerujparametryLicznikow(3); }
    	      }
    	      if((Const.s_GPIO[m].val==7)&&(Var.gpio[m]!=7))
    	      {
    	    	       if((m+1)==__Port_Ogoln_Etr_1){ Init_Tim1_Etr(); ZerujparametryLicznikow(0); }
    	    	  else if((m+1)==__Port_Ogoln_Etr_2){ Init_Tim2_Etr(); ZerujparametryLicznikow(1); }
    	    	  else if((m+1)==__Port_Ogoln_Etr_3){ if(Const.s_GPIO[__Port_PWM_3-1].val!=1){ Init_Tim3_Etr(); ZerujparametryLicznikow(2); } }
    	    	  else if((m+1)==__Port_Ogoln_Etr_4){ if(Const.s_GPIO[__Port_PWM_1-1].val!=1){ Init_Tim4_Etr(); ZerujparametryLicznikow(3); } }
    	      }


    	   	  if((Const.s_GPIO[m].val==3)&&(Var.gpio[m]!=3))
    	   	  {        Kasuj_Czujki_port(m+1); Wprowaz_Jedno_Temp(m+1);
	   	               n=WyszukajNrCzujkiDlaPortu(m+1);
	   	               if(n!=-1){
	   	            	    wartTemp_Pt("+050", &Const.s_Czujki[n].prog_max1);
	   	            	    wartTemp_Pt("+050", &Const.s_Czujki[n].prog_max2);
	   	            	    wartTemp_Pt("-020", &Const.s_Czujki[n].prog_min1);
	   	            	    wartTemp_Pt("-020", &Const.s_Czujki[n].prog_min2);
	   	            	    wartTemp_Pt("+002", &Const.s_Czujki[n].hist);
	   	            	    wartTemp_Pt("+000", &Const.s_Czujki[n].korekcja);
	   	               }
    	   	  }
    	   	  else  if((Const.s_GPIO[m].val==2)&&(Var.gpio[m]!=2))
    	   	  {        Kasuj_Czujki_port(m+1); Wprowaz_Jedno_Temp(m+1);
	   	               n=WyszukajNrCzujkiDlaPortu(m+1);
	   	               if(n!=-1){
	   	            	    wartTemp("+50.0", &Const.s_Czujki[n].prog_max1);
	   	            	    wartTemp("+50.0", &Const.s_Czujki[n].prog_max2);
	   	            	    wartTemp("-20.0", &Const.s_Czujki[n].prog_min1);
	   	            	    wartTemp("-20.0", &Const.s_Czujki[n].prog_min2);
	   	            	    wartTemp("+00.5", &Const.s_Czujki[n].hist);
	   	            	    wartTemp("+00.0", &Const.s_Czujki[n].korekcja);
	   	               }
    	   	  }
    	   	  else  if((Const.s_GPIO[m].val==9)&&(Var.gpio[m]!=9))
    	   	  {        Kasuj_Czujki_port(m+1); Wprowaz_Temp_i_Wilg(m+1);
	   	               n=WyszukajNrCzujkiDlaPortu(m+1);
	   	               if(n!=-1){
	   	            	    wartTemp("+50.0", &Const.s_Czujki[n].prog_max1);
	   	            	    wartTemp("+50.0", &Const.s_Czujki[n].prog_max2);
	   	            	    wartTemp("-20.0", &Const.s_Czujki[n].prog_min1);
	   	            	    wartTemp("-20.0", &Const.s_Czujki[n].prog_min2);
	   	            	    wartTemp("+00.5", &Const.s_Czujki[n].hist);
	   	            	    wartTemp("+00.0", &Const.s_Czujki[n].korekcja);
	   	               }
	   	               n=WyszukajNrCzujkiDlaPortu_2(m+1);
	   	               if(n!=-1){
	   	            	    wartTemp("+50.0", &Const.s_Czujki[n].prog_max1);
	   	            	    wartTemp("+50.0", &Const.s_Czujki[n].prog_max2);
	   	            	    wartTemp("+00.0", &Const.s_Czujki[n].prog_min1);
	   	            	    wartTemp("+00.0", &Const.s_Czujki[n].prog_min2);
	   	            	    wartTemp("+04.0", &Const.s_Czujki[n].hist);
	   	            	    wartTemp("+00.0", &Const.s_Czujki[n].korekcja);
	   	               }

    	   	  }

    	   /*	  if((Const.s_GPIO[m].val==2)&&(Var.gpio[m]!=2))
    	   	  {   Kasuj_Czujki_port(m+1); Wprowaz_Jedno_Temp(m+1);
    	   	      if(Var.gpio[m]==3)
    	   	      {    n=WyszukajNrCzujkiDlaPortu(m+1);
    	   	           if(n!=-1){
                              TransPrgPtDS(&Const.s_Czujki[n].prog_max1);
                              TransPrgPtDS(&Const.s_Czujki[n].prog_max2);
                              TransPrgPtDS(&Const.s_Czujki[n].prog_min1);
                              TransPrgPtDS(&Const.s_Czujki[n].prog_min2);
                              TransPrgPtDS(&Const.s_Czujki[n].hist);
                              TransPrgPtDS(&Const.s_Czujki[n].korekcja);
    	   	           }
    	   	      }
    	   	  }
    	   	  else if((Const.s_GPIO[m].val==3)&&(Var.gpio[m]!=3))
    	   	  {   Kasuj_Czujki_port(m+1); Wprowaz_Jedno_Temp(m+1);
	   	          if((Var.gpio[m]==2)||(Var.gpio[m]==9))
	   	          {    n=WyszukajNrCzujkiDlaPortu(m+1);
	   	               if(n!=-1){
	   	            	      TransPrgDSPt(&Const.s_Czujki[n].prog_max1);
                              TransPrgDSPt(&Const.s_Czujki[n].prog_max2);
                              TransPrgDSPt(&Const.s_Czujki[n].prog_min1);
                              TransPrgDSPt(&Const.s_Czujki[n].prog_min2);
                              TransPrgDSPt(&Const.s_Czujki[n].hist);
                              TransPrgDSPt(&Const.s_Czujki[n].korekcja);
	   	               }
	   	          }
    	   	  }
    	      else if((Const.s_GPIO[m].val==9)&&(Var.gpio[m]!=9))
    	      {   Kasuj_Czujki_port(m+1); Wprowaz_Temp_i_Wilg(m+1);
	   	          if(Var.gpio[m]==3)
	   	          {    n=WyszukajNrCzujkiDlaPortu(m+1);
	   	               if(n!=-1){
                              TransPrgPtDS(&Const.s_Czujki[n].prog_max1);
                              TransPrgPtDS(&Const.s_Czujki[n].prog_max2);
                              TransPrgPtDS(&Const.s_Czujki[n].prog_min1);
                              TransPrgPtDS(&Const.s_Czujki[n].prog_min2);
                              TransPrgPtDS(&Const.s_Czujki[n].hist);
                              TransPrgPtDS(&Const.s_Czujki[n].korekcja);
	   	               }
	   	          }
    	      }*/
    	      else if(Const.s_GPIO[m].val==Var.gpio[m]);
    	      else  { Kasuj_Czujki_port(m+1);    }
    	 // }

    	  //if((Const.s_GPIO[m].val!=0)&&(Var.gpio[m]==0)){   Const.s_WY[0].val&=~(1<<m);  }
    	  if((Const.s_GPIO[m].val==11)&&(Var.gpio[m]!=11)){  Var.wsk_termost[m]=0; Var.delay_termost[m]=0;  }
    	  if((Const.s_GPIO[m].val==4)&&(Var.gpio[m]!=4)){  Var.wsk_we[m]=0; Var.delay_we[m]=0; Var.delay_wepk[m]=0;  }
    	  if((Const.s_GPIO[m].val==13)&&(Var.gpio[m]!=13)){  Var.wsk_we[m]=0; Var.delay_we[m]=0; Var.delay_wepk[m]=0;  }

    	  Var.gpio[m]=Const.s_GPIO[m].val;
    	  Wybierz_Tryb_Portu(m+1);

        /*
    	  if(m==ddo-1)   //TIM4 i TIM3  obsluguje ETR i PWM  jedno musi wykluczac drugi
    	  {
    	     if(Const.s_GPIO[__Port_Ogoln_Etr_3-1].val==7){ if(Const.s_GPIO[__Port_PWM_3-1].val==1) Const.s_GPIO[__Port_PWM_3-1].val=6; }
    	     if(Const.s_GPIO[__Port_Ogoln_Etr_4-1].val==7){ if(Const.s_GPIO[__Port_PWM_1-1].val==1) Const.s_GPIO[__Port_PWM_1-1].val=6; }

    	     if(Const.s_GPIO[__Port_PWM_3-1].val==1){ if(Const.s_GPIO[__Port_Ogoln_Etr_3-1].val==7) Const.s_GPIO[__Port_Ogoln_Etr_3-1].val=6; }
    	     if(Const.s_GPIO[__Port_PWM_1-1].val==1){ if(Const.s_GPIO[__Port_Ogoln_Etr_4-1].val==7) Const.s_GPIO[__Port_Ogoln_Etr_4-1].val=6; }

    	  }
        */


          enddfderrg:
		  asm("nop");
      #ifndef _LCD_TFT
		  lcd_init();
      #else
		  ekran_bit&=~0x01;
      #endif
    	 // if(m==7){   if(Const.s_Poz[0].ekran==1) lcd_init();   CzyJestCosNaWykresie();   }


		  for(r=20;r<_Size__s_gpio;r++)
		  {
		  	 if(Const.s_GPIO[r].val!=6) break;
		  }
		  if(r==_Size__s_gpio) Const.OutNameLogic[0].name=0xFF;
		  else                 Const.OutNameLogic[0].name=r-20;
    }

    else if(ptr=strstr(ptr1,"n_109_zapisz"))
    {
       // if(w==1) delay_funkc[0]=1;

        //delay_funkc[6]=1;
    	if(WhichPage!=1669)
    	{
            dbg3("\r\nZapis...");
             Flash_in_progr_char_na_32(_AdrFlashToRAM, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
             if(WhichPage==9) delay_funkc[19]=1; //zmiana init_lora
            dbg3("ok ");

    	}
        return 1;
    }
    else if(ptr=strstr(ptr1,"n_czas_zapisz"))
    {
    	//HAL_Delay(30);  UstawCzas(czas[0],czas[1],czas[2], czas[4],czas[5],czas[6]);  HAL_Delay(30);
    	delay_funkc[7]=1;
    	//UstawCzas(1,1,1, 1,1,1);  dbg3("\r\nZegar zapis ");
    	//HAL_Delay(1000); UstawCzas(17,6,3, 13,20,1);  HAL_Delay(1000);
    }
    else if(ptr=strstr(ptr1,"wykres1_"))
    {
    	Var.wykres_nr = 10*(*(ptr+8)&0x0f)+(*(ptr+9)&0x0f);  sprintf(buf_p,"\r\nWYKRES  %d @@@@",Var.wykres_nr); dbg3(buf_p);
    }
    else if(ptr=strstr(ptr1,"wykres2"))  //wykres2g_01
    {
      if(*(ptr+8)=='_')
      {
    	 j=10*(ptr[9]&0x0f)+(ptr[10]&0x0f);
    	      if(*(ptr+7)=='g') i=0;
    	 else if(*(ptr+7)=='d') i=1;
    	 else if(*(ptr+7)=='t') i=2;
    	 else if(*(ptr+7)=='m') i=3;

    	 WykrNrEtr = _Size_imp*j+i;
    	 sprintf(buf_p,"\r\nWYKRES_ETR  %d @@@@",WykrNrEtr); dbg3(buf_p);
      }
    }
    else if(ptr=strstr(ptr1,"wykrprzedz"))
    {
    	//IteracjaFlashZewnOdczyt(&adr_flash);
    	i=IloscCzujnikow();
        if((i>0)&&(adr_flash>0)){   //adr_flash - aktualny adres zapisu probek we flash`u
    	     if(*(ptr+10)=='1'){ if((adr_flash_start+1000*(i+4))<=adr_flash_end) adr_flash_start+= 1000*(i+4); else adr_flash_start=adr_flash_end;  }    //4 bo czas zapisywany jest w 4-rech bajtach
    	else if(*(ptr+10)=='2'){ if((adr_flash_start+100*(i+4))<=adr_flash_end)  adr_flash_start+= 100*(i+4); else adr_flash_start=adr_flash_end;   }
    	else if(*(ptr+10)=='3'){ if((adr_flash_start+10*(i+4))<=adr_flash_end)   adr_flash_start+= 10*(i+4); else adr_flash_start=adr_flash_end;   }
    	else if(*(ptr+10)=='4'){ if(adr_flash_start>=1000*(i+4)) adr_flash_start-= 1000*(i+4);  else adr_flash_start=0;   }
    	else if(*(ptr+10)=='5'){ if(adr_flash_start>=100*(i+4)) adr_flash_start-= 100*(i+4);   else adr_flash_start=0;  }
    	else if(*(ptr+10)=='6'){ if(adr_flash_start>=10*(i+4))  adr_flash_start-= 10*(i+4);   else adr_flash_start=0;  }

    	     if(*(ptr+10)=='7'){ if((adr_flash_end+1000*(i+4))<=adr_flash) adr_flash_end+= 1000*(i+4); else adr_flash_end= adr_flash;  }
    	else if(*(ptr+10)=='8'){ if((adr_flash_end+100*(i+4))<=adr_flash)  adr_flash_end+= 100*(i+4);  else adr_flash_end= adr_flash;  }
    	else if(*(ptr+10)=='9'){ if((adr_flash_end+10*(i+4))<=adr_flash)   adr_flash_end+= 10*(i+4);   else adr_flash_end= adr_flash;  }
    	else if(*(ptr+10)=='a'){ if(adr_flash_end>=(adr_flash_start+1000*(i+4))) adr_flash_end-= 1000*(i+4);  else adr_flash_end = adr_flash_start;  }
    	else if(*(ptr+10)=='b'){ if(adr_flash_end>=(adr_flash_start+100*(i+4)))  adr_flash_end-= 100*(i+4);  else adr_flash_end = adr_flash_start;  }
    	else if(*(ptr+10)=='c'){ if(adr_flash_end>=(adr_flash_start+10*(i+4)))   adr_flash_end-= 10*(i+4);  else adr_flash_end = adr_flash_start;  }

        }

    }
    else if(ptr=strstr(ptr1,"n_112_zapisz"))
    {
    	//Zaprogramuj_DS18B20();
    }
	else if(ptr=strstr(ptr1,"pprzek"))  //ster www przekaznikami
	{
         j=10*(ptr[6]&0x0f)+(ptr[7]&0x0f);
        // sprintf(buf_p,"\r\nPRZEK %d  ",j);  dbg3(buf_p);  HAL_Delay(1000);
         if((Const.s_WY[0].val&(1<<j))>0) Const.s_WY[0].val&=~(1<<j);
         else                             Const.s_WY[0].val|=(1<<j);
         GPIO_PK(j);
         ZapiszFlash();

	}
   	else if(ptr=strstr(ptr1,"scroll="))
    {
   		scroll=odczyt_liczby_cgi(ptr+7);
   		//sprintf(buf_p,"\r\nScrollAA: %d  ",scroll);    dbg3(buf_p);
    }
   	else if(ptr=strstr(ptr1,"rstDPT"))
    {
   		Const.s_Rej[0].param&=~(0x01<<k);   wybor2_bitowy|=0x0001;  WhichPage=80;
   		Flash_in_progr_char_na_32(_AdrFlashToRAM, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
   		ZapisFlashLicznikStart();
    }
   	else if(ptr=strstr(ptr1,"rstDPN"))  WhichPage=80;
   	else if(ptr=strstr(ptr1,"rstRZT"))
    {
   		wybor2_bitowy|=0x0008;  //kasuj reestr zdarzen
   		WhichPage=80;
    }
   	else if(ptr=strstr(ptr1,"rstRZN")) WhichPage=80;
    else{ *zm=0;  return ptr; }

   	*zm=1; return ptr;
}

void korektaPtnaDS(char *txt)
{
	int tt;
	if((txt[0]!='.')&&(txt[1]!='.')&&(txt[2]!='.')&&(txt[3]!='.')&&(txt[4]!='.'))
	{
	    tt = 100*(txt[1]&0x0f) + 10*(txt[2]&0x0f) + (txt[3]&0x0f);
	    if(tt>99) sprintf(txt,"%c99.9",buf_p[0]);
	    else      sprintf(txt,"%c%02d.0",buf_p[0],tt);
	}
	dbg3("\r\n"); dbg3(txt); dbg3("  ");
}

void CGI_Wpis(char *var, char *buf_p)
{
  signed short ttemp;  char *ptr,*p1,*ptr3, a;  int nr,nr2,k,k1,k2,h;  char xx[20];


//------------------  Czujki  ----------------------------------------------------------------------------------------------

	     if(ptr=strstr(var,"s_Czujki_nazwa")){ nr=Cgi_liczba(ptr); sprintf(Const.s_Czujki[nr-1].nazwa,buf_p); }
	else if(ptr=strstr(var,"s_Czujki_prog_max1")){ nr=Cgi_liczba(ptr); if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3){ wzor_wpisu_temp_Pt(buf_p); wartTemp_Pt(buf_p, &ttemp); } else{ wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);}   Const.s_Czujki[nr-1].prog_max1 = ttemp;  }
	else if(ptr=strstr(var,"s_Czujki_prog_max2")){ nr=Cgi_liczba(ptr); if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3){ wzor_wpisu_temp_Pt(buf_p); wartTemp_Pt(buf_p, &ttemp); } else{ wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);}   Const.s_Czujki[nr-1].prog_max2 = ttemp;  }
	else if(ptr=strstr(var,"s_Czujki_prog_min1")){ nr=Cgi_liczba(ptr); if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3){ wzor_wpisu_temp_Pt(buf_p); wartTemp_Pt(buf_p, &ttemp); } else{ wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);}   Const.s_Czujki[nr-1].prog_min1 = ttemp;  }
	else if(ptr=strstr(var,"s_Czujki_prog_min2")){ nr=Cgi_liczba(ptr); if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3){ wzor_wpisu_temp_Pt(buf_p); wartTemp_Pt(buf_p, &ttemp); } else{ wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);}   Const.s_Czujki[nr-1].prog_min2 = ttemp;  }
	else if(ptr=strstr(var,"s_Czujki_hist")){ nr=Cgi_liczba(ptr);      if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3){ wzor_wpisu_temp_Pt(buf_p); wartTemp_Pt(buf_p, &ttemp); } else{ wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);}   Const.s_Czujki[nr-1].hist = ttemp;       }
	else if(ptr=strstr(var,"s_Czujki_korekcja")){ nr=Cgi_liczba(ptr);  if(Const.s_GPIO[  Const.s_Czujki[nr-1].addr_DS[8]-1  ].val==3){ wzor_wpisu_temp_Pt(buf_p); wartTemp_Pt(buf_p, &ttemp); } else{ wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);}   Const.s_Czujki[nr-1].korekcja = ttemp;   }
	else if(ptr=strstr(var,"s_Czujki_ilosc_alarm"))
	{    nr=Cgi_liczba(ptr);
	    if(buf_p[1]==0) Const.s_Czujki[nr-1].ilosc_alarm = buf_p[0]&0x0f;
	    else            Const.s_Czujki[nr-1].ilosc_alarm = 10*(buf_p[0]&0x0f) + (buf_p[1]&0x0f);
	}
	else if(ptr=strstr(var,"s_Czujki_czas_reak"))
	{    nr=Cgi_liczba(ptr);
		wzor_wpisu_minsec(buf_p);  Const.s_Czujki[nr-1].czas_reak = 60*buf_p[0] + buf_p[1];
      #ifdef _LCD_TFT
		ekran_bit&=~0x01;
      #endif
	}
	else if(ptr=strstr(var,"s_Czujki_alarm_pk"))
	{     nr=Cgi_liczba(ptr);
		      if(strstr(buf_p,"tak")) Const.s_Czujki[nr-1].alarm_pk|=0x01;
		 else if(strstr(buf_p,"nie")) Const.s_Czujki[nr-1].alarm_pk&=~0x01;
	}

//------------------  Ustaw Etr Impuls  ----------------------------------------------------------------------------------------------
else if(ptr=strstr(var,"s_Imp_licz")){  nr=Cgi_liczba(ptr);    Const.s_Imp[nr-1].licz= wybierz_cyfry(buf_p);    }
else if(ptr=strstr(var,"s_Imp_wart")){  nr=Cgi_liczba(ptr);    Const.s_Imp[nr-1].wart= wybierz_cyfry(buf_p);    }
else if(ptr=strstr(var,"s_Imp_jedn")){  nr=Cgi_liczba(ptr);    sprintf(Const.s_Imp[nr-1].jedn, buf_p);     }

else if(ptr=strstr(var,"s_pEtr")){  nr=Cgi_liczba(ptr);   //n_s_pEtrgd1_004

     if(*(ptr+6)=='g')  //godzinny
     {  if(*(ptr+7)=='d')  //dziesietny
        {
    	        if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_dzies[1] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_dzies[1] = wybierz_cyfry_7(buf_p);
        }
        else if(*(ptr+7)=='u')  //ulamkowy
        {
	            if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_ulam[1] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_ulam[1] = wybierz_cyfry_7(buf_p);
        }
     }
     else if(*(ptr+6)=='d')  //dobowy
     {  if(*(ptr+7)=='d')  //dziesietny
        {
    	        if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_dzies[2] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_dzies[2] = wybierz_cyfry_7(buf_p);
        }
        else if(*(ptr+7)=='u')  //ulamkowy
        {
	            if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_ulam[2] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_ulam[2] = wybierz_cyfry_7(buf_p);
        }
     }
     else if(*(ptr+6)=='t')  //tygodniowy
     {  if(*(ptr+7)=='d')  //dziesietny
        {
    	        if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_dzies[3] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_dzies[3] = wybierz_cyfry_7(buf_p);
        }
        else if(*(ptr+7)=='u')  //ulamkowy
        {
	            if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_ulam[3] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_ulam[3] = wybierz_cyfry_7(buf_p);
        }
     }
     else if(*(ptr+6)=='m')  //miesieczny
     {  if(*(ptr+7)=='d')  //dziesietny
        {
    	        if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_dzies[4] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_dzies[4] = wybierz_cyfry_7(buf_p);
        }
        else if(*(ptr+7)=='u')  //ulamkowy
        {
	            if(*(ptr+8)=='1')  Const.s_Imp[nr-1].prog1_ulam[4] = wybierz_cyfry_7(buf_p);
           else if(*(ptr+8)=='2')  Const.s_Imp[nr-1].prog2_ulam[4] = wybierz_cyfry_7(buf_p);
        }
     }

}

//--------------  Tabela TT ---------------------------------------------------------------------------------------------------
 else if(ptr=strstr(var,"s_tt"))
 {
	 nr=  (*(ptr+5))&0x0f;   unsigned short ush;
	 nr2= Cgi_liczba(ptr);

	      if(*(ptr+4)=='j'){ sprintf(Const.s_TT[nr].jm, buf_p);  }
	 else if(*(ptr+4)=='d')
	 {
		 ush=0;
		 ush = wybierz_cyfry(buf_p);
		 Const.s_TT[nr].dd[nr2]&=~0x7FFF;
		 Const.s_TT[nr].dd[nr2] |= ush;
		 //Const.s_TT[nr].dd[nr2] = wybierz_cyfry(buf_p);// (0x0f&buf_p[0])*1000 + (0x0f&buf_p[1])*100 +(0x0f&buf_p[2])*10 +(0x0f&buf_p[3]);
	 }
	 else if(*(ptr+4)=='m')
	 {
		 Const.s_TT[nr].mm[nr2] = wybierz_cyfry(buf_p);// (0x0f&buf_p[0])*10 +(0x0f&buf_p[1]);
	 }
	 else if(*(ptr+4)=='p')
	 {
	         if(buf_p[0]=='+') Const.s_TT[nr].dd[nr2]&=~0x8000;
	    else if(buf_p[0]=='-') Const.s_TT[nr].dd[nr2]|=0x8000;
	 }
 }
//--------------  Odswiezanie strony ---------------------------------------------------------------------------------------------------
 else if(ptr=strstr(var,"s_Poz_refr"))
 {    nr=Cgi_liczba(ptr);
      wzor_wpisu_minsec(buf_p);  Const.s_Poz[nr-1].refr = 60*buf_p[0] + buf_p[1];
 }

//--------------  Haslo do WWW ---------------------------------------------------------------------------------------------------
 else if(ptr=strstr(var,"s_log")){ nr=Cgi_liczba(ptr); sprintf(Const.s_Log[nr-1].login,buf_p); }
 else if(ptr=strstr(var,"s_has")){ nr=Cgi_liczba(ptr); sprintf(Const.s_Log[nr-1].haslo,buf_p); }

//--------------  Termostaty ---------------------------------------------------------------------------------------------------

  else if(ptr=strstr(var,"s_Term_prog"))
  {
	  nr=Cgi_liczba(ptr);
      if(Const.s_GPIO[  Const.s_Term[nr-1].nr_temp-1  ].val==3)
      {
    	    wzor_wpisu_temp_Pt(buf_p);
    	    wartTemp_Pt(buf_p, &ttemp);
      }
      else
      {
    		int tt;
    		j=0;
    		for(i=0;i<strlen(buf_p);i++)
    		{
    			if(buf_p[i]=='.') j++;
    		}
    		if(j==0)//jezeli nie ma kropki
    		{
    			if((strlen(buf_p)==5)&&(buf_p[4]==' '))
    		    {
    				tt = 100*(buf_p[1]&0x0f) + 10*(buf_p[2]&0x0f) + (buf_p[3]&0x0f);
    				 if(buf_p[0]=='-')
    				 {
    				     if(tt>99) sprintf(buf_p,"-99.9");
    				     else      sprintf(buf_p,"-%02d.0",tt);
    		         }
    				 else
    				 {
    					 if(tt>99) sprintf(buf_p,"99.9");
    				     else      sprintf(buf_p,"%02d.0",tt);
    				 }
    		    }
    			//dbg3("\r\nAAAAAAAAAAAAAAAAA:"); dbg3(buf_p); dbg3("XXXX ");
    		}
            wzor_wpisu_temp(buf_p);
            wartTemp(buf_p, &ttemp);
      }
      Const.s_Term[nr-1].prog = ttemp;

  }
  else if(ptr=strstr(var,"s_Term_hist"))
  {
	  nr=Cgi_liczba(ptr);
	  if(Const.s_GPIO[  Const.s_Term[nr-1].nr_temp-1  ].val==3)
	  {
		  wzor_wpisu_temp_Pt(buf_p);
		  wartTemp_Pt(buf_p, &ttemp);
	  }
      else
      {
    		int tt;
    		j=0;
    		for(i=0;i<strlen(buf_p);i++)
    		{
    			if(buf_p[i]=='.') j++;
    		}
    		if(j==0)//jezeli nie ma kropki
    		{
    			if((strlen(buf_p)==5)&&(buf_p[4]==' '))
    		    {
    				tt = 100*(buf_p[1]&0x0f) + 10*(buf_p[2]&0x0f) + (buf_p[3]&0x0f);
    				 if(buf_p[0]=='-')
    				 {
    				     if(tt>99) sprintf(buf_p,"-99.9");
    				     else      sprintf(buf_p,"-%02d.0",tt);
    		         }
    				 else
    				 {
    					 if(tt>99) sprintf(buf_p,"99.9");
    				     else      sprintf(buf_p,"%02d.0",tt);
    				 }
    		    }
    			//dbg3("\r\nAAAAAAAAAAAAAAAAA:"); dbg3(buf_p); dbg3("XXXX ");
    		}
            wzor_wpisu_temp(buf_p);
            wartTemp(buf_p, &ttemp);
      }
	  Const.s_Term[nr-1].hist = ttemp;
  }
  /*else if(ptr=strstr(var,"s_Term_nr_temp"))
  {    nr=Cgi_liczba(ptr);
      if(buf_p[1]==0) Const.s_Term[nr-1].nr_temp = buf_p[0]&0x0f;
      else            Const.s_Term[nr-1].nr_temp = 10*(buf_p[0]&0x0f) + (buf_p[1]&0x0f);
  }*/
  else if(ptr=strstr(var,"s_Term_czas_reak"))
  {    nr=Cgi_liczba(ptr);
       wzor_wpisu_minsec(buf_p);  Const.s_Term[nr-1].czas_reak = 60*buf_p[0] + buf_p[1];
  }
  /*else if(ptr=strstr(var,"s_Term_akt"))
  {   nr=Cgi_liczba(ptr);
	         if(strstr(buf_p,"tak")) Const.s_Term[nr-1].akt|=0x01;
       else if(strstr(buf_p,"nie")) Const.s_Term[nr-1].akt&=~0x01;
  }*/
  else if(ptr=strstr(var,"s_Term_seltempwilg"))
  {   nr=Cgi_liczba(ptr);

      nr2=10*(buf_p[1]&0x0f)+(buf_p[2]&0x0f);
      if(buf_p[0]=='W') nr2|=0x80;
      Const.s_Term[nr-1].nr_temp=nr2;

    /* j=nr2&0x7F;
     if(j>0)
     {
         if(Const.s_GPIO[j-1].val==3)
         {
    	    wartTemp_Pt("+050", &Const.s_Term[nr-1].prog);
    	    wartTemp_Pt("+002", &Const.s_Term[nr-1].hist);
         }
         else if(Const.s_GPIO[j-1].val==9)
         {
    	    wartTemp("+50.0", &Const.s_Term[nr-1].prog);
            wartTemp("+04.0", &Const.s_Term[nr-1].hist);
         }
         else
         {
    	    wartTemp("+50.0", &Const.s_Term[nr-1].prog);
            wartTemp("+00.5", &Const.s_Term[nr-1].hist);
         }
     }*/

	/*        if(strstr(buf_p,"00")) Const.s_Term[nr-1].nr_temp=0;
       else if(strstr(buf_p,"T1")) Const.s_Term[nr-1].nr_temp=0x01;
       else if(strstr(buf_p,"T2")) Const.s_Term[nr-1].nr_temp=0x02;
       else if(strstr(buf_p,"T3")) Const.s_Term[nr-1].nr_temp=0x03;
       else if(strstr(buf_p,"T4")) Const.s_Term[nr-1].nr_temp=0x04;
       else if(strstr(buf_p,"T5")) Const.s_Term[nr-1].nr_temp=0x05;
       else if(strstr(buf_p,"T6")) Const.s_Term[nr-1].nr_temp=0x06;
       else if(strstr(buf_p,"T7")) Const.s_Term[nr-1].nr_temp=0x07;
      else if(strstr(buf_p,"W1")) Const.s_Term[nr-1].nr_temp=0x81;
      else if(strstr(buf_p,"W2")) Const.s_Term[nr-1].nr_temp=0x82;
      else if(strstr(buf_p,"W3")) Const.s_Term[nr-1].nr_temp=0x83;
      else if(strstr(buf_p,"W4")) Const.s_Term[nr-1].nr_temp=0x84;
      else if(strstr(buf_p,"W5")) Const.s_Term[nr-1].nr_temp=0x85;
      else if(strstr(buf_p,"W6")) Const.s_Term[nr-1].nr_temp=0x86;
      else if(strstr(buf_p,"W7")) Const.s_Term[nr-1].nr_temp=0x87;
*/
  }

//------------ Telefony ------------------------------------------------------------------------------------------------------
  else if(ptr=strstr(var,"s_nrtel"))
  {
	  k=(*(ptr+7))&0x0f;   nr=Cgi_liczba(ptr);        if(k==1){   for(i=0;i<_Ilosc_Tel;i++){  Const.s_Dzw[0].ma[i]=0;  Const.s_Rap[0].sms[i]=0;  }     }
	  if(buf_p[0]=='+')
	  {
         j=0;
		 for(i=0;i<(strlen(buf_p)-1);i++)
         {
        	 if((buf_p[i+1]>0x29)&&(buf_p[i+1]<0x3A)) j++;
         }
		 if(i==j) sprintf(Const.s_Tel[nr-1].tel0+17*k,buf_p);
		 else     sprintf(Const.s_Tel[nr-1].tel0+17*k,"+48000000000");
	  }
	  else
	  {
		 sprintf(Const.s_Tel[nr-1].tel0+17*k,"+48000000000");
	  }

    if(k>0){
	 /* Const.s_Ma[0].sms[0]&=~0x7F;
	  Const.s_Ma[1].sms[0]&=~0x7F;
	  Const.s_Ma[2].sms[0]&=~0x7F;
	  Const.s_Ma[3].sms[0]&=~0x7F;
	  Const.s_Ma[4].sms[0]&=~0x7F;
	  Const.s_Ma[5].sms[0]&=~0x7F;
	  Const.s_Ma[6].sms[0]&=~0x7F;
	  Const.s_Ma[7].sms[0]&=~0x7F;*/

    	   for(i=0;i<_Ilosc_Tel;i++)
    	   {  for(j=0;j<(1+(_Size_te-1)/16);j++){
    		     Const.s_Ma[i].sms[j]=0;
    		   }
    	   }
    }
  }


// ----------- Usatw Rotacje  ----------------------------------
else if(ptr=strstr(var,"s_xxROTxx"))
{
	for(i=0;i<_Size_wy;i++) Const.s_Rot[0].ma[i]=0;  //dbg3("\r\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	NrRot=0;        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, NrRot);
}
else if(ptr=strstr(var,"s_Rot_akt"))  //name="n_s_Rot_akt001_001"
{
	k= 100*((*(ptr+9))&0x0f)+10*((*(ptr+10))&0x0f)+((*(ptr+11))&0x0f);     nr=Cgi_liczba(ptr);
    Const.s_Rot[0].ma[nr-1] |= 0x01<<(k-1);
}
// ----------- Usatw Maski SMS  ----------------------------------
 else if(ptr=strstr(var,"s_MaS_akt"))
 {
     k= 100*((*(ptr+9))&0x0f)+10*((*(ptr+10))&0x0f)+((*(ptr+11))&0x0f);     nr=Cgi_liczba(ptr);
     h=WyszukujPortElement(t_te,_Size_te,k);   //lub  t_we
     Const.s_Ma[nr-1].sms[h/16] |= 0x0001<<(h-16*(h/16));
 }
// ----------- Usatw Maski Dzwonienia  ----------------------------------
else if(ptr=strstr(var,"s_xerrr"))
{
	          k= 100*((*(ptr+7))&0x0f)+10*((*(ptr+8))&0x0f)+((*(ptr+9))&0x0f);
	          Const.s_Dzw[0].ma[k-1] |= 0x01;
}
	     // ----------- Usatw Maski SMS Raport  ----------------------------------
	     else if(ptr=strstr(var,"s_derrr"))
	     {
	     	          k= 100*((*(ptr+7))&0x0f)+10*((*(ptr+8))&0x0f)+((*(ptr+9))&0x0f);
	     	          Const.s_Rap[0].sms[k-1] |= 0x01;
	     }
// ----------- Usatw Maski E-MAIL  ----------------------------------
else if(ptr=strstr(var,"s_MaE_akt"))
{
	k= 100*((*(ptr+9))&0x0f)+10*((*(ptr+10))&0x0f)+((*(ptr+11))&0x0f);      nr=Cgi_liczba(ptr);
	h=WyszukujPortElement(t_te,_Size_te,k);   //lub  t_we
	Const.s_Ma[nr-1].mail[h/16] |= 0x0001<<(h-16*(h/16));
}
//------------------------------- Czas ------------------------------------------------------
          else if(ptr=strstr(var,"s_Czas_"))
	     {
	     	   if(buf_p[1]==0) a = (buf_p[0]&0x0f);
	     	   else            a = (10*(buf_p[0]&0x0f)) + (buf_p[1]&0x0f);

	     	   //sprintf(buf_p,"\r\na: %d  ",a); dbg3(buf_p);
	     	   //sprintf(buf_p,"\r\n%02d.%02d.%02d  %02d:%02d:%02d   %d ", czas[0], czas[1], czas[2], czas[4], czas[5], czas[6],a); dbg3(buf_p);

	     	      if(strstr(var,"s_Czas_godz")){  czas[4]=a; /*sprintf(buf_p,"\r\nA1: %d     ",czas[4]); dbg3(buf_p); */   }
	     	 else if(strstr(var,"s_Czas_min")){   czas[5]=a; }
	     	 else if(strstr(var,"s_Czas_sec")){   czas[6]=a;      }

	     	 else if(strstr(var,"s_Czas_rok")){   czas[0]=a;     }
	     	 else if(strstr(var,"s_Czas_mies")){  czas[1]=a;    }
	     	 else if(strstr(var,"s_Czas_dzien")){ czas[2]=a;      }


	     }
//---------------------------------------------- Rejestrator -----------------------------------------
/*else if(ptr=strstr(var,"s_Rej_intr")){ nr=Cgi_liczba(ptr);
 	wzor_wpisu_minsec(buf_p);  Const.s_Rej[nr-1].intr = 60*buf_p[0] + buf_p[1];
}
else if(ptr=strstr(var,"s_Rej_wstrz"))
{ nr=Cgi_liczba(ptr);
	     if(strstr(buf_p,"tak")) Const.s_Rej[nr-1].param|=0x02;
	else if(strstr(buf_p,"nie")) Const.s_Rej[nr-1].param&=~0x02;
}
else if(ptr=strstr(var,"s_Rej_akt"))
{ nr=Cgi_liczba(ptr);
	     if(strstr(buf_p,"tak")) Const.s_Rej[nr-1].param|=0x01;
	else if(strstr(buf_p,"nie")) Const.s_Rej[nr-1].param&=~0x01;
}*/

else if(ptr=strstr(var,"s_Rej_"))
{
   nr=Cgi_liczba(ptr);
   if(*(ptr+6)=='i'){  wzor_wpisu_minsec(buf_p);  Const.s_Rej[nr-1].intr = 60*buf_p[0] + buf_p[1];   if(Const.s_Rej[nr-1].intr<2) Const.s_Rej[nr-1].intr=2;   if(Const.s_Rej[nr-1].intr>60*60) Const.s_Rej[nr-1].intr=60*60; }
   else
   {   if(*(ptr+6)=='w') k=1;  else k=0;

	   if(strstr(buf_p,"tak")){   if((k==0)&&((Const.s_Rej[nr-1].param&0x01)==0)) ZapisFlashLicznikStart();   if(k==0){ Const.s_Rap[0].start_po=0; }       Const.s_Rej[nr-1].param|=(0x01<<k);  if(k==0) Const.s_Rej[nr-1].param&=~0x02;    CzyJestCosNaWykresie();   }
	   else                   {                                      if(k==0){   WhichPage=7; }   }
   }
}
//---------------- Wykres ----------------------
else if(ptr=strstr(var,"s_Wykr_prgm"))
{
   nr=Cgi_liczba(ptr);    k = wybierz_cyfry(buf_p);
   if(*(ptr+11)=='a'){    Const.s_Wykr[nr-1].prgmax = k;  }
   else              {    Const.s_Wykr[nr-1].prgmin = k;  }

}
else if(ptr=strstr(var,"s_Wykr_zaw"))
{
   nr=Cgi_liczba(ptr);    k = wybierz_cyfry(buf_p);
   Const.s_Wykr[nr-1].zawezen = k;

}
else if(ptr=strstr(var,"s_Wykr_sze"))
{
   nr=Cgi_liczba(ptr);    k = wybierz_cyfry(buf_p);
   Const.s_Wykr[nr-1].szerwykr = k;

}

//---------------- Wykres Liczniki ----------------------
	     else if(ptr=strstr(var,"s_WykrEtr_prgm"))
	     {
	        nr=Cgi_liczba(ptr);    k = wybierz_cyfry(buf_p);
	        if(*(ptr+14)=='a'){    Const.s_WykrEtr[nr-1].prgmax = k;  }
	        else              {    Const.s_WykrEtr[nr-1].prgmin = k;  }

	     }
	     else if(ptr=strstr(var,"s_WykrEtr_zaw"))
	     {
	        nr=Cgi_liczba(ptr);    k = wybierz_cyfry(buf_p);
	        Const.s_WykrEtr[nr-1].zawezen = k;

	     }
	     else if(ptr=strstr(var,"s_WykrEtr_sze"))
	     {
	        nr=Cgi_liczba(ptr);    k = wybierz_cyfry(buf_p);
	        Const.s_WykrEtr[nr-1].szerwykr = k;

	     }
	     //-----------------------  - Raport o godz --------------------------------------------------------
	     else if(ptr=strstr(var,"s_RR_godzON1")){ nr=Cgi_liczba(ptr); wzor_wpisu_minsec(buf_p);  Const.s_Rap[0].godzON1 = 60*buf_p[0] + buf_p[1]; }

	     //------------------------------- Timery ------------------------------------------------------
	     else if(ptr=strstr(var,"s_Tim_godzON1")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzON1 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; }
	     else if(ptr=strstr(var,"s_Tim_godzON2")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzON2 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; }
	     else if(ptr=strstr(var,"s_Tim_godzON3")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzON3 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; }
	     else if(ptr=strstr(var,"s_Tim_godzON4")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzON4 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; }

	     else if(ptr=strstr(var,"s_Tim_godzOF1")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzOF1 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; }
	     else if(ptr=strstr(var,"s_Tim_godzOF2")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzOF2 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; }
	     else if(ptr=strstr(var,"s_Tim_godzOF3")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzOF3 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; }
	     else if(ptr=strstr(var,"s_Tim_godzOF4")){ nr=Cgi_liczba(ptr); wzor_wpisu_godzminsec(buf_p);  Const.s_Tim[nr-1].godzOF4 = 3600*buf_p[0] + 60*buf_p[1] + buf_p[2]; Const.s_Tim[nr-1].akt&=~0x7F; }

	     else if(ptr=strstr(var,"s_Tim_akt1")){ nr=Cgi_liczba(ptr);   Const.s_Tim[nr-1].akt|=0x01;  }
	     else if(ptr=strstr(var,"s_Tim_akt2")){ nr=Cgi_liczba(ptr);   Const.s_Tim[nr-1].akt|=0x02;   }
	     else if(ptr=strstr(var,"s_Tim_akt3")){ nr=Cgi_liczba(ptr);   Const.s_Tim[nr-1].akt|=0x04;   }
	     else if(ptr=strstr(var,"s_Tim_akt4")){ nr=Cgi_liczba(ptr);   Const.s_Tim[nr-1].akt|=0x08;   }
	     else if(ptr=strstr(var,"s_Tim_akt5")){ nr=Cgi_liczba(ptr);   Const.s_Tim[nr-1].akt|=0x10;   }
	     else if(ptr=strstr(var,"s_Tim_akt6")){ nr=Cgi_liczba(ptr);   Const.s_Tim[nr-1].akt|=0x20;  }
	     else if(ptr=strstr(var,"s_Tim_akt7")){ nr=Cgi_liczba(ptr);   Const.s_Tim[nr-1].akt|=0x40;   }

	     else if(ptr=strstr(var,"s_Tim_akt8"))
	     { nr=Cgi_liczba(ptr);
	     	     if(strstr(buf_p,"tak")) Const.s_Tim[nr-1].akt|=0x80;
	     	else if(strstr(buf_p,"nie")) Const.s_Tim[nr-1].akt&=~0x80;
	     }

	//----------------------------------Raportowanie ------------------------------------

	     else if(ptr=strstr(var,"s_RRapZawSel"))
	     { nr=Cgi_liczba(ptr);
	        Const.s_Rap[nr-1].plik&=~0x03;
	     	     if(strstr(buf_p,"0"));
	        else if(strstr(buf_p,"1")) Const.s_Rap[nr-1].plik=0x01;
	        else if(strstr(buf_p,"2")) Const.s_Rap[nr-1].plik=0x02;
	     }
	     else if(ptr=strstr(var,"s_RRapSiecSel"))
	     { nr=Cgi_liczba(ptr);
	        Const.s_Rap[nr-1].siec&=~0x03;
	     	     if(strstr(buf_p,"0"));
	        else if(strstr(buf_p,"1")) Const.s_Rap[nr-1].siec=0x01;
	        else if(strstr(buf_p,"2")) Const.s_Rap[nr-1].siec=0x02;
	     }
	     else if(ptr=strstr(var,"s_RRapTimSel"))
	     { nr=Cgi_liczba(ptr);
	        Const.s_Rap[nr-1].czas&=~0x07;
	     	     if(strstr(buf_p,"0"));
	        else if(strstr(buf_p,"1")) Const.s_Rap[nr-1].czas=0x01;
	        else if(strstr(buf_p,"2")) Const.s_Rap[nr-1].czas=0x02;
	        else if(strstr(buf_p,"3")) Const.s_Rap[nr-1].czas=0x03;
	        else if(strstr(buf_p,"4")) Const.s_Rap[nr-1].czas=0x04;
	        else if(strstr(buf_p,"5")) Const.s_Rap[nr-1].czas=0x05;
	        else if(strstr(buf_p,"6")) Const.s_Rap[nr-1].czas=0x06;
	        else if(strstr(buf_p,"7")) Const.s_Rap[nr-1].czas=0x07;
	     }
	     else if(ptr=strstr(var,"s_RRapTimGodz")){ nr=Cgi_liczba(ptr); wzor_wpisu_minsec(buf_p);  Const.s_Rap[0].godzina = 60*buf_p[0] + buf_p[1]; }

	     //------------------------------- Wejscia Cyfrowe ------------------------------------------------------
	     else if(ptr=strstr(var,"s_WE_nazwa_ro")){ nr=Cgi_liczba(ptr); sprintf(Const.s_WE[nr-1].nazwa_ro,buf_p);}
	     else if(ptr=strstr(var,"s_WE_nazwa_zw")){ nr=Cgi_liczba(ptr); sprintf(Const.s_WE[nr-1].nazwa_zw,buf_p);}
	     else if(ptr=strstr(var,"s_WE_nazwa_uszk")){ nr=Cgi_liczba(ptr); sprintf(Const.s_WE[nr-1].nazwa_uszk,buf_p);}
	     else if(ptr=strstr(var,"s_WE_nazwa")){ nr=Cgi_liczba(ptr);    sprintf(Const.s_WE[nr-1].nazwa,buf_p);}
	     else if(ptr=strstr(var,"s_WE_czas_reak")){ nr=Cgi_liczba(ptr);
	        wzor_wpisu_minsec(buf_p);  Const.s_WE[nr-1].czas_reak = 60*buf_p[0] + buf_p[1];
#ifdef _LCD_TFT
   ekran_bit&=~0x01;
 #endif
	     }
	     else if(ptr=strstr(var,"s_WE_ilosc_alarm"))
	     { nr=Cgi_liczba(ptr);
	     	    if(buf_p[1]==0) Const.s_WE[nr-1].ilosc_alarm = buf_p[0]&0x0f;
	     	    else            Const.s_WE[nr-1].ilosc_alarm = 10*(buf_p[0]&0x0f) + (buf_p[1]&0x0f);
	     }
	    /* else if(ptr=strstr(var,"s_WE_przek"))
	     { nr=Cgi_liczba(ptr);
	     	    if(buf_p[1]==0) Const.s_WE[nr-1].pk = buf_p[0]&0x0f;
	     	    else            Const.s_WE[nr-1].pk = 10*(buf_p[0]&0x0f) + (buf_p[1]&0x0f);
	     }*/
	     else if(ptr=strstr(var,"s_WE_czas_przyt")){ nr=Cgi_liczba(ptr);
	        wzor_wpisu_minsec(buf_p);  Const.s_WE[nr-1].czas_przyt = 60*buf_p[0] + buf_p[1];
	     }
	     else if(ptr=strstr(var,"s_WE_param1"))
	     { nr=Cgi_liczba(ptr);
	        Const.s_WE[nr-1].param&=~0x03;
	     	    if(strstr(buf_p,"0"));
	        else if(strstr(buf_p,"1")) Const.s_WE[nr-1].param|=0x01;
	        else if(strstr(buf_p,"2")) Const.s_WE[nr-1].param|=0x02;
	     }
	     else if(ptr=strstr(var,"s_We_selautowe"))
	     {   nr=Cgi_liczba(ptr);

	        nr2=10*(buf_p[1]&0x0f)+(buf_p[2]&0x0f);
	        Const.s_WE[nr-1].pk=nr2;
	     /*

	     	        if(strstr(buf_p,"P0")) Const.s_WE[nr-1].pk=0;
	            else if(strstr(buf_p,"P1")) Const.s_WE[nr-1].pk=1;
	            else if(strstr(buf_p,"P2")) Const.s_WE[nr-1].pk=2;
	            else if(strstr(buf_p,"P3")) Const.s_WE[nr-1].pk=3;
	            else if(strstr(buf_p,"P4")) Const.s_WE[nr-1].pk=4;
	            else if(strstr(buf_p,"P5")) Const.s_WE[nr-1].pk=5;
	            else if(strstr(buf_p,"P6")) Const.s_WE[nr-1].pk=6;
	            else if(strstr(buf_p,"P7")) Const.s_WE[nr-1].pk=7;
	            else if(strstr(buf_p,"P8")) Const.s_WE[nr-1].pk=8;*/
	     }

	     //------------------------------- Wyjscia Cyfrowe ------------------------------------------------------
	     else if(ptr=strstr(var,"s_PK_nazwa_on")){ nr=Cgi_liczba(ptr); sprintf(Const.s_PK[nr-1].nazwa_on,buf_p);}
	     else if(ptr=strstr(var,"s_PK_nazwa_of")){ nr=Cgi_liczba(ptr); sprintf(Const.s_PK[nr-1].nazwa_of,buf_p); }
	     else if(ptr=strstr(var,"s_PK_nazwa"))
	     {  nr=Cgi_liczba(ptr);    sprintf(Const.s_PK[nr-1].nazwa,buf_p);
            #ifdef _LCD_TFT
	          ekran_bit&=~0x01;
            #endif
	     }

//------------------------------- Ustaw LoRa ------------------------------------------------------

else if(ptr=strstr(var,"s_Lora_"))
{
        if(strstr(var,"s_Lora_freq")) Const.s_Lora[0].freq = (buf_p[0]&0x0f);
   else if(strstr(var,"s_Lora_power")) Const.s_Lora[0].power = (buf_p[0]&0x0f);
   else if(strstr(var,"s_Lora_spread")) Const.s_Lora[0].spread = (buf_p[0]&0x0f);
   else if(strstr(var,"s_Lora_bandwidth")) Const.s_Lora[0].bandwidth = (buf_p[0]&0x0f);
   else if(strstr(var,"s_Lora_packetlen")) Const.s_Lora[0].packetlen = (buf_p[0]&0x0f);

}

//-------------------------  Rotacja  ------------------------
else if(ptr=strstr(var,"s_Rot_dob"))  //&n_s_Rot_dob_001=04&
{   nr=Cgi_liczba(ptr);
    Const.s_Rot[nr-1].dob= 10*(buf_p[0]&0x0f)+(buf_p[1]&0x0f);     RotDoby=0; HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, RotDoby);
}
else if(ptr=strstr(var,"s_Rot_int")){ nr=Cgi_liczba(ptr);
    if(Const.s_Rot[nr-1].dob==0){    wzor_wpisu_minsec(buf_p);  Const.s_Rot[nr-1].intrv = 60*buf_p[0] + buf_p[1];    if(Const.s_Rot[nr-1].intrv==0) Const.s_Rot[nr-1].intrv=1;  }
    else                                                        Const.s_Rot[nr-1].intrv = 0;

}
else if(ptr=strstr(var,"s_Rot_sta")){ nr=Cgi_liczba(ptr);
    wzor_wpisu_minsec(buf_p);  Const.s_Rot[nr-1].sta = 60*buf_p[0] + buf_p[1];
    if(Const.s_Rot[nr-1].dob==0){  RotCzasNext = Const.s_Rot[nr-1].sta;  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, RotCzasNext);  }
}
else if(ptr=strstr(var,"s_Rot_max")){ nr=Cgi_liczba(ptr);  wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);   Const.s_Rot[nr-1].max = ttemp;  }
else if(ptr=strstr(var,"s_Rot_min")){ nr=Cgi_liczba(ptr);  wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);   Const.s_Rot[nr-1].min = ttemp;  }
else if(ptr=strstr(var,"s_Rot_hist")){ nr=Cgi_liczba(ptr);  wzor_wpisu_temp(buf_p); wartTemp(buf_p, &ttemp);   Const.s_Rot[nr-1].hist = ttemp;  }
else if(ptr=strstr(var,"s_Rot_rea")){ nr=Cgi_liczba(ptr);
   wzor_wpisu_minsec(buf_p);  Const.s_Rot[nr-1].reak = 60*buf_p[0] + buf_p[1];
}

//------------------------------- Ustaw GPRS ------------------------------------------------------
else  if(ptr=strstr(var,"s_GPRSapn")){ nr=Cgi_liczba(ptr); sprintf(Const.s_GPRS[nr-1].apn,buf_p); }
else  if(ptr=strstr(var,"s_GPRSusr")){ nr=Cgi_liczba(ptr); sprintf(Const.s_GPRS[nr-1].usr,buf_p); }
else  if(ptr=strstr(var,"s_GPRShas")){ nr=Cgi_liczba(ptr); sprintf(Const.s_GPRS[nr-1].has,buf_p); }

	       //------------------------------- Ustaw Siec ------------------------------------------------------
	     	      	else if(ptr=strstr(var,"s_Lan_mac")){ nr=Cgi_liczba(ptr);  if(strlen(buf_p)==12){  if(BufNotIdentity(Const.s_Lan[nr-1].eth, buf_p)) w=1;   sprintf(Const.s_Lan[nr-1].eth, buf_p); } }
	     	      	else if(ptr=strstr(var,"s_Lan_ip")){  nr=Cgi_liczba(ptr);  if(BufNotIdentity(Const.s_Lan[nr-1].ip,  buf_p)) w=1;    sprintf(Const.s_Lan[nr-1].ip,  buf_p); }
	     	      	else if(ptr=strstr(var,"s_Lan_mask")){ nr=Cgi_liczba(ptr); if(BufNotIdentity(Const.s_Lan[nr-1].mask,buf_p)) w=1;    sprintf(Const.s_Lan[nr-1].mask,buf_p); }
	     	      	else if(ptr=strstr(var,"s_Lan_br")){  nr=Cgi_liczba(ptr);  if(BufNotIdentity(Const.s_Lan[nr-1].br,  buf_p)) w=1;    sprintf(Const.s_Lan[nr-1].br,  buf_p); }
	     	      	else if(ptr=strstr(var,"s_Lan_port")){ nr=Cgi_liczba(ptr); if(BufNotIdentity(Const.s_Lan[nr-1].port,buf_p)) w=1;    sprintf(Const.s_Lan[nr-1].port,buf_p); }
	     	      	else if(ptr=strstr(var,"s_Lan_param"))
	     	      	{ nr=Cgi_liczba(ptr);
	     	      		      if(strstr(buf_p,"tak")){  if((Const.s_Lan[nr-1].param&0x01)==0) w=1;   Const.s_Lan[nr-1].param|=0x01; }  //DHCP ON
	     	      		 else if(strstr(buf_p,"nie")){  if((Const.s_Lan[nr-1].param&0x01)>0) w=1;    Const.s_Lan[nr-1].param&=~0x01;}  //DHCP OFF
	     	      	}
	     	      	else if(ptr=strstr(var,"s_Wifi_par1"))
	     	      	{ nr=Cgi_liczba(ptr);

	     	      	   if(nr==1){
	     	      	          if(strstr(buf_p,"tak")){  if((Const.s_Lan[0].param&0x04)==0) w=1;   Const.s_Lan[0].param|=0x04; }   //STA ON
	     	      		 else if(strstr(buf_p,"nie")){  if((Const.s_Lan[0].param&0x04)>0) w=1;    Const.s_Lan[0].param&=~0x04;  Const.s_Lan[nr-1].param|=0x01; }   //STA OFF  (jesli STA OFF to DHCP ON)
	     	      	   }
	     	      	   else if(nr==2){
	     	      		      if(strstr(buf_p,"tak")){  if((Const.s_Lan[0].param&0x08)==0) w=1;   Const.s_Lan[0].param|=0x08; }   //AP ON
	     	      		 else if(strstr(buf_p,"nie")){  if((Const.s_Lan[0].param&0x08)>0) w=1;    Const.s_Lan[0].param&=~0x08;}   //AP OFF
	     	      	   }
	     	      	}
	     	      	else if(ptr=strstr(var,"s_Wifi_ssid"))
	     	      	{  nr=Cgi_liczba(ptr);
	     	      	   if(nr==1) sprintf(Const.s_Wifi[0].ssid, buf_p);  else sprintf(Const.s_Wifi[0].AP_ssid, buf_p);
	     	      	}
	     	      	else if(ptr=strstr(var,"s_Wifi_haslo"))
	     	      	{  nr=Cgi_liczba(ptr);
	     	      	   if(nr==1) sprintf(Const.s_Wifi[0].haslo, buf_p);  else sprintf(Const.s_Wifi[0].AP_haslo, buf_p);
	     	      	}
	     	      	//else if(ptr=strstr(var,"s_Wifi_AP_ssid")){ nr=Cgi_liczba(ptr);  sprintf(Const.s_Wifi[nr-1].AP_ssid, buf_p);}
	     	      	//else if(ptr=strstr(var,"s_Wifi_AP_haslo")){ nr=Cgi_liczba(ptr); sprintf(Const.s_Wifi[nr-1].AP_haslo, buf_p);}
	     	      	else if(ptr=strstr(var,"s_email_ip")){   nr=Cgi_liczba(ptr);sprintf(Const.s_Email[nr-1].ip, buf_p);}
	     	      	else if(ptr=strstr(var,"s_email_nad")){   nr=Cgi_liczba(ptr);sprintf(Const.s_Email[nr-1].nad, buf_p);}
	     	      	else if(ptr=strstr(var,"s_email_has")){   nr=Cgi_liczba(ptr);sprintf(Const.s_Email[nr-1].has, buf_p);}
	     	      	else if(ptr=strstr(var,"s_email_port")){   nr=Cgi_liczba(ptr);sprintf(Const.s_Email[nr-1].port, buf_p);}

else if(ptr=strstr(var,"s_email_odb"))
{  k=(*(ptr+11))&0x0f;  k--;      if(k==0){   for(i=0;i<_Ilosc_Mail;i++) Const.s_Rap[0].mail[i]=0;    }
   nr=Cgi_liczba(ptr);   sprintf(Const.s_Email[nr-1].odb1+41*k, buf_p);
	 /* Const.s_Ma[0].mail[0]&=~0x7F;
	  Const.s_Ma[1].mail[0]&=~0x7F;
	  Const.s_Ma[2].mail[0]&=~0x7F;
	  Const.s_Ma[3].mail[0]&=~0x7F;
	  Const.s_Ma[4].mail[0]&=~0x7F;
	  Const.s_Ma[5].mail[0]&=~0x7F;
	  Const.s_Ma[6].mail[0]&=~0x7F;
	  Const.s_Ma[7].mail[0]&=~0x7F;*/

   for(i=0;i<_Ilosc_Tel;i++)
   {   for(j=0;j<(1+(_Size_te-1)/16);j++){
	     Const.s_Ma[i].mail[j]=0;
	   }
   }


}
	     // ----------- Usatw Maski EMAIL Raport  ----------------------------------
	     else if(ptr=strstr(var,"s_merrr"))
	     {
	     	          k= 100*((*(ptr+7))&0x0f)+10*((*(ptr+8))&0x0f)+((*(ptr+9))&0x0f);
	     	          Const.s_Rap[0].mail[k-1] |= 0x01;
	     }

else if(ptr=strstr(var,"s_email_tryb"))
{    nr=Cgi_liczba(ptr);
     	  if(strstr(buf_p,"0")){   Const.s_Lan[0].param&=~0x10; }  //Email ENC
     else if(strstr(buf_p,"1")){   Const.s_Lan[0].param|=0x10;}    //Email ESP
}

//------------------------------- Nazwy Portów ------------------------------------------------------
else if(ptr=strstr(var,"s_Port_nazwa"))
{  k= wybierz_cyfry(ptr+12);  k--;
   nr=Cgi_liczba(ptr);  sprintf(Const.s_GPIO[k].nazwa, buf_p);
}
else
{
	HttpLogicParser_ENC(var,buf_p);

	if(strstr(var,"s_SigWy"))
	{
	        dbg3("\r\nZapis...");
	         Flash_in_progr_char_na_32(_AdrFlashToRAM, Const.s_Lan[0].eth ,_Size_Zmienne_zapisywalne);
	        dbg3("ok ");
	        FunctionLogicOperation();
	}
}
/*else if(ptr=strstr(var,"s_SW1a")) Const.InitGate[0].NameInputSignal[0] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW2a")) Const.InitGate[0].NameInputSignal[1] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW3a")) Const.InitGate[0].NameInputSignal[2] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW4a")) Const.InitGate[0].NameInputSignal[3] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW5a")) Const.InitGate[0].NameInputSignal[4] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW6a")) Const.InitGate[0].NameInputSignal[5] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW7a")) Const.InitGate[0].NameInputSignal[6] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW8a")) Const.InitGate[0].NameInputSignal[7] = (buf_p[0]&0x0f);

else if(ptr=strstr(var,"s_SW1b")) Const.InitGate[1].NameInputSignal[0] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW2b")) Const.InitGate[1].NameInputSignal[1] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW3b")) Const.InitGate[1].NameInputSignal[2] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW4b")) Const.InitGate[1].NameInputSignal[3] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW5b")) Const.InitGate[1].NameInputSignal[4] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW6b")) Const.InitGate[1].NameInputSignal[5] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW7b")) Const.InitGate[1].NameInputSignal[6] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW8b")) Const.InitGate[1].NameInputSignal[7] = (buf_p[0]&0x0f);

else if(ptr=strstr(var,"s_SW1c")) Const.InitGate[2].NameInputSignal[0] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW2c")) Const.InitGate[2].NameInputSignal[1] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW3c")) Const.InitGate[2].NameInputSignal[2] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW4c")) Const.InitGate[2].NameInputSignal[3] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW5c")) Const.InitGate[2].NameInputSignal[4] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW6c")) Const.InitGate[2].NameInputSignal[5] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW7c")) Const.InitGate[2].NameInputSignal[6] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW8c")) Const.InitGate[2].NameInputSignal[7] = (buf_p[0]&0x0f);

else if(ptr=strstr(var,"s_SW1d")) Const.InitGate[3].NameInputSignal[0] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW2d")) Const.InitGate[3].NameInputSignal[1] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW3d")) Const.InitGate[3].NameInputSignal[2] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW4d")) Const.InitGate[3].NameInputSignal[3] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW5d")) Const.InitGate[3].NameInputSignal[4] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW6d")) Const.InitGate[3].NameInputSignal[5] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW7d")) Const.InitGate[3].NameInputSignal[6] = (buf_p[0]&0x0f);
else if(ptr=strstr(var,"s_SW8d")) Const.InitGate[3].NameInputSignal[7] = (buf_p[0]&0x0f);


else if(ptr=strstr(var,"s_SP1e")){ Const.AddGate[0].NameInputSignal[0] = (buf_p[0]&0x0f); dbg3("\r\ns_SP1e  "); dbg3(buf_p); }
else if(ptr=strstr(var,"s_SP2e")){ Const.AddGate[0].NameInputSignal[1] = (buf_p[0]&0x0f);  dbg3("\r\ns_SP2e  ");  dbg3(buf_p);}
else if(ptr=strstr(var,"s_SP3e")){ Const.AddGate[0].NameInputSignal[2] = (buf_p[0]&0x0f); dbg3("\r\ns_SP3e  "); dbg3(buf_p); }
else if(ptr=strstr(var,"s_SP4e")){ Const.AddGate[0].NameInputSignal[3] = (buf_p[0]&0x0f); dbg3("\r\ns_SP4e  "); dbg3(buf_p); }

else if(ptr=strstr(var,"s_SP1f")){ Const.AddGate[1].NameInputSignal[0] = (buf_p[0]&0x0f); dbg3("\r\ns_SP1f  "); dbg3(buf_p); }
else if(ptr=strstr(var,"s_SP2f")){ Const.AddGate[1].NameInputSignal[1] = (buf_p[0]&0x0f); dbg3("\r\ns_SP2f  "); dbg3(buf_p); }
else if(ptr=strstr(var,"s_SP3f")){ Const.AddGate[1].NameInputSignal[2] = (buf_p[0]&0x0f); dbg3("\r\ns_SP3f  "); dbg3(buf_p); }
else if(ptr=strstr(var,"s_SP4f")){ Const.AddGate[1].NameInputSignal[3] = (buf_p[0]&0x0f); dbg3("\r\ns_SP4f  "); dbg3(buf_p); }

else if(ptr=strstr(var,"s_SP1g")){ Const.FinGate[0].NameInputSignal[0] = (buf_p[0]&0x0f); dbg3("\r\ns_SP1g  "); dbg3(buf_p); }
else if(ptr=strstr(var,"s_SP2g")){ Const.FinGate[0].NameInputSignal[1] = (buf_p[0]&0x0f); dbg3("\r\ns_SP2g  "); dbg3(buf_p); }
*/



}

void Http_cgi(char *buf)   //cgi
{
	char *ptr,*ptr1,m,a; char var[31],buf_p[220],pi;  int i,j,k,nr,bb;  signed short ttemp;



     k=0; m=0;  nr=0;  bb=_Size_BufWifi;
	 next_ipd:
	                  buf_IPD[0]='+';
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

     ptr1=&buf_rx2[0];
	 if(ptr=strstr(ptr1,buf_IPD))
	 {
		  if(m==1) k=ptr-2-ptr1; else k=0;    // dbg3("\r\nKFFFFF !!!!  ");

		        while(*ptr!=',') ptr++;  ptr++;
		        while(*ptr!=',') ptr++;  ptr++;
		        while(*ptr!=':') ptr++;  ptr++;
		       // i=0; while(*ptr!=':'){ buf_p[i++]=*ptr;  ptr++;}  ptr++;  buf_p[i]=0;

		       // sprintf(buf_p,"\r\nBBBBBB: %d  ",liczba_char_na_int() ); dbg3(buf_p);


		       // if(liczba_char_na_int()<2){ goto omin_ipd; }

		  i=0; do{  buf_rx2[k+i]=*(ptr+i);  }while(*(ptr+i++)!=0);
          m=1; nr++;// if(nr>((bb/1460)+1)) goto omin_ipd;
          goto next_ipd;
	 }
	 omin_ipd:
    // dbg3("\r\n\r\n");   dbg3(buf_rx2);  dbg3("\r\n\r\n");

    w=0;  //sprawdzenie identycznosci parametrów LAN
	ptr1=buf;
	do
	{
	    if(ptr=strstr(ptr1,"n_s_"))   //cgi
	    {
	      	i=0;
	      	powtorz_n_s:
	      	if( (*(ptr+2+i+0)=='_') && ((*(ptr+2+i+1)>0x2F)&&(*(ptr+2+i+1)<0x3A)) && ((*(ptr+2+i+2)>0x2F)&&(*(ptr+2+i+2)<0x3A)) && ((*(ptr+2+i+3)>0x2F)&&(*(ptr+2+i+3)<0x3A)) )
	      	{
	      	     nr = (0x0f&(*(ptr+2+i+1)))*100 + (0x0f&(*(ptr+2+i+2)))*10 +(0x0f&(*(ptr+2+i+3)))*1;
	      	     k=2+i;
	      	   var[i+0]=*(ptr+2+i+0);
	      	   var[i+1]=*(ptr+2+i+1);
	      	   var[i+2]=*(ptr+2+i+2);
	      	   var[i+3]=*(ptr+2+i+3);
	      	   var[i+4]=0;
	      	}
	      	else{ var[i]=*(ptr+2+i); i++; goto powtorz_n_s; }



	      	i=0;  do{  i++; if(i>200){  goto omincgi;} }while(*(ptr+k+i)!='=');  i++;  j=i+k;
	      	i=0;
	      	do
	      	{
	      	    buf_p[i]=*(ptr+j+i);
	      	    if(buf_p[i]=='&') break;
	      	    if(buf_p[i]=='+') buf_p[i]=' ';
	      	    i++;  if(i>200){ goto omincgi;}
	      	}while(*(ptr+j+i)!='&');  buf_p[i]=0;
	      	while(korekcja_formatu_GET(buf_p,strlen(buf_p)));

	     // dbg3("\r\nZZZZ ");dbg3(var); dbg3("  ");  dbg3(buf_p);

//------------------------------- Wpisy ASP ------------------------------------------------------

               CGI_Wpis(var,buf_p);

	    }
	    else
	    {
             ptr=CGI_Wpis2(ptr1,&pi);
             if(pi==0) break;

	    }
	    omin_cgi:
	    ptr1=ptr+5;

	 }while(1);
	 omincgi:
	 asm("nop");
}



void HttpOffsetService()
{
	 if((WhichPage==18)||(WhichPage==19))
	 {
		 if((WhichFragmentPage>2)&&(WhichFragmentPage<5+_Ilosc_Tel+1))  HttpCountOffset[HttpCount-1]=-1;
	 }
}

void SendHttpData(char *txt, int size)
{
	 while(HAL_UART_Transmit_DMA(&huart1, txt, size)==HAL_BUSY);
	//while(HAL_UART_Transmit(&huart2, txt, strlen(txt),10)==HAL_BUSY);
}

int fff()
{
	int k,i,jj;  char *ptr,*ptr1,id[6];

	jj=0;
	dfdsfvv:
	wsk_1s=0;
	//HAL_IWDG_Refresh(&hiwdg);
	k=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); k++; if(k>100) return 1;}
	HttpDataSendService();
	k=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	ptr=buf_rx2;


	desax:
	if(ptr1=strstr(ptr,"+IPD,"))
	{
		if(jj<HTTP_Len_Reload)
		{
		       if(strstr(ptr,"GET /lpc.cgi")){ ptr=ptr1+5; goto desax; }
		  else if(strstr(ptr,"GET /"))
		  {
			if(HttpReload[0+jj*50]==0)
			{
			   k=0;
		       HttpReload[0+jj*50]=1;
		       HttpReload[1+jj*50]='\r';
		       HttpReload[2+jj*50]='\n';
		       i=0; do{   HttpReload[3+jj*50+i]=*(ptr1+i);  if(i>49) break;  }while(*(ptr1+i++)!='\r');
		       HttpReload[3+jj*50+i-1]=0;  dbg3("\r\n"); dbg3(&HttpReload[1+jj*50]);
			}
			else
			{
				jj++; goto desax;
			}
		  }
		  else{ ptr=ptr1+5; goto desax; }
		  ptr=ptr1+5;  jj++; goto desax;
		}
	}

	ResumptionReceiveDataFromWifi();
	if(HttpCommandSendService()==0) goto dfdsfvv;

	k=0; while(strstr(buf_rx2,"CLOSED")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	ResumptionReceiveDataFromWifi(); licz_htt=20;


	/*for(i=0;i<jj;i++)
	{
       sprintf(buf_p,"\r\n*%d %s",i,&HttpReload[3+i*50]); dbg3(buf_p);
	}*/






/*
 	int k,i,jj;  char *ptr,*ptr1,id[6];

	jj=0;
	dfdsfvv:
	HAL_IWDG_Refresh(&hiwdg);
	k=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	HttpDataSendService();
	k=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	ptr=buf_rx2;

	//if(ptr1=strstr(ptr,"+IPD,")) ptr=ptr1+5;
	desax:
	if(ptr1=strstr(ptr,"+IPD,"))
	{
        id[jj]=*(ptr1+5);
        if(ptr=strstr(ptr1,"GET /"))

        k=0;
		do{   HttpReload[1+jj*20+k]=*(ptr+5+k);    }while(*(ptr+5+k++)!=' ');
		HttpReload[1+jj*20+k-1]=0;  dbg3("\r\n"); dbg3(&HttpReload[1+jj*20]);

		ptr=ptr1+5;  jj++; goto desax;
	}

	ResumptionReceiveDataFromWifi();
	if(HttpCommandSendService()==0) goto dfdsfvv;

	k=0; while(strstr(buf_rx2,"CLOSED")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	ResumptionReceiveDataFromWifi(); licz_htt=20;


	for(i=0;i<jj;i++)
    {
	    sprintf(buf_p,"AT+CIPSEND=%c,10\x0D\x0A",id[i]); dbg3("   "); dbg3(buf_p);
	    SendDataToModemWifi(buf_p);
	    k=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	    SendHttpData("0123456789", 10);
	    k=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}

	    ResumptionReceiveDataFromWifi();
	    sprintf(buf_p,"AT+CIPCLOSE=%c\x0D\x0A",id[i]); dbg3("   "); dbg3(buf_p);
	    SendDataToModemWifi(buf_p);
	    k=0; while(strstr(buf_rx2,",CLOSED")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
    }

	for(i=0;i<jj;i++)
	{
       sprintf(buf_p,"\r\n*: %c %s",id[i],&HttpReload[1+i*20]); dbg3(buf_p);
	}
    if(jj>0) HttpReload[0]=1;
    else     HttpReload[0]=0;




 */

	return 0;
}


void SprawdzZadanieGET()
{
	int jj,k;    char *ptr,*ptr1;

	jj=0;
	ptr=buf_rx2;
	if(ptr1=strstr(ptr,"+IPD,")) ptr=ptr1+5;

	desax1:
	if(ptr1=strstr(ptr,"+IPD,"))
	{
		if(jj<HTTP_Len_Reload)
		{
		       if(strstr(ptr,"GET /lpc.cgi")){ ptr=ptr1+5; goto desax1; }
		  else if(strstr(ptr,"GET /"))
		  {
			if(HttpReload[0+jj*50]==0)
			{
			   k=0;
		       HttpReload[0+jj*50]=1;
		       HttpReload[1+jj*50]='\r';
		       HttpReload[2+jj*50]='\n';
		       i=0; do{   HttpReload[3+jj*50+i]=*(ptr1+i);  if(i>49) break;  }while(*(ptr1+i++)!='\r');
		       HttpReload[3+jj*50+i-1]=0;  dbg3("\r\n"); dbg3(&HttpReload[1+jj*50]);
			}
			else
			{
				jj++; goto desax1;
			}
		  }
		  else{ ptr=ptr1+5; goto desax1; }
		  ptr=ptr1+5;  jj++; goto desax1;
		}
	}
}

/*int fff2()
{
	int k,i,jj;  char *ptr,*ptr1,id[6],buf[6*50];

	jj=0;
	dfdsfvv:
	HAL_IWDG_Refresh(&hiwdg);


	ptr=buf_rx2;
	desax:
	if(ptr1=strstr(ptr,"+IPD,"))
	{
        id[jj]=*(ptr1+5);
        if(ptr=strstr(ptr1,"GET /"))

        k=0;
		do{   buf[jj*50+k]=*(ptr+5+k);    }while(*(ptr+5+k++)!=' ');
		buf[jj*50+k-1]=0;

		ptr=ptr1+5;  jj++; goto desax;
	}



	for(i=0;i<jj;i++)
    {
	    sprintf(buf_p,"AT+CIPSEND=%c,10\x0D\x0A",id[i]); dbg3("   "); dbg3(buf_p);
	    SendDataToModemWifi(buf_p);
	    k=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	    SendHttpData("0123456789", 10);
	    k=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}

	    ResumptionReceiveDataFromWifi();
	    sprintf(buf_p,"AT+CIPCLOSE=%c\x0D\x0A",id[i]); dbg3("   "); dbg3(buf_p);
	    SendDataToModemWifi(buf_p);
	    k=0; while(strstr(buf_rx2,",CLOSED")==0){ HAL_Delay(5); k++; if(k>3000) return 1;}
    }

	for(i=0;i<jj;i++)
	{
       sprintf(buf_p,"\r\n*: %c %s",id[i],&buf[i*50]); dbg3(buf_p);
	}








	return 0;
}*/

void HttpPageStartSend(void)
{
	int k;
	HttpCount=1;
	//HttpCountOffset=0;

	FragmentPagePacketCounter=0;
       	     WhichFragmentPage=0;
       	     load_page(WhichPage);
       	     FragmentPagePointer = HttpPagesBuffer[WhichFragmentPage];
       	     k=strlen(FragmentPagePointer);
       	     FragmentPagePacketNumber = k/PacketLen;
       	     if(strlen(FragmentPagePointer)%PacketLen==0);  else FragmentPagePacketNumber++;
       	     //sprintf(buf_p,"\r\nDLG%d: %d %d ",WhichFragmentPage,k,FragmentPagePacketNumber);  dbg3(buf_p);

    		  if(k<PacketLen) sprintf(buf_p,"AT+CIPSEND=%c,%d\x0D\x0A",HttpPort,k);
    		  else            sprintf(buf_p,"AT+CIPSEND=%c,%d\x0D\x0A",HttpPort,PacketLen);
    		  dbg3(buf_p);
    		  SendDataToModemWifi(buf_p);  li_WifiCipsend[0]=1;


      if(fff()==1) InitUartWifiHttp();

	  ResumptionReceiveDataFromWifi();
	  for(i=0;i<HTTP_Len_Reload;i++)
	  {
		  if(HttpReload[0+i*50]==1)
		  {
			  if(strstr(&HttpReload[1+i*50],"+IPD,"))
			  {
				  sprintf(buf_rx2,&HttpReload[1+i*50]);
				  HttpReload[0+i*50]=0;
				  break;
			  }
			  else
			  {
				  HttpReload[0+i*50]=0;
			  }
		  }
	  }
	  wsk_1s=0;
}

int HttpRefreshSend(int size)
{
	int k;
	FragmentPagePacketCounter=999;
	sprintf(buf_p,"AT+CIPSEND=%c,%d\x0D\x0A",HttpPort,size);  dbg3(buf_p);
	SendDataToModemWifi(buf_p);  li_WifiCipsend[0]=1;
	 k=0; while(strstr(buf_rx2,"\r\n>")==0){ HAL_Delay(5); k++; if(k>100) return 1;}
	 HttpDataSendService();
	 k=0; while(strstr(buf_rx2,"\r\nSEND OK")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
	 HttpCommandSendService();
 	 k=0; while(strstr(buf_rx2,"CLOSED")==0){ HAL_Delay(5); k++; if(k>1000) return 1;}
 	ResumptionReceiveDataFromWifi();

 	return 0;
}

void NrPageToString(int nr, char *tx)
{
      if(nr==11) sprintf(tx,"temp");
 else if(nr==18)  sprintf(tx,"maskimail");
 else if(nr==19)  sprintf(tx,"maskisms");
 else if(nr==10) sprintf(tx,"term");
 else if(nr==12) sprintf(tx,"tim");
 else if(nr==14) sprintf(tx,"gsm");
 else if(nr==2)  sprintf(tx,"upload");
 else if(nr==999)  sprintf(tx,"load");
 else if(nr==7)  sprintf(tx,"rstdp");
 else if(nr==4)  sprintf(tx,"wifi");
 else if(nr==78) sprintf(tx,"lan");
 else if(nr==79) sprintf(tx,"czas");
 else if(nr==80) sprintf(tx,"rej");
 else if(nr==81) sprintf(tx,"plik.txt");
 else if(nr==82) sprintf(tx,"pomiar");
 else if(nr==83) sprintf(tx,"plik.csv");
 else if(nr==6)  sprintf(tx,"main");
 else if(nr==5)  sprintf(tx,"searchwifi");
 else if(nr==3)  sprintf(tx,"wykres1");
 else if(nr==997)  sprintf(tx,"wykres2");
 //else if(nr==7)  sprintf(tx,"gpio");
 else if(nr==9)  sprintf(tx,"nrf");
 else if(nr==8)  sprintf(tx,"favicon");
 else if(nr==15)  sprintf(tx,"3main");
 else if(nr==20) sprintf(tx,"Auth");
 else if(nr==990) sprintf(tx,"obraz");
 else if(nr==991) sprintf(tx,"KRZ");
 else if(nr==992) sprintf(tx,"sesjaSMTP.txt");
 else if(nr==998) sprintf(tx,"sesjaLogGSM.txt");
 else if(nr==993) sprintf(tx,"liczgodz.txt");
 else if(nr==994) sprintf(tx,"liczdob.txt");
 else if(nr==995) sprintf(tx,"licztyg.txt");
 else if(nr==996) sprintf(tx,"liczmies.txt");
 else if(nr==21) sprintf(tx,"agh");
 else if(nr==902) sprintf(tx,"Rot");
 else if(nr==1669) sprintf(tx,"logic");


  else if((nr>30)&&(nr<100)) sprintf(tx,"etr%03d",nr-30);
  else if((nr>200)&&(nr<400)) sprintf(tx,"temp%03d",nr-200);
  else if((nr>400)&&(nr<500)) sprintf(tx,"we%03d",  nr-400);
  else if((nr>500)&&(nr<600)) sprintf(tx,"wypk%03d",nr-500);
  else if((nr>600)&&(nr<700)) sprintf(tx,"tim%03d", nr-600);
  else if((nr>700)&&(nr<800)) sprintf(tx,"term%03d",nr-700);
  else if((nr>800)&&(nr<900)) sprintf(tx,"nap%03d", nr-800);
  else if((nr>1000)&&(nr<1100)) sprintf(tx,"eol%03d", nr-1000);
  else sprintf(tx,"3main");

}

int HttpWhichPage(void)
{
	char *ptr,buf_p[20];

	     sprintf(&buf_rx2[_Size_BufWifi],HttpRefresh);
	     if(ptr=strstr(&buf_rx2[_Size_BufWifi],";url=../"))
	     {
	    	 NrPageToString(WhichPage, buf_p);

             for(i=0;i<strlen(buf_p);i++) *(ptr+8+i)=buf_p[i];
   	              if((WhichPage==3)||(WhichPage==997)) sprintf(ptr+8+i,"\"></head><body></body></html>");
   	         else if(WhichPage==82)                    sprintf(ptr+8+i,"\"></head><body bgcolor=\"bbbbbb\"></body></html>");
   	         else                                      sprintf(ptr+8+i,"\"></head><body bgcolor=\"000000\"></body></html>");
	     }
	     return  strlen(&buf_rx2[_Size_BufWifi]);
}

void HttpDataSendService(void)   //">"
{
   	char buf_p[20]; int k,i;

	 if(FragmentPagePacketCounter<(FragmentPagePacketNumber-1)){    SendHttpData(FragmentPagePointer+PacketLen*FragmentPagePacketCounter, PacketLen);  }
   	 else if(FragmentPagePacketCounter==999)
   	 {
   		    SendHttpData(&buf_rx2[_Size_BufWifi], HttpWhichPage());

   	 }
   	 else{    SendHttpData(FragmentPagePointer+PacketLen*FragmentPagePacketCounter, strlen(FragmentPagePointer)-PacketLen*(FragmentPagePacketNumber-1));}

   	 FragmentPagePacketCounter++;
}

int HttpCommandSendService(void)    //"SEND OK"
{
	int a1,a2,a3;
	int k,k1,x, _return;   _return=0;     if((wybor_bitowy&0x0400)>0) ObslugaTimer();
	    if(FragmentPagePacketCounter<(FragmentPagePacketNumber-1))
    	{
    		sprintf(buf_p,"AT+CIPSEND=%c,%d\x0D\x0A",HttpPort,PacketLen); dbg3("   "); dbg3(buf_p);
    		SendDataToModemWifi(buf_p);  li_WifiCipsend[0]=1;

    	}
    	else if(FragmentPagePacketCounter==(FragmentPagePacketNumber-1))
    	{
    		sprintf(buf_p,"AT+CIPSEND=%c,%d\x0D\x0A",HttpPort, strlen(FragmentPagePointer)-PacketLen*(FragmentPagePacketNumber-1)); dbg3("   "); dbg3(buf_p);
    		SendDataToModemWifi(buf_p);  li_WifiCipsend[0]=1;

    	}
    	else if(FragmentPagePacketCounter==(FragmentPagePacketNumber-1)+1)
    	{
    	    if(HttpPagesBuffer[WhichFragmentPage+1]!=0)
    		{
    		   	  WhichFragmentPage++;



    		 if(WhichPage==6){
    				if(WhichFragmentPage==1) http_itx=odd-1;

    				if((WhichFragmentPage>1)&&(WhichFragmentPage<10))
    				{
    					if(WhichFragmentPage==2)
    					{
    						i=MainPanel_temp(&http_itx);
    						if(i!=-1) WhichFragmentPage--;  else http_itx=odd-1;
    					}
    					else if(WhichFragmentPage==3)
    					{
    						i=MainPanel_licznik(&http_itx);
    						if(i!=-1) WhichFragmentPage--;
    					}
    				    else if(WhichFragmentPage==4) MainPanel_we();
    				    else if(WhichFragmentPage==5) MainPanel_eol();
    				    else if(WhichFragmentPage==6) MainPanel_adc();
    				    else if(WhichFragmentPage==7) MainPanel_pwm();
    					else if(WhichFragmentPage==8) MainPanel_pk();
    					else if(WhichFragmentPage==9){   MainPanel_poz();  http_itx=0;  }

    					HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    				}
    				else if(WhichFragmentPage>9)
    				{
    					i= HttpMainScript(&http_itx);
    					if(i==-1)
    				 	{
    				 		HttpPagesBuffer[WhichFragmentPage+1]=0;
    				 		sprintf(buf_p,"\r\nKONIEC: %d  ",http_itx); dbg3(buf_p);     a3=strlen(&buf_rx2[_Size_BufWifi]);  sprintf(&buf_rx2[_Size_BufWifi+a3],"\r\n</html>");
    				 	}
    				 	else
    				 	{
    				 		sprintf(buf_p,"\r\nXXX: %d  %d ",http_itx, i); dbg3(buf_p);
    				 	}
    				    HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    				}
    		 }


    		 if((WhichPage==18)||(WhichPage==19)){
    		 	  if(WhichFragmentPage==2)
    		 	  {	 if(WhichPage==19) HttpMaskiSmsEmail_startA('S');
    		 	     else              HttpMaskiSmsEmail_startA('E');
    		          HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    		 	  }
    		 	  else if(WhichFragmentPage==3)
    		 	  {	 if(WhichPage==19) HttpMaskiSmsEmail_startAB('S');
    		 	     else              HttpMaskiSmsEmail_startAB('E');
    		          HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    		 	  }
    		 	  else if(WhichFragmentPage==4)
    		 	  {	 if(WhichPage==19) HttpMaskiSmsEmail_startB('S');
    		 	     else              HttpMaskiSmsEmail_startB('E');
    		          HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    		 	  }
    		 	  else if(WhichFragmentPage==5)
    		 	  {   HttpMaskiSmsEmail_start_2();
    		 	      HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    		 	  }
    		 if((WhichFragmentPage>5)&&(WhichFragmentPage<(5+_Ilosc_Tel+1)))
    		 {
    		 	if(WhichPage==19) HttpMaskiSmsEmail_wsad(WhichFragmentPage-5,'S');
    		 	else              HttpMaskiSmsEmail_wsad(WhichFragmentPage-5,'E');
    		     HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    		     HttpCount= WhichFragmentPage-5;
    		 }
    		 }


if(WhichPage==15){


	a3= (3+((ddo-(odd-1))/4)+(((ddo-(odd-1))%4)== 0 ? 0 : 1));


	      if(WhichFragmentPage==1) http_itx=0;
	 else if(WhichFragmentPage==2)
	 {
		HttpNazwyPortow(&http_itx);
        if(http_itx==(ddo+1)) http_itx=1;
        else
        {	WhichFragmentPage--;

        }
        HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	 }
	 else if( (WhichFragmentPage>2) && (WhichFragmentPage<a3)    )
	 {
		if(http_itx < (ddo+1))
		{
			a1= http_itx+(odd-1);    a2= a1+3;   if(a2>ddo) a2=ddo;
			HttpMain_22(a1,a2); http_itx+=4;
			HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		}
	 }
	 else if( (WhichFragmentPage>(a3-1)) && (WhichFragmentPage<(a3+ddo-(odd-1))))
	 {
		 NewPanelID_Dyn(WhichFragmentPage+(odd-1)-a3);  http_itx=0;
		 HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	 }
	/* else if(WhichFragmentPage>(a3+ddo-(odd-2)))
	 {
		 i= HttpMainScript(&http_itx);
		if(i==-1)
	 	{
	 		HttpPagesBuffer[WhichFragmentPage+1]=0;
	 		sprintf(buf_p,"\r\nKONIEC: %d  ",http_itx); dbg3(buf_p);
	 	}
	 	else
	 	{
	 		sprintf(buf_p,"\r\nXXX: %d  %d ",http_itx, i); dbg3(buf_p);
	 	}
	    HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	 }*/
	 else if(WhichFragmentPage>(a3+ddo-(odd-2)))
	 {
		if(http_itx==20000)
		{
			HttpAntenaLora();
			HttpPagesBuffer[WhichFragmentPage+1]=0;
		    sprintf(buf_p,"\r\nKONIEC: %d  ",http_itx); dbg3(buf_p);
		}
		else
		{
		    i= HttpMainScript(&http_itx);    HAL_IWDG_Refresh(&hiwdg);
		    if(i==-1)
	 	    {
		    	http_itx=20000;
	 		    sprintf(buf_p,"\r\naaaaaa: %d  ",http_itx); dbg3(buf_p);
	 	    }
	 	    else
	 	    {
	 		    sprintf(buf_p,"\r\nXXX: %d  %d ",http_itx, i); dbg3(buf_p);
	 	    }
		}
	    HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	 }
}

if((WhichPage>30)&&(WhichPage<35))
{
	      if(WhichFragmentPage==3){ HttpLiczProgi_Start();  HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi]; }
	 else if(WhichFragmentPage==4){ HttpLiczProgi((WhichPage-30),'g'); HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];}
	 else if(WhichFragmentPage==5){ HttpLiczProgi((WhichPage-30),'d'); HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];}
	 else if(WhichFragmentPage==6){ HttpLiczProgi((WhichPage-30),'t'); HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];}
	 else if(WhichFragmentPage==7){ HttpLiczProgi((WhichPage-30),'m'); HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];}


}

if((WhichPage>70)&&(WhichPage<78)){
    if((WhichFragmentPage>2)&&(WhichFragmentPage<23))
    {
	    HttpTabelaTranslacji((WhichFragmentPage-3)*5, WhichPage-71 );
	    HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    }
    else if(WhichFragmentPage==23)
    {
	    Http_TT_zapisz(WhichPage-71);
	    HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    }
}

if(WhichPage==3)  //Wykres
{
	if(WhichFragmentPage==1)
	{
		HttpWykresStart_0();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==2)
	{
		HttpWykresStart_1();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==3)
	{
		HttpWykresStart_2();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==4)
	{
		HttpWykresStart_3();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if((WhichFragmentPage>4)&&(WhichFragmentPage<83))
	{
		HttpWykresDane();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		HAL_IWDG_Refresh(&hiwdg);
	}
}

if(WhichPage==997)  //Wykres dla liczników
{
	if(WhichFragmentPage==1)
	{
		HttpWykresStartEtr_0();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==2)
	{
		HttpWykresStartEtr_1();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==3)
	{
		HttpWykresStartEtr_2();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==4)
	{
		HttpWykresStartEtr_3();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if((WhichFragmentPage>4)&&(WhichFragmentPage<83))
	{
		HttpWykresDaneEtr();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		HAL_IWDG_Refresh(&hiwdg);
	}
}

if((WhichPage==7)||(WhichPage==991))  //RST
{
	if(WhichFragmentPage==1)
	{
		     if(WhichPage==7)   HttpRst(1);
		else if(WhichPage==991) HttpRst(2);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
}

if(WhichPage==1669)  //Operacje logiczne
{
	if(WhichFragmentPage==4)
	{
		HttpLogicEntry();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
}

if((WhichPage>800)&&(WhichPage<900)){
    if((WhichFragmentPage>2)&&(WhichFragmentPage<23))
    {
	    HttpTabelaTranslacji((WhichFragmentPage-3)*5, WhichPage-801 );
	    HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    }
    else if(WhichFragmentPage==23)
    {
	    Http_TT_zapisz(WhichPage-801);
	    HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
    }
}

if(WhichPage==82)     //wybor_wykres
{  if(WhichFragmentPage==1)
   {     HttpWyborWykres();
	     HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
   }
   else if(WhichFragmentPage==2)
   {     HttpWyborWykres2();
	     HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
   }
}

if(WhichPage==81)  //plik.txt
{
	if((WhichFragmentPage>0)&&(WhichFragmentPage<81))
	{
        if(WhichFragmentPage==1)
        {
           adr_flash_read=adr_flash_start;  IteracjaFlashZewnOdczyt(&adr_flash);
           if((adr_flash==0)||(adr_flash_start==adr_flash_end)){ WhichPage=WhichPage_p;  WhichFragmentPage=20; goto koniecWP81; }

           j=0;   //01.01 00:08   +00.0   +00.0   +20.7
       	   for(i=0;i<_Size__s_gpio;i++)
       	   {
       		       x= WyszukajNrCzujkiDlaPortu(i+1);
       			   if(x!=-1)
       			   {
       		    	  if(Const.s_GPIO[i].val==9)
       		    	  {
       		    		sprintf(&buf_rx2[_Size_BufWifi+j],"\r\nTemp port %d - %s",i+1,Const.s_Czujki[x].nazwa); j+=strlen(&buf_rx2[_Size_BufWifi+j]);  //temp z DHT

       		    	     x= WyszukajNrCzujkiDlaPortu_2(i+1);
       		    	     if(x!=-1)
       		    	     {
       		    	    	sprintf(&buf_rx2[_Size_BufWifi+j],"\r\nWilg port %d - %s",i+1,Const.s_Czujki[x].nazwa); j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //Wilg z DHT
       		    	     }
       		    	  }
       			      else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))  //DS || Pt
       			      {
       			    	sprintf(&buf_rx2[_Size_BufWifi+j],"\r\nTemp port %d - %s",i+1,Const.s_Czujki[x].nazwa); j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //temp z DS
       			      }
       			   }
          	}
          	sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n\r\n"); j+=strlen(&buf_rx2[_Size_BufWifi+j]);
            sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n  Data      Czas      "); j+=strlen(&buf_rx2[_Size_BufWifi+j]);
            sprintf(&buf_p[0],"      ");
            sprintf(&buf_p[10],"     ");

            for(i=0;i<_Size__s_gpio;i++)
            {
                   x= WyszukajNrCzujkiDlaPortu(i+1);
                   if(x!=-1)
                   {
                   		 if(Const.s_GPIO[i].val==9)
                   		 {
                   		     if(i+1>9) k=10; else k=0;
                   			 sprintf(&buf_rx2[_Size_BufWifi+j],"T%d%s",i+1,&buf_p[k]); j+=strlen(&buf_rx2[_Size_BufWifi+j]);  //temp z DHT

                   		     x= WyszukajNrCzujkiDlaPortu_2(i+1);
                   		     if(x!=-1)
                   		     {
                   		    	 if(i+1>9) k=10; else k=0;
                   		    	 sprintf(&buf_rx2[_Size_BufWifi+j],"W%d%s",i+1,&buf_p[k]); j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //Wilg z DHT
                   		     }
                   		 }
                   		 else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))  //DS || Pt
                   		 {
                   			 if(i+1>9) k=10; else k=0;
                   			 sprintf(&buf_rx2[_Size_BufWifi+j],"T%d%s",i+1,&buf_p[k]); j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //temp z DS
                   		 }
                  }
            }

            sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n                    "); j+=strlen(&buf_rx2[_Size_BufWifi+j]);

            for(i=0;i<_Size__s_gpio;i++)
            {
                   x= WyszukajNrCzujkiDlaPortu(i+1);
                   if(x!=-1)
                   {
                   		 if(Const.s_GPIO[i].val==9)
                   		 {
                   			 sprintf(&buf_rx2[_Size_BufWifi+j],"  [°C]  "); j+=strlen(&buf_rx2[_Size_BufWifi+j]);  //temp z DHT

                   		     x= WyszukajNrCzujkiDlaPortu_2(i+1);
                   		     if(x!=-1)
                   		     {
                   		    	 sprintf(&buf_rx2[_Size_BufWifi+j],"  [%%%%]   "); j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //Wilg z DHT
                   		     }
                   		 }
                   		 else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))  //DS || Pt
                   		 {
                   			 sprintf(&buf_rx2[_Size_BufWifi+j],"  [°C]  "); j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //temp z DS
                   		 }
                  }
            }
            sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n"); j+=strlen(&buf_rx2[_Size_BufWifi+j]);


        }
        else
        {
        	if(adr_flash_read==adr_flash_end){ WhichPage=WhichPage_p;  WhichFragmentPage=20; goto koniecWP81;  }
        	j=0;
        }

        k=IloscCzujnikow();  k+=4;   //4 bo czas zapisywany jest w 4-rech bajtach
        poesdfdfggqqq:
        sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n");  j+=strlen(&buf_rx2[_Size_BufWifi+j]);
		OdczytPomiarowFlashZewn_p(k,&j);

        if(adr_flash_read==adr_flash_end){ goto poesdfdfggqqq_koniec;  }
		if(j < ( _Size_WskFragmentPage-(22+8*(k/2)) )) goto poesdfdfggqqq;

		poesdfdfggqqq_koniec:
		buf_rx2[_Size_BufWifi+j]=0;

		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		sprintf(buf_p,"\r\nAAA:  %d  %d",adr_flash_read, adr_flash_end); dbg3(buf_p);
		if(WhichFragmentPage>1) WhichFragmentPage--;

	}
	koniecWP81:
	HAL_IWDG_Refresh(&hiwdg);
    asm("nop");
}


if(WhichPage==83)  //plik.csv
{
	if((WhichFragmentPage>0)&&(WhichFragmentPage<81))
	{
        if(WhichFragmentPage==1)
        {
           adr_flash_read=adr_flash_start;  IteracjaFlashZewnOdczyt(&adr_flash);
           if((adr_flash==0)||(adr_flash_start==adr_flash_end)){ WhichPage=WhichPage_p;  WhichFragmentPage=20; goto koniecWP83; }
           j=0;
           sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n"); j+=strlen(&buf_rx2[_Size_BufWifi+j]);
        }
        else
        {
        	if(adr_flash_read==adr_flash_end){ WhichPage=WhichPage_p;   WhichFragmentPage=20; goto koniecWP83;  }
        	j=0;
        }

        k=IloscCzujnikow();  k+=4;   //4 bo czas zapisywany jest w 4-rech bajtach
        poesdfdfggqqq3:
        OdczytPomiarowFlashZewn_p_CSV(k,&j);
        sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n");  j+=strlen(&buf_rx2[_Size_BufWifi+j]);
        if(adr_flash_read==adr_flash_end){ goto poesdfdfggqqq3_koniec;  }
        if(j < ( _Size_WskFragmentPage-(22+8*(k/2)) )) goto poesdfdfggqqq3;

		poesdfdfggqqq3_koniec:
		buf_rx2[_Size_BufWifi+j]=0;

		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		sprintf(buf_p,"\r\nAAA:  %d  %d",adr_flash_read, adr_flash_end); dbg3(buf_p);
		if(WhichFragmentPage>1) WhichFragmentPage--;

	}
	koniecWP83:
	HAL_IWDG_Refresh(&hiwdg);
    asm("nop");
}

if(WhichPage==10)  //TempWilg.txt
{
	if((WhichFragmentPage>0)&&(WhichFragmentPage<81))
	{
		if(WhichFragmentPage==1) adr_flash_2_read=0;
		else
		{
			if(adr_flash_2_read==adr_flash_2){ WhichPage=WhichPage_p;   WhichFragmentPage=20;  goto koniecWP10;  }
		}

		if(OdczytZdarzen(&adr_flash_2_read)==0){  WhichPage=WhichPage_p;   WhichFragmentPage=20;  goto koniecWP10;   }

		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		if(WhichFragmentPage>1) WhichFragmentPage--;

	}
	koniecWP10:
	HAL_IWDG_Refresh(&hiwdg);
    asm("nop");
}


if(WhichPage==992)  //sesjaSMTP.txt
{
	if((WhichFragmentPage>0)&&(WhichFragmentPage<81))
	{
		if(WhichFragmentPage==1){ adr_flash_3=0;  dbg3("\r\nStart sesja SMTP ");  }
		else
		{
			if(adr_flash_3>=adr_flash_SesjaSMTP){ dbg3("\r\nSTOP sesja SMTP "); WhichPage=WhichPage_p;   WhichFragmentPage=20;  goto koniec992;  }
		}

		if((Const.s_Lan[0].param&0x10)==0)     //ENC email
		{
			 AT25SF_3_ReadArray(_Sesja_SMTP_Start+2+adr_flash_3, buf_p, 2 );   adr_flash_3+=2;
		     k =  0xff00&(((uint16_t)buf_p[0])<<8) | 0x00ff&(((uint16_t)buf_p[1])<<0);
		     AT25SF_3_ReadArray(_Sesja_SMTP_Start+2+adr_flash_3, &buf_rx2[2000+_Size_BufWifi], k );    adr_flash_3+=k;
		       debugPrintHexTable_(k, &buf_rx2[2000+_Size_BufWifi]);

		}
		else       //ESP email
		{
			j=0;
			aedfrvfrfgvtd:
			   AT25SF_3_ReadArray(_Sesja_SMTP_Start+2+adr_flash_3, buf_p, 2 );   adr_flash_3+=2;
		       k =  0xff00&(((uint16_t)buf_p[0])<<8) | 0x00ff&(((uint16_t)buf_p[1])<<0);
		       AT25SF_3_ReadArray(_Sesja_SMTP_Start+2+adr_flash_3, &buf_rx2[2000+_Size_BufWifi], k );    adr_flash_3+=k;
	           for(i=0;i<k;i++) buf_rx2[_Size_BufWifi+j+i]=buf_rx2[2000+_Size_BufWifi+i];    buf_rx2[_Size_BufWifi+j+i]=0;  j+=k;
		     if(j<1000)
		     {
		    	if(adr_flash_3>=adr_flash_SesjaSMTP);
		    	else goto aedfrvfrfgvtd;
		     }
		}

		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		if(WhichFragmentPage>1) WhichFragmentPage--;

	}
	koniec992:
    asm("nop");
}

if(WhichPage==998)  //sesjaLogGSM.txt
{
	if((WhichFragmentPage>0)&&(WhichFragmentPage<81))
	{
		if(WhichFragmentPage==1) adr_flash_SesjaLogGSM_read=0;
		else
		{
			if(adr_flash_SesjaLogGSM_read==adr_flash_SesjaLogGSM){ WhichPage=WhichPage_p;   WhichFragmentPage=20;  goto koniecWP10a;  }
		}

		if(OdczytSesjiLogGSM(&adr_flash_SesjaLogGSM_read)==0){  WhichPage=WhichPage_p;   WhichFragmentPage=20;  goto koniecWP10a;   }

		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		if(WhichFragmentPage>1) WhichFragmentPage--;

	}
	koniecWP10a:
    asm("nop");
}

if((WhichPage>992)&&(WhichPage<997))  //etr godz....mies
{
	if((WhichFragmentPage>0)&&(WhichFragmentPage<81))
	{
		k=WhichPage-992;
		if(WhichFragmentPage==1){ OdczytFlashLicznikowStart(k);  dbg3("\r\nStart Etr "); }


		j=OdczytFlashLicznikow(k);
		if(j==0)
		{
			 if(WhichFragmentPage>1) WhichFragmentPage--;
	    }
		else if(j==1)
		{
			WhichPage=WhichPage_p;   WhichFragmentPage=20;
		}
		else if(j==2)
		{
			WhichPage=WhichPage_p;   WhichFragmentPage=20;  goto koniec993;
		}


		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];


	}
	koniec993:
    asm("nop");
}

if(WhichPage==990)
{
	if(WhichFragmentPage==1)
	{
		HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_1;
		itx_mapa=0;
		size_mapa=0;
		x=_Obraz_Start;
		AT25SF_3_ReadArray(_Obraz_Start, buf_p, 3 );
		size_mapa= 0x00ff0000&(((uint32_t)buf_p[0])<<16) | 0x0000ff00&(((uint32_t)buf_p[1])<<8) | 0x000000ff&((uint32_t)buf_p[2]);
		sprintf(&buf_p[100],"\r\n:Size img:  %d    ",size_mapa);  dbg3(&buf_p[100]);
		if(size_mapa>_Obraz_Size){  dbg3("\r\nZA DUZY ROZMIAR OBRAZU "); return; }
		adr_flash_Obraz_read=x+3;

		AT25SF_3_ReadArray(x+3, &buf_rx2[_Size_BufWifi], 36 );
		base64_decode(&buf_rx2[_Size_BufWifi], buf_p, 36,&i);
		if((buf_p[0]==0x89)&&(buf_p[1]=='P')&&(buf_p[2]=='N')&&(buf_p[3]=='G')&&(buf_p[4]=='\r')&&(buf_p[5]=='\n')&&(buf_p[6]==0x1A)&&(buf_p[7]=='\n'));
		else {  dbg3("\r\nNIE WLASCIWY FORMAT OBRAZU "); return; }

		Const.s_Map[0].resX= 256*buf_p[18]+buf_p[19];
		Const.s_Map[0].resY= 256*buf_p[22]+buf_p[23];
		sprintf(&buf_p[100],"\r\nResol img:  %d  %d   ",Const.s_Map[0].resX, Const.s_Map[0].resY);  dbg3(&buf_p[100]);

	}
	else if((WhichFragmentPage>1)&&(WhichFragmentPage<(2+(size_mapa/3990))))
	{
		//for(i=0;i<3990;i++) buf_rx2[_Size_BufWifi+i]= HttpImg[itx_mapa+i];    buf_rx2[_Size_BufWifi+i]=0;
		AT25SF_3_ReadArray(adr_flash_Obraz_read, &buf_rx2[_Size_BufWifi], 3990 );  buf_rx2[_Size_BufWifi+3990]=0;
		adr_flash_Obraz_read+=3990;
		itx_mapa+=3990;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
		HAL_IWDG_Refresh(&hiwdg);
	}
	else if(WhichFragmentPage==(2+(size_mapa/3990)))
	{
		//for(i=0;i<(size_mapa-itx_mapa);i++) buf_rx2[_Size_BufWifi+i]= HttpImg[itx_mapa+i];    buf_rx2[_Size_BufWifi+i]=0;
		AT25SF_3_ReadArray(adr_flash_Obraz_read, &buf_rx2[_Size_BufWifi], size_mapa-itx_mapa );  buf_rx2[_Size_BufWifi+(size_mapa-itx_mapa)]=0;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(2+(size_mapa/3990)+1))
	{
		sprintf(&buf_rx2[_Size_BufWifi],"%s",HttpMapaLora_2);   j=strlen(&buf_rx2[_Size_BufWifi]);
		for(i=0;i<_IleLor;i++)
		{
		    if((Const.s_Map[0].x[i]>0)||(Const.s_Map[0].y[i]>0)){
			   sprintf(&buf_rx2[_Size_BufWifi+j],"x[%d]=%d;y[%d]=%d;itx[%d]=1;\r\n",i+1,Const.s_Map[0].x[i],i+1,Const.s_Map[0].y[i],i+1);
			   j=strlen(&buf_rx2[_Size_BufWifi]);
		    }
		}
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(2+(size_mapa/3990)+2))
	{
		 HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_3;
	}
	else if(WhichFragmentPage==(2+(size_mapa/3990)+3))
	{
		 HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_4;
		 HttpPagesBuffer[WhichFragmentPage+1]=0;
	}
}


if(WhichPage==902)  //Rotacja
{
	if(WhichFragmentPage==2)
	{
		HttpRot_start();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==3)
	{
		if(HttpRot_it()==1) WhichFragmentPage--;
		else                WhichFragmentPage=5;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==6)
	{
		HttpRot_1();  HttpCount--;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==7)
	{
		HttpRot_2();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
}










    		   	  if(HttpPagesBuffer[WhichFragmentPage]==HttpPagesBuffer[WhichFragmentPage-1]) HttpCount++;   else HttpCount=1;

    		   	  HttpOffsetService();
    		   	  FragmentPagePointer = HttpParser(HttpPagesBuffer[WhichFragmentPage],HttpCount+HttpCountOffset[HttpCount-1]);

    		   	  FragmentPagePacketNumber = strlen(FragmentPagePointer)/PacketLen;
    		   	  if(strlen(FragmentPagePointer)%PacketLen==0);  else FragmentPagePacketNumber++;
    		   	  //sprintf(buf_p,"\r\nDLGx%d: %d %d ",WhichFragmentPage,strlen(FragmentPagePointer),FragmentPagePacketNumber);  dbg3(buf_p);
    		   	  FragmentPagePacketCounter=0;

    		   	 if(strlen(FragmentPagePointer)<PacketLen) sprintf(buf_p,"AT+CIPSEND=%c,%d\x0D\x0A",HttpPort,strlen(FragmentPagePointer)-PacketLen*(FragmentPagePacketNumber-1));
    		   	 else                                      sprintf(buf_p,"AT+CIPSEND=%c,%d\x0D\x0A",HttpPort,PacketLen);
    		   	 dbg3("   "); dbg3(buf_p);
    		   	 SendDataToModemWifi(buf_p);  li_WifiCipsend[0]=1;

    		}
    	    else
    	    {
    	    	sprintf(buf_p,"AT+CIPCLOSE=%c\x0D\x0A",HttpPort); dbg3("   "); dbg3(buf_p);
    	    	SendDataToModemWifi(buf_p);  _return=1;
    	    	licz_htt=50;  li_WifiCipsend[1]=1;  //dbg3("\r\nXXXXX11 ");
       	    	if(WhichPage==2)
          	    {
       	    	      HAL_Delay(1000);
       	    		  dbg3("\r\n[WWDG]");    //Jump to BOOT

          	      	  hwwdg.Instance = WWDG;
          	      	  hwwdg.Init.Prescaler = WWDG_PRESCALER_1;
          	      	  hwwdg.Init.Window = 64;
          	      	  hwwdg.Init.Counter = 64;
          	      	  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
          	      	  HAL_WWDG_Init(&hwwdg);

          	      	   while(1);
          	    }

             #ifdef _BootLoader
    	    	if(WhichPage==1)
    	    	{
    	    	     HAL_Delay(200);
    	    	     JumpToUserProgram(ADDR_FLASH_PAGE_25);
    	    	}
             #endif



    	    }


    	}
    	else if(FragmentPagePacketCounter==1000)
    	{
    	   sprintf(buf_p,"AT+CIPCLOSE=%c\x0D\x0A",HttpPort); dbg3("   "); dbg3(buf_p);
    	   SendDataToModemWifi(buf_p);  _return=1;
    	   licz_htt=50;  li_WifiCipsend[1]=1;  //dbg3("\r\nXXXXX22 ");
    	}
    	else
    	{
    	   sprintf(buf_p,"AT+CIPCLOSE=%c\x0D\x0A",HttpPort); dbg3("   "); dbg3(buf_p);
    	   SendDataToModemWifi(buf_p);  _return=1;
    	   licz_htt=50;  li_WifiCipsend[1]=1;   //dbg3("\r\nXXXXX33 ");
    	}

	    return _return;
}

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void encodeblock( unsigned char in[3], unsigned char out[4], int len )
{
   out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
   // cb64[0]=0;
}

void base64_encode( char *in_buf, char *out_buf, int in_len)
{
    unsigned char in[3], out[4];
    int i, bb;

    while(in_len)
    {

        bb = 0;
        for( i = 0; i < 3; i++ )
		{
	       if(in_len>0)
	       {
        	   in[i] = (unsigned char) *in_buf++;
		       bb++;
		       in_len--;
	       }
           else
		   {
                in[i] = 0;
           }
        }

        if( bb )
		{
            encodeblock( in, out, bb );
            for( i = 0; i < 4; i++ ) *out_buf++ = out[i];

	    }
    }
    *out_buf++ = 0;
}






static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void decodeblock( unsigned char in[4], unsigned char out[3] )
{
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

/*
** base64_decode
**
** decode a base64 string  discarding padding, line breaks and noise
*/
void base64_decode( char *in_buf, char *out_buf, int in_len, int *out_len)
{
    unsigned char in[4], out[3], v;
    int i, len, len_p, in_len_p, aa;    in_len_p= in_len;

    if ((in_len % 4) != 0){ dbg3("\r\nBLAD !!!");  return; }

    aa=0;
    if(in_buf[in_len-1] == '=') aa++;
    if(in_buf[in_len-2] == '=') aa++;


    while( in_len ) {

        for( len = 0, i = 0; i < 4 && in_len; i++ ) {

            v = 0;

            while( in_len && v == 0 ) {
	        v = *in_buf++;
		in_len --;
                v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
                if( v ) {
                    v = (unsigned char) ((v == '$') ? 0 : v - 61);
                }
            }
            if( in_len>=0 ) {
                len++;
                if( v ) {
                    in[ i ] = (unsigned char) (v - 1);
                }
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {

            decodeblock( in, out );

            for( i = 0; i < len - 1; i++ ) {
                *out_buf++=out[i];
            }
        }
    }
    *out_buf++=0;      *out_len= (in_len_p/4)*3;     *out_len= *out_len- aa;

}

/*

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};




void base64_decode(const char *data,
                             size_t input_length,
							 const char *out,
                             size_t *output_length) {

    if (decoding_table == NULL)
    {

        decoding_table = malloc(256);

        for (int i = 0; i < 64; i++)
            decoding_table[(unsigned char) encoding_table[i]] = i;

    }

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data;// = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    decoded_data[j]=0;   return decoded_data;
}

*/

int zmHttpRot;
void HttpRot_start()
{
	int i,j,itx,kx,k;  itx=0;   zmHttpRot=0;


sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"corner-radius2\" style=\"margin-left: 0px; width: 440px; height: 1960px;\" >\r\n\
<div class=\"corner-radius\"  style=\"margin-left: 5px; width: 410px; height: 1930px;\" >\r\n\
<table><tr><td><font class=\"css-font-style\">&nbsp;Ustalanie Cyklu Rotacji</font></td></tr></table>\r\n\
<table><tr><td><font size=\"4\" color=\"#9edf0f\" style=\"padding: 0px;\"><b> &nbsp;Cykl rot.&nbsp;</b></font></td><td width=\"27px\"></td>");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

for(i=0;i<8;i++){
  sprintf(&buf_rx2[_Size_BufWifi+itx],"<td width=\"30px\"><font size=\"4\" color=\"#9edf0f\"><b>%d</b></font></td>\r\n",i+1);
  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}
sprintf(&buf_rx2[_Size_BufWifi+itx],"</tr></table>\r\n<table><tr>\r\n");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

sprintf(&buf_rx2[_Size_BufWifi+itx],"<td width=\"40px\"><font size=\"4\" color=\"#ccc\">Port</font></td>\r\n<td width=\"40px\"><font size=\"4\" color=\"#ccc\">Typ</font></td>\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

sprintf(&buf_rx2[_Size_BufWifi+itx],"</tr></table>\r\n<table>\r\n<td><input type=\"checkbox\" name=\"n_s_xxROTxx\" class=\"regular-checkbox checkbox\" checked /></td>\r\n");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpRot_start itx !!!");
}


int HttpRot_it()
{
	int i,j,itx,kx,k;  itx=0;

  k=0;
  for(i=zmHttpRot;i<_Size_wy;i++)
  {
	  if((Const.s_GPIO[t_wy[i]-1].val==8)||(Const.s_GPIO[t_wy[i]-1].val==0))    // IR || PK
	  {

	      if(Const.s_GPIO[t_wy[i]-1].val==8)  //IR
	      {
              sprintf(buf_p,"IR");
	      }
	      else if(Const.s_GPIO[t_wy[i]-1].val==0)  //PK
	      {
	    	  sprintf(buf_p,"PK");
	      }

	      sprintf(&buf_rx2[_Size_BufWifi+itx],"<tr><td width=\"40px\"><font size=\"4\" color=\"#ccc\">%02d </font></td><td width=\"40px\"><font size=\"4\" color=\"#ccc\">%s</font></td></td><td width=\"27px\"></td>\r\n",t_wy[i],buf_p);
	      itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	      for(j=0;j<8;j++)
	      {
	         sprintf(&buf_rx2[_Size_BufWifi+itx],"<td width=\"30px\"><input type=\"checkbox\" id=\"wr_checkboxS%03d_%03d\" name=\"n_s_Rot_akt%03d_%03d\" class=\"regular-checkbox checkbox\"         /><label for=\"wr_checkboxS%03d_%03d\"></label></td>\r\n",j+1,i+1,j+1,i+1,j+1,i+1);
	         itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	      }
	      k++;
	  }
      if(k==2){  zmHttpRot=i+1;    return 1;   }
  }

  sprintf(&buf_rx2[_Size_BufWifi+itx],"</tr></table><font size=\"2\" color=\"#eeeeee\"><b>Wybrane porty ustawic jako 'Przekaznik' lub 'Sterowanie IR'</b></font><br><br>\r\n<table><tr>\r\n<td><font class=\"css-font-style2\">Interwal&nbsp;czasowy&nbsp;miedzy&nbsp;cyklami</font></td></tr></table>");
  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  return 0;

}

int HttpRot_1()
{
	int i,j,itx,kx,k;  itx=0;


sprintf(&buf_rx2[_Size_BufWifi+itx],"<table><tr>\r\n\ 
 <td><td width=\"100px\"></td><td><div id=\"mainselection\"><select name=\"n_s_Rot_dob_001\" ><option value=\"00\"          > -- </option><option value=\"01\"          >1 doba</option><option value=\"02\"          >2 doby</option><option value=\"03\"          >3 doby</option><option value=\"04\"          >4 doby</option><option value=\"05\"          >5 dob</option><option value=\"06\"          >6 dob</option><option value=\"07\"          >7 dob</option></select></div></td>\r\n\
 <td><td width=\"40px\"></td><td><input type=\"text\" class=\"textbox\" style=\"width:50px;\" name=\"n_s_Rot_int_001\" size=\"5\" maxlength=\"5\" value=\"00:01\"  onkeypress=\"return maskujKlawisze(event, ':0123456789');\"><font class=\"css-font-style2\">g:m</font></td>\r\n\  
</tr></table> \r\n\
<table><tr>\r\n\
 <td><font class=\"css-font-style2\">odmierzany od godziny</font></td>\r\n\ 
<td width=\"20px\"></td><td><input type=\"text\" class=\"textbox\" style=\"width:50px;\" name=\"n_s_Rot_sta_001\" size=\"5\" maxlength=\"5\" value=\"00:01\"  onkeypress=\"return maskujKlawisze(event, ':0123456789');\"><font class=\"css-font-style2\">g:m</font></td>\r\n\ 
</tr></table>  <br>\r\n\
 <table><tr>\r\n\
 <td><font class=\"css-font-style2\">Alarmowy prog temperatury</font></td>\r\n\ 
<td width=\"5px\"></td><td><input type=\"text\" class=\"textbox\" style=\"width:50px;\" name=\"n_s_Rot_max_001\" size=\"5\" maxlength=\"5\" value=\"+50.0\"  onkeypress=\"return maskujKlawisze(event, '+-.0123456789');\"><font class=\"css-font-style2\">°C</font></td>\r\n\ 
</tr></table> \r\n\
 <table><tr>\r\n\
 <td><font class=\"css-font-style2\">prog zatrzymujacy rotacje</font></td>\r\n\ 
 <td width=\"20px\"></td><td><input type=\"text\" class=\"textbox\" style=\"width:50px;\" name=\"n_s_Rot_min_001\" size=\"5\" maxlength=\"5\" value=\"+50.0\"  onkeypress=\"return maskujKlawisze(event, '+-.0123456789');\"><font class=\"css-font-style2\">°C</font></td>\r\n\ 
 </tr></table><table><tr>\r\n\
 <td><font class=\"css-font-style2\">o czasie zwloki</font></td>\r\n\ 
<td width=\"20px\"></td><td><input type=\"text\" class=\"textbox\" style=\"width:50px;\" name=\"n_s_Rot_rea_001\" size=\"5\" maxlength=\"5\" value=\"00:01\"  onkeypress=\"return maskujKlawisze(event, ':0123456789');\"><font class=\"css-font-style2\">m:s</font></td>\r\n\ 
</tr></table>\r\n\
<table><tr>\r\n\
<td><font class=\"css-font-style2\">i histerezie</font></td>\r\n\ 
<td width=\"20px\"></td><td><input type=\"text\" class=\"textbox\" style=\"width:50px;\" name=\"n_s_Rot_hist_001\" size=\"5\" maxlength=\"5\" value=\"+00.5\"  onkeypress=\"return maskujKlawisze(event, '+-.0123456789');\"><font class=\"css-font-style2\">°C</font></td>\r\n\ 
</tr></table>\r\n\
 <table><tr><td><font class=\"css-font-style2\">dla czujnika temperatury z portu nr </font><font size=\"5\" color=\"#eeeeee\">1</font><br><font size=\"2\" color=\"#eeeeee\"><b>( port 1&nbsp; nalezy ustawic jako &nbsp;' Temperatura ' )</b></font></td></tr></table><br> \r\n\
 <table><tr><td><font class=\"css-font-style2\">Alarm przekaznikowy z portu nr </font><font size=\"5\" color=\"#eeeeee\">33</font><br><font size=\"2\" color=\"#eeeeee\"><b>( port 33&nbsp; nalezy ustawic jako &nbsp;' Termostat ' )</b></font></td></tr></table> \r\n ");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<br><br><button type=\"submit\" name=\"n_109_zapisz\" class=\"css_button\">Zapisz</button>\r\n</form>\r\n</div></div>\r\n");
	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpRot_1 itx !!!");

}

int HttpRot_2()
{
	int i,j,itx,kx,k;  itx=0;

	  sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"corner-radius2\" style=\"margin-left: 0px width: 430px; height: 940px;\">\r\n\
 <div class=\"corner-radius\" style=\"margin-left: 35px width: 400px; height: 910px;\">\r\n\
 <table><tr><td><font class=\"css-font-style\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nazwy Portów  </font></td></tr></table>\r\n\
 <table><tr><td><font size=\"4\" color=\"#ccc\" style=\"padding: 20px;\">PK - &nbsp;&nbsp;wyjscie przekaznikowe</font></td></tr></table>\r\n\
 <table><tr><td><font size=\"4\" color=\"#ccc\" style=\"padding: 20px;\">IR - &nbsp;&nbsp;wyjscie IRDA</font></td></tr></table>\r\n\
 <table>\r\n");
	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	  for(i=0;i<_Size_wy;i++)
	  {  if((Const.s_GPIO[t_wy[i]-1].val==8)||(Const.s_GPIO[t_wy[i]-1].val==0))    // IR || PK
	     {
		      sprintf(&buf_rx2[_Size_BufWifi+itx],"<tr><td><font size=\"4\" color=\"#ccc\">&nbsp;- &nbsp; </font><font size=\"4\" color=\"#eee\"> %s</font></td></tr>\r\n",Const.s_GPIO[t_wy[i]-1].nazwa);
		 	  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	     }
	  }

	  sprintf(&buf_rx2[_Size_BufWifi+itx],"</table>\r\n\
 </div></div>\r\n");
      itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpRot_2 itx !!!");

}



