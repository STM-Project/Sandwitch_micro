/*
 * nrf24l01.c
 *
 *  Created on: 24 sty 2017
 *      Author: VA
 */

#include "stm32f1xx_hal.h"
#include "nrf24l01.h"

/* NRF24L01+ registers*/
#define NRF24L01_REG_CONFIG			0x00	//Configuration Register
#define NRF24L01_REG_EN_AA			0x01	//Enable ‘Auto Acknowledgment’ Function
#define NRF24L01_REG_EN_RXADDR		0x02	//Enabled RX Addresses
#define NRF24L01_REG_SETUP_AW		0x03	//Setup of Address Widths (common for all data pipes)
#define NRF24L01_REG_SETUP_RETR		0x04	//Setup of Automatic Retransmission
#define NRF24L01_REG_RF_CH			0x05	//RF Channel
#define NRF24L01_REG_RF_SETUP		0x06	//RF Setup Register
#define NRF24L01_REG_STATUS			0x07	//Status Register
#define NRF24L01_REG_OBSERVE_TX		0x08	//Transmit observe register
#define NRF24L01_REG_RPD			0x09
#define NRF24L01_REG_RX_ADDR_P0		0x0A	//Receive address data pipe 0. 5 Bytes maximum length.
#define NRF24L01_REG_RX_ADDR_P1		0x0B	//Receive address data pipe 1. 5 Bytes maximum length.
#define NRF24L01_REG_RX_ADDR_P2		0x0C	//Receive address data pipe 2. Only LSB
#define NRF24L01_REG_RX_ADDR_P3		0x0D	//Receive address data pipe 3. Only LSB
#define NRF24L01_REG_RX_ADDR_P4		0x0E	//Receive address data pipe 4. Only LSB
#define NRF24L01_REG_RX_ADDR_P5		0x0F	//Receive address data pipe 5. Only LSB
#define NRF24L01_REG_TX_ADDR		0x10	//Transmit address. Used for a PTX device only
#define NRF24L01_REG_RX_PW_P0		0x11
#define NRF24L01_REG_RX_PW_P1		0x12
#define NRF24L01_REG_RX_PW_P2		0x13
#define NRF24L01_REG_RX_PW_P3		0x14
#define NRF24L01_REG_RX_PW_P4		0x15
#define NRF24L01_REG_RX_PW_P5		0x16
#define NRF24L01_REG_FIFO_STATUS	0x17	//FIFO Status Register
#define NRF24L01_REG_DYNPD			0x1C	//Enable dynamic payload length
#define NRF24L01_REG_FEATURE		0x1D

/* Registers default values */
#define NRF24L01_REG_DEFAULT_VAL_CONFIG			0x08
#define NRF24L01_REG_DEFAULT_VAL_EN_AA			0x3F
#define NRF24L01_REG_DEFAULT_VAL_EN_RXADDR		0x03
#define NRF24L01_REG_DEFAULT_VAL_SETUP_AW		0x03
#define NRF24L01_REG_DEFAULT_VAL_SETUP_RETR		0x03
#define NRF24L01_REG_DEFAULT_VAL_RF_CH			0x02
#define NRF24L01_REG_DEFAULT_VAL_RF_SETUP		0x0E
#define NRF24L01_REG_DEFAULT_VAL_STATUS			0x0E
#define NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX		0x00
#define NRF24L01_REG_DEFAULT_VAL_RPD			0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4	0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4	0xC2
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2		0xC3
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3		0xC4
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4		0xC5
#define NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5		0xC6
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3		0xE7
#define NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4		0xE7
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P0		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P1		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P2		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P3		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P4		0x00
#define NRF24L01_REG_DEFAULT_VAL_RX_PW_P5		0x00
#define NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS	0x11
#define NRF24L01_REG_DEFAULT_VAL_DYNPD			0x00
#define NRF24L01_REG_DEFAULT_VAL_FEATURE		0x00

/* Configuration register*/
#define NRF24L01_MASK_RX_DR		6
#define NRF24L01_MASK_TX_DS		5
#define NRF24L01_MASK_MAX_RT	4
#define NRF24L01_EN_CRC			3
#define NRF24L01_CRCO			2
#define NRF24L01_PWR_UP			1
#define NRF24L01_PRIM_RX		0

