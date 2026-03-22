/*
 * wejscie_licznikowe.c
 *
 *  Created on: 15 lis 2016
 *      Author: VA
 */

#include "stm32f1xx_hal.h"

#include "wejscia_licznikowe.h"
#include "modem_uart.h"
#include "gpio.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

static void MX_TIM2_ETR_Init__moj(void)
{

	  Set_input(GPIOA,GPIO_PIN_15);

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim2.Instance = TIM2;
	  htim2.Init.Prescaler = 0;
	  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim2.Init.Period = 60000;
	  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	  sClockSourceConfig.ClockFilter = 15;
	  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }



	  __HAL_TIM_SET_COUNTER(&htim2,0);

	  if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
	    {
	      Error_Handler();
	    }

}

static void MX_TIM1_ETR_Init__moj(void)
{

	  Set_input(GPIOE,GPIO_PIN_7);

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim1.Instance = TIM1;
	  htim1.Init.Prescaler = 0;
	  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim1.Init.Period = 60000;
	  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	  sClockSourceConfig.ClockFilter = 15;
	  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }



	  __HAL_TIM_SET_COUNTER(&htim1,0);

	  if (HAL_TIM_Base_Start(&htim1) != HAL_OK)
	    {
	      Error_Handler();
	    }

}

static void MX_TIM3_ETR_Init__moj(void)
{

	  Set_input(GPIOD,GPIO_PIN_2);

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim3.Instance = TIM3;
	  htim3.Init.Prescaler = 0;
	  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim3.Init.Period = 60000;
	  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	  sClockSourceConfig.ClockFilter = 15;
	  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }



	  __HAL_TIM_SET_COUNTER(&htim3,0);

	  if (HAL_TIM_Base_Start(&htim3) != HAL_OK)
	    {
	      Error_Handler();
	    }

}

static void MX_TIM4_ETR_Init__moj(void)
{

	  Set_input(GPIOE,GPIO_PIN_0);

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim4.Instance = TIM4;
	  htim4.Init.Prescaler = 0;
	  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim4.Init.Period = 60000;
	  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
	  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
	  sClockSourceConfig.ClockFilter = 15;
	  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }



	  __HAL_TIM_SET_COUNTER(&htim4,0);

	  if (HAL_TIM_Base_Start(&htim4) != HAL_OK)
	    {
	      Error_Handler();
	    }

}

static void MX_TIM2_ETR_DeInit__moj(void)
{

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim2.Instance = TIM2;
	  htim2.Init.Prescaler = 0;
	  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim2.Init.Period = 60000;
	  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_DeInit(&htim2) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }
}
static void MX_TIM1_ETR_DeInit__moj(void)
{

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim1.Instance = TIM1;
	  htim1.Init.Prescaler = 0;
	  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim1.Init.Period = 60000;
	  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_DeInit(&htim1) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }
}
static void MX_TIM3_ETR_DeInit__moj(void)
{

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim3.Instance = TIM3;
	  htim3.Init.Prescaler = 0;
	  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim3.Init.Period = 60000;
	  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_DeInit(&htim3) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }
}
static void MX_TIM4_ETR_DeInit__moj(void)
{

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;

	  htim4.Instance = TIM4;
	  htim4.Init.Prescaler = 0;
	  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim4.Init.Period = 60000;
	  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_DeInit(&htim4) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }
}


void Init_Tim1_Etr()
{
	MX_TIM1_ETR_Init__moj();  dbg3("\r\nInit_Tim1_Etr");
}
void DeInit_Tim1_Etr()
{
	MX_TIM1_ETR_DeInit__moj();
}

void Init_Tim2_Etr()
{
	MX_TIM2_ETR_Init__moj(); dbg3("\r\nInit_Tim2_Etr");
}
void DeInit_Tim2_Etr()
{
	MX_TIM2_ETR_DeInit__moj();
}

