/*
 * enc28j60.c
 *
 *  Created on: 29 lip 2017
 *      Author: Elektronika RM
 */

#include "stm32f1xx_hal.h"
#include "enc28j60.h"
#include "eth.h"

#define TM_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))
#define TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)






uint8_t ENC_SPI_Send(uint8_t data)
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

void enc28j60WriteOp(unsigned char op, unsigned char address, unsigned char data)
{
	ENC28j60_CE_LOW;
	 ENC_SPI_Send(op | (address & ADDR_MASK));
	 ENC_SPI_Send(data);
	ENC28j60_CE_HIGH;
}

void enc28j60Write(unsigned char address, unsigned char data)
{
	// set the bank
	enc28j60SetBank(address);
	// do the write
	enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}




unsigned char enc28j60ReadOp(unsigned char op, unsigned char address)
{
	unsigned char data;

	ENC28j60_CE_LOW;
	 ENC_SPI_Send(op | (address & ADDR_MASK));
	 data = ENC_SPI_Send(0x00);

		if(address & 0x80)
		{
			data = ENC_SPI_Send(0x00);
		}

	ENC28j60_CE_HIGH;

	return data;


}

void enc28j60SetBank(unsigned char address)
{
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank)
	{
		// set the bank
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
	}
}

unsigned char enc28j60Read(unsigned char address)
{
	// set the bank
	enc28j60SetBank(address);
	// do the read
	return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}

void enc28j60PhyWrite(unsigned char address, unsigned short data)
{
	// set the PHY register address
	enc28j60Write(MIREGADR, address);

	// write the PHY data
	enc28j60Write(MIWRL, data);
	enc28j60Write(MIWRH, data>>8);
 dbg3("1");
	// wait until the PHY write completes
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);   dbg3("2");
}

void nicInit()
{
	enc28j60Init();
}

unsigned short enc28j60PhyRead(unsigned char address)
{
	unsigned short data;

	// Set the right address and start the register read operation
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MICMD, MICMD_MIIRD);

	// wait until the PHY read completes
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);

	// quit reading
	enc28j60Write(MICMD, 0x00);

	// get data value
	data  = enc28j60Read(MIRDL);
	data |= enc28j60Read(MIRDH);
	// return the data
	return data;
}

void enc28j60Init(void)
{
	{

	// initialize I/O


	// perform system reset
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	HAL_Delay(100);

	NextPacketPtr = RXSTART_INIT;

   // Rx start
	enc28j60Write(ERXSTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXSTH, RXSTART_INIT>>8);
	// set receive pointer address
	enc28j60Write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXRDPTH, RXSTART_INIT>>8);
	// RX end
	enc28j60Write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60Write(ERXNDH, RXSTOP_INIT>>8);
	// TX start
	enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60Write(ETXSTH, TXSTART_INIT>>8);
	// TX end
	enc28j60Write(ETXNDL, TXSTOP_INIT&0xFF);
	enc28j60Write(ETXNDH, TXSTOP_INIT>>8);

	enc28j60Write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	enc28j60Write(EPMM0, 0x3f);
	enc28j60Write(EPMM1, 0x30);
	enc28j60Write(EPMCSL, 0xf9);
	enc28j60Write(EPMCSH, 0xf7);

	// enable MAC receive
	enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// bring MAC out of reset
	enc28j60Write(MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	// set inter-frame gap (non-back-to-back)
	enc28j60Write(MAIPGL, 0x12);
	enc28j60Write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	enc28j60Write(MABBIPG, 0x12);
	// Set the maximum packet size which the controller will accept
        // Do not send packets longer than MAX_FRAMELEN:
	enc28j60Write(MAMXFLL, MAX_FRAMELEN&0xFF);
	enc28j60Write(MAMXFLH, MAX_FRAMELEN>>8);
	// do bank 3 stuff
        // write MAC address
        // NOTE: MAC address in ENC28J60 is byte-backward
        enc28j60Write(MAADR5, ETHADDRESS[0]);
        enc28j60Write(MAADR4, ETHADDRESS[1]);
        enc28j60Write(MAADR3, ETHADDRESS[2]);
        enc28j60Write(MAADR2, ETHADDRESS[3]);
        enc28j60Write(MAADR1, ETHADDRESS[4]);
        enc28j60Write(MAADR0, ETHADDRESS[5]);

	// no loopback of transmitted frames
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	enc28j60SetBank(ECON1);
	// enable interrutps
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
	// enable packet reception
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	}
}