/* Enable auto acknowledgment*/
#define NRF24L01_ENAA_P5		5
#define NRF24L01_ENAA_P4		4
#define NRF24L01_ENAA_P3		3
#define NRF24L01_ENAA_P2		2
#define NRF24L01_ENAA_P1		1
#define NRF24L01_ENAA_P0		0

/* Enable rx addresses */
#define NRF24L01_ERX_P5			5
#define NRF24L01_ERX_P4			4
#define NRF24L01_ERX_P3			3
#define NRF24L01_ERX_P2			2
#define NRF24L01_ERX_P1			1
#define NRF24L01_ERX_P0			0

/* Setup of address width */
#define NRF24L01_AW				0 //2 bits

/* Setup of auto re-transmission*/
#define NRF24L01_ARD			4 //4 bits
#define NRF24L01_ARC			0 //4 bits

/* RF setup register*/
#define NRF24L01_PLL_LOCK		4
#define NRF24L01_RF_DR_LOW		5
#define NRF24L01_RF_DR_HIGH		3
#define NRF24L01_RF_DR			3
#define NRF24L01_RF_PWR			1 //2 bits

/* General status register */
#define NRF24L01_RX_DR			6
#define NRF24L01_TX_DS			5
#define NRF24L01_MAX_RT			4
#define NRF24L01_RX_P_NO		1 //3 bits
#define NRF24L01_TX_FULL		0

/* Transmit observe register */
#define NRF24L01_PLOS_CNT		4 //4 bits
#define NRF24L01_ARC_CNT		0 //4 bits

/* FIFO status*/
#define NRF24L01_TX_REUSE		6
#define NRF24L01_FIFO_FULL		5
#define NRF24L01_TX_EMPTY		4
#define NRF24L01_RX_FULL		1
#define NRF24L01_RX_EMPTY		0

//Dynamic length
#define NRF24L01_DPL_P0			0
#define NRF24L01_DPL_P1			1
#define NRF24L01_DPL_P2			2
#define NRF24L01_DPL_P3			3
#define NRF24L01_DPL_P4			4
#define NRF24L01_DPL_P5			5

/* Transmitter power*/
#define NRF24L01_M18DBM			0 //-18 dBm
#define NRF24L01_M12DBM			1 //-12 dBm
#define NRF24L01_M6DBM			2 //-6 dBm
#define NRF24L01_0DBM			3 //0 dBm

/* Data rates */
#define NRF24L01_2MBPS			0
#define NRF24L01_1MBPS			1
#define NRF24L01_250KBPS		2

/* Configuration */
#define NRF24L01_CONFIG			((1 << NRF24L01_EN_CRC) | (0 << NRF24L01_CRCO))

/* Instruction Mnemonics */
#define NRF24L01_REGISTER_MASK				0x1F

#define NRF24L01_READ_REGISTER_MASK(reg)	(0x00 | (NRF24L01_REGISTER_MASK & reg)) //Last 5 bits will indicate reg. address
#define NRF24L01_WRITE_REGISTER_MASK(reg)	(0x20 | (NRF24L01_REGISTER_MASK & reg)) //Last 5 bits will indicate reg. address
#define NRF24L01_R_RX_PAYLOAD_MASK			0x61
#define NRF24L01_W_TX_PAYLOAD_MASK			0xA0
#define NRF24L01_FLUSH_TX_MASK				0xE1
#define NRF24L01_FLUSH_RX_MASK				0xE2
#define NRF24L01_REUSE_TX_PL_MASK			0xE3
#define NRF24L01_ACTIVATE_MASK				0x50
#define NRF24L01_R_RX_PL_WID_MASK			0x60
#define NRF24L01_NOP_MASK					0xFF

#define AT25SF_NOP_MASK					0xFF

/* Flush FIFOs */
#define NRF24L01_FLUSH_TX					do { NRF24L01_CSN_LOW; TM_SPI_Send(NRF24L01_FLUSH_TX_MASK); NRF24L01_CSN_HIGH; } while (0)
#define NRF24L01_FLUSH_RX					do { NRF24L01_CSN_LOW; TM_SPI_Send(NRF24L01_FLUSH_RX_MASK); NRF24L01_CSN_HIGH; } while (0)

