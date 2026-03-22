/*
 * lcd.c
 *
 *  Created on: 6 kwi 2017
 *      Author: Elektronika RM
 */


#include "stm32f1xx_hal.h"
#include "lcd.h"
#include "define.h"
#include "st7735/fonts.h"
#include "st7735/st7735.h"



#define LCD_DC			GPIO_PIN_8   //GPIOB
#define LCD_CE			GPIO_PIN_1    //GPIOE
#define LCD_RST			GPIO_PIN_9    //GPIOB

#define TM_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))


char tabwsklcdwy[_Size_wy],tabwsklcdwe[_Size_we];

int TempKolorEkr(int nr)
{
	int a; char b;
	b=KolorTemp(nr);
	if(b==0x31)      a=ST7735_RED;
	else if(b==0x32) a=ST7735_COLOR565(0x40, 0x40, 0xFF);
	else             a=ST7735_WHITE;
  return a;
}

int IloscWE()
{
	int i,j,k;  k=0;

	for(i=0;i<_Size_we;i++)
	{
		if(Const.s_GPIO[t_we[i]-1].val==4) k++;
	}
	return k;
}
int IloscWY()
{
	int i,j,k;  k=0;

	for(i=0;i<_Size_wy;i++)
	{
		if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))  k++;
	}
	return k;
}

void EkranLcdZegarStart()
{
    int i,j,k,f;
	Xtft=0; Ytft=5;

   ST7735_FillScreen(ST7735_BLACK);
   f=0;

	 for(i=0;i<_Size_we;i++)
	 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
	    {
		   if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){
		      DrawWindowWE(Xtft,Ytft,155,50,Const.s_WE[i].nazwa_ro,ST7735_WHITE,Const.s_WE[i].nazwa); tabwsklcdwe[i]=1; }
		   else{
			  DrawWindowWE(Xtft,Ytft,155,50,Const.s_WE[i].nazwa_zw,ST7735_RED,Const.s_WE[i].nazwa); tabwsklcdwe[i]=0; }

		   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
		   f++;
	    }
	 }
	 for(i=0;i<_Size_wy;i++)  //WY
 	 {     if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY i IR
 		   {
 			   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){
 			      DrawWindowPK(Xtft,Ytft,155,50,Const.s_PK[i].nazwa_on,ST7735_WHITE,Const.s_PK[i].nazwa); tabwsklcdwy[i]=1; }
 			   else{
 			      DrawWindowPK(Xtft,Ytft,155,50,Const.s_PK[i].nazwa_of,ST7735_BLACK,Const.s_PK[i].nazwa); tabwsklcdwy[i]=0; }

 			   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
 			   f++;
 		   }
 	 }

}
void EkranLcdZegar()
{
    int i,j,k,f;
	Xtft=0; Ytft=5;

   f=0;

	 for(i=0;i<_Size_we;i++)
	 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
	 	{
		   if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){
		      if(tabwsklcdwe[i]==0){ DrawWindowWE(Xtft,Ytft,155,50,Const.s_WE[i].nazwa_ro,ST7735_WHITE,Const.s_WE[i].nazwa); tabwsklcdwe[i]=1;}
		   }
		   else{
			  if(tabwsklcdwe[i]==1){ DrawWindowWE(Xtft,Ytft,155,50,Const.s_WE[i].nazwa_zw,ST7735_RED,Const.s_WE[i].nazwa); tabwsklcdwe[i]=0;}
		   }

		   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
		   f++;
	   }
	}

	for(i=0;i<_Size_wy;i++)  //WY
	{  if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY i IR
	   {
		   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){
		      if(tabwsklcdwy[i]==0){ DrawWindowPK(Xtft,Ytft,155,50,Const.s_PK[i].nazwa_on,ST7735_WHITE,Const.s_PK[i].nazwa); tabwsklcdwy[i]=1;}
		   }
		   else{
			  if(tabwsklcdwy[i]==1){ DrawWindowPK(Xtft,Ytft,155,50,Const.s_PK[i].nazwa_of,ST7735_BLACK,Const.s_PK[i].nazwa); tabwsklcdwy[i]=0;}
		   }

		   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
		   f++;
	   }
	}

}
void EkranLcdZegarStartSmall(int ile)
{
    int i,j,k,f;
	Xtft=0; Ytft=0;

   ST7735_FillScreen(ST7735_BLACK);
   f=0;


	 for(i=0;i<_Size_we;i++)
	 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
	    {
		   if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){
		      DrawWindowWESmall(Xtft,Ytft,155,30,Const.s_WE[i].nazwa_ro,ST7735_WHITE,Const.s_WE[i].nazwa); tabwsklcdwe[i]=1; }
		   else{
			  DrawWindowWESmall(Xtft,Ytft,155,30,Const.s_WE[i].nazwa_zw,ST7735_RED,Const.s_WE[i].nazwa); tabwsklcdwe[i]=0; }

		   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=35; }
		   f++;
           if(f>ile-1) return;

	    }
	 }
	 for(i=0;i<_Size_wy;i++)  //WY
 	 {     if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY i IR
 		   {
 			   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){
 			      DrawWindowPKSmall(Xtft,Ytft,155,30,Const.s_PK[i].nazwa_on,ST7735_WHITE,Const.s_PK[i].nazwa); tabwsklcdwy[i]=1; }
 			   else{
 				  DrawWindowPKSmall(Xtft,Ytft,155,30,Const.s_PK[i].nazwa_of,ST7735_BLACK,Const.s_PK[i].nazwa); tabwsklcdwy[i]=0; }

 			   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=35; }
 			   f++;
 	           if(f>ile-1) return;
 		   }
 	 }

}
void EkranLcdZegarSmall(int ile)
{
    int i,j,k,f;
	Xtft=0; Ytft=0;

   f=0;
	 for(i=0;i<_Size_we;i++)
	 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
	 	{
		   if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){
		      if(tabwsklcdwe[i]==0){ DrawWindowWESmall(Xtft,Ytft,155,30,Const.s_WE[i].nazwa_ro,ST7735_WHITE,Const.s_WE[i].nazwa); tabwsklcdwe[i]=1;}
		   }
		   else{
			  if(tabwsklcdwe[i]==1){ DrawWindowWESmall(Xtft,Ytft,155,30,Const.s_WE[i].nazwa_zw,ST7735_RED,Const.s_WE[i].nazwa); tabwsklcdwe[i]=0;}
		   }

		   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=35; }
		   f++;
           if(f>ile-1) return;
	   }
	}

	for(i=0;i<_Size_wy;i++)  //WY
	{  if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY i IR
	   {
		   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){
		      if(tabwsklcdwy[i]==0){ DrawWindowPKSmall(Xtft,Ytft,155,30,Const.s_PK[i].nazwa_on,ST7735_WHITE,Const.s_PK[i].nazwa); tabwsklcdwy[i]=1;}
		   }
		   else{
			  if(tabwsklcdwy[i]==1){ DrawWindowPKSmall(Xtft,Ytft,155,30,Const.s_PK[i].nazwa_of,ST7735_BLACK,Const.s_PK[i].nazwa); tabwsklcdwy[i]=0;}
		   }

		   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=35; }
		   f++;
           if(f>ile-1) return;
	   }
	}

}

