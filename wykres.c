/*
 * wykres.c
 *
 *  Created on: 4 lip 2018
 *      Author: ssss
 */

#include "stm32f1xx_hal.h"
#include "modem_uart.h"
#include "http.h"
#include "wejscia_licznikowe.h"
#include "wykres.h"

//################################  ---  Wykres dla Licznika   ---  ###############################################
//*******************************************************************************************************************

void HttpWykresStartEtr_1()  //Liczniki impl
{
	int i,j,itx,kx;  itx=0;   char *ptr,*ptr1,*ptr2,*ptr3;    buf_rx2[_Size_BufWifi]=0;

  sprintf(&buf_rx2[_Size_BufWifi+itx],"<br>\r\n");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
  for(i=0;i<_Size__s_gpio;i++)
  {
    if(Const.s_GPIO[i].val==7)  //licznik impl
    {
	  j=WyszukujPortElement(t_imp,_Size_imp,i+1);
	  if(j!=-1)
	  {
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykres2g_%02d\"; return false><button type=\"button\">%s - Godzinne</button></a> &nbsp;",j,Const.s_GPIO[i].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykres2d_%02d\"; return false><button type=\"button\">%s - Dobowe</button></a> &nbsp;",j,Const.s_GPIO[i].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykres2t_%02d\"; return false><button type=\"button\">%s - Tygodniowe</button></a> &nbsp;",j,Const.s_GPIO[i].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"<a href=\"lpc.cgi/wykres2m_%02d\"; return false><button type=\"button\">%s - Miesieczne</button></a> &nbsp;<br>\r\n",j,Const.s_GPIO[i].nazwa);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	  }
    }
  }
  if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStartEtr_1 itx !!!");
}

//*******************************************************************************************************************