#define NRF24L01_TRANSMISSON_OK 			0
#define NRF24L01_MESSAGE_LOST   			1

#define NRF24L01_CHECK_BIT(reg, bit)       (reg & (1 << bit))



/* Clear interrupt flags */
#define NRF24L01_CLEAR_INTERRUPTS   do { TM_NRF24L01_WriteRegister(0x07, 0x70); } while (0)

/* Gets interrupt status from device */
#define NRF24L01_GET_INTERRUPTS     TM_NRF24L01_GetStatus()

/* Interrupt masks */
#define NRF24L01_IRQ_DATA_READY     0x40 /*!< Data ready for receive */
#define NRF24L01_IRQ_TRAN_OK        0x20 /*!< Transmission went OK */
#define NRF24L01_IRQ_MAX_RT         0x10 /*!< Max retransmissions reached, last transmission failed */





typedef struct {
	uint8_t PayloadSize;				//Payload size
	uint8_t Channel;					//Channel selected
	TM_NRF24L01_OutputPower_t OutPwr;	//Output power
	TM_NRF24L01_DataRate_t DataRate;	//Data rate
} TM_NRF24L01_t;

/* NRF structure */
TM_NRF24L01_t TM_NRF24L01_Struct;


#define TM_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))
#define TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)


/* Pins configuration */
#define NRF24L01_CE_LOW				TM_GPIO_SetPinLow(GPIOF, GPIO_PIN_0);   //CE
#define NRF24L01_CE_HIGH			TM_GPIO_SetPinHigh(GPIOF, GPIO_PIN_0);   //CE

#define NRF24L01_CSN_LOW			TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_3);   //CS
#define NRF24L01_CSN_HIGH			TM_GPIO_SetPinHigh(GPIOB, GPIO_PIN_3);   //CS




//PB0  - CS      TM_GPIO_SetPinLow(GPIOB, GPIO_PIN_0);     //CS
//PA11 - SDO
//PA8  - SDI     TM_GPIO_SetPinHigh(GPIOA, GPIO_PIN_8);    //SDI
//PB1  - SCK     TM_GPIO_SetPinHigh(GPIOB, GPIO_PIN_1);    //SCK
//PA4  - nIRQ


uint8_t TM_SPI_Send(uint8_t data)
{
	unsigned char ttt,i,j;
    ttt=0;

	        for(i=0;i<8;i++)
			{
		       if((data&0x80)==0x80)  TM_GPIO_SetPinHigh(GPIOF, GPIO_PIN_1);    //SDI
		       else TM_GPIO_SetPinLow(GPIOF, GPIO_PIN_1);    //SDI

			   data = data << 1;

			   TM_GPIO_SetPinHigh(GPIOC, GPIO_PIN_14);    //SCK

			   ttt = ttt << 1;

			   if(TM_GPIO_GetInputPinValue(GPIOC, GPIO_PIN_13)==GPIO_PIN_SET) ttt |= 0x01;   //SDO
			   else ttt &= ~0x01;

			   TM_GPIO_SetPinLow(GPIOC, GPIO_PIN_14);    //SCK

			}

	        return ttt;
}


uint8_t AT25SF_SPI_Send(uint8_t data)
{
	unsigned char ttt,i,j;
    ttt=0;

	        for(i=0;i<8;i++)
			{
			       if((data&0x80)==0x80)  TM_GPIO_SetPinHigh(GPIOD, GPIO_PIN_8);    //SDI
			       else TM_GPIO_SetPinLow(GPIOD, GPIO_PIN_8);    //SDI

				   data = data << 1;

				   TM_GPIO_SetPinHigh(GPIOD, GPIO_PIN_9);    //SCK
				   ttt = ttt << 1;

				   if(TM_GPIO_GetInputPinValue(GPIOB, GPIO_PIN_14)==GPIO_PIN_SET) ttt |= 0x01;   //SDO
				   else ttt &= ~0x01;

				   TM_GPIO_SetPinLow(GPIOD, GPIO_PIN_9);    //SCK

			}

	        return ttt;
}