void EkranLcdTempBigStart()
{
    int i,j,k;
	Xtft=0; Ytft=15;

   ST7735_FillScreen(ST7735_BLACK);
   k=0;
	  for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
	  {
		j= WyszukajNrCzujkiDlaPortu(i+1);
		if(j!=-1)
		{
		    if(Const.s_GPIO[i].val==9)
			{
		    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
		    	XYpom[k]=DrawWindowTempBig(Xtft,Ytft,320,90,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa); k++;
		    	Ytft+=120;

		    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	if(j!=-1)
		    	{
		    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  buf_p[60]=' ';buf_p[63]=' ';buf_p[64]='%';buf_p[65]=0;
			    	XYpom[k]=DrawWindowTempBig(Xtft,Ytft,320,90,&buf_p[60],TempKolorEkr(j),Const.s_Czujki[j].nazwa); k++;
			    	Ytft+=120;
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
		    	XYpom[k]=DrawWindowTempBig(Xtft,Ytft,320,90,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa); k++;
		    	Ytft+=120;
			}
		    else if(Const.s_GPIO[i].val==3)
			{
		    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);
		    	XYpom[k]=DrawWindowTempBigPt(Xtft,Ytft,320,90,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa); k++;
		        Ytft+=120;

			}
		}
	  }
}
void EkranLcdTempStart()
{
    int i,j,k;
	Xtft=0; Ytft=5;

   ST7735_FillScreen(ST7735_BLACK);
   k=0;
	  for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
	  {
		j= WyszukajNrCzujkiDlaPortu(i+1);
		if(j!=-1)
		{
		    if(Const.s_GPIO[i].val==9)
			{
		    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
		    	XYpom[k]=DrawWindowTemp(Xtft,Ytft,155,50,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
		    	if((k%2)==0){ Xtft=0; Ytft+=60; }  else Xtft=165;

		    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	if(j!=-1)
		    	{
		    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  buf_p[60]=' ';buf_p[63]=' ';buf_p[64]='%';buf_p[65]=0;
			    	XYpom[k]=DrawWindowTemp(Xtft,Ytft,155,50,&buf_p[60],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
			    	if((k%2)==0){ Xtft=0; Ytft+=60; }  else Xtft=165;
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
		    	XYpom[k]=DrawWindowTemp(Xtft,Ytft,155,50,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
		    	if((k%2)==0){ Xtft=0; Ytft+=60; }  else Xtft=165;
			}
		    else if(Const.s_GPIO[i].val==3)
			{
		    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);
		    	XYpom[k]=DrawWindowTempPt(Xtft,Ytft,155,50,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
		    	if((k%2)==0){ Xtft=0; Ytft+=60; }  else Xtft=165;


			}
		}
	  }
}






void EkranLcdZgrTempWeWyStart()
{
    int i,j,k,f,rrr;
	Xtft=0; Ytft=5;

   ST7735_FillScreen(ST7735_BLACK);
   k=0;
   PobierzCzas(buf_p);
   sprintf(&buf_p[50],"%02d.%02d.%02d  %02d:%02d:%02d    GSM:%s",buf_p[0],buf_p[1],buf_p[2],buf_p[4],buf_p[5],buf_p[6],zasieg_gsm);
   XYpom[k]=DrawWindowTempSmallZegar(Xtft,Ytft,320,30,&buf_p[50],ST7735_MAGENTA,"Zegar czasu systemowego          Zasieg"); k++;
   Ytft+=50;

    if(IloscCzujnikow()/2>1) rrr=155;   else rrr=320;
     f=0;
	  for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
	  {
		j= WyszukajNrCzujkiDlaPortu(i+1);
		if(j!=-1)
		{
		    if(Const.s_GPIO[i].val==9)
			{
		    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
		    	XYpom[k]=DrawWindowTemp(Xtft,Ytft,155,50,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
		    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60;  }
		    	f++; if(f>1) goto omin23434dd;

		    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	if(j!=-1)
		    	{
		    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  buf_p[60]=' ';buf_p[63]=' ';buf_p[64]='%';buf_p[65]=0;
			    	XYpom[k]=DrawWindowTemp(Xtft,Ytft,155,50,&buf_p[60],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
			    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60;  }
			    	f++; if(f>1) goto omin23434dd;
		    	}
			}
		}
	  }
	  f=0;
	  for(i=0;i<_Size__s_gpio;i++)  //Temp
	  {
		j= WyszukajNrCzujkiDlaPortu(i+1);
	    if(j!=-1)
		{
		    if(Const.s_GPIO[i].val==2)
			{
		    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);     // if(rrr==320) sprintf(&buf_p[100],"%s 'C", &buf_p[50]); else sprintf(&buf_p[100],"%s", &buf_p[50]);
		    	XYpom[k]=DrawWindowTemp(Xtft,Ytft,rrr,50,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
		    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60;  }
		    	f++; if(f>1) goto omin23434dd;
			}
		    else if(Const.s_GPIO[i].val==3)
			{
		    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);   //  if(rrr==320) sprintf(&buf_p[100],"%s 'C", &buf_p[50]); else sprintf(&buf_p[100],"%s", &buf_p[50]);
		    	XYpom[k]=DrawWindowTempPt(Xtft,Ytft,rrr,50,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;
		    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60;  }
		    	f++; if(f>1) goto omin23434dd;

			}
		}
	  }
	  omin23434dd:

	   if(IloscWE()>1) rrr=155;   else rrr=320;
	     f=0;  Xtft=0; Ytft=5+50+60;
		 for(i=0;i<_Size_we;i++)
		 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
		    {
			   if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){
			      DrawWindowWE(Xtft,Ytft,rrr,50,Const.s_WE[i].nazwa_ro,ST7735_WHITE,Const.s_WE[i].nazwa); tabwsklcdwe[i]=1; }
			   else{
				  DrawWindowWE(Xtft,Ytft,rrr,50,Const.s_WE[i].nazwa_zw,ST7735_RED,Const.s_WE[i].nazwa); tabwsklcdwe[i]=0; }

			   if((f%2)==0){  Xtft=165;    }  else{  Xtft=0; Ytft+=60;  }
			   f++;   if(f>1) goto omin23434ddwe;
		    }
		 }

		 omin23434ddwe:

	   if(IloscWY()>1) rrr=155;   else rrr=320;
		 f=0;   Xtft=0; Ytft=5+50+60+60;
		 for(i=0;i<_Size_wy;i++)  //WY
	 	 {     if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY i IR
	 		   {
	 			   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){
	 			      DrawWindowPK(Xtft,Ytft,rrr,50,Const.s_PK[i].nazwa_on,ST7735_WHITE,Const.s_PK[i].nazwa); tabwsklcdwy[i]=1; }
	 			   else{
	 			      DrawWindowPK(Xtft,Ytft,rrr,50,Const.s_PK[i].nazwa_of,ST7735_BLACK,Const.s_PK[i].nazwa); tabwsklcdwy[i]=0; }

				   if((f%2)==0){  Xtft=165;    }  else{  Xtft=0; Ytft+=60;  }
				   f++;   if(f>1) goto omin23434ddwedfdf;
	 		   }
	 	 }

		 omin23434ddwedfdf:
		 asm("nop");

}

