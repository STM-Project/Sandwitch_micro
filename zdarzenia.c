/*
 * zdarzenia.c
 *
 *  Created on: 03.06.2018
 *      Author: Elektronika RM
 */

#include "stm32f1xx_hal.h"
#include "zdarzenia.h"
#include "define.h"
#include "modem_uart.h"



int OdczytZdarzen(int *addr)
{
	int i,k; char a;

    if(*addr==adr_flash_2) return 0;


	if((adr_flash_2-*addr)<3000){
		  AT25SF_2_ReadArray(4096+*addr, &buf_rx2[_Size_BufWifi], adr_flash_2-*addr);
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)]=0;
	      *addr+=(adr_flash_2-*addr);
	}
	else{
		  AT25SF_2_ReadArray(4096+*addr, &buf_rx2[_Size_BufWifi], 3000);
		  buf_rx2[_Size_BufWifi+ 3000]=0;
		  *addr+=3000;

		  AT25SF_2_ReadArray(4096+*addr, buf_p, 200);
		  for(i=0;i<200;i++)
		  {
			  if((buf_p[i]!='\n')&&(buf_p[i]!=0)) buf_rx2[_Size_BufWifi+ 3000+i]=buf_p[i];
			  else
			  {
				  buf_rx2[_Size_BufWifi+ 3000+i]=buf_p[i];
				  buf_rx2[_Size_BufWifi+ 3000+i+1]=0;
				  *addr+=(i+1);
				  break;
			  }
		  }

		  //dbg3(&buf_rx2[_Size_BufWifi+2900]);


	}

    return 1;

	//dbg3("\r\n");    dbg3(&buf_rx2[_Size_BufWifi]);  dbg3("   ");


}

int OdczytZdarzenSmtp(int *addr)
{
	int i,k; char a;  int sizeB;    sizeB=800;

    if(*addr==adr_flash_2) return 0;


	if((adr_flash_2-*addr)<sizeB){
		  AT25SF_2_ReadArray(4096+*addr, &buf_rx2[_Size_BufWifi], adr_flash_2-*addr);
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)+0]='\r';
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)+1]='\n';
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)+2]=0;
	      *addr+=(adr_flash_2-*addr)+2;
	      return 2;
	}
	else{
		  AT25SF_2_ReadArray(4096+*addr, &buf_rx2[_Size_BufWifi], sizeB);
		  buf_rx2[_Size_BufWifi+ sizeB]=0;
		  *addr+=sizeB;

		  AT25SF_2_ReadArray(4096+*addr, buf_p, 200);
		  for(i=0;i<200;i++)
		  {
			  if((buf_p[i]!='\n')&&(buf_p[i]!=0)) buf_rx2[_Size_BufWifi+ sizeB+i]=buf_p[i];
			  else
			  {
				  buf_rx2[_Size_BufWifi+ sizeB+i+0]='\n';
				  buf_rx2[_Size_BufWifi+ sizeB+i+1]=0;
				  *addr+=(i+1);
				  break;
			  }
		  }

		  //dbg3(&buf_rx2[_Size_BufWifi+2900]);


	}

    return 1;

	//dbg3("\r\n");    dbg3(&buf_rx2[_Size_BufWifi]);  dbg3("   ");


}

