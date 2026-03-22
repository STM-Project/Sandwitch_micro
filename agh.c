/*
 * agh.c
 *
 *  Created on: 08.09.2018
 *      Author: Elektronika RM
 */


#include "agh.h"
#include <modem_uart.h>
#include "http.h"
#include "zdarzenia.h"


void HttpAgh(int ood, int doo)
{
	int i,j,itx,kx,k;  itx=0;  char *buf[5],*ptr,*ptr1,*ptr2,*ptr3;   uint32_t addr;

 if(ood==0){
	sprintf(&buf_rx2[_Size_BufWifi+itx],"\" width=\"1600\" height=\"500\">\x0D\x0A");
    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
 }

 for(i=ood;i<doo;i++)    //rozmiar  jednej iteracji: 500 B
 {
       if(i==0){ sprintf(&buf_p[0],"408"); sprintf(&buf_p[5],"193");  }
  else if(i==1){ sprintf(&buf_p[0],"453"); sprintf(&buf_p[5],"130");  }
  else if(i==2){ sprintf(&buf_p[0],"282"); sprintf(&buf_p[5],"169");  }
  else if(i==3){ sprintf(&buf_p[0],"527"); sprintf(&buf_p[5],"128");  }
  else if(i==4){ sprintf(&buf_p[0],"527"); sprintf(&buf_p[5],"180");  }
  else if(i==5){ sprintf(&buf_p[0],"517"); sprintf(&buf_p[5],"231");  }
  else if(i==6){ sprintf(&buf_p[0],"516"); sprintf(&buf_p[5],"288");  }
  else if(i==7){ sprintf(&buf_p[0],"527"); sprintf(&buf_p[5],"340");  }
  else if(i==8){ sprintf(&buf_p[0],"670"); sprintf(&buf_p[5],"127");  }
  else if(i==9){ sprintf(&buf_p[0],"680"); sprintf(&buf_p[5],"179");  }
  else if(i==10){ sprintf(&buf_p[0],"680"); sprintf(&buf_p[5],"230");  }
  else if(i==11){ sprintf(&buf_p[0],"646"); sprintf(&buf_p[5],"288");  }
  else if(i==12){ sprintf(&buf_p[0],"657"); sprintf(&buf_p[5],"339");  }
  else if(i==13){ sprintf(&buf_p[0],"737"); sprintf(&buf_p[5],"292");  }
  else if(i==14){ sprintf(&buf_p[0],"186"); sprintf(&buf_p[5],"345");  }
  else if(i==15){ sprintf(&buf_p[0],"186"); sprintf(&buf_p[5],"294");  }
  else if(i==16){ sprintf(&buf_p[0],"195"); sprintf(&buf_p[5],"242");  }
  else if(i==17){ sprintf(&buf_p[0],"150"); sprintf(&buf_p[5],"184");  }
  else if(i==18){ sprintf(&buf_p[0],"159"); sprintf(&buf_p[5],"132");  }
  else if(i==19){ sprintf(&buf_p[0],"1115"); sprintf(&buf_p[5],"322");  }
  else if(i==20){ sprintf(&buf_p[0],"472"); sprintf(&buf_p[5],"213");  }

	 sprintf(&buf_rx2[_Size_BufWifi+itx],"\x0D\x0A\
 <div style=\"position:absolute; left:%spx; top:%spx;\"><table border=\"0\" width=\"30px\" height=\"55px\"><tbody><tr><td><a href=\"#\" rel=\"external\" onClick=\"xpk%d(); this.href='javascript:return false;';\"><div id=\"tu_sens%02d\"> </div></a></td></tr></tbody></table></div>",&buf_p[0],&buf_p[5],i+1,i+1);
   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
 }
 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpAgh itx !!!");

}