void EkranLcdZgrTempWeWy()
{
	int i,j,k,f,color,rrr;   Xtft=0;  Ytft=5;

			k=0;    if(IloscCzujnikow()/2>1) rrr=155;   else rrr=320;

			   f=0;
			   PobierzCzas(buf_p);  RRRRRWIFI();
			   sprintf(&buf_p[50],"%02d.%02d.%02d  %02d:%02d:%02d    GSM:%s",buf_p[0],buf_p[1],buf_p[2],buf_p[4],buf_p[5],buf_p[6],zasieg_gsm);
			   ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_11x18, ST7735_MAGENTA, ST7735_COLOR565(0x20,0x20,0x20));   k++;
			   Ytft+=50;

			                   f=0;
			                      for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
								  {
									j= WyszukajNrCzujkiDlaPortu(i+1);
									if(j!=-1)
									{
									    if(Const.s_GPIO[i].val==9)
										{
									    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+85, XYpom[k]+6, "`C",   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
									    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
			                                k++;  f++; if(f>1) goto omin23434dd1;
									    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
									    	if(j!=-1)
									    	{
									    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);   buf_p[60]=' ';buf_p[63]=0;
										    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[60] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
										    	buf_p[180]='%';buf_p[181]=0; ST7735_WriteString((XYpom[k]>>16)+58, XYpom[k]+6, &buf_p[180],   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
										    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
				                                k++;   f++; if(f>1) goto omin23434dd1;
									    	}
										}
									}
								  }
			                      f=0;
								  for(i=0;i<_Size__s_gpio;i++)  //Temp
								  {
									j= WyszukajNrCzujkiDlaPortu(i+1);
								    if(j!=-1)
									{
									    if(Const.s_GPIO[i].val==2)
										{
									    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);      //if(rrr==320) sprintf(&buf_p[100],"%s 'C", &buf_p[50]); else sprintf(&buf_p[100],"%s", &buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+85, XYpom[k]+6, "`C",   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
									    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
			                                k++;   f++; if(f>1) goto omin23434dd1;
										}
									    else if(Const.s_GPIO[i].val==3)
										{
									    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);     // if(rrr==320) sprintf(&buf_p[100],"%s 'C", &buf_p[50]); else sprintf(&buf_p[100],"%s", &buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+70, XYpom[k]+6, "`C",   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
									    	if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
			                                k++;   f++; if(f>1) goto omin23434dd1;
										}
									}
								  }

								  omin23434dd1:

								  if(IloscWE()>1) rrr=155;   else rrr=320;
								   f=0;   Xtft=0; Ytft=5+50+60;
									 for(i=0;i<_Size_we;i++)
									 {  if(Const.s_GPIO[t_we[i]-1].val==4)  //WE
									 	{
										   if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){
										      if(tabwsklcdwe[i]==0){ DrawWindowWE(Xtft,Ytft,rrr,50,Const.s_WE[i].nazwa_ro,ST7735_WHITE,Const.s_WE[i].nazwa); tabwsklcdwe[i]=1;}
										   }
										   else{
											  if(tabwsklcdwe[i]==1){ DrawWindowWE(Xtft,Ytft,rrr,50,Const.s_WE[i].nazwa_zw,ST7735_RED,Const.s_WE[i].nazwa); tabwsklcdwe[i]=0;}
										   }

										   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
										   f++;   if(f>1) goto omin23434ddwedfdfffff3;
									   }
									}
									omin23434ddwedfdfffff3:

									if(IloscWY()>1) rrr=155;   else rrr=320;
									f=0;  Xtft=0; Ytft=5+50+60+60;
									for(i=0;i<_Size_wy;i++)  //WY
									{  if((Const.s_GPIO[t_wy[i]-1].val==0)||(Const.s_GPIO[t_wy[i]-1].val==10)||(Const.s_GPIO[t_wy[i]-1].val==11)||(Const.s_GPIO[t_wy[i]-1].val==8))   //WY i IR
									   {
										   if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){
										      if(tabwsklcdwy[i]==0){ DrawWindowPK(Xtft,Ytft,rrr,50,Const.s_PK[i].nazwa_on,ST7735_WHITE,Const.s_PK[i].nazwa); tabwsklcdwy[i]=1;}
										   }
										   else{
											  if(tabwsklcdwy[i]==1){ DrawWindowPK(Xtft,Ytft,rrr,50,Const.s_PK[i].nazwa_of,ST7735_BLACK,Const.s_PK[i].nazwa); tabwsklcdwy[i]=0;}
										   }

										   if((f%2)==0){ Xtft=165; }  else{  Xtft=0; Ytft+=60; }
										   f++;   if(f>1) goto omin23434ddwedfdfffff;
									   }
									}

									omin23434ddwedfdfffff:
									asm("nop");

}