void dbg3u08(unsigned char cc)
{
	char buf_p[10],c1,c2;
    c1=hex_to_ASCII((cc>>4)&0x0F);
	c2=hex_to_ASCII(cc&0x0F);
	sprintf(buf_p,"%c%c",c1,c2);
	dbg3(buf_p);
}

void dbg3u16(unsigned short cc)
{
	char buf_p[10],c1,c2,c3,c4;
    c1=hex_to_ASCII((cc>>12)&0x000F);
    c2=hex_to_ASCII((cc>>8)&0x000F);
    c3=hex_to_ASCII((cc>>4)&0x000F);
	c4=hex_to_ASCII(cc&0x000F);
	sprintf(buf_p,"%c%c%c%c",c1,c2,c3,c4);
	dbg3(buf_p);
}

void enc28j60RegDump(void)
{
   char buf_p[50];
//	unsigned char macaddr[6];
//	result = ax88796Read(TR);

//	dbg3("Media State: ");
//	if(!(result & AUTOD))
//		dbg3("Autonegotiation\r\n");
//	else if(result & RST_B)
//		dbg3("PHY in Reset   \r\n");
//	else if(!(result & RST_10B))
//		dbg3("10BASE-T       \r\n");
//	else if(!(result & RST_TXB))
//		dbg3("100BASE-T      \r\n");

	sprintf(buf_p,"RevID: 0x%x\r\n", enc28j60Read(EREVID));
	dbg3(buf_p);

	dbg3("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\r\n");
	dbg3("         ");
	dbg3u08(enc28j60Read(ECON1));
	dbg3("    ");
	dbg3u08(enc28j60Read(ECON2));
	dbg3("    ");
	dbg3u08(enc28j60Read(ESTAT));
	dbg3("    ");
	dbg3u08(enc28j60Read(EIR));
	dbg3("   ");
	dbg3u08(enc28j60Read(EIE));
	dbg3("\r\n");

	dbg3("MAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\r\n");
	dbg3("        0x");
	dbg3u08(enc28j60Read(MACON1));
	dbg3("    0x");
	dbg3u08(enc28j60Read(MACON2));
	dbg3("    0x");
	dbg3u08(enc28j60Read(MACON3));
	dbg3("    0x");
	dbg3u08(enc28j60Read(MACON4));
	dbg3("   ");
	dbg3u08(enc28j60Read(MAADR5));
	dbg3u08(enc28j60Read(MAADR4));
	dbg3u08(enc28j60Read(MAADR3));
	dbg3u08(enc28j60Read(MAADR2));
	dbg3u08(enc28j60Read(MAADR1));
	dbg3u08(enc28j60Read(MAADR0));
	dbg3("\r\n");

	dbg3("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\r\n");
	dbg3("       0x");
	dbg3u08(enc28j60Read(ERXSTH));
	dbg3u08(enc28j60Read(ERXSTL));
	dbg3(" 0x");
	dbg3u08(enc28j60Read(ERXNDH));
	dbg3u08(enc28j60Read(ERXNDL));
	dbg3("  0x");
	dbg3u08(enc28j60Read(ERXWRPTH));
	dbg3u08(enc28j60Read(ERXWRPTL));
	dbg3("  0x");
	dbg3u08(enc28j60Read(ERXRDPTH));
	dbg3u08(enc28j60Read(ERXRDPTL));
	dbg3("   0x");
	dbg3u08(enc28j60Read(ERXFCON));
	dbg3("    0x");
	dbg3u08(enc28j60Read(EPKTCNT));
	dbg3("  0x");
	dbg3u08(enc28j60Read(MAMXFLH));
	dbg3u08(enc28j60Read(MAMXFLL));
	dbg3("\r\n");

	dbg3("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\r\n");
	dbg3("       0x");
	dbg3u08(enc28j60Read(ETXSTH));
	dbg3u08(enc28j60Read(ETXSTL));
	dbg3(" 0x");
	dbg3u08(enc28j60Read(ETXNDH));
	dbg3u08(enc28j60Read(ETXNDL));
	dbg3("   0x");
	dbg3u08(enc28j60Read(MACLCON1));
	dbg3("     0x");
	dbg3u08(enc28j60Read(MACLCON2));
	dbg3("     0x");
	dbg3u08(enc28j60Read(MAPHSUP));
	dbg3("\r\n");

	HAL_Delay(25);
}