void HttpAgh_2()
{
	int i,j,itx,kx,k;  itx=0;  char *buf[5],*ptr,*ptr1,*ptr2,*ptr3;   uint32_t addr;


  sprintf(&buf_rx2[_Size_BufWifi+itx],"if((txt[0]=='1')");
  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 for(i=1;i<_Ile_Czujek;i++)
 {
	 sprintf(&buf_rx2[_Size_BufWifi+itx],"||(txt[%d]=='1')",i*5);
     itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
 }
 sprintf(&buf_rx2[_Size_BufWifi+itx],"){ txt3=\"<font size='6' color='red'>Alarm 2 stopnia</font>\";wsk6s=1;}\x0D\x0A else if((txt[1]=='1')");
 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 for(i=1;i<_Ile_Czujek;i++)
 {
	   sprintf(&buf_rx2[_Size_BufWifi+itx],"||(txt[%d]=='1')",i*5+1);
 	   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);	                        //21  MS   zamist alarmu 1st -> alarm wlamaniowy        (nie ma alarmu 2 st,)
 }
 sprintf(&buf_rx2[_Size_BufWifi+itx],"){ if(txt[101]=='1'){ txt3=\"<font size='6' color='orange'>Alarm wlamaniowy</font>\";wsk6sA=1;} else{ txt3=\"<font size='6' color='orange'>Alarm 1 stopnia</font>\";wsk6s=1;}  }\x0D\x0A else{ if(wsk6sA>0){ txt3=\"<font size='6' color='black'>Alarm wlamaniowy odwolany</font>\"; if(wsk6sA>10) wsk6sA=0;} else if(wsk6s>0){ txt3=\"<font size='6' color='black'>Alarm odwolany</font>\"; if(wsk6s>10) wsk6s=0;} else txt3=\"<font size='6' color='white'>.</font>\"}\x0D\x0A document.getElementById(\"tu_nagl01a\").innerHTML=txt3;\x0D\x0A if((txt[2]=='1')");
 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 for(i=1;i<_Ile_Czujek;i++)
 {
	   sprintf(&buf_rx2[_Size_BufWifi+itx],"||(txt[%d]=='1')",i*5+2);
 	   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);                           //23   sekretat   alarm wlamaniowy == zamist napadu
 }
 sprintf(&buf_rx2[_Size_BufWifi+itx],"){ if(txt[112]=='1'){ txt3=\"<font size='6' color='blue'>Napad</font>\"; wsk5sA=1 }  else{ txt3=\"<font size='6' color='blue'>Napad</font>\";  wsk5s=1;} }\x0D\x0A else{ if(wsk5sA>0){ txt3=\"<font size='6' color='black'>Alarm wlamaniowy odwolany</font>\"; if(wsk5sA>10) wsk5sA=0;} else if(wsk5s>0){ txt3=\"<font size='6' color='black'>Napad odwolany</font>\"; if(wsk5s>10) wsk5s=0;} else txt3=\"<font size='6' color='white'>.</font>\"}\x0D\x0A document.getElementById(\"tu_nagl01b\").innerHTML=txt3;\x0D\x0A");
 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpAgh_2 itx !!!");

}