void EkranLcdTempSmallStart(int ile)
{
    int i,j,k;
	Xtft=0; Ytft=0;

   ST7735_FillScreen(ST7735_BLACK);
   k=0;
	  for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
	  {
		j= WyszukajNrCzujkiDlaPortu(i+1);
		if(j!=-1)
		{
		    if(Const.s_GPIO[i].val==9)
			{
		    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
		    	XYpom[k]=DrawWindowTempSmall(Xtft,Ytft,155,30,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;  if(k>ile-1) return;
		    	if((k%2)==0){ Xtft=0; Ytft+=35; }  else Xtft=165;

		    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
		    	if(j!=-1)
		    	{
		    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);  buf_p[60]=' ';buf_p[63]=' ';buf_p[64]='%';buf_p[65]=0;
			    	XYpom[k]=DrawWindowTempSmall(Xtft,Ytft,155,30,&buf_p[60],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;  if(k>ile-1) return;
			    	if((k%2)==0){ Xtft=0; Ytft+=35; }  else Xtft=165;
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
		    	XYpom[k]=DrawWindowTempSmall(Xtft,Ytft,155,30,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;  if(k>ile-1) return;
		    	if((k%2)==0){ Xtft=0; Ytft+=35; }  else Xtft=165;
			}
		    else if(Const.s_GPIO[i].val==3)
			{
		    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);
		    	XYpom[k]=DrawWindowTempSmallPt(Xtft,Ytft,155,30,&buf_p[50],TempKolorEkr(j),Const.s_Czujki[j].nazwa);  k++;  if(k>ile-1) return;
		    	if((k%2)==0){ Xtft=0; Ytft+=35; }  else Xtft=165;


			}
		}
	  }
}
void EkranLcdTempBig()
{
	int i,j,k;
			k=0;
						          for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
								  {
									j= WyszukajNrCzujkiDlaPortu(i+1);
									if(j!=-1)
									{
									    if(Const.s_GPIO[i].val==9)
										{
									    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_32x64, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+175, XYpom[k]+35, "`C",   Font_16x26, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;
									    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
									    	if(j!=-1)
									    	{
									    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]); buf_p[60]=' ';buf_p[63]=0;
										    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[60] , Font_32x64, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
										    	buf_p[180]='%';buf_p[181]=0; ST7735_WriteString((XYpom[k]>>16)+125, XYpom[k]+35, &buf_p[180],   Font_16x26, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
				                                k++;
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
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_32x64, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+175, XYpom[k]+35, "`C",   Font_16x26, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;
										}
									    else if(Const.s_GPIO[i].val==3)
										{
									    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_32x64, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+145, XYpom[k]+35, "`C",   Font_16x26, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;
										}
									}
								  }
}
void EkranLcdTemp()
{
	int i,j,k,color;
			k=0;
						          for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
								  {
									j= WyszukajNrCzujkiDlaPortu(i+1);
									if(j!=-1)
									{
									    if(Const.s_GPIO[i].val==9)
										{
									    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+85, XYpom[k]+6, "`C",   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;
									    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
									    	if(j!=-1)
									    	{
									    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);   buf_p[60]=' ';buf_p[63]=0;
										    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[60] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
										    	buf_p[180]='%';buf_p[181]=0; ST7735_WriteString((XYpom[k]>>16)+58, XYpom[k]+6, &buf_p[180],   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
				                                k++;
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
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+85, XYpom[k]+6, "`C",   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;
										}
									    else if(Const.s_GPIO[i].val==3)
										{
									    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_16x26, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+70, XYpom[k]+6, "`C",   Font_11x18, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;
										}
									}
								  }
}
void EkranLcdTempSmall(int ile)
{
	int i,j,k;
			k=0;
						          for(i=0;i<_Size__s_gpio;i++)  //Temp i RHU
								  {
									j= WyszukajNrCzujkiDlaPortu(i+1);
									if(j!=-1)
									{
									    if(Const.s_GPIO[i].val==9)
										{
									    	wartTemp_odwrotnie(&buf_p[50],Var.value[j]);  korekta_lcd(&buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_11x18, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+60, XYpom[k]+6, "`C",   Font_7x10, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;    if(k>ile-1) return;
									    	j= WyszukajNrCzujkiDlaPortu_2(i+1);
									    	if(j!=-1)
									    	{
									    		wartTemp_odwrotnie(&buf_p[60],Var.value[j]);    buf_p[60]=' ';buf_p[63]=0;
										    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[60] , Font_11x18, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
										    	buf_p[180]='%';buf_p[181]=0; ST7735_WriteString((XYpom[k]>>16)+38, XYpom[k]+6, &buf_p[180],   Font_7x10, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
				                                k++;   if(k>ile-1) return;
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
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_11x18, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+60, XYpom[k]+6, "`C",   Font_7x10, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;    if(k>ile-1) return;
										}
									    else if(Const.s_GPIO[i].val==3)
										{
									    	wartTemp_odwrotnie_Pt_http(&buf_p[50],Var.value[j]); korekta_Pt_lcd(&buf_p[50]);
									    	ST7735_WriteString(XYpom[k]>>16 , XYpom[k], &buf_p[50] , Font_11x18, TempKolorEkr(j), ST7735_COLOR565(0x20,0x20,0x20));
									    	ST7735_WriteString((XYpom[k]>>16)+49, XYpom[k]+6, "`C",   Font_7x10, ST7735_COLOR565(0x90,0x90,0x90), ST7735_COLOR565(0x20,0x20,0x20));
			                                k++;    if(k>ile-1) return;
										}
									}
								  }
}


uint8_t s(uint8_t data)
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
               asm("nop");asm("nop");asm("nop");
               asm("nop");asm("nop");asm("nop");
               asm("nop");asm("nop");asm("nop");


			   TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_6);    //SCK
               asm("nop");asm("nop");asm("nop");
               asm("nop");asm("nop");asm("nop");
               asm("nop");asm("nop");asm("nop");

			}

	        return ttt;
}

/*
uint8_t s(uint8_t byte)
{
	// poczekaj az bufor nadawczy bedzie wolny
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, byte);

	// poczekaj na dane w buforze odbiorczym
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}
*/
void lcd_reset()
{
	HAL_GPIO_WritePin(GPIOB, LCD_RST, GPIO_PIN_RESET);
	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	HAL_GPIO_WritePin(GPIOB, LCD_RST, GPIO_PIN_SET);
}

void lcd_init()
{
	lcd_start();
	lcd_cmd(0x21);
   	lcd_cmd(0x14);
   	lcd_cmd(0x80 | 0x3B); //Ustawienie kontrastu
   	lcd_cmd(0x20);
   	lcd_cmd(0x0c);

	lcd_clear();

	HAL_GPIO_WritePin(GPIOB, LCD_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_RESET);
	lcd_stop();
}

void lcd_init_2()
{
	lcd_start();
	lcd_cmd(0x21);
   	lcd_cmd(0x14);
   	lcd_cmd(0x80 | 0x3B); //Ustawienie kontrastu
   	lcd_cmd(0x20);
   	lcd_cmd(0x0c);
#ifdef _LCD
	lcd_clear2();
#endif
	HAL_GPIO_WritePin(GPIOB, LCD_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_RESET);
	lcd_stop();
}

void lcd_cmd(uint8_t cmd)
{
	lcd_start();
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LCD_DC, GPIO_PIN_RESET);
	s(cmd);
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_SET);
	lcd_stop();
}

void lcd_stop()
{
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_SET);
}