void AT25SF_BlockErase(uint32_t adr )
{
	   AT25SF_CSN_LOW;
	   AT25SF_SPI_Send(0x06);   //Write Enable
	   AT25SF_CSN_HIGH
	   HAL_Delay(10);

	   AT25SF_CSN_LOW
	   AT25SF_SPI_Send(0x20);
	   AT25SF_SPI_Send(adr>>16);
	   AT25SF_SPI_Send(adr>>8);
	   AT25SF_SPI_Send(adr>>0);
	   AT25SF_CSN_HIGH

	   HAL_Delay(10);

	   AT25SF_CSN_LOW;
	   AT25SF_SPI_Send(0x04);   //Write Disable
	   AT25SF_CSN_HIGH
}

void AT25SF_ReadArray(uint32_t adr, uint8_t *data, uint32_t len )
{
   AT25SF_CSN_LOW
   AT25SF_SPI_Send(0x03);  //for lower frequency
	//TM_SPI_Send(0x0B);  //for uper frequency
   AT25SF_SPI_Send(adr>>16);
   AT25SF_SPI_Send(adr>>8);
   AT25SF_SPI_Send(adr>>0);
   AT25SF_SPI_ReadMulti(data, AT25SF_NOP_MASK, len);
   AT25SF_CSN_HIGH
}

void AT25SF_PageProgram(uint32_t adr, uint8_t *data, uint32_t len )
{
   AT25SF_CSN_LOW;
   AT25SF_SPI_Send(0x06);   //Write Enable
   AT25SF_CSN_HIGH
   HAL_Delay(10);

   AT25SF_CSN_LOW
   AT25SF_SPI_Send(0x02);
   AT25SF_SPI_Send(adr>>16);
   AT25SF_SPI_Send(adr>>8);
   AT25SF_SPI_Send(adr>>0);
   AT25SF_SPI_WriteMulti(data, len);
   AT25SF_CSN_HIGH

   HAL_Delay(10);

   AT25SF_CSN_LOW;
   AT25SF_SPI_Send(0x04);   //Write Disable
   AT25SF_CSN_HIGH
}








void AT25SF_2_BlockErase(uint32_t adr )
{
	   AT25SF_2_CSN_LOW;
	   AT25SF_SPI_Send(0x06);   //Write Enable
	   AT25SF_2_CSN_HIGH
	   HAL_Delay(10);

	   AT25SF_2_CSN_LOW
	   AT25SF_SPI_Send(0x20);
	   AT25SF_SPI_Send(adr>>16);
	   AT25SF_SPI_Send(adr>>8);
	   AT25SF_SPI_Send(adr>>0);
	   AT25SF_2_CSN_HIGH

	   HAL_Delay(10);

	   AT25SF_2_CSN_LOW;
	   AT25SF_SPI_Send(0x04);   //Write Disable
	   AT25SF_2_CSN_HIGH
}

void AT25SF_2_ReadArray(uint32_t adr, uint8_t *data, uint32_t len )
{
   AT25SF_2_CSN_LOW
   AT25SF_SPI_Send(0x03);  //for lower frequency
	//TM_SPI_Send(0x0B);  //for uper frequency
   AT25SF_SPI_Send(adr>>16);
   AT25SF_SPI_Send(adr>>8);
   AT25SF_SPI_Send(adr>>0);
   AT25SF_SPI_ReadMulti(data, AT25SF_NOP_MASK, len);
   AT25SF_2_CSN_HIGH
}

void AT25SF_2_PageProgram(uint32_t adr, uint8_t *data, uint32_t len )
{
   AT25SF_2_CSN_LOW;
   AT25SF_SPI_Send(0x06);   //Write Enable
   AT25SF_2_CSN_HIGH
   HAL_Delay(10);

   AT25SF_2_CSN_LOW
   AT25SF_SPI_Send(0x02);
   AT25SF_SPI_Send(adr>>16);
   AT25SF_SPI_Send(adr>>8);
   AT25SF_SPI_Send(adr>>0);
   AT25SF_SPI_WriteMulti(data, len);
   AT25SF_2_CSN_HIGH

   HAL_Delay(10);

   AT25SF_2_CSN_LOW;
   AT25SF_SPI_Send(0x04);   //Write Disable
   AT25SF_2_CSN_HIGH
}