void HttpWykresStartEtr_3()
{
	int i,j,itx,kx,p,cc;  itx=0;  char bug1[7],bug2[7],a,buf[10];    buf_rx2[_Size_BufWifi]=0;
	uint32_t ulam;

	i=WykrNrEtr/4;       j=1+WykrNrEtr - 4*(WykrNrEtr/4);
	ulam= FormatHttpImpUlam(i, Const.s_Imp[i].prog1_ulam[j], buf);

     sprintf(bug1,"%d.%d", Const.s_Imp[i].prog1_dzies[j], ulam);
     sprintf(bug2,"0");

     Const.s_WykrEtr[WykrNrEtr].prgmin=0;
// if(Const.s_Wykr[Var.wykres_nr].prgmax>100) cc=60; else cc=20;
 cc=(Const.s_WykrEtr[WykrNrEtr].prgmax+Const.s_WykrEtr[WykrNrEtr].prgmin)/10;  // sprintf(buf_p,"\r\nXXXXXXXXX: %d  ",cc); dbg3(buf_p);
 if(cc<10);
 else
 {
	 ergtythyht3:
	 if((cc%10)!=0){ cc--; goto ergtythyht3; }
 }
 if(cc==0) sprintf(buf,"0.1");
 else      sprintf(buf,"%d",cc);

   sprintf(&buf_rx2[_Size_BufWifi+itx],"</body><script>var i,j,k,k1,k2,can,ctx,przes1,przes2,przes3,krtn, ilosc_pomiarow,rozciag_zawez,Const_width,Samples,minVal,maxVal,xScalar, yScalar,y,const_y;var prg_max, prg_min;\x0D\x0A\			
 Const_width=4000;Samples = 144;\x0D\x0A\
 prg_max =%s;\x0D\x0A\
 prg_min =%s;\x0D\x0A\
 ilosc_pomiarow = 144*3000;\x0D\x0A\
 rozciag_zawez = %d;\x0D\x0A\
 function Round(n,k){var factor=Math.pow(10,k);return Math.round(n*factor)/factor;}\x0D\x0A\
 function init(){maxVal = %d;minVal= -%d;var stepSize = %s;var colHead = 50;var rowHead = 60;var margin = 10;var header = \"%s\";\x0D\x0A\
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
 }\x0D\x0A",bug1,bug2, Const.s_WykrEtr[WykrNrEtr].zawezen, Const.s_WykrEtr[WykrNrEtr].prgmax, Const.s_WykrEtr[WykrNrEtr].prgmin,buf,Const.s_Imp[i].jedn);
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStart_3 itx !!!");

//sprintf(buf_p,"\r\nHttpWykresStart_2   %d  ",itx);  dbg3(buf_p);
	//sprintf(buf_p,"\r\nHttpWykresStart_2:   Var.wykres_nr: %d  %d  ---%d ",Var.wykres_nr, Const.s_Wykr[Var.wykres_nr].zawezen,itx);  dbg3(buf_p);

}

//*******************************************************************************************************************

void HttpWykresDaneEtr() // ["12:09","06.16",+10.5,"12:00","06.17",+11.5,.....
{
	int i,j,itx,kx,k,k1,k2,nr_licz,nr_nval;  uint32_t aa;   itx=0;  char a, tabnr[_Size_imp];   buf_rx2[_Size_BufWifi]=0;

	nr_licz= WykrNrEtr/4;       nr_nval= 1+WykrNrEtr - 4*(WykrNrEtr/4);

	if(WhichFragmentPage==5)
	{
		     if(nr_nval==1) adr_flash_Etr_read[nr_nval]=_Etr_godz_Start;
		else if(nr_nval==2) adr_flash_Etr_read[nr_nval]=_Etr_dob_Start;
		else if(nr_nval==3) adr_flash_Etr_read[nr_nval]=_Etr_tyg_Start;
		else if(nr_nval==4) adr_flash_Etr_read[nr_nval]=_Etr_mies_Start;

		IteracjaFlash_3_OdczEtr(nr_nval);
		if((adr_flash_Etr[nr_nval]==0)||(adr_flash_Etr[nr_nval]==adr_flash_Etr_read[nr_nval])){ dbg3("\r\nKoniec Wykres Etr NIC   "); WhichFragmentPage=22; sprintf(&buf_rx2[_Size_BufWifi+itx],"            "); itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);  goto HttpWykresDane2_bbb2; }
		sprintf(&buf_rx2[_Size_BufWifi+itx],"var dataValue = [");    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}





	HttpWykresDane2_aaa:

	for(i=0;i<_Size_imp;i++) tabnr[i]=0;
	k1=0; for(i=0;i<_Size_imp;i++){  if(Const.s_GPIO[t_imp[i]-1].val==7) tabnr[k1++]=i+1;   }

	     AT25SF_3_ReadArray(adr_flash_Etr_read[nr_nval], buf_p, 4);    adr_flash_Etr_read[nr_nval]+=4;
	   	  time=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
	   	  ZamianaCzasuZFlash(time);
	   	  sprintf(&buf_rx2[_Size_BufWifi+itx],"\"%02d:%02d\",\"%02d.%02d\",",mies,dzien,godz,min);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);



	   for(i=0;i<_Size_imp;i++)
	   {
		   if(tabnr[i]>0)
		   {
			   if(tabnr[i]==(nr_licz+1)){ tabnr[i]=0;  break;  }
			   else{                      tabnr[i]=0;  adr_flash_Etr_read[nr_nval]+=4;  }
		   }
	   }

	   		AT25SF_3_ReadArray(adr_flash_Etr_read[nr_nval], buf_p, 4);      adr_flash_Etr_read[nr_nval]+=4;
	         aa=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
	         PrzeliczLicznikRej(i,aa,buf_p);
	         sprintf(&buf_rx2[_Size_BufWifi+itx],"%s,",buf_p );    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


	  	   for(i=0;i<_Size_imp;i++)
	  	   {
	  		   if(tabnr[i]>0)
	  		   {
	  			   if(tabnr[i]==(nr_licz+1)){   adr_flash_Etr_read[nr_nval]+=4;  }
	  			   else                         adr_flash_Etr_read[nr_nval]+=4;
	  		   }
	  	   }




	if(adr_flash_Etr_read[nr_nval]>=adr_flash_Etr[nr_nval]){ dbg3("\r\nKoniec Wykres Etr ");  WhichFragmentPage=22; goto HttpWykresDane2_bbb;  }
	if(itx < (_Size_WskFragmentPage-100)) goto HttpWykresDane2_aaa;  //Nie wiem dlaczego 500 a nie np 100 !!!!!!
	else
	{      sprintf(buf_p,"\r\nAAA:  %d  %d",adr_flash_Etr_read[nr_nval], adr_flash_Etr[nr_nval]); dbg3(buf_p);
		   if(WhichFragmentPage>5) WhichFragmentPage--;
		   goto HttpWykresDane2_bbb2;
	}


	   //sprintf(&buf_rx2[_Size_BufWifi+itx],"\"01:04\",\"00.00\",30,\"01:04\",\"00.00\",40,\"01:04\",\"00.00\",50,\"01:04\",\"00.00\",60");    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	  // itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);



	   WhichFragmentPage=22;

	HttpWykresDane2_bbb:
	sprintf(&buf_rx2[_Size_BufWifi+itx-1],"];</script></html>");    itx+=strlen(&buf_rx2[_Size_BufWifi+itx-1]);
	HttpWykresDane2_bbb2:
	buf_rx2[_Size_BufWifi+itx]=0;

	 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresDane Etr itx !!!");

}