void lcd_start()
{
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_RESET);
}

void lcd_clear2(void)
{
	int i,j;
	lcd_start();
	HAL_GPIO_WritePin(GPIOB, LCD_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_RESET);
	for (j = 0; j < (84*48 + 3*8+2); j++) s(0);
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_SET);
	lcd_stop();
}

void lcd_clear(void)
{
	int i,j;
	lcd_start();
	HAL_GPIO_WritePin(GPIOB, LCD_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_RESET);
#ifdef _LCD
	for (j = 0; j < (84*48 - 7*8-2); j++) s(0);
#endif
#ifndef _LCD
	for (j = 0; j < (84*48); j++) s(0);
#endif
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_SET);
	lcd_stop();
}

void NewLineFont12Bold(int size)
{
	lcd_start(); for (int j = 0; j < 84-size*9; j++) s(0);  lcd_stop();
}

void NewLineFont8Bold(int size, int przes)
{
	lcd_start(); for (int j = 0; j < 84+przes-size*8; j++) s(0);  lcd_stop();
}

void NewLineFont18(int size)
{
	lcd_start(); for (int j = 0; j < 84-size*14; j++) s(0);  lcd_stop();
}

void NewLineFont5x7(int size, int przes)
{
	lcd_start(); for (int j = 0; j < 84+przes-size*6; j++) s(0);  lcd_stop();
}