int OdczytZdarzenSmtpGSM(int *addr)
{
	int i,k; char a;  int sizeB;    sizeB=800;

    if(*addr==adr_flash_2) return 0;


	if((adr_flash_2-*addr)<sizeB){
		  AT25SF_2_ReadArray(4096+*addr, &buf_rx2[_Size_BufWifi], adr_flash_2-*addr);
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)+0]='\r';
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)+1]='\n';
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)+2]='\x1A';
	      buf_rx2[_Size_BufWifi+ (adr_flash_2-*addr)+3]=0;
	      *addr+=(adr_flash_2-*addr);
	      return 2;
	}
	else{
		  AT25SF_2_ReadArray(4096+*addr, &buf_rx2[_Size_BufWifi], sizeB);
		  buf_rx2[_Size_BufWifi+ sizeB]=0;
		  *addr+=sizeB;

		  AT25SF_2_ReadArray(4096+*addr, buf_p, 200);
		  for(i=0;i<200;i++)
		  {
			  if((buf_p[i]!='\n')&&(buf_p[i]!=0)) buf_rx2[_Size_BufWifi+ sizeB+i]=buf_p[i];
			  else
			  {
				  buf_rx2[_Size_BufWifi+ sizeB+i+0]='\r';
				  buf_rx2[_Size_BufWifi+ sizeB+i+1]='\n';
				  buf_rx2[_Size_BufWifi+ sizeB+i+2]='\x1A';
				  buf_rx2[_Size_BufWifi+ sizeB+i+3]=0;
				  *addr+=(i+1);
				  break;
			  }
		  }

		  //dbg3(&buf_rx2[_Size_BufWifi+2900]);


	}

    return 1;

	//dbg3("\r\n");    dbg3(&buf_rx2[_Size_BufWifi]);  dbg3("   ");


}

int OdczytRejTempSmtp(int GsmLan)
{
   int k,i,j,x;  uint32_t KONIEC_p;   int sizeB;    sizeB=800;    IteracjaFlashZewnOdczyt(&KONIEC_p);

   j=0;


   if(RapAddr<10){              //01.01 00:08   +00.0   +00.0   +20.7
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
           		    	if(GsmLan==0) sprintf(&buf_rx2[_Size_BufWifi+j],"  [%%%%]   "); else sprintf(&buf_rx2[_Size_BufWifi+j],"  [  ]   ");
           		    	 j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //Wilg z DHT
           		     }
           		 }
           		 else if((Const.s_GPIO[i].val==2)||(Const.s_GPIO[i].val==3))  //DS || Pt
           		 {
           			 sprintf(&buf_rx2[_Size_BufWifi+j],"  [°C]  "); j+=strlen(&buf_rx2[_Size_BufWifi+j]);     //temp z DS
           		 }
          }
    }
    sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n"); j+=strlen(&buf_rx2[_Size_BufWifi+j]);


    if(j<sizeB) RapAddr=100;
    else
    {
    	     if(RapAddr==0){ dbg3("1"); if(GsmLan==0) buf_rx2[_Size_BufWifi+sizeB]=0; else{ buf_rx2[_Size_BufWifi+sizeB+0]='\x1A'; buf_rx2[_Size_BufWifi+sizeB+1]=0; }   }
    	else if(RapAddr==1){ dbg3("2"); if(j<2*sizeB){ RapAddr=100; x=j-1*sizeB;} else{ x=sizeB;}   for(i=0;i<x;i++) buf_rx2[_Size_BufWifi+i]=buf_rx2[_Size_BufWifi+i+1*sizeB];  if(GsmLan==0) buf_rx2[_Size_BufWifi+i]=0; else{ buf_rx2[_Size_BufWifi+i+0]='\x1A';buf_rx2[_Size_BufWifi+i+1]=0; }       }
    	else if(RapAddr==2){ dbg3("3"); if(j<3*sizeB){ RapAddr=100; x=j-2*sizeB;} else{ x=sizeB;}   for(i=0;i<x;i++) buf_rx2[_Size_BufWifi+i]=buf_rx2[_Size_BufWifi+i+2*sizeB];  if(GsmLan==0) buf_rx2[_Size_BufWifi+i]=0; else{ buf_rx2[_Size_BufWifi+i+0]='\x1A';buf_rx2[_Size_BufWifi+i+1]=0; }       }
    	else if(RapAddr==3){ dbg3("4"); if(j<4*sizeB){ RapAddr=100; x=j-3*sizeB;} else{ x=sizeB;}   for(i=0;i<x;i++) buf_rx2[_Size_BufWifi+i]=buf_rx2[_Size_BufWifi+i+3*sizeB];  if(GsmLan==0) buf_rx2[_Size_BufWifi+i]=0; else{ buf_rx2[_Size_BufWifi+i+0]='\x1A';buf_rx2[_Size_BufWifi+i+1]=0; }       }
    	else if(RapAddr==4){ dbg3("5"); if(j<5*sizeB){ RapAddr=100; x=j-4*sizeB;} else{ x=sizeB;}   for(i=0;i<x;i++) buf_rx2[_Size_BufWifi+i]=buf_rx2[_Size_BufWifi+i+4*sizeB];  if(GsmLan==0) buf_rx2[_Size_BufWifi+i]=0; else{ buf_rx2[_Size_BufWifi+i+0]='\x1A';buf_rx2[_Size_BufWifi+i+1]=0; }      }
    	     RapAddr++;
    }
    return 1;
  }




	k=IloscCzujnikow();  k+=4;   //4 bo czas zapisywany jest w 4-rech bajtach
       poesdfdfggqqqa:
       sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n");  j+=strlen(&buf_rx2[_Size_BufWifi+j]);
		OdczytPomiarowFlashZewn_p(k,&j);

       if(adr_flash_read>KONIEC_p-1){ dbg3("\r\nXXXXXXXXXXXXXXXXXXXXXXX"); goto poesdfdfggqqq_konieca;  }
		if(j < ( sizeB-(22+8*(k/2)) )) goto poesdfdfggqqqa;
		sprintf(buf_p,"\r\nSSS: %d   %d   %d  ",adr_flash_read,  KONIEC_p, j);  dbg3(buf_p);
		if(GsmLan==1){ sprintf(&buf_rx2[_Size_BufWifi+j],"\x1A");  j+=strlen(&buf_rx2[_Size_BufWifi+j]); }
		return 1;

		poesdfdfggqqq_konieca:
		sprintf(buf_p,"\r\nKKK: %d   %d   %d  ",adr_flash_read,  KONIEC_p, j);  dbg3(buf_p);
		if(GsmLan==1){ sprintf(&buf_rx2[_Size_BufWifi+j],"\x1A");  j+=strlen(&buf_rx2[_Size_BufWifi+j]); }
		 buf_rx2[_Size_BufWifi+j]=0;
		 return 2;


}