//*******************************************************************************************************************

void HttpWykresStartEtr_0()
{
	int i,j,itx,kx,nr_val;  itx=0;   char *ptr,*ptr1,*ptr2,*ptr3,bufn[20];    buf_rx2[_Size_BufWifi]=0;

	nr_val= 1+WykrNrEtr - 4*(WykrNrEtr/4);
	     if(nr_val==1) sprintf(bufn,"godzinny");
	else if(nr_val==2) sprintf(bufn,"dobowy");
	else if(nr_val==3) sprintf(bufn,"tygodniowy");
	else if(nr_val==4) sprintf(bufn,"miesieczny");

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
  <body onload=\"init()\"><div align=\"center\"><h2><a href=\"pomiar\" return false><button type=\"button\"><--</button></a> &nbsp; Archiwizacja Danych Pomiarowych</h2><font size=\"4\" color=\"blue\">%s - %s</font><br><br><canvas id=\"can\" height=\"400\"\x0D\x0A\
  width=\"%d\">\x0D\x0A\
  </canvas></div>",Const.s_GPIO[t_imp[WykrNrEtr/4]-1].nazwa,bufn, Const.s_WykrEtr[WykrNrEtr].szerwykr);
 itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 sprintf(&buf_rx2[_Size_BufWifi+itx],"<br><br><font size=\"4\" color=\"blue\">Wybor czujnika:</font> &nbsp;");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


 if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStart_0 itx !!!");

}

//*******************************************************************************************************************

void HttpWykresStartEtr_2()
{
	int i,j,itx,kx;  itx=0;   char *ptr,*ptr1,*ptr2,*ptr3;    buf_rx2[_Size_BufWifi]=0;

sprintf(&buf_rx2[_Size_BufWifi+itx],"<br><br><br><br><br><br><div class=\"y\"><div class=\"x\"><br><form method=\"get\" action=\"lpc.cgi/A117\" data-ajax=\"false\"><font size=\"5\" color=\"black\">Przedzial wyswietlanych wartosci </font><div id=\"mainselection\"><select style=\"background: #909090;\" name=\"n_s_WykrEtr_prgmax_0%02d\" >",WykrNrEtr+1);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
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
sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value=\"1\"           >1</option>");
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

sprintf(&buf_rx2[_Size_BufWifi+itx],"</select> &nbsp;&nbsp; <select style=\"background: #909090;\" name=\"n_s_WykrEtr_prgmin_0%02d\" >",WykrNrEtr+1);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	sprintf(&buf_rx2[_Size_BufWifi+itx],"<option value=\"0\"           >0</option>");
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);


sprintf(&buf_rx2[_Size_BufWifi+itx],"</select></div><br><font size=\"5\" color=\"black\">Zawezenie wykresu</font><br> <input class=\"textboxB\" style=\"width:40px; background-color:#909090;\" type=\"text\" name=\"n_s_WykrEtr_zaw_0%02d\" size=\"3\" maxlength=\"3\"   value=\"%d\"      > <font size=\"4\" color=\"black\">(1-500)</font><br><br>",WykrNrEtr+1,Const.s_WykrEtr[WykrNrEtr].zawezen);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
sprintf(&buf_rx2[_Size_BufWifi+itx],"<font size=\"5\" color=\"black\">Szerokosc Wykresu</font><br> <input class=\"textboxB\" style=\"width:60px; background-color:#909090;\" type=\"text\" name=\"n_s_WykrEtr_sze_0%02d\" size=\"5\" maxlength=\"5\"   value=\"%d\"      > <font size=\"4\" color=\"black\">(1000 - 30000)</font><br><br>",WykrNrEtr+1,Const.s_WykrEtr[WykrNrEtr].szerwykr);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

sprintf(&buf_rx2[_Size_BufWifi+itx],"<button  class=\"css_button\" type=\"submit\" name=\"n_109_zapisz\">Ustaw</button></form></font></div></div>");
itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

if(itx>_Size_WskFragmentPage) dbg3("\r\nPRZEKROCZONO HttpWykresStart_2 itx !!!");
//sprintf(buf_p,"\r\nHttpWykresStart_2   %d  ",itx);  dbg3(buf_p);
//sprintf(buf_p,"\r\nHttpWykresStart_1:   Var.wykres_nr: %d  %d    ---%d ",Var.wykres_nr, Const.s_Wykr[Var.wykres_nr].zawezen,itx);  dbg3(buf_p);

}


//*******************************************************************************************************************



//*******************************************************************************************************************