void Init_Tim3_Etr()
{
	MX_TIM3_ETR_Init__moj();  dbg3("\r\nInit_Tim3_Etr");
}
void DeInit_Tim3_Etr()
{
	MX_TIM3_ETR_DeInit__moj();
}

void Init_Tim4_Etr()
{
	MX_TIM4_ETR_Init__moj();  dbg3("\r\nInit_Tim4_Etr");
}
void DeInit_Tim4_Etr()
{
	MX_TIM4_ETR_DeInit__moj();
}


int stan_licznika_Tim1_Etr()
{
	return __HAL_TIM_GET_COUNTER(&htim1);
}
int stan_licznika_Tim2_Etr()
{
	return __HAL_TIM_GET_COUNTER(&htim2);
}
int stan_licznika_Tim3_Etr()
{
	return __HAL_TIM_GET_COUNTER(&htim3);
}
int stan_licznika_Tim4_Etr()
{
	return __HAL_TIM_GET_COUNTER(&htim4);
}

void HttpLiczProgi_Start()
{
	int i,j,itx,kx,ap;  itx=0;

	sprintf(&buf_rx2[_Size_BufWifi+itx],"<div class=\"corner-radius4\" style=\"margin-left: 0px;width: 550px;height: 830px;\">\r\n\
 <div class=\"corner-radius3\" style=\"margin-left: 10px;width: 520px;height: 800px;\">\r\n\
 <font class=\"css-font-style\" style=\"font-family: Courier\"><b>Progi Alarmowe dla powiadamiania  </b></font><br>\r\n\
 <font size=\"3\" color=\"#eeeeee\" >&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --</font><br>\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
}

void HttpLiczProgi(int Etr_nr, char cc)
{
	int i,j,itx,kx,ap;  itx=0;

     if(cc=='g') sprintf(buf_p,"godzinny");
else if(cc=='d') sprintf(buf_p,"dobowy");
else if(cc=='t') sprintf(buf_p,"tygodniowy");
else if(cc=='m') sprintf(buf_p,"miesieczny");

 sprintf(&buf_rx2[_Size_BufWifi+itx],"<font class=\"css_list\">Prog 1 (%s)</font>&nbsp;&nbsp;\r\n\
 <input type=\"text\" class=\"textbox\" style=\"width: 70px\" name=\"n_s_pEtr%cd1_%03d\" size=\"7\" maxlength=\"7\" value=\"       \">\r\n\
 <font class=\"css_list\" style=\"padding: 0px;color:#e0e0e0;\" ><b>.</b></font>\r\n\
 <input type=\"text\" class=\"textbox\" style=\"width: 50px\" name=\"n_s_pEtr%cu1_%03d\" size=\"5\" maxlength=\"5\" value=\"     \">&nbsp;\r\n\
 <font class=\"css_list\" style=\"padding: 0px;color:#e0e0e0;\" >%s</font><br>\r\n\
 <font class=\"css_list\">Prog 2 (%s)</font>&nbsp;&nbsp;\r\n\
 <input type=\"text\" class=\"textbox\" style=\"width: 70px\" name=\"n_s_pEtr%cd2_%03d\" size=\"7\" maxlength=\"7\" value=\"       \">\r\n\
 <font class=\"css_list\" style=\"padding: 0px;color:#e0e0e0;\" ><b>.</b></font>\r\n\
 <input type=\"text\" class=\"textbox\" style=\"width: 50px\" name=\"n_s_pEtr%cu2_%03d\" size=\"5\" maxlength=\"5\" value=\"     \">&nbsp;\r\n\
 <font class=\"css_list\" style=\"padding: 0px;color:#e0e0e0;\" >%s</font><br>\r\n\
 <br>\r\n",buf_p,cc,Etr_nr,cc,Etr_nr,Const.s_Imp[Etr_nr-1].jedn,buf_p,cc,Etr_nr,cc,Etr_nr,Const.s_Imp[Etr_nr-1].jedn);  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

 if(cc=='m'){ sprintf(&buf_rx2[_Size_BufWifi+itx],"</div></div>");  itx+=strlen(&buf_rx2[_Size_BufWifi+itx]); }

}

void LicznikRej()
{
   #ifdef _REJ
	 if(((Const.s_Rej[0].param&0x01)>0)&&((Const.s_Rej[0].param&0x02)==0))  //Rejestrator ON i kontynyowany
	 {
		 ZapisFlashLicznikow();
	 }
   #endif
}

void Zeruj_Liczniki_Etr(int i)
{
	uint32_t stan;

        if(i==0) stan=stan_licznika_Tim1_Etr();
   else if(i==1) stan=stan_licznika_Tim2_Etr();
   else if(i==2) stan=stan_licznika_Tim3_Etr();
   else if(i==3) stan=stan_licznika_Tim4_Etr();

  PobierzCzas(buf_p);
   if(                               buf_p[5]==0){    if((Var.wsk_etr[i]&0x01)==0){ Var.wsk_etr[i]|=0x01;  znkZap=1; znak_zapisu[i][1]=1; Impl_do_Rej[i][1]=Const.s_Imp[i].value[1];  Var.EtrPocz[i][1]=0; Var.EtrVal[i][1]=stan;  Const.s_Imp[i].value[1]=0;  dbg3("\r\nZERUJ LICZNIK 1 ");}   } else Var.wsk_etr[i]&=~0x01; //co godzine
   if(               (buf_p[4]==0)&&(buf_p[5]==0)){   if((Var.wsk_etr[i]&0x02)==0){ Var.wsk_etr[i]|=0x02;  znkZap=1; znak_zapisu[i][2]=1; Impl_do_Rej[i][2]=Const.s_Imp[i].value[2];  Var.EtrPocz[i][2]=0; Var.EtrVal[i][2]=stan;  Const.s_Imp[i].value[2]=0;  dbg3("\r\nZERUJ LICZNIK 2 ");}   } else Var.wsk_etr[i]&=~0x02; //co dobe
   if((buf_p[3]==1)&&(buf_p[4]==0)&&(buf_p[5]==0)){   if((Var.wsk_etr[i]&0x04)==0){ Var.wsk_etr[i]|=0x04;  znkZap=1; znak_zapisu[i][3]=1; Impl_do_Rej[i][3]=Const.s_Imp[i].value[3];  Var.EtrPocz[i][3]=0; Var.EtrVal[i][3]=stan;  Const.s_Imp[i].value[3]=0;  dbg3("\r\nZERUJ LICZNIK 3 ");}   } else Var.wsk_etr[i]&=~0x04; //co tydzien
   if(buf_p[1]==Var.month_etr[i])                 {   if((Var.wsk_etr[i]&0x08)==0){ Var.wsk_etr[i]|=0x08;  znkZap=1; znak_zapisu[i][4]=1; Impl_do_Rej[i][4]=Const.s_Imp[i].value[4];  Var.EtrPocz[i][4]=0; Var.EtrVal[i][4]=stan;  Const.s_Imp[i].value[4]=0;  dbg3("\r\nZERUJ LICZNIK 4 ");}   } else{Var.wsk_etr[i]&=~0x08;  Var.month_etr[i]=buf_p[1]; } //co miesiac
}


void PrzeliczEtrValue(int nr_licz, int nr_val, uint32_t *dzies, uint32_t *ulam)
{
    int i,j,k;
	uint32_t poprzecinku, pom;

	i=nr_licz;
	j=nr_val;

      if(Const.s_Imp[i].licz<=10)     poprzecinku=10;
 else if(Const.s_Imp[i].licz<=100)    poprzecinku=100;
 else if(Const.s_Imp[i].licz<=1000)   poprzecinku=1000;
 else if(Const.s_Imp[i].licz<=10000)  poprzecinku=10000;
 else if(Const.s_Imp[i].licz<=100000) poprzecinku=100000;

  *dzies=   (Const.s_Imp[i].value[j]*Const.s_Imp[i].wart)/Const.s_Imp[i].licz;
  pom=  ((Const.s_Imp[i].value[j]*Const.s_Imp[i].wart*Const.s_Imp[i].licz)/Const.s_Imp[i].licz)  -  ((*dzies)*Const.s_Imp[i].licz);
  *ulam= (pom*poprzecinku)/Const.s_Imp[i].licz;

}

uint32_t FormatHttpImpUlam(int nr_licz, uint32_t val, char *buf)
{
  int i,k,x;   uint32_t pom;

  i=nr_licz;

        if(val<10)     k=1;
   else if(val<100)    k=2;
   else if(val<1000)   k=3;
   else if(val<10000)  k=4;
   else if(val<100000) k=5;

	  if(Const.s_Imp[i].licz<=10)     x=1;
 else if(Const.s_Imp[i].licz<=100)    x=2;
 else if(Const.s_Imp[i].licz<=1000)   x=3;
 else if(Const.s_Imp[i].licz<=10000)  x=4;
 else if(Const.s_Imp[i].licz<=100000) x=5;

	  if(k<x)
	  {
              if((x-k)==1) sprintf(buf,"0%d",val);
         else if((x-k)==2) sprintf(buf,"00%d",val);
         else if((x-k)==3) sprintf(buf,"000%d",val);
         else if((x-k)==4) sprintf(buf,"0000%d",val);

	  }
	  else if(k==x)
	  {
		  sprintf(buf,"%d",val);
	  }
	  else if(k>x)
	  {
		       if((k-x)==1) sprintf(buf,"%d",val/10);
		  else if((k-x)==2) sprintf(buf,"%d",val/100);
		  else if((k-x)==3) sprintf(buf,"%d",val/1000);
		  else if((k-x)==4) sprintf(buf,"%d",val/10000);
	  }

	 if(x==1) return (                                                                                      (buf[0]&0x0f)    );
	 if(x==2) return (                                                                   10*(buf[0]&0x0f) + (buf[1]&0x0f)    );
	 if(x==3) return (                                               100*(buf[0]&0x0f) + 10*(buf[1]&0x0f) + (buf[2]&0x0f)    );
	 if(x==4) return (                          1000*(buf[0]&0x0f) + 100*(buf[1]&0x0f) + 10*(buf[2]&0x0f) + (buf[3]&0x0f)    );
	 if(x==5) return (    10000*(buf[0]&0x0f) + 1000*(buf[1]&0x0f) + 100*(buf[2]&0x0f) + 10*(buf[3]&0x0f) + (buf[4]&0x0f)    );
}

void sprawdz_prog_Etr(void)
{
   int i,j,k;   uint32_t dzies, ulam, ulam2;

   for(i=0;i<_Size_imp;i++){
	for(j=1;j<_Imp_IleVal;j++){
    if(Const.s_GPIO[t_imp[i]-1].val==7){


		PrzeliczEtrValue(i, j, &dzies, &ulam);


		ulam2= FormatHttpImpUlam(0, Const.s_Imp[i].prog1_ulam[j], buf_p);
		if(dzies==Const.s_Imp[i].prog1_dzies[j]){
			if(ulam > ulam2)
			{
				if(Var.etrwsk[i][j]==0){  Var.etrwsk[i][j]|=(0x01<<j);
				   sprintf(buf_p,"\r\n a ETR %d-%d Prog 1  POWYZEJ !!!  %d.%d    %d.%d ",i,j, dzies,ulam,  Const.s_Imp[i].prog1_dzies[j],ulam2    ); dbg3(buf_p);
				}
		    }
			else Var.etrwsk[i][j]&=~(0x01<<j);
		}
		else if(dzies > Const.s_Imp[i].prog1_dzies[j])
		{
			    if(Var.etrwsk[i][j]==0){  Var.etrwsk[i][j]|=(0x01<<j);
			       sprintf(buf_p,"\r\nETR %d-%d Prog 1  POWYZEJ !!!  %d.%d    %d.%d ",i,j, dzies,ulam,  Const.s_Imp[i].prog1_dzies[j],ulam2    ); dbg3(buf_p);
			    }
	    }
		else Var.etrwsk[i][j]&=~(0x01<<j);




		ulam2= FormatHttpImpUlam(0, Const.s_Imp[i].prog2_ulam[j], buf_p);
		if(dzies==Const.s_Imp[i].prog2_dzies[j]){
			if(ulam > ulam2)
			{
				if(Var.etrwsk2[i][j]==0){  Var.etrwsk2[i][j]|=(0x01<<j);
				   sprintf(buf_p,"\r\nETR %d-%d Prog 2  POWYZEJ !!! ",i,j); dbg3(buf_p);
				}
		    }
			else Var.etrwsk2[i][j]&=~(0x01<<j);
		}
		else if(dzies > Const.s_Imp[i].prog2_dzies[j])
		{
			    if(Var.etrwsk2[i][j]==0){  Var.etrwsk2[i][j]|=(0x01<<j);
			       sprintf(buf_p,"\r\nETR %d-%d Prog 2  POWYZEJ !!! ",i,j); dbg3(buf_p);
			    }
		}
		else Var.etrwsk2[i][j]&=~(0x01<<j);



    }
	}
   }


}

void ZerujparametryLicznikow(int nr_licz)
{
	int i,j;

		 for(j=0;j<_Imp_IleVal;j++)
		 {
		   	    	Var.EtrPocz[nr_licz][j]= 0;
		   	    	Var.EtrVal[nr_licz][j]=0;
		   	    	Const.s_Imp[nr_licz].value[j]=0;
		   	    	HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR11+nr_licz*5+j,0);
         }
}