void HttpAgh_end(int ood, int doo)
{
	int i,j,itx,kx,k;  itx=0;  char *buf[5],*ptr,*ptr1,*ptr2,*ptr3;   uint32_t addr;  char zlikw[5];

	if(i==0){ sprintf(&buf_rx2[_Size_BufWifi+itx],"var col=[]; var px;\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);   }

	for(i=ood;i<doo;i++)
	{
		 if(i==14) sprintf(zlikw,"//");  //DS15 nie ma na razie
		 else      zlikw[0]=0;
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"%stxt5[0]=\"\";px=0;if(txt[5*%d+0]==1){px=1;col=\"red\";} else if(txt[5*%d+1]==1){px=1;col=\"orange\";} else if(txt[5*%d+2]==1){px=1; col=\"blue\";ko%d[0]=\" \";} else if(txt[5*%d+3]==0){px=1; col=\"gray\"; txt5[0]=\" brak zasiegu  &nbsp;\";ko%d[0]=\" \";} if(px==1) document.getElementById(\"tu_ds%d\").innerHTML=\"<font size='5' color='\"+col+\"'>\"+txt5+ds%d+ko%d+\"</font>\"; else document.getElementById(\"tu_ds%d\").innerHTML=\"\";\r\n",zlikw,i,i,i,i+1,i,i+1,i+1,i+1,i+1,i+1);
		 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}
	if(i==23)
	{
	    sprintf(&buf_rx2[_Size_BufWifi+itx],"\
 }\x0D\x0A\
 </script>\x0D\x0A\
 <script>\x0D\x0A\ 
 var tyt=[];   tyt[0]=\"   \";\x0D\x0A\
 document.getElementById(\"tu_menu\").innerHTML=tyt;\x0D\x0A\
 </script>\x0D\x0A\
 </html>\x0D\x0A");
	    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}
   if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpAgh_end itx !!!");
}


void HttpAgh_3(int ood, int doo)
{
	int i,j,itx,kx,k;  itx=0;  char *buf[5],*ptr,*ptr1,*ptr2,*ptr3;   uint32_t addr;


 for(i=ood;i<doo;i++)    //rozmiar  jednej iteracji: 500 B
 {

  if((i<3)||(i==13))
  {
	 sprintf(&buf_rx2[_Size_BufWifi+itx]," \
      if(txt[%d]=='1') txt3=kula_czerwD[0]; \x0D\x0A\
 else if(txt[%d]=='1') txt3=kula_pomarD[0];\x0D\x0A\
 else if(txt[%d]=='1') txt3=kula_niebiD[0];\x0D\x0A\
 else if(txt[%d]=='1') txt3=kula_zieloD[0];\x0D\x0A\
 else  txt3=kula_szaryD[0];  document.getElementById(\"tu_sens%02d\").innerHTML=txt3+\"DS%d</span></u></div>\";\x0D\x0A\
 \x0D\x0A\ 
 \x0D\x0A",5*i+0, 5*i+1, 5*i+2, 5*i+3, i+1, i+1);
  }
  else if(i==19)
  {
		 sprintf(&buf_rx2[_Size_BufWifi+itx]," \
	      if(txt[%d]=='1') txt3=kula_czerwDA[0]; \x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_pomarDA[0];\x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_niebiDA[0];\x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_zieloDA[0];\x0D\x0A\
	 else txt3=kula_szaryDA[0];  document.getElementById(\"tu_sens%02d\").innerHTML=txt3;\x0D\x0A\
	 \x0D\x0A\ 
	 \x0D\x0A",5*i+0, 5*i+1, 5*i+2, 5*i+3, i+1, i+1);

  }
  else if(i==20)
  {
		 sprintf(&buf_rx2[_Size_BufWifi+itx]," \
	      if(txt[%d]=='1') txt3=kula_czerwMS[0]; \x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_pomarMS[0];\x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_niebiMS[0];\x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_zieloMS[0];\x0D\x0A\
	 else  txt3=kula_szaryMS[0];  document.getElementById(\"tu_sens%02d\").innerHTML=txt3;\x0D\x0A\
	 \x0D\x0A\ 
	 \x0D\x0A",5*i+0, 5*i+1, 5*i+2, 5*i+3, i+1, i+1);

  }
  else if(i==4)
  {
		 sprintf(&buf_rx2[_Size_BufWifi+itx]," \
	      if((txt[20]=='1')||(txt[105]=='1')||(txt[110]=='1')) txt3=kula_czerw[0]; \x0D\x0A\
	 else if((txt[21]=='1')||(txt[106]=='1')||(txt[111]=='1')) txt3=kula_pomar[0];\x0D\x0A\
	 else if((txt[22]=='1')||(txt[107]=='1')||(txt[112]=='1')) txt3=kula_niebi[0];\x0D\x0A\
	 else if((txt[23]=='1')&&(txt[108]=='1')&&(txt[113]=='1')) txt3=kula_zielo[0];\x0D\x0A\
	 else txt3=kula_szary[0];   document.getElementById(\"tu_sens05\").innerHTML=txt3+\"DS5</span></u></div>\";\x0D\x0A\
	 \x0D\x0A\ 
	 \x0D\x0A");

  }
  else
  {
		 sprintf(&buf_rx2[_Size_BufWifi+itx]," \
	      if(txt[%d]=='1') txt3=kula_czerw[0]; \x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_pomar[0];\x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_niebi[0];\x0D\x0A\
	 else if(txt[%d]=='1') txt3=kula_zielo[0];\x0D\x0A\
	 else txt3=kula_szary[0];   document.getElementById(\"tu_sens%02d\").innerHTML=txt3+\"DS%d</span></u></div>\";\x0D\x0A\
	 \x0D\x0A\ 
	 \x0D\x0A",5*i+0, 5*i+1, 5*i+2, 5*i+3, i+1, i+1);

  }
  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 }
 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpAgh_3 itx !!!");


}

void AGH_stan_alarmu()
{
  int i,i1,i2,i3;

  if(start_alarm==1)
  {
     i1=0;i2=0;i3=0;
     for(i=0;i<_Ile_Czujek;i++)
  	 {
  		 if((Lora[i]&0x01)>0) i1=1;
  		 if((Lora[i]&0x02)>0) i2=1;
  		 if((Lora[i]&0x04)>0) i3=1;
  	 }
     if(i1==0){ j=0; if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){ Const.s_WY[j/16].val&=~(1<<(j-16*(j/16))); HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,GPIO_PIN_RESET); FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) ); } }
     if(i2==0){ j=1; if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){ Const.s_WY[j/16].val&=~(1<<(j-16*(j/16))); HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET); FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) ); } }
     if(i3==0){ j=2; if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){ Const.s_WY[j/16].val&=~(1<<(j-16*(j/16))); HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET); FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) ); } }

  }

  TM_GPIO_SetPinLow(GPIOE, GPIO_PIN_11);  // //Buzzer OFF
  for(i=0;i<3;i++)
  {
	  if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0)  TM_GPIO_SetPinHigh(GPIOE, GPIO_PIN_11);   //Buzzer ON
  }

}



