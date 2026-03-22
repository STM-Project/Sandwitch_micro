/*
 * flash_in.c
 *
 *  Created on: 10.03.2018
 *      Author: Elektronika RM
 */


#include "stm32f1xx_hal.h"
#include "flash_in.h"
#include "modem_uart.h"
#include "irda.h"


uint32_t Address = 0, PAGEError = 0;


void Flash_Pages_Protected_Enable(uint32_t pages)    //Flash_Pages_Protected_Enable( OB_WRP_PAGES0TO1 | OB_WRP_PAGES2TO3 );
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Unlock the Options Bytes *************************************************/
	  HAL_FLASH_OB_Unlock();

	  /* Get pages write protection status ****************************************/
	  HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);



	  /* Check if desired pages are not yet write protected ***********************/
	  if(((~OptionsBytesStruct.WRPPage) & pages )!= pages)
	  {
		  dbg3("\r\nFlash for BOOT  NOT Write Protected !!!  -> PROTECTED ENABLE");

		  /* Enable the pages write protection **************************************/
	    OptionsBytesStruct.OptionType = OPTIONBYTE_WRP;
	    OptionsBytesStruct.WRPState   = OB_WRPSTATE_ENABLE;
	    OptionsBytesStruct.WRPPage    = pages;
	    if(HAL_FLASHEx_OBProgram(&OptionsBytesStruct) != HAL_OK)
	    {
	      /* Error occurred while options bytes programming. **********************/
	      while (1)
	      {
	        dbg3("\r\nERROR");
	      }
	    }

	    /* Generate System Reset to load the new option byte values ***************/
	    HAL_FLASH_OB_Launch();
	  }
	  else dbg3("\r\nFlash for BOOT Write Protected ");

	  /* Lock the Options Bytes *************************************************/
	  HAL_FLASH_OB_Lock();
	  HAL_FLASH_Lock();


}

void Flash_Pages_Protected_Disenable(uint32_t pages)    //Flash_Pages_Protected_Disenable( OB_WRP_PAGES0TO1 | OB_WRP_PAGES2TO3 );
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Unlock the Options Bytes *************************************************/
	  HAL_FLASH_OB_Unlock();

	  /* Get pages write protection status ****************************************/
	  HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);



	  /* Check if desired pages are already write protected ***********************/
	  if((OptionsBytesStruct.WRPPage & pages) != pages)
	  {
		  dbg3("\r\nPages Protected   PROTECTED DISENABLE !!!");

	    /* Restore write protected pages */
	    OptionsBytesStruct.OptionType   = OPTIONBYTE_WRP;
	    OptionsBytesStruct.WRPState     = OB_WRPSTATE_DISABLE;
	    OptionsBytesStruct.WRPPage = pages;
	    if(HAL_FLASHEx_OBProgram(&OptionsBytesStruct) != HAL_OK)
	    {
	      /* Error occurred while options bytes programming. **********************/
	      while (1)
	      {
	    	  dbg3("\r\nERROR");
	      }
	    }

	    /* Generate System Reset to load the new option byte values ***************/
	    HAL_FLASH_OB_Launch();
	  }
	  else dbg3("\r\nPages NOT Write Protected !!!");

	  /* Lock the Options Bytes *************************************************/
	  HAL_FLASH_OB_Lock();
	  HAL_FLASH_Lock();

	  dbg3("\r\nOK Flash_Pages_Protected_Disenable  ");
}


void Flash_in_progr_char_na_32_TEST(uint8_t *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = ADDR_FLASH_PAGE_0;
	  EraseInitStruct.NbPages     = (ADDR_FLASH_PAGE_17 - ADDR_FLASH_PAGE_0) / FLASH_PAGE_SIZE;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {


		  /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */

	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
		     dbg3("\r\nERROR WRITE TO FLASH !!!!!");
	    	 HAL_Delay(600);  return;

	  }
	  else dbg3("\r\nErase OK");

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = ADDR_FLASH_PAGE_0;  int i;

	        for(i=0;i<size;i++)
	        {
	        	long_p= 0xff000000&(((unsigned long)(*(buf+i+0)))<<24) | 0x00ff0000&(((unsigned long)(*(buf+i+1)))<<16) \
	        		  | 0x0000ff00&(((unsigned long)(*(buf+i+2)))<<8) | 0x000000ff&(((unsigned long)(*(buf+i+3)))<<0);
	        	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, long_p) == HAL_OK);
	            Address = Address + 4;
	            i=i+3;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}