void ObslugaLicznikow()
{
   int i,j;

   for(i=0;i<_Size_imp;i++){
	   for(j=0;j<_Imp_IleVal;j++) znak_zapisu[i][j]=0;
   }
   znkZap=0;

	 j=0;
	  if(Const.s_GPIO[__Port_Ogoln_Etr_1-1].val==7)
	  {
		  for(i=0;i<_Imp_IleVal;i++){  Const.s_Imp[0].value[i]=Var.EtrPocz[0][i]+stan_licznika_Tim1_Etr()-Var.EtrVal[0][i];   HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR11+0*5+i,Const.s_Imp[0].value[i]); }
		  Zeruj_Liczniki_Etr(0);  j=1;
	  }
	  if(Const.s_GPIO[__Port_Ogoln_Etr_2-1].val==7)
	  {
		  for(i=0;i<_Imp_IleVal;i++){  Const.s_Imp[1].value[i]=Var.EtrPocz[1][i]+stan_licznika_Tim2_Etr()-Var.EtrVal[1][i];   HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR11+1*5+i,Const.s_Imp[1].value[i]); }
          Zeruj_Liczniki_Etr(1);  j=1;
	  }
	  if(Const.s_GPIO[__Port_Ogoln_Etr_3-1].val==7)
	  {
		  for(i=0;i<_Imp_IleVal;i++){  Const.s_Imp[2].value[i]=Var.EtrPocz[2][i]+stan_licznika_Tim3_Etr()-Var.EtrVal[2][i];   HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR11+2*5+i,Const.s_Imp[2].value[i]); }
          Zeruj_Liczniki_Etr(2);  j=1;
	  }
	  if(Const.s_GPIO[__Port_Ogoln_Etr_4-1].val==7)
	  {
		  for(i=0;i<_Imp_IleVal;i++){  Const.s_Imp[3].value[i]=Var.EtrPocz[3][i]+stan_licznika_Tim4_Etr()-Var.EtrVal[3][i];   HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR11+3*5+i,Const.s_Imp[3].value[i]); }
          Zeruj_Liczniki_Etr(3);  j=1;
	  }
      if(j==1) sprawdz_prog_Etr();
      if(znkZap==1) LicznikRej();
}