#ifdef _LoRa
void Obraz_Agh()
{
	if(WhichFragmentPage==1)
	{
		HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_1agh;
		itx_mapa=0;
		//size_mapa= strlen(HttpImg);
		AT25SF_3_ReadArray(_Obraz_Start, buf_p, 3 );
	    size_mapa= 0x00ff0000&(((uint32_t)buf_p[0])<<16) | 0x0000ff00&(((uint32_t)buf_p[1])<<8) | 0x000000ff&((uint32_t)buf_p[2]);
	    adr_flash_Obraz_read=_Obraz_Start+3;
	}
	else if((WhichFragmentPage>1)&&(WhichFragmentPage<(2+(size_mapa/2990))))
	{
		//for(i=0;i<2990;i++) buf_rx2[_Size_BufWifi+i]= HttpImg[itx_mapa+i];    buf_rx2[_Size_BufWifi+i]=0;
		//itx_mapa+=i;
		AT25SF_3_ReadArray(adr_flash_Obraz_read, &buf_rx2[_Size_BufWifi], 2990 );  buf_rx2[_Size_BufWifi+2990]=0;
		adr_flash_Obraz_read+=2990;
		itx_mapa+=2990;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(2+(size_mapa/2990)))
	{
		//for(i=0;i<(size_mapa-itx_mapa);i++) buf_rx2[_Size_BufWifi+i]= HttpImg[itx_mapa+i];    buf_rx2[_Size_BufWifi+i]=0;
		AT25SF_3_ReadArray(adr_flash_Obraz_read, &buf_rx2[_Size_BufWifi], size_mapa-itx_mapa );  buf_rx2[_Size_BufWifi+(size_mapa-itx_mapa)]=0;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(2+(size_mapa/2990)+1))
	{
		HttpAgh(0,10);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(2+(size_mapa/2990)+2))
	{
		HttpAgh(10,20);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(2+(size_mapa/2990)+3))
	{
		HttpAgh(20,21);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(2+(size_mapa/2990)+4))
	{
		if((wybor_bitowy&0x0200)==0) HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_2agh;
		else                         HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_2agh_esp;
	}
	else if(WhichFragmentPage==(2+(size_mapa/2990)+5))
	{
		 HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_2agh_b;
			itx_mapa=0;
			size_mapa2= strlen(Http_DSalfa);
	}

	else if((WhichFragmentPage>(2+(size_mapa/2990)+5))&&(WhichFragmentPage<  (3+(size_mapa/2990)+5) + (size_mapa2/2990)  ))
	{
		for(i=0;i<2990;i++) buf_rx2[_Size_BufWifi+i]= Http_DSalfa[itx_mapa+i];    buf_rx2[_Size_BufWifi+i]=0;
		itx_mapa+=i;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage== (3+(size_mapa/2990)+5) + (size_mapa2/2990)+0 )
	{
		for(i=0;i<(size_mapa2-itx_mapa);i++) buf_rx2[_Size_BufWifi+i]= Http_DSalfa[itx_mapa+i];    buf_rx2[_Size_BufWifi+i]=0;
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}

	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+1)
    {
		  HttpPagesBuffer[WhichFragmentPage]= Http_MS;
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+2)
	{
		 HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_3agh_a;
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+3)
	{
		 HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_3agh_b;
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+4)
	{
		HttpAgh_2();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+5)
	{
		HttpAgh_3(0,10);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+6)
	{
		HttpAgh_3(10,20);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+7)
	{
		HttpAgh_3(20,21);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+8)
	{
		HttpPagesBuffer[WhichFragmentPage]= HttpMapaLora_4agh;
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+9)
	{
		HttpAgh_end(0,8);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+10)
	{
		HttpAgh_end(8,16);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==(3+(size_mapa/2990)+5) + (size_mapa2/2990)+11)
	{
		HttpAgh_end(16,23);
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	    HttpPagesBuffer[WhichFragmentPage+1]=0;
	}

}
#endif;