void Flash_in_progr_char_na_32_TEST2(uint8_t *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = ADDR_FLASH_PAGE_16;
	  EraseInitStruct.NbPages     = (ADDR_FLASH_PAGE_17 - ADDR_FLASH_PAGE_16) / FLASH_PAGE_SIZE;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {


		  /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */

	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
		     dbg3("\r\nERROR WRITE TO FLASH !!!!!");
	    	 HAL_Delay(600);   return;

	  }
	  else dbg3("\r\nErase OK");

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = ADDR_FLASH_PAGE_0;  int i;

	        for(i=0;i<size;i++)
	        {
	        	long_p= 0xff000000&(((unsigned long)(*(buf+i+0)))<<24) | 0x00ff0000&(((unsigned long)(*(buf+i+1)))<<16) \
	        		  | 0x0000ff00&(((unsigned long)(*(buf+i+2)))<<8) | 0x000000ff&(((unsigned long)(*(buf+i+3)))<<0);
	        	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, long_p) == HAL_OK);
	            Address = Address + 4;
	            i=i+3;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}







void Flash_in_progr_xx(unsigned long adr, char *buf)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = adr; // FLASH_USER_START_ADDR;
	  EraseInitStruct.NbPages     = 1;  //(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {
	    /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */
	    while (1)
	    {
	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
	    	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
	    	 HAL_Delay(600);
	    }
	  }

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = adr;

	        while(*buf>0)
	        {
	           if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *buf++) == HAL_OK);
	           Address = Address + 4;
	        }
	        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *buf++) == HAL_OK);


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}


void Flash_in_progr_hex(unsigned long adr, char *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = adr; // FLASH_USER_START_ADDR;
	  EraseInitStruct.NbPages     = 1;  //(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {
	    /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */
	    while (1)
	    {
	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
	    	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
	    	 HAL_Delay(600);
	    }
	  }

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = adr;  int i;

	        for(i=0;i<size;i++)
	        {
	           if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *buf++) == HAL_OK);
	           Address = Address + 4;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}


void Flash_in_progr_16(unsigned long adr, uint16_t *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = adr; // FLASH_USER_START_ADDR;
	  EraseInitStruct.NbPages     = 1;  //(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {
	    /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */
	    while (1)
	    {
	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
	    	// HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
	    	 HAL_Delay(600);
	    }
	  }

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = adr;  int i;

	        for(i=0;i<size;i++)
	        {
	           if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address, *buf++) == HAL_OK);
	           Address = Address + 2;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}

void Flash_Upload(unsigned long adr, uint8_t *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = adr; // FLASH_USER_START_ADDR;
	  EraseInitStruct.NbPages     = 1;  //(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {
	    /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */

	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
	    	 dbg3("\r\nERROR WRITE TO FLASH !!!!!");
	    	 HAL_Delay(600);  return;

	  }

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = adr;  int i;

	        for(i=0;i<size;i++)
	        {
	        	long_p= 0xff000000&(((unsigned long)(*(buf+i+0)))<<24) | 0x00ff0000&(((unsigned long)(*(buf+i+1)))<<16) \
	        		  | 0x0000ff00&(((unsigned long)(*(buf+i+2)))<<8) | 0x000000ff&(((unsigned long)(*(buf+i+3)))<<0);
	        	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, long_p) == HAL_OK);
	            Address = Address + 4;
	            i=i+3;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}