void ZapisFlashLicznikow()
{
  int i,j,k,itx,a;  itx=0;  char buf[50];


	for(j=1;j<_Imp_IleVal;j++)
	{
		k=0;
		ZamianaCzasuNaFlash();
		buf[k++]=time>>24;
		buf[k++]=time>>16;
		buf[k++]=time>>8;
	    buf[k++]=time>>0;
        a=0;
		for(i=0;i<_Size_imp;i++)
		{
			if(Const.s_GPIO[t_imp[i]-1].val==7)
			{
			   if(znak_zapisu[i][j]==1)
			   {
				  buf[k++]=Impl_do_Rej[i][j]>>24;
				  buf[k++]=Impl_do_Rej[i][j]>>16;
				  buf[k++]=Impl_do_Rej[i][j]>>8;
				  buf[k++]=Impl_do_Rej[i][j]>>0;
				  a=1;
			   }
			}
		}
		if(a==1)
		{	ZapiszFlash_3_Zewn(&adr_flash_Etr[j], buf, k);
		    IteracjaFlash_3_ZapEtr(j);
		}
	}

    dbg3("\r\nZapis Liczniki");
}

void ZapisFlashLicznikow_DOWYMAZANIA()
{
  int i,j,k,itx,a;  itx=0;  char buf[50];


	for(j=1;j<_Imp_IleVal;j++)
	{

		k=0;
		ZamianaCzasuNaFlash();
		buf[k++]=time>>24;
		buf[k++]=time>>16;
		buf[k++]=time>>8;
	    buf[k++]=time>>0;
        a=0;
		for(i=0;i<_Size_imp;i++)
		{
			if(Const.s_GPIO[t_imp[i]-1].val==7)
			{
			   //if(znak_zapisu[i][j]==1)
			  // {
				   Impl_do_Rej[i][j]=250+(j-1)*100;
				  buf[k++]=Impl_do_Rej[i][j]>>24;
				  buf[k++]=Impl_do_Rej[i][j]>>16;
				  buf[k++]=Impl_do_Rej[i][j]>>8;
				  buf[k++]=Impl_do_Rej[i][j]>>0;
				  a=1;
			  // }
			}
		}
		if(a==1)
		{	ZapiszFlash_3_Zewn(&adr_flash_Etr[j], buf, k);
		    IteracjaFlash_3_ZapEtr(j);
		}
	}

    dbg3("\r\nZapis Liczniki");
}