int OdczytRejTempSmtpWifi(int *pp)
{
   int k,i,j,x;   uint32_t KONIEC_p;   int sizeB;    sizeB=800;    IteracjaFlashZewnOdczyt(&KONIEC_p);

   j=0;

	k=IloscCzujnikow();  k+=4;   //4 bo czas zapisywany jest w 4-rech bajtach
       poesdfdfggqqqa:
		OdczytPomiarowFlashZewn_p(k,&j);
		sprintf(&buf_rx2[_Size_BufWifi+j],"\r\n");  j+=strlen(&buf_rx2[_Size_BufWifi+j]);

       if(adr_flash_read>KONIEC_p-1){   goto poesdfdfggqqq_konieca;  }
		if(j < ( sizeB-(22+8*(k/2)) )) goto poesdfdfggqqqa;
		sprintf(buf_p,"\r\nSSS: %d   %d   %d  ",adr_flash_read,  KONIEC_p, j);  dbg3(buf_p);
		*pp=j;
		return 1;

		poesdfdfggqqq_konieca:
		 buf_rx2[_Size_BufWifi+j]=0;
		 *pp=j;   sprintf(buf_p,"\r\nKKKK: %d   %d   %d  ",adr_flash_read,  KONIEC_p, j);  dbg3(buf_p);
		 return 2;


}