void AT25SF_3_BlockErase(uint32_t adr )
{
	   AT25SF_3_CSN_LOW;
	   AT25SF_SPI_Send(0x06);   //Write Enable
	   AT25SF_3_CSN_HIGH
	   HAL_Delay(10);

	   AT25SF_3_CSN_LOW
	   AT25SF_SPI_Send(0x20);
	   AT25SF_SPI_Send(adr>>16);
	   AT25SF_SPI_Send(adr>>8);
	   AT25SF_SPI_Send(adr>>0);
	   AT25SF_3_CSN_HIGH

	   HAL_Delay(10);

	   AT25SF_3_CSN_LOW;
	   AT25SF_SPI_Send(0x04);   //Write Disable
	   AT25SF_3_CSN_HIGH
}

void AT25SF_3_ReadArray(uint32_t adr, uint8_t *data, uint32_t len )
{
   AT25SF_3_CSN_LOW
   AT25SF_SPI_Send(0x03);  //for lower frequency
	//TM_SPI_Send(0x0B);  //for uper frequency
   AT25SF_SPI_Send(adr>>16);
   AT25SF_SPI_Send(adr>>8);
   AT25SF_SPI_Send(adr>>0);
   AT25SF_SPI_ReadMulti(data, AT25SF_NOP_MASK, len);
   AT25SF_3_CSN_HIGH
}

void AT25SF_3_PageProgram(uint32_t adr, uint8_t *data, uint32_t len )
{
   AT25SF_3_CSN_LOW;
   AT25SF_SPI_Send(0x06);   //Write Enable
   AT25SF_3_CSN_HIGH
   HAL_Delay(1);

   AT25SF_3_CSN_LOW
   AT25SF_SPI_Send(0x02);
   AT25SF_SPI_Send(adr>>16);
   AT25SF_SPI_Send(adr>>8);
   AT25SF_SPI_Send(adr>>0);
   AT25SF_SPI_WriteMulti(data, len);
   AT25SF_3_CSN_HIGH

   HAL_Delay(1);

   AT25SF_3_CSN_LOW;
   AT25SF_SPI_Send(0x04);   //Write Disable
   AT25SF_3_CSN_HIGH
}








void AT25SF_SPI_WriteMulti(uint8_t* dataOut, uint32_t count) {
	uint32_t i;

	for (i = 0; i < count; i++) {
		AT25SF_SPI_Send(dataOut[i]);
	}
}
void AT25SF_SPI_ReadMulti(uint8_t* dataIn, uint8_t dummy, uint32_t count) {
	uint32_t i;

	for (i = 0; i < count; i++) {
		*(dataIn+i) = AT25SF_SPI_Send(dummy);
	}
}





void TM_SPI_WriteMulti(uint8_t* dataOut, uint32_t count) {
	uint32_t i;

	for (i = 0; i < count; i++) {
		TM_SPI_Send(dataOut[i]);
	}
}

void TM_SPI_ReadMulti(uint8_t* dataIn, uint8_t dummy, uint32_t count) {
	uint32_t i;

	for (i = 0; i < count; i++) {
		*(dataIn+i) = TM_SPI_Send(dummy);
	}
}

void TM_SPI_SendMulti(uint8_t* dataOut, uint8_t* dataIn, uint32_t count) {
	uint32_t i;

	for (i = 0; i < count; i++) {
		dataIn[i] = TM_SPI_Send(dataOut[i]);
	}
}



void TM_NRF24L01_InitPins(void) {
	/* Init pins */
	/* CNS pin */
	//TM_GPIO_Init(NRF24L01_CSN_PORT, NRF24L01_CSN_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);

	/* CE pin */
	//TM_GPIO_Init(NRF24L01_CE_PORT, NRF24L01_CE_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);

	/* CSN high = disable SPI */
	NRF24L01_CSN_HIGH;

	/* CE low = disable TX/RX */
	NRF24L01_CE_LOW;
}