void NewLine(int przes)
{
	lcd_start(); for (int j = 0; j < przes; j++) s(0);  lcd_stop();
}

void StringFont12Bold(char* d, int size)
{
	int i,j;
	lcd_start();
	for (i = 0; i < size; i++)
	{
	        if(d[i]=='A'){ s(0xF8);s(0xFC);s(0x46);s(0x43);s(0x46);s(0xFC);s(0xF8);s(0x00);s(0x00);}
	   else if(d[i]=='B'){ s(0x01);s(0xFF);s(0xFF);s(0x11);s(0x11);s(0xFF);s(0xCE);s(0x00);s(0x00);}
	   else if(d[i]=='C'){ s(0xfc);s(0xfe);s(0x03);s(0x01);s(0x01);s(0x03);s(0x86);s(0x00);s(0x00);}
	   else if(d[i]=='D'){ s(0x01);s(0xff);s(0xff);s(0x01);s(0x03);s(0xfe);s(0xfc);s(0x00);s(0x00);}
	   else if(d[i]=='E'){ s(0x01);s(0xff);s(0xff);s(0x11);s(0x39);s(0x03);s(0x03);s(0x00);s(0x00);}
	   else if(d[i]=='F'){ s(0x01);s(0xff);s(0xff);s(0x11);s(0x39);s(0x03);s(0x03);s(0x00);s(0x00);}
	   else if(d[i]=='G'){ s(0xfc);s(0xf3);s(0x03);s(0x01);s(0x21);s(0x23);s(0x26);s(0x00);s(0x00);}
	   else if(d[i]==' '){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='0'){ s(0xfc);s(0xfe);s(0x03);s(0x31);s(0x31);s(0x03);s(0xfe);s(0xfc);s(0x00);}
	   else if(d[i]=='1'){ s(0x00);s(0x04);s(0x06);s(0xff);s(0xff);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='2'){ s(0x82);s(0xc3);s(0x61);s(0x31);s(0x19);s(0x0f);s(0x06);s(0x00);s(0x00);}
	   else if(d[i]=='3'){ s(0x02);s(0x03);s(0x01);s(0x11);s(0x11);s(0xff);s(0xee);s(0x00);s(0x00);}
	   else if(d[i]=='4'){ s(0x70);s(0x78);s(0x4c);s(0x46);s(0xff);s(0xff);s(0x40);s(0x00);s(0x00);}
	   else if(d[i]=='5'){ s(0x1f);s(0x1f);s(0x11);s(0x11);s(0x11);s(0xf1);s(0xe1);s(0x00);s(0x00);}
	   else if(d[i]=='6'){ s(0xfc);s(0xfe);s(0x13);s(0x11);s(0x11);s(0xf1);s(0xe0);s(0x00);s(0x00);}
	   else if(d[i]=='7'){ s(0x03);s(0x03);s(0xe1);s(0xf1);s(0x19);s(0x0f);s(0x07);s(0x00);s(0x00);}
	   else if(d[i]=='8'){ s(0xee);s(0xff);s(0x11);s(0x11);s(0x11);s(0xff);s(0xee);s(0x00);s(0x00);}
	   else if(d[i]=='9'){ s(0x0e);s(0x1f);s(0x11);s(0x11);s(0x11);s(0xff);s(0xfe);s(0x00);s(0x00);}
	   else if(d[i]=='.'){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='+'){ s(0x20);s(0x20);s(0x20);s(0xf8);s(0xf8);s(0x20);s(0x20);s(0x20);s(0x00);}
	   else if(d[i]=='-'){ s(0x20);s(0x20);s(0x20);s(0xf1);s(0xf1);s(0x20);s(0x20);s(0x20);s(0x00);}
    }

	NewLineFont12Bold(size);

    for (i = 0; i < size; i++)
    {
            if(d[i]=='A'){ s(0x03);s(0x03);s(0x00);s(0x00);s(0x00);s(0x03);s(0x03);s(0x00);s(0x00);}
	   else if(d[i]=='B'){ s(0x02);s(0x03);s(0x03);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);}
	   else if(d[i]=='C'){ s(0x00);s(0x01);s(0x03);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);}
	   else if(d[i]=='D'){ s(0x02);s(0x03);s(0x03);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='E'){ s(0x02);s(0x03);s(0x03);s(0x02);s(0x02);s(0x03);s(0x03);s(0x00);s(0x00);}
	   else if(d[i]=='F'){ s(0x02);s(0x03);s(0x03);s(0x02);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='G'){ s(0x00);s(0x01);s(0x03);s(0x02);s(0x02);s(0x01);s(0x03);s(0x00);s(0x00);}
	   else if(d[i]==' '){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='0'){ s(0x00);s(0x01);s(0x03);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);}
	   else if(d[i]=='1'){ s(0x00);s(0x02);s(0x02);s(0x03);s(0x03);s(0x02);s(0x02);s(0x00);s(0x00);}
	   else if(d[i]=='2'){ s(0x03);s(0x03);s(0x02);s(0x02);s(0x02);s(0x03);s(0x03);s(0x00);s(0x00);}
	   else if(d[i]=='3'){ s(0x01);s(0x03);s(0x02);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);}
	   else if(d[i]=='4'){ s(0x00);s(0x00);s(0x00);s(0x02);s(0x03);s(0x03);s(0x02);s(0x00);s(0x00);}
	   else if(d[i]=='5'){ s(0x01);s(0x03);s(0x02);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);}
	   else if(d[i]=='6'){ s(0x01);s(0x03);s(0x02);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);}
	   else if(d[i]=='7'){ s(0x00);s(0x00);s(0x03);s(0x03);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='8'){ s(0x01);s(0x03);s(0x02);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);}
	   else if(d[i]=='9'){ s(0x00);s(0x02);s(0x02);s(0x02);s(0x03);s(0x01);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='.'){ s(0x00);s(0x00);s(0x00);s(0x03);s(0x03);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='+'){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='-'){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
    }
    lcd_stop();
}