int OdczytSesjiLogGSM(int *addr)
{
	int i,k; char a;

    if(*addr==adr_flash_SesjaLogGSM) return 0;


	if((adr_flash_SesjaLogGSM-*addr)<3000){
		  AT25SF_3_ReadArray(_Log_GSM_Start+*addr, &buf_rx2[_Size_BufWifi], adr_flash_SesjaLogGSM-*addr);
	      buf_rx2[_Size_BufWifi+ (adr_flash_SesjaLogGSM-*addr)]=0;
	      *addr+=(adr_flash_SesjaLogGSM-*addr);
	}
	else{
		  AT25SF_3_ReadArray(_Log_GSM_Start+*addr, &buf_rx2[_Size_BufWifi], 3000);
		  buf_rx2[_Size_BufWifi+ 3000]=0;
		  *addr+=3000;

		  AT25SF_3_ReadArray(_Log_GSM_Start+*addr, buf_p, 200);
		  for(i=0;i<200;i++)
		  {
			  if((buf_p[i]!='\n')&&(buf_p[i]!=0)) buf_rx2[_Size_BufWifi+ 3000+i]=buf_p[i];
			  else
			  {
				  buf_rx2[_Size_BufWifi+ 3000+i]=buf_p[i];
				  buf_rx2[_Size_BufWifi+ 3000+i+1]=0;
				  *addr+=(i+1);
				  break;
			  }
		  }
	}
    return 1;
}