void Flash_in_progr_char_na_32(unsigned long adr, uint8_t *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = adr; // FLASH_USER_START_ADDR;
	  EraseInitStruct.NbPages     = (((ADDR_FLASH_PAGE_238+_Size_Zmienne_zapisywalne) - ADDR_FLASH_PAGE_238) / FLASH_PAGE_SIZE) + 1;
                                    //((ADDR_FLASH_PAGE_255+FLASH_PAGE_SIZE) - ADDR_FLASH_PAGE_238) / FLASH_PAGE_SIZE;
	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {
	    /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */

	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
	    	 dbg3("\r\nERROR WRITE TO FLASH !!!!!");
	    	 HAL_Delay(600);  return;

	  }

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = adr;  int i;

	        for(i=0;i<size;i++)
	        {
	        	long_p= 0xff000000&(((unsigned long)(*(buf+i+0)))<<24) | 0x00ff0000&(((unsigned long)(*(buf+i+1)))<<16) \
	        		  | 0x0000ff00&(((unsigned long)(*(buf+i+2)))<<8) | 0x000000ff&(((unsigned long)(*(buf+i+3)))<<0);
	        	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, long_p) == HAL_OK);
	            Address = Address + 4;
	            i=i+3;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}

void Flash2_in_progr_char_na_32(unsigned long adr, uint8_t *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = adr; // FLASH_USER_START_ADDR;
	  EraseInitStruct.NbPages     = 1;
                                    //((ADDR_FLASH_PAGE_255+FLASH_PAGE_SIZE) - ADDR_FLASH_PAGE_238) / FLASH_PAGE_SIZE;
	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {
	    /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */

	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
	    	 dbg3("\r\nERROR WRITE TO FLASH !!!!!");
	    	 HAL_Delay(600);  return;

	  }

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = adr;  int i;

	        for(i=0;i<size;i++)
	        {
	        	long_p= 0xff000000&(((unsigned long)(*(buf+i+0)))<<24) | 0x00ff0000&(((unsigned long)(*(buf+i+1)))<<16) \
	        		  | 0x0000ff00&(((unsigned long)(*(buf+i+2)))<<8) | 0x000000ff&(((unsigned long)(*(buf+i+3)))<<0);
	        	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, long_p) == HAL_OK);
	            Address = Address + 4;
	            i=i+3;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}


void Flash_mapa_read(uint32_t adr, uint8_t *ptt, int size)
{
   unsigned long Address_;  int i,j;  char *pt;


     pt=ptt;
	 Address_ = adr;
   	  	j=0;
   	  	 for(i=0;i<_Size_Zmienne_zapisywalne;i++)
   	  	 {
   	  		long_p =  *(__IO uint32_t*)Address_;
   	  	    *(pt+j)=   0x000000ff&(long_p>>24);   j++;
   	  	    *(pt+j)=   0x000000ff&(long_p>>16);   j++;
   	        *(pt+j)=   0x000000ff&(long_p>>8);    j++;
   	        *(pt+j)=   0x000000ff&(long_p>>0);    j++;
   	  	   Address_=Address_+4;
   	  	   i=i+3;
   	  	 }
}