void StringFont18(char* d, int size)
{
	int i,j;
	lcd_start();
	for (i = 0; i < size; i++)
	{
	        if(d[i]=='A'){ s(0x00);s(0x00);s(0x00);s(0x80);s(0x60);s(0x18);s(0x03);s(0x1f);s(0xfc);s(0xe0);s(0x80);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]==' '){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
    }

	NewLineFont18(size);

    for (i = 0; i < size; i++)
    {
            if(d[i]=='A'){ s(0x40);s(0x70);s(0x2c);s(0x06);s(0x02);s(0x02);s(0x02);s(0x02);s(0x02);s(0x07);s(0x4f);s(0x7c);s(0x70);s(0x40);}
	   else if(d[i]==' '){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
    }
    lcd_stop();
}

void StringFont8Bold(char* d, int size)
{
	int i,j,k;
	lcd_start();
	for (i = 0; i < size; i++)
	{
	        if(d[i]=='A'){ s(0x00);s(0x7e);s(0x7f);s(0x11);s(0x11);s(0x7f);s(0x7e);s(0x00);}
	   else if(d[i]=='B'){ s(0x00);s(0x7f);s(0x7f);s(0x45);s(0x45);s(0x7f);s(0x3a);s(0x00);}
	   else if(d[i]=='0'){ s(0x00);s(0x3e);s(0x7f);s(0x41);s(0x7f);s(0x3e);s(0x00);s(0x00);}
	   else if(d[i]==' '){ s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='1'){ s(0x00);s(0x02);s(0x7f);s(0x7f);s(0x00);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='2'){ s(0x00);s(0x71);s(0x79);s(0x49);s(0x4f);s(0x46);s(0x00);s(0x00);}
	   else if(d[i]=='3'){ s(0x00);s(0x41);s(0x49);s(0x49);s(0x7f);s(0x36);s(0x00);s(0x00);}
	   else if(d[i]=='4'){ s(0x00);s(0x18);s(0x14);s(0x12);s(0x7f);s(0x7f);s(0x00);s(0x00);}
	   else if(d[i]=='5'){ s(0x00);s(0x47);s(0x45);s(0x45);s(0x7d);s(0x38);s(0x00);s(0x00);}
	   else if(d[i]=='6'){ s(0x00);s(0x3e);s(0x7f);s(0x45);s(0x7d);s(0x38);s(0x00);s(0x00);}
	   else if(d[i]=='7'){ s(0x00);s(0x01);s(0x71);s(0x7d);s(0x0f);s(0x03);s(0x00);s(0x00);}
	   else if(d[i]=='8'){ s(0x00);s(0x36);s(0x7f);s(0x49);s(0x7f);s(0x36);s(0x00);s(0x00);}
	   else if(d[i]=='9'){ s(0x00);s(0x0e);s(0x5f);s(0x51);s(0x7f);s(0x3e);s(0x00);s(0x00);}
	   else if(d[i]=='.'){ s(0x00);s(0x60);s(0x60);s(0x00);s(0x00);}
	   else if(d[i]=='+'){ s(0x00);s(0x08);s(0x08);s(0x3e);s(0x08);s(0x08);s(0x00);s(0x00);}
	   else if(d[i]=='-'){ s(0x00);s(0x08);s(0x08);s(0x08);s(0x08);s(0x08);s(0x00);s(0x00);}
	   else if(d[i]==':'){ s(0x00);s(0x6c);s(0x6c);s(0x00);s(0x00);s(0x00);}
	   else if(d[i]=='%'){ s(0x00);s(0x00);s(0x42);s(0x30);s(0x0c);s(0x22);s(0x00);s(0x00);}
    }

	lcd_stop();

}

void StringFont5x7(char* d, int size)
{
  int i,j,k;
  lcd_start();
  for (j = 0; j < size; j++)
  {
	  if(d[j]=='.') k=3; else k=5;
	  for (i = 0; i < k; i++) s(Font5x7[ 5*(d[j]-0x20)+i ]);   s(0x00);
  }
  lcd_stop();
}

void lcd_data(const uint8_t* d, int size)
{
    int i,j;

	lcd_clear();

	HAL_GPIO_WritePin(GPIOB, LCD_DC, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_RESET);


	StringFont5x7("192.168.1.47",12);  NewLineFont5x7(12,3*2);
	NewLineFont5x7(0,0);
	StringFont12Bold("  +29.7",7);  NewLineFont12Bold(7);
	//StringFont8Bold("  +26.5",7);  NewLineFont8Bold(7,1*3);

	StringFont5x7("Temperatura",11);  NewLineFont5x7(11,0);
	StringFont8Bold("      -73.2",11);  //NewLineFont8Bold(8,1*3);


//	StringFont18("AA",2);  NewLineFont18(2);
	//StringFont18("A",1);  NewLineFont18(1);
	//StringFont8Bold("AB",2);  NewLineFont8Bold(2);
	// NewLineFont8Bold(0);
	// StringFont12Bold("ABCD",4);  NewLineFont12Bold(4);
	//StringFont12Bold("DDDDDDD",7);  NewLineFont12Bold(7);

	//StringFont5x7("B@#$%!1234+-_",10);  NewLineFont5x7(10);






	HAL_GPIO_WritePin(GPIOE, LCD_CE, GPIO_PIN_SET);
}
/*
int main(void)
{
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7; // SCK, MOSI
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_6; // MISO
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = LCD_DC|LCD_CE|LCD_RST;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &gpio);
	GPIO_SetBits(GPIOC, LCD_CE|LCD_RST);

	SPI_StructInit(&spi);
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_NSS = SPI_NSS_Soft;
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_Init(SPI1, &spi);
	SPI_Cmd(SPI1, ENABLE);

	lcd_reset();

	lcd_cmd(0x21);
	lcd_cmd(0x14);
	lcd_cmd(0x80 | 0x2f); //Ustawienie kontrastu
	lcd_cmd(0x20);
	lcd_cmd(0x0c);

	lcd_data(logo_mini_mono, sizeof(logo_mini_mono));

	while (1) {}
}
 */