uint8_t TM_NRF24L01_Init(uint8_t channel, uint8_t payload_size) {
	/* Initialize CE and CSN pins */
	TM_NRF24L01_InitPins();

	/* Initialize SPI */
	//TM_SPI_Init(NRF24L01_SPI, NRF24L01_SPI_PINS);

	/* Max payload is 32bytes */
	if (payload_size > 32) {
		payload_size = 32;
	}

	/* Fill structure */
	TM_NRF24L01_Struct.Channel = !channel; /* Set channel to some different value for TM_NRF24L01_SetChannel() function */
	TM_NRF24L01_Struct.PayloadSize = payload_size;
	TM_NRF24L01_Struct.OutPwr = TM_NRF24L01_OutputPower_0dBm;
	TM_NRF24L01_Struct.DataRate = TM_NRF24L01_DataRate_2M;

	/* Reset nRF24L01+ to power on registers values */
	TM_NRF24L01_SoftwareReset();

	/* Channel select */
	TM_NRF24L01_SetChannel(channel);

	/* Set pipeline to max possible 32 bytes */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, TM_NRF24L01_Struct.PayloadSize); // Auto-ACK pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, TM_NRF24L01_Struct.PayloadSize); // Data payload pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, TM_NRF24L01_Struct.PayloadSize);

	/* Set RF settings (2mbps, output power) */
	TM_NRF24L01_SetRF(TM_NRF24L01_Struct.DataRate, TM_NRF24L01_Struct.OutPwr);

	/* Config register */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG);

	/* Enable auto-acknowledgment for all pipes */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA, 0x3F);

	/* Enable RX addresses */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 0x3F);

	/* Auto retransmit delay: 1000 (4x250) us and Up to 15 retransmit trials */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 0x4F);

	/* Dynamic length configurations: No dynamic length */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, (0 << NRF24L01_DPL_P0) | (0 << NRF24L01_DPL_P1) | (0 << NRF24L01_DPL_P2) | (0 << NRF24L01_DPL_P3) | (0 << NRF24L01_DPL_P4) | (0 << NRF24L01_DPL_P5));

	/* Clear FIFOs */
	NRF24L01_FLUSH_TX;
	NRF24L01_FLUSH_RX;

	/* Clear interrupts */
	NRF24L01_CLEAR_INTERRUPTS;

	/* Go to RX mode */
	TM_NRF24L01_PowerUpRx();

	/* Return OK */
	return 1;
}

void TM_NRF24L01_SetMyAddress(uint8_t *adr) {
	NRF24L01_CE_LOW;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, adr, 5);
	NRF24L01_CE_HIGH;
}

void TM_NRF24L01_SetTxAddress(uint8_t *adr) {
	NRF24L01_CE_LOW;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, adr, 5);
	NRF24L01_CE_HIGH;
}

uint8_t TM_NRF24L01_ReadRegister(uint8_t reg) {
	uint8_t value;
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_READ_REGISTER_MASK(reg));
	value = TM_SPI_Send(NRF24L01_NOP_MASK);
	NRF24L01_CSN_HIGH;

	return value;
}

void TM_NRF24L01_WriteBit(uint8_t reg, uint8_t bit, uint8_t value) {
	uint8_t tmp;
	/* Read register */
	tmp = TM_NRF24L01_ReadRegister(reg);
	/* Make operation */
	if (value) {
		tmp |= 1 << bit;
	} else {
		tmp &= ~(1 << bit);
	}
	/* Write back */
	TM_NRF24L01_WriteRegister(reg, tmp);
}

uint8_t TM_NRF24L01_ReadBit(uint8_t reg, uint8_t bit) {
	uint8_t tmp;
	tmp = TM_NRF24L01_ReadRegister(reg);
	if (!NRF24L01_CHECK_BIT(tmp, bit)) {
		return 0;
	}
	return 1;
}

void TM_NRF24L01_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count) {
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_READ_REGISTER_MASK(reg));
	TM_SPI_ReadMulti(data, NRF24L01_NOP_MASK, count);
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_WriteRegister(uint8_t reg, uint8_t value) {
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_WRITE_REGISTER_MASK(reg));
	TM_SPI_Send(value);
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count) {
	NRF24L01_CSN_LOW;
	TM_SPI_Send(NRF24L01_WRITE_REGISTER_MASK(reg));
	TM_SPI_WriteMulti(data, count);
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_PowerUpTx(void) {
	NRF24L01_CLEAR_INTERRUPTS;
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | (0 << NRF24L01_PRIM_RX) | (1 << NRF24L01_PWR_UP));
}