void Flash_in_xxx_char_na_32(unsigned long adr, uint8_t *buf, int size)
{

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  /* Erase the user Flash area
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.PageAddress = adr; // FLASH_USER_START_ADDR;
	  EraseInitStruct.NbPages     = 12;  //(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	  {
	    /*
	      Error occurred while page erase.
	      User can add here some code to deal with this error.
	      PAGEError will contain the faulty page and then to know the code error on this page,
	      user can call function 'HAL_FLASH_GetError()'
	    */
	    /* Infinite loop */
	    while (1)
	    {
	      /* Make LED3 blink (100ms on, 2s off) to indicate error in Erase operation */
	    	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
	    	 HAL_Delay(600);
	    }
	  }

	  /* Program the user Flash area word by word
	    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	  Address = adr;  int i;

	        for(i=0;i<size;i++)
	        {
	        	long_p= 0xff000000&(((unsigned long)(*(buf+i+3)))<<24) | 0x00ff0000&(((unsigned long)(*(buf+i+2)))<<16) \
	        		  | 0x0000ff00&(((unsigned long)(*(buf+i+1)))<<8) | 0x000000ff&(((unsigned long)(*(buf+i+0)))<<0);
	        	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, long_p) == HAL_OK);
	            Address = Address + 4;
	            i=i+3;
	        }


	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();


}

void Flash_xxx_read(uint32_t adr, uint8_t *ptt, int size)
{
   unsigned long Address_;  int i,j;  char *pt;


     pt=ptt;
	 Address_ = adr;
   	  	j=0;
   	  	 for(i=0;i<size;i++)
   	  	 {
   	  		long_p =  *(__IO uint32_t*)Address_;
   	  	    *(pt+j)=   0x000000ff&(long_p>>24);   j++;
   	  	    *(pt+j)=   0x000000ff&(long_p>>16);   j++;
   	        *(pt+j)=   0x000000ff&(long_p>>8);    j++;
   	        *(pt+j)=   0x000000ff&(long_p>>0);    j++;
   	  	   Address_=Address_+4;
   	  	   i=i+3;
   	  	 }
}

void Flash_upload_read(uint32_t adr, uint8_t *ptt, int size)
{
   unsigned long Address_;  int i,j;  char *pt;


     pt=ptt;
	 Address_ = adr;
   	  	j=0;
   	  	 for(i=0;i<size;i++)
   	  	 {
   	  		long_p =  *(__IO uint32_t*)Address_;
   	  	    *(pt+j)=   0x000000ff&(long_p>>0);   j++;
   	  	    *(pt+j)=   0x000000ff&(long_p>>8);   j++;
   	        *(pt+j)=   0x000000ff&(long_p>>16);    j++;
   	        *(pt+j)=   0x000000ff&(long_p>>24);    j++;
   	  	   Address_=Address_+4;
   	  	   i=i+3;
   	  	 }
}

void Flash_mapa_write(void)
{
	////Flash_in_progr_hex(FLASH_USER_START_ADDR, &Const.s_PK[0].value1 ,Const.start);

	//Flash_in_progr_hex(FLASH_USER_START_ADDR, "X" ,Const.start);
}

int Flash_read_to_BufIrda(int nr)
{
   unsigned long Address_;  int i,j;

	Address_ = _AdrFlashToIR + nr*FLASH_PAGE_SIZE;

	buf_irda[0] =  *(__IO uint32_t*)Address_;
	Address_=Address_+2;

	HAL_Delay(1);
	j = buf_irda[0];   if(j>990) return 0;


	 for(i=0;i<j;i++)
	 {
	   buf_irda[i+1] =  *(__IO uint32_t*)Address_;
	   Address_=Address_+2;
	 }

	 return 1;
}

int Flash_read_to_BufIrda_2(void)
{
   unsigned long Address_;  int i,j;

	Address_ = ADDR_FLASH_PAGE_30;

	////buf_irda[0] =  *(__IO uint32_t*)Address_;
	Address_=Address_+2;

	////HAL_Delay(1);
	////j = buf_irda[0];  if(j>990) return 0;


	 for(i=0;i<j;i++)
	 {
	   ////buf_irda[i+1] =  *(__IO uint32_t*)Address_;
	   Address_=Address_+2;
	 }

	 return 1;
}


void FlashWsadPrg(int cel, int len)
{
    int i,j,p,p1,p2;  int addr;
    i=cel;
    j= &Const.s_Lan[0].eth[0];

    p1=(i-j)/2048;
    p2=(i+len-j)/2048;

    if(p1==p2)
    {
       p=(i-j)/2048;
       addr= _AdrFlashToRAM + p*2048;
       dbg3("\r\nZapis part...");
       Flash2_in_progr_char_na_32(addr, Const.s_Lan[0].eth + 2048*p ,2048);
       dbg3("ok ");
    }
    else
    {
    	dbg3("\r\nZapis part 2x...");
    	addr= _AdrFlashToRAM + p1*2048;
        Flash2_in_progr_char_na_32(addr, Const.s_Lan[0].eth + 2048*p1 ,2048);
        HAL_Delay(200);
        addr= _AdrFlashToRAM + p2*2048;
        Flash2_in_progr_char_na_32(addr, Const.s_Lan[0].eth + 2048*p2 ,2048);
        dbg3("ok ");
    }
}