void enc28j60ReadBuffer(unsigned short len, unsigned char* data)
{


	ENC28j60_CE_LOW;
	 ENC_SPI_Send(ENC28J60_READ_BUF_MEM);

		while(len--)
		{
			// read data
			*data++ = ENC_SPI_Send(0x00);
		}

	ENC28j60_CE_HIGH;


}

// Gets a packet from the network receive buffer, if one is available.
// The packet will by headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
unsigned int enc28j60PacketReceive(unsigned int maxlen, unsigned char* packet)
	{
	unsigned int rxstat;
	unsigned int len;

	// check if a packet has been received and buffered
	//if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
        // The above does not work. See Rev. B4 Silicon Errata point 6.
	if( enc28j60Read(EPKTCNT) ==0 )
		{
		return(0);
        }

	// Set the read pointer to the start of the received packet
	enc28j60Write(ERDPTL, (NextPacketPtr));
	enc28j60Write(ERDPTH, (NextPacketPtr)>>8);

	// read the next packet pointer
	NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

	// read the packet length (see datasheet page 43)
	len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

    len-=4; //remove the CRC count
	// read the receive status (see datasheet page 43)
	rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	// limit retrieve length
    if (len>maxlen-1)
		{
        len=maxlen-1;
    	}

        // check CRC and symbol errors (see datasheet page 44, table 7-3):
        // The ERXFCON.CRCEN is set by default. Normally we should not
        // need to check this.
    if ((rxstat & 0x80)==0)
			{
		    // invalid
		    len=0;
		    }
		else
			{
            // copy the packet from the receive buffer
            enc28j60ReadBuffer(len, packet);
        	}
	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	enc28j60Write(ERXRDPTL, (NextPacketPtr));
	enc28j60Write(ERXRDPTH, (NextPacketPtr)>>8);

	// decrement the packet counter indicate we are done with this packet
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return(len);
	}




void enc28j60WriteBuffer(unsigned short len, unsigned char* data)
{
	ENC28j60_CE_LOW;
	    ENC_SPI_Send(ENC28J60_WRITE_BUF_MEM);
		while(len--)
		{
			// write data
			ENC_SPI_Send(*data++);
		}
	ENC28j60_CE_HIGH;
}

void enc28j60PacketSend(uint16_t len, unsigned char* packet)
	{
       // Check no transmit in progress
        while (enc28j60ReadOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS)
        {
                // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
                if( (enc28j60Read(EIR) & EIR_TXERIF) ) {
                        enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
                        enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);

                }
        }

	// Set the write pointer to start of transmit buffer area
	enc28j60Write(EWRPTL, TXSTART_INIT&0xFF);
	enc28j60Write(EWRPTH, TXSTART_INIT>>8);
	// Set the TXND pointer to correspond to the packet size given
	enc28j60Write(ETXNDL, (TXSTART_INIT+len)&0xFF);
	enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);
	// write per-packet control byte (0x00 means use macon3 settings)
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	// copy the packet into the transmit buffer
	enc28j60WriteBuffer(len, packet);
	// send the contents of the transmit buffer onto the network
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

	}

void nicSend(unsigned int len, unsigned char* packet)
{

	//debugPrintHexTable(len, packet);
	//HAL_Delay(1);
	//czekaj_us(100);
	   enc28j60PacketSend(len, packet);


	   //if(wsk_replay==1)
	  // {
        //  enc28j60PacketSend(len, packet);
	   //}
}