void PrzeliczLicznikRej(int nr_licz, int val, char *buf)
{
    int i,j,k;
	uint32_t poprzecinku, dzies, ulam, pom;

	i=nr_licz;

      if(Const.s_Imp[i].licz<=10)     poprzecinku=10;
 else if(Const.s_Imp[i].licz<=100)    poprzecinku=100;
 else if(Const.s_Imp[i].licz<=1000)   poprzecinku=1000;
 else if(Const.s_Imp[i].licz<=10000)  poprzecinku=10000;
 else if(Const.s_Imp[i].licz<=100000) poprzecinku=100000;


    dzies=   (val*Const.s_Imp[i].wart)/Const.s_Imp[i].licz;


    pom=  ((val*Const.s_Imp[i].wart*Const.s_Imp[i].licz)/Const.s_Imp[i].licz)  -  (dzies*Const.s_Imp[i].licz);

  ulam= (pom*poprzecinku)/Const.s_Imp[i].licz;

 if((Const.s_Imp[i].licz==1)&&(Const.s_Imp[i].wart==1)) sprintf(buf,"%10d      ",dzies);
 else
 {
        if(Const.s_Imp[i].licz<=10)     sprintf(buf,"%10d.%01d    ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=100)    sprintf(buf,"%10d.%02d   ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=1000)   sprintf(buf,"%10d.%03d  ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=10000)  sprintf(buf,"%10d.%04d ",dzies,ulam);
   else if(Const.s_Imp[i].licz<=100000) sprintf(buf,"%10d.%05d",dzies,ulam);
 }

}

void OdczytFlashLicznikowStart(int nr_val)
{
	     if(nr_val==1) adr_flash_Etr_read[1]=_Etr_godz_Start;
	else if(nr_val==2) adr_flash_Etr_read[2]=_Etr_dob_Start;
	else if(nr_val==3) adr_flash_Etr_read[3]=_Etr_tyg_Start;
	else if(nr_val==4) adr_flash_Etr_read[4]=_Etr_mies_Start;
}

int SprawdzWszystkieOdczytyLicz(int nr_val)
{
	     if(adr_flash_Etr_read[nr_val]>=adr_flash_Etr[nr_val]) return 1;
	     else                                                  return 0;

}

int OdczytFlashLicznikow(int nr_val)
{
	int i,j,x,itx; char a, bbuf[30];   uint32_t aa;

    itx=0;
	j=nr_val;    if(adr_flash_Etr_read[j] >= adr_flash_Etr[j]) return 2;

	if((adr_flash_Etr_read[j]==_Etr_godz_Start)||(adr_flash_Etr_read[j]==_Etr_dob_Start)||(adr_flash_Etr_read[j]==_Etr_tyg_Start)||(adr_flash_Etr_read[j]==_Etr_mies_Start))
	{
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 for(i=0;i<_Size_imp;i++)
		 {   if(Const.s_GPIO[t_imp[i]-1].val==7){
			   sprintf(&buf_rx2[_Size_BufWifi+itx],"Licznik %d  -  %s\r\n",i+1,Const.s_GPIO[t_imp[i]-1].nazwa);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			 }
	     }
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n  Data      Czas    ");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		 for(i=0;i<_Size_imp;i++)
		 {   if(Const.s_GPIO[t_imp[i]-1].val==7){
			   sprintf(&buf_rx2[_Size_BufWifi+itx],"      Licznik %d      ",i+1);   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
			 }
	     }
		 sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n\r\n");   itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
	}

	powtorzOdczytFlashLicznikow:
	AT25SF_3_ReadArray(adr_flash_Etr_read[j], buf_p, 4);    adr_flash_Etr_read[j]+=4;

	time=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
	ZamianaCzasuZFlash(time);

	sprintf(&buf_rx2[_Size_BufWifi+itx],"%02d.%02d.%02d  %02d:%02d:%02d - ",rok,mies,dzien,godz,min,sec);
	itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	for(i=0;i<_Size_imp;i++)
	{
		if(Const.s_GPIO[t_imp[i]-1].val==7)
		{
			AT25SF_3_ReadArray(adr_flash_Etr_read[j], buf_p, 4);      adr_flash_Etr_read[j]+=4;
			aa=   0xff000000&(((uint32_t)buf_p[0])<<24) | 0x00ff0000&(((uint32_t)buf_p[1])<<16) | 0x0000ff00&(((uint32_t)buf_p[2])<<8) | 0x000000ff&((uint32_t)buf_p[3]);
			PrzeliczLicznikRej(i,aa,bbuf);
			sprintf(&buf_rx2[_Size_BufWifi+itx],"%s %s ",bbuf, Const.s_Imp[i].jedn );    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);
		}
	}
	sprintf(&buf_rx2[_Size_BufWifi+itx],"\r\n");    itx+=strlen(&buf_rx2[_Size_BufWifi+itx]);

	if(adr_flash_Etr_read[j] < adr_flash_Etr[j])
	{
		if(itx<3000) goto powtorzOdczytFlashLicznikow;
		else return 0;
	}
	else return 1;

}