void TM_NRF24L01_PowerUpRx(void) {
	/* Disable RX/TX mode */
	NRF24L01_CE_LOW;
	/* Clear RX buffer */
	NRF24L01_FLUSH_RX;
	/* Clear interrupts */
	NRF24L01_CLEAR_INTERRUPTS;
	/* Setup RX mode */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | 1 << NRF24L01_PWR_UP | 1 << NRF24L01_PRIM_RX);
	/* Start listening */
	NRF24L01_CE_HIGH;
}
/*
void TM_NRF24L01_PowerDown(void) {
	NRF24L01_CE_LOW;
	TM_NRF24L01_WriteBit(NRF24L01_REG_CONFIG, NRF24L01_PWR_UP, Bit_RESET);
}
*/
void TM_NRF24L01_Transmit(uint8_t *data) {
	uint8_t count = TM_NRF24L01_Struct.PayloadSize;

	/* Chip enable put to low, disable it */
	NRF24L01_CE_LOW;

	/* Go to power up tx mode */
	TM_NRF24L01_PowerUpTx();

	/* Clear TX FIFO from NRF24L01+ */
	NRF24L01_FLUSH_TX;

	/* Send payload to nRF24L01+ */
	NRF24L01_CSN_LOW;
	/* Send write payload command */
	TM_SPI_Send(NRF24L01_W_TX_PAYLOAD_MASK);
	/* Fill payload with data*/
	TM_SPI_WriteMulti(data, count);
	/* Disable SPI */
	NRF24L01_CSN_HIGH;

	/* Send data! */
	NRF24L01_CE_HIGH;
}


uint8_t TM_NRF24L01_RxFifoEmpty(void) {
	uint8_t reg = TM_NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS);
	return NRF24L01_CHECK_BIT(reg, NRF24L01_RX_EMPTY);
}

char kupa()
{   uint8_t status;
	NRF24L01_CSN_LOW;
		/* First received byte is always status register */
		status = TM_SPI_Send(0x07);
		/* Pull up chip select */
		NRF24L01_CSN_HIGH;

		return status;
}

uint8_t TM_NRF24L01_GetStatus(void) {
	uint8_t status;

	NRF24L01_CSN_LOW;
	/* First received byte is always status register */
	status = TM_SPI_Send(NRF24L01_NOP_MASK);
	/* Pull up chip select */
	NRF24L01_CSN_HIGH;

	return status;
}

void TM_NRF24L01_GetData(uint8_t* data) {
	/* Pull down chip select */
	NRF24L01_CSN_LOW;
	/* Send read payload command*/
	TM_SPI_Send(NRF24L01_R_RX_PAYLOAD_MASK);
	/* Read payload */
	TM_SPI_SendMulti(data, data, TM_NRF24L01_Struct.PayloadSize);
	/* Pull up chip select */
	NRF24L01_CSN_HIGH;

	/* Reset status register, clear RX_DR interrupt flag */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, (1 << NRF24L01_RX_DR));
}

uint8_t TM_NRF24L01_DataReady(void) {
	uint8_t status = TM_NRF24L01_GetStatus();

	if (NRF24L01_CHECK_BIT(status, NRF24L01_RX_DR)) {
		return 1;
	}
	return !TM_NRF24L01_RxFifoEmpty();
}



void TM_NRF24L01_SoftwareReset(void) {
	uint8_t data[5];

	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, 		NRF24L01_REG_DEFAULT_VAL_CONFIG);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA,		NRF24L01_REG_DEFAULT_VAL_EN_AA);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 	NRF24L01_REG_DEFAULT_VAL_EN_RXADDR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_AW, 	NRF24L01_REG_DEFAULT_VAL_SETUP_AW);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 	NRF24L01_REG_DEFAULT_VAL_SETUP_RETR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, 		NRF24L01_REG_DEFAULT_VAL_RF_CH);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, 	NRF24L01_REG_DEFAULT_VAL_RF_SETUP);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, 		NRF24L01_REG_DEFAULT_VAL_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_OBSERVE_TX, 	NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RPD, 		NRF24L01_REG_DEFAULT_VAL_RPD);

	//P0
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, data, 5);

	//P1
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, data, 5);

	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5);

	//TX
	data[0] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, data, 5);

	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P0);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P1);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P5);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FIFO_STATUS, NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, 		NRF24L01_REG_DEFAULT_VAL_DYNPD);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FEATURE, 	NRF24L01_REG_DEFAULT_VAL_FEATURE);
}