void ZapisZdarzenia(char TypZda, int RodzZda, int NrZda)
{
   int p,itx,ss;   itx=0;

    PobierzCzas(buf_p);
	sprintf(&buf_TME[itx],"\r\n%c%02d.%02d.%02d %02d:%02d:%02d - ",TypZda,buf_p[0],buf_p[1],buf_p[2],buf_p[4],buf_p[5],buf_p[6]);
	itx+=strlen(&buf_TME[itx]);

	    p=NrZda;
/*#ifdef _LoRa
	    if(TypZda==_ZdaLoRa)
	    {
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
	    }
#endif*/

//######## -- Restart Ukladu  -- #######################################

	    if(RodzZda==_PogrUrzytk) sprintf(&buf_TME[itx],"Uruchamiam Program Uzytkownika");


//######## -- Temperatura Wilgotnosc  -- #######################################

	    else if(RodzZda==_TempPowPmax1)
	    {
	    	if(Const.s_GPIO[  Const.s_Czujki[p].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt(&buf_p[20],Const.s_Czujki[p].prog_max1 );
	    	else                                                        wartTemp_odwrotnie(   &buf_p[20],Const.s_Czujki[p].prog_max1 );
	    	sprintf(&buf_TME[itx],"%s POWYZEJ %s",Const.s_Czujki[p].nazwa, &buf_p[20]);
	    }
		else if(RodzZda==_TempPowPmax2)
	    {
	    	if(Const.s_GPIO[  Const.s_Czujki[p].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt(&buf_p[20],Const.s_Czujki[p].prog_max2 );
	    	else                                                        wartTemp_odwrotnie(   &buf_p[20],Const.s_Czujki[p].prog_max2 );
	    	sprintf(&buf_TME[itx],"%s POWYZEJ %s",Const.s_Czujki[p].nazwa, &buf_p[20]);
	    }

		else if(RodzZda==_TempNorm1)
	    {
	    	if(Const.s_GPIO[  Const.s_Czujki[p].addr_DS[8]-1  ].val==3){ wartTemp_odwrotnie_Pt(&buf_p[20],Const.s_Czujki[p].prog_max1 ); wartTemp_odwrotnie_Pt(&buf_p[40],Const.s_Czujki[p].prog_min1 ); }
	    	else                                                       { wartTemp_odwrotnie(   &buf_p[20],Const.s_Czujki[p].prog_max1 ); wartTemp_odwrotnie(   &buf_p[40],Const.s_Czujki[p].prog_min1 ); }
	    	sprintf(&buf_TME[itx],"%s pomiedzy %s i %s",Const.s_Czujki[p].nazwa, &buf_p[20],&buf_p[40]);
	    }
		else if(RodzZda==_TempNorm2)
	    {
	    	if(Const.s_GPIO[  Const.s_Czujki[p].addr_DS[8]-1  ].val==3){ wartTemp_odwrotnie_Pt(&buf_p[20],Const.s_Czujki[p].prog_max2 ); wartTemp_odwrotnie_Pt(&buf_p[40],Const.s_Czujki[p].prog_min2 ); }
	    	else                                                       { wartTemp_odwrotnie(   &buf_p[20],Const.s_Czujki[p].prog_max2 ); wartTemp_odwrotnie(   &buf_p[40],Const.s_Czujki[p].prog_min2 ); }
	    	sprintf(&buf_TME[itx],"%s pomiedzy %s i %s",Const.s_Czujki[p].nazwa, &buf_p[20],&buf_p[40]);
	    }

	    else if(RodzZda==_TempPonPmin1)
	    {
	    	if(Const.s_GPIO[  Const.s_Czujki[p].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt(&buf_p[20],Const.s_Czujki[p].prog_min1 );
	    	else                                                        wartTemp_odwrotnie(   &buf_p[20],Const.s_Czujki[p].prog_min1 );
	    	sprintf(&buf_TME[itx],"%s PONIZEJ %s",Const.s_Czujki[p].nazwa, &buf_p[20]);
	    }
		else if(RodzZda==_TempPonPmin2)
	    {
	    	if(Const.s_GPIO[  Const.s_Czujki[p].addr_DS[8]-1  ].val==3) wartTemp_odwrotnie_Pt(&buf_p[20],Const.s_Czujki[p].prog_min2 );
	    	else                                                        wartTemp_odwrotnie(   &buf_p[20],Const.s_Czujki[p].prog_min2 );
	    	sprintf(&buf_TME[itx],"%s PONIZEJ %s",Const.s_Czujki[p].nazwa, &buf_p[20]);
	    }

//######## -- Wejscia WE  -- #######################################

	    else if(RodzZda==_WeZwa) sprintf(&buf_TME[itx],"%s %s",Const.s_WE[p].nazwa, Const.s_WE[p].nazwa_zw);
		else if(RodzZda==_WeRoz) sprintf(&buf_TME[itx],"%s %s",Const.s_WE[p].nazwa, Const.s_WE[p].nazwa_ro);
		else if(RodzZda==_WeUsz) sprintf(&buf_TME[itx],"%s %s",Const.s_WE[p].nazwa, Const.s_WE[p].nazwa_uszk);

//######## -- Termostat  -- #######################################

	   else if(RodzZda==_TermON)  sprintf(&buf_TME[itx],"Termostat: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_on);
	   else if(RodzZda==_TermOFF) sprintf(&buf_TME[itx],"Termostat: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_of);


//######## -- WEPK  -- #######################################

       else if(RodzZda==_WEPKON)  sprintf(&buf_TME[itx],"WE-PK: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_on);
	   else if(RodzZda==_WEPKOFF) sprintf(&buf_TME[itx],"WE-PK: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_of);


//######## -- Timer  -- #######################################

	   else if(RodzZda==_TimON)  sprintf(&buf_TME[itx],"Timer: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_on);
	   else if(RodzZda==_TimOFF) sprintf(&buf_TME[itx],"Timer: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_of);

 //######## -- PK WWW  -- #######################################

	   else if(RodzZda==_PKon)  sprintf(&buf_TME[itx],"WY str WWW: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_on);
	   else if(RodzZda==_PKoff) sprintf(&buf_TME[itx],"WY str WWW: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_of);

	    //######## -- PK SMS  -- #######################################

	   	   else if(RodzZda==_PKsmson)  sprintf(&buf_TME[itx],"WY sms: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_on);
	   	   else if(RodzZda==_PKsmsoff) sprintf(&buf_TME[itx],"WY sms: %s %s",Const.s_PK[p].nazwa, Const.s_PK[p].nazwa_of);


  //######## -- Rotacja  -- #######################################

       else if((RodzZda==_RotKon)||(RodzZda==_AllPkOn)||(RodzZda==_AllPkOf))
	   {
	       	   if(RodzZda==_RotKon) sprintf(&buf_TME[itx],"Rot: ");
	       	   if(RodzZda==_AllPkOn) sprintf(&buf_TME[itx],"Rot Alarm: ");
	       	   if(RodzZda==_AllPkOf) sprintf(&buf_TME[itx],"Rot OFF: ");

	       	   itx+=strlen(&buf_TME[itx]);

	   	        for(i=0;i<_Size_wy;i++)
	      		    {
	   	              /*   if(t_wy[i]==31) ss=6;   //to dla routi
	   	            else if(t_wy[i]==32) ss=7;
	   	            else if(t_wy[i]==33) ss=8;
	   	            else if(t_wy[i]==21) ss=3;
	   	            else if(t_wy[i]==22) ss=4;
	   	            else if(t_wy[i]==23) ss=5;*/

	   	        	if(Const.s_GPIO[t_wy[i]-1].val==0)    //PK
	      			    {
	      			    	if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0)  sprintf(&buf_TME[itx],"PK%d:on   ",t_wy[i]);
	      			    	else                                             sprintf(&buf_TME[itx],"PK%d:off  ",t_wy[i]);
	      			    }
	      			    else  if(Const.s_GPIO[t_wy[i]-1].val==8)  //IR
	      			    {
	      			    	if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0)  sprintf(&buf_TME[itx],"IR%d:on   ",t_wy[i]);
	      			    	else                                             sprintf(&buf_TME[itx],"IR%d:off  ",t_wy[i]);
	      			    }
	      			    itx+=strlen(&buf_TME[itx]);
	      		    }
	    }


    //######## -- Test -- #######################################

	   else if(RodzZda==_Esp1)  sprintf(&buf_TME[itx],"ESP NIE ODPOWIADA 111 !!!");
	   else if(RodzZda==_Esp2)  sprintf(&buf_TME[itx],"ESP NIE ODPOWIADA 222 !!!");



  //######## --    LoRa  -- #######################################
	//DS-5 "Strumyk"    =>   DS5-portiernia    DS22-pokój(rzekomo 2 lory ale bedzie jedna)    DS23-sekretariat
/*#ifdef _LoRa
       else if(RodzZda==_LoZasBrak)  sprintf(&buf_TME[itx],"DS-%d%s%s:  BRAK ZASIEGU",p,&buf_p[100],&buf_p[150]);
       else if(RodzZda==_LoAlarm1){  if(NrZda==21) sprintf(&buf_TME[itx],"DS-%d%s%s:  ALARM WLAMANIOWY",p,&buf_p[100],&buf_p[150]);  else sprintf(&buf_TME[itx],"DS-%d%s%s:  ALARM I st.",p,&buf_p[100],&buf_p[150]);   }
       else if(RodzZda==_LoAlarm2)   sprintf(&buf_TME[itx],"DS-%d%s%s:  ALARM II st.",p,&buf_p[100],&buf_p[150]);
       else if(RodzZda==_LoNapad){    if(NrZda==23) sprintf(&buf_TME[itx],"DS-%d%s%s:  NAPAD",p,&buf_p[100],&buf_p[150]);  else sprintf(&buf_TME[itx],"DS-%d%s%s:  NAPAD",p,&buf_p[100],&buf_p[150]);   }
       else if(RodzZda==_LoAlarm1o){   if(NrZda==21) sprintf(&buf_TME[itx],"DS-%d%s%s:  alarm wlamaniowy odwolany",p,&buf_p[100],&buf_p[150]);  else sprintf(&buf_TME[itx],"DS-%d%s%s:  alarm I st. odwolany",p,&buf_p[100],&buf_p[150]);   }
       else if(RodzZda==_LoAlarm2o)   sprintf(&buf_TME[itx],"DS-%d%s%s:  alarm II st. odwolany",p,&buf_p[100],&buf_p[150]);
       else if(RodzZda==_LoNapado){   if(NrZda==23) sprintf(&buf_TME[itx],"DS-%d%s%s:  napad odwolany",p,&buf_p[100],&buf_p[150]);  else sprintf(&buf_TME[itx],"DS-%d%s%s:  napad odwolany",p,&buf_p[100],&buf_p[150]);   }
       else if(RodzZda==_LoObiektOdw)  sprintf(&buf_TME[itx],"DS-%d%s%s:  Manualne odwolanie alarmu",p,&buf_p[100],&buf_p[150]);
#endif

*/








	    itx+=strlen(&buf_TME[itx]);
	    ZapiszFlash_2_Zewn(&adr_flash_2,buf_TME, itx);     IteracjaFlash_2_ZewnZapis(adr_flash_2);

      // dbg3("\r\n");    dbg3(&buf_rx2[_Size_BufWifi]);  dbg3("   ");

}

void ZapiszFlash_2_Zewn(uint32_t* adres, uint8_t* data, uint16_t len)
{
	uint32_t  adr_p,adrr,adr,k1,k2;  uint8_t  buf[256];

	if((*adres+len)>(_Zda_Flash_2_Size-4096))
	{    dbg3("\r\nPRZEWIJAM Flash_2 !!!"); *adres=0;
	     AT25SF_2_BlockErase(0); HAL_Delay(200); AT25SF_2_BlockErase(4096); dbg3("  ERASE_2 block 4K for iter. ");
	     adr_flash_2=0; //adr_flash_end=0;  adr_flash_start=0;
	     Const.s_Rap[0].start_zd= 0;         delay_funkc[6]=1;
	     return;
	}

	adr=*adres;    adr_p= 256*(adr/256);    adrr= adr_p+256-adr;

	NRF_Start();

	k1= (adr+  0)/4096;
	k2= (adr+len)/4096;

	if(k1<k2){ AT25SF_2_BlockErase(4096+4096*k2); HAL_Delay(500); dbg3("\r\nERASE_2 block 4K "); }

	AT25SF_2_ReadArray(4096+adr_p, buf, 256 );

	if(len<=adrr)
	{	AT25SF_2_PageProgram(4096+adr, data, len );
	}
	else
	{   AT25SF_2_PageProgram(4096+adr, data, adrr );
        AT25SF_2_PageProgram(4096+adr_p+256, &data[adrr], len-adrr );
	}
	*adres+=len;

	NRF_Stop();
}

void ZapisSesjiSMTP(char *data, int len, uint32_t *it)
{
	char buf[3];    uint32_t addr;

	buf[0]=len>>8;
	buf[1]=len;
	addr = _Sesja_SMTP_Start + 2+*it;    if(addr > (_Sesja_SMTP_Start + _Sesja_SMTP_Size)) return;
    ZapiszFlash_3_Zewn(&addr, buf, 2);   *it+=2;
    addr = _Sesja_SMTP_Start + 2+*it;    if(addr > (_Sesja_SMTP_Start + _Sesja_SMTP_Size)) return;
	ZapiszFlash_3_Zewn(&addr, data, len);   *it+=len;
}

void ZapisSesjiLogGSM(char *data, int len, uint32_t *it)
{
    uint32_t addr;
  //dbg3("\r\nZAPIS SESJI LOG GSM ");
    addr = _Log_GSM_Start +*it;    if(addr > (_Log_GSM_Start + _Log_GSM_Size)) return;
	ZapiszFlash_3_Zewn(&addr, data, len);   *it+=len;
}

void ZapiszFlash_3_Zewn(uint32_t* adres, uint8_t* data, uint16_t len)
{
	uint32_t  adr_p,adrr,adr,k1,k2;  uint8_t  buf[256];

	if((*adres+len)>(_Zda_Flash_3_Size-4096))
	{    dbg3("\r\nKONIEC Flash_3 !!!");
	     return;
	}

	adr=*adres;    adr_p= 256*(adr/256);    adrr= adr_p+256-adr;

	NRF_Start();

	k1= (adr+  0)/4096;
	k2= (adr+len)/4096;

	if(k1<k2){ AT25SF_3_BlockErase(0+4096*k2); HAL_Delay(100); dbg3("\r\nERASE_3 block 4K "); }

	AT25SF_3_ReadArray(0+adr_p, buf, 256 );

	if(len<=adrr)
	{	AT25SF_3_PageProgram(0+adr, data, len );
	}
	else
	{   AT25SF_3_PageProgram(0+adr, data, adrr );
        AT25SF_3_PageProgram(0+adr_p+256, &data[adrr], len-adrr );
	}
	*adres+=len;

	NRF_Stop();
}