void ZapisFlashLicznikStart()
{
	AT25SF_3_BlockErase(_Etr_iter_godz); HAL_Delay(50); AT25SF_3_BlockErase(_Etr_godz_Start);  HAL_Delay(50);
	AT25SF_3_BlockErase(_Etr_iter_dob);  HAL_Delay(50); AT25SF_3_BlockErase(_Etr_dob_Start);   HAL_Delay(50);
	AT25SF_3_BlockErase(_Etr_iter_tyg);  HAL_Delay(50); AT25SF_3_BlockErase(_Etr_tyg_Start);   HAL_Delay(50);
	AT25SF_3_BlockErase(_Etr_iter_mies); HAL_Delay(50); AT25SF_3_BlockErase(_Etr_mies_Start);  HAL_Delay(50);

	adr_flash_Etr[1]=_Etr_godz_Start;
	adr_flash_Etr[2]=_Etr_dob_Start;
	adr_flash_Etr[3]=_Etr_tyg_Start;
	adr_flash_Etr[4]=_Etr_mies_Start;

    adr_flash_Etr_read[1]=_Etr_godz_Start;
    adr_flash_Etr_read[2]=_Etr_dob_Start;
    adr_flash_Etr_read[3]=_Etr_tyg_Start;
    adr_flash_Etr_read[4]=_Etr_mies_Start;

    IteracjaFlash_3_ZapEtr(1);
    IteracjaFlash_3_ZapEtr(2);
    IteracjaFlash_3_ZapEtr(3);
    IteracjaFlash_3_ZapEtr(4);

	dbg3("\r\nKasuje Rejestr Licznikow !!!");
}

void IteracjaFlash_3_ZapEtr(uint32_t nr_val)
{
  uint8_t  buf[256], *p1;    uint32_t nrAddr;     p1=buf;
  int i,j;

       if(nr_val==1) nrAddr= _Etr_iter_godz;
  else if(nr_val==2) nrAddr= _Etr_iter_dob;
  else if(nr_val==3) nrAddr= _Etr_iter_tyg;
  else if(nr_val==4) nrAddr= _Etr_iter_mies;

  NRF_Start();
  powtorzIFZ3:
  for(j=0;j<16;j++)
  {
	 AT25SF_3_ReadArray(nrAddr + 256*j, p1, 256 );
	 for(i=0;i<64;i++)
	 {
	    if(*(p1+4*i)==0xFF)
	    {
		   *(p1+4*i+0)= 0x00;
		   *(p1+4*i+1)= adr_flash_Etr[nr_val]>>16;
		   *(p1+4*i+2)= adr_flash_Etr[nr_val]>>8;
		   *(p1+4*i+3)= adr_flash_Etr[nr_val]>>0;

		   AT25SF_3_PageProgram(nrAddr + 256*j+4*i, p1+4*i, 4 );  //dbg3("\r\nZapis adr_licznik");
		   goto ominIFZ3;
	    }
	 }
  }
  AT25SF_3_BlockErase(nrAddr); HAL_Delay(200);  dbg3("\r\nERASE 3 block 4K for iter. Licznik "); goto powtorzIFZ3;
  ominIFZ3:
  NRF_Stop();
}

void IteracjaFlash_3_OdczEtr(uint32_t nr_val)
{
  uint8_t  buf[256],a,b,c,d, *p1;   uint32_t nrAddr;     p1=buf;
  int i,j;  a=0; b=0; c=0; d=0;

        if(nr_val==1) nrAddr= _Etr_iter_godz;
   else if(nr_val==2) nrAddr= _Etr_iter_dob;
   else if(nr_val==3) nrAddr= _Etr_iter_tyg;
   else if(nr_val==4) nrAddr= _Etr_iter_mies;


  for(j=0;j<16;j++)
  {
	 AT25SF_3_ReadArray(nrAddr + 256*j, p1, 256 );
	 for(i=0;i<64;i++)
	 {
	    if(*(p1+4*i)==0x00)
	    {
		   a= *(p1+4*i+0);
		   b= *(p1+4*i+1);
		   c= *(p1+4*i+2);
		   d= *(p1+4*i+3);
	    }
	    else if(*(p1+4*i)==0xFF)
	    {
           goto ominIFZ2df;
	    }
	 }
  }

  ominIFZ2df:
  adr_flash_Etr[nr_val]=0;
  adr_flash_Etr[nr_val]= 0x00FF0000&(((uint32_t)b)<<16) | 0x0000FF00&(((uint32_t)c)<<8) | 0x000000FF&(((uint32_t)d)<<0);

}

