/*
 * eth.c
 *
 *  Created on: 31 lip 2017
 *      Author: VA
 */

#include "stm32f1xx_hal.h"
#include "eth.h"
#include "http.h"
#include "enc28j60.h"
#include <modem_uart.h>
#include "flash_in.h"
#include "modbus.h"
#include "gpio.h"
#include "zdarzenia.h"
#include "agh.h"
#include "irda.h"

IWDG_HandleTypeDef hiwdg;

int http_itx;  char tt,ww,a;  int licz_do_usun;        int test_mapa;

WWDG_HandleTypeDef hwwdg;

unsigned short htons(unsigned short val)
{
	return (val<<8) | (val>>8);
}

unsigned long htonl(unsigned long val)
{
	return (htons(val>>16) | (unsigned long)htons(val&0x0000FFFF)<<16);
}

void arpInit(void)
{
	unsigned char i;
	// initialize all ArpTable elements to unused
	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		ArpTable[i].ipaddr = 0;
		ArpTable[i].time = 0;
	}
}
void netPrintIPAddr(unsigned long ipaddr)
{
	char buf_p[20];
	sprintf(buf_p,"%d.%d.%d.%d",
		((unsigned char*)&ipaddr)[3],
		((unsigned char*)&ipaddr)[2],
		((unsigned char*)&ipaddr)[1],
		((unsigned char*)&ipaddr)[0]);
	dbg3(buf_p);
}

void netPrintEthAddr(unsigned char *ee)
{
	dbg3u08(*ee);
	dbg3("-");
	dbg3u08(*(ee+1));
	dbg3("-");
	dbg3u08(*(ee+2));
	dbg3("-");
	dbg3u08(*(ee+3));
	dbg3("-");
	dbg3u08(*(ee+4));
	dbg3("-");
	dbg3u08(*(ee+5));
}
void arpPrintTable(void)
{
	unsigned char i;

	// print ARP table
	dbg3("Time    Eth Address    IP Address\r\n");
	dbg3("---------------------------------------\r\n");
	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		dbg3u08(ArpTable[i].time);
		dbg3("   ");
		netPrintEthAddr(&ArpTable[i].ethaddr);
		dbg3("  ");
		netPrintIPAddr(ArpTable[i].ipaddr);
		dbg3("\r\n");
	}
	dbg3("\r\n");
}

void arpSetAddress(struct netEthAddr* myeth, uint32_t myip)
{
	// set local address record
	ArpMyAddr.ethaddr = *myeth;
	ArpMyAddr.ipaddr = myip;
}
void nicGetMacAddress(unsigned char *macaddr)
{
	*macaddr++ = enc28j60Read(MAADR5);
	*macaddr++ = enc28j60Read(MAADR4);
	*macaddr++ = enc28j60Read(MAADR3);
	*macaddr++ = enc28j60Read(MAADR2);
	*macaddr++ = enc28j60Read(MAADR1);
	*macaddr++ = enc28j60Read(MAADR0);
}
void ipSetConfig(uint32_t myIp, uint32_t netmask, uint32_t gatewayIp)
{
	struct netEthAddr ethaddr;

	// set local addressing
	IpMyConfig.ip = myIp;
	IpMyConfig.netmask = netmask;
	IpMyConfig.gateway = gatewayIp;

	// set ARP association
	nicGetMacAddress(ethaddr.addr);
	arpSetAddress(&ethaddr, myIp);
}

struct ipConfig* ipGetConfig(void)
{
	return &IpMyConfig;
}

uint16_t netChecksum(void *data, uint16_t len)
{
    register uint32_t sum = 0;

    for (;;) {
        if (len < 2)
            break;
		//sum += *((uint16_t *)data)++;
		sum += *((uint16_t *)data);
		data+=2;
        len -= 2;
    }
    if (len)
        sum += *(uint8_t *) data;

    while ((len = (uint16_t) (sum >> 16)) != 0)
        sum = (uint16_t) sum + len;

    return (uint16_t) sum ^ 0xFFFF;
}

void icmpEchoRequest(icmpip_hdr* packet)
{
	uint32_t tempIp;
	int index;
	struct netEthArpHeader* packet1;

	// change type to reply
	packet->icmp.type = ICMP_TYPE_ECHOREPLY;
	// recalculate checksum
	packet->icmp.icmpchksum = 0;
	packet->icmp.icmpchksum = netChecksum((int8_t*)&packet->icmp, htons(packet->ip.len)-IP_HEADER_LEN);
	// return to sender
	tempIp = packet->ip.destipaddr;
	packet->ip.destipaddr = packet->ip.srcipaddr;
	packet->ip.srcipaddr = tempIp;
	// add ethernet routing


    index = arpMatchIp(HTONL(packet->ip.destipaddr));

	packet1 = (struct netEthIpHeader*)(((int8_t*)packet)-ETH_HEADER_LEN);

    packet1->eth.src  = ArpMyAddr.ethaddr;
	packet1->eth.dest = ArpTable[index].ethaddr;
	packet1->eth.type = HTONS(ETHTYPE_IP);



	//debugPrintHexTable(htons(packet->ip.len)+ETH_HEADER_LEN, (((int8_t*)packet)-ETH_HEADER_LEN));

	// debugging
//	#if NET_DEBUG >= 2
		//icmpPrintHeader(packet);
		//debugPrintHexTable(htons(packet->ip.len), (u08*)packet);
//	#endif

	// send it (packet->ip.len+ETH_HEADER_LEN
	nicSend(htons(packet->ip.len)+ETH_HEADER_LEN, (((int8_t*)packet)-ETH_HEADER_LEN));

    dbg3("Odpowiadam na zadanie ICMP\r\n");
}

void Update_table_for_ARP(struct netEthArpHeader* packet)
{
	int8_t index;

	// check if sender is already present in arp table
	index = arpMatchIp(HTONL(packet->arp.sipaddr));
	if(index != -1)
	{
		// sender's IP address found, update ARP entry
		ArpTable[index].ethaddr = packet->eth.src;
		// and we're done
		return;
	}

	// sender was not present in table,
	// must add in empty/expired slot
	for(index=0; index<ARP_TABLE_SIZE; index++)
	{
		if(!ArpTable[index].time)
		{
			// write entry
			ArpTable[index].ethaddr = packet->eth.src;
			ArpTable[index].ipaddr = HTONL(packet->arp.sipaddr);
			if(ipGetConfig()->gateway==HTONL(packet->arp.sipaddr))
			{
				ArpTable[index].time = 0xFF;
				wybor2_bitowy|=0x0080;
			}
			else ArpTable[index].time = ARP_CACHE_TIME_TO_LIVE;
			// and we're done
			return;
		}
	}

	// no space in table, we give up
}

void clear_arp_table(void)
{
    uint8_t i;

	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		 ArpTable[i].ipaddr = 0;
		 ArpTable[i].ethaddr.addr[0] = 0;
		 ArpTable[i].ethaddr.addr[1] = 0;
		 ArpTable[i].ethaddr.addr[2] = 0;
		 ArpTable[i].ethaddr.addr[3] = 0;
		 ArpTable[i].ethaddr.addr[4] = 0;
		 ArpTable[i].ethaddr.addr[5] = 0;
		 ArpTable[i].time = 0;
	}

}

int arpMatchIp(uint32_t ipaddr)
{
	uint8_t i;

	// check if IP address is present in arp table
	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		if(ArpTable[i].ipaddr == ipaddr)
		{
			// IP address found
			return i;
		}
	}

	// no match
	return -1;
}

void Update_table_for_IP(struct netEthIpHeader* packet)
{
	int8_t index;

	// check if sender is already present in arp table
	index = arpMatchIp(HTONL(packet->ip.srcipaddr));
	if(index != -1)
	{
		// sender's IP address found, update ARP entry
		ArpTable[index].ethaddr = packet->eth.src;
		// and we're done
		return;
	}

	// sender was not present in table,
	// must add in empty/expired slot
	for(index=0; index<ARP_TABLE_SIZE; index++)
	{
		if(!ArpTable[index].time)
		{
			// write entry
			ArpTable[index].ethaddr = packet->eth.src;
			ArpTable[index].ipaddr = HTONL(packet->ip.srcipaddr);
			ArpTable[index].time = ARP_CACHE_TIME_TO_LIVE;
			// and we're done
			return;
		}
	}

	// no space in table, we give up
}

void SEND_echo_request(unsigned long ip_addr, unsigned char *ptr)
{
    int index=0;

	struct netEthHeader* packet;
	icmpip_hdr * packet1;

    packet = (struct netEthHeader*)ptr;
	packet1 = (icmpip_hdr*)(ptr + ETH_HEADER_LEN);




           if( (ip_addr & IpMyConfig.netmask) == (IpMyConfig.ip & IpMyConfig.netmask) )
	       {
		         index = arpMatchIp(ip_addr);       //Eth_Header
		         if(index != -1)
	             {
		              packet->dest = ArpTable[index].ethaddr;
					  packet->src = ArpMyAddr.ethaddr;
					  packet->type = htons(ETHTYPE_IP);
	             }
		         else return;
	       }
	       else
	       {
		         index = arpMatchIp(IpMyConfig.gateway);       //Eth_Header
		         if(index != -1)
	             {
		              packet->dest = ArpTable[index].ethaddr;
					  packet->src = ArpMyAddr.ethaddr;
					  packet->type = htons(ETHTYPE_IP);
	             }
		         else return;
	       }





           packet1->ip.vhl = 0x45;             //Ip_Header
		   packet1->ip.tos = 0x00;
		   packet1->ip.len = 0x3C00;
		   packet1->ip.ipid = 0x2300;
		   packet1->ip.ipoffset = 0x0000;
		   packet1->ip.ttl = 0xFF;
		   packet1->ip.proto = 0x01;
		   packet1->ip.ipchksum = 0x0000;
		   packet1->ip.srcipaddr =  HTONL(IpMyConfig.ip);
		   packet1->ip.destipaddr =  htonl(ip_addr);

		   packet1->ip.ipchksum = netChecksum((int8_t*)&packet1->ip, IP_HEADER_LEN);



		   packet1->icmp.type = 0x08;          //Icmp_Header
		   packet1->icmp.icode = 0x00;
		   packet1->icmp.icmpchksum = 0x0000;
		   packet1->icmp.id = 0x0000;
		   packet1->icmp.seqno = 0x0000;


        for(index=0;index<40;index++) *(ptr + index + ETH_HEADER_LEN + IP_HEADER_LEN + ICMP_HEADER_LEN) = 0x61 + index;


		  packet1->icmp.icmpchksum = netChecksum((int8_t*)&packet1->icmp, htons(packet1->ip.len)-IP_HEADER_LEN);


		   nicSend(htons(packet1->ip.len)+ETH_HEADER_LEN, ptr);

		   rprintf("Wysylam ICMP\r\n");

		   //debugPrintHexTable(htons(packet1->ip.len)+ETH_HEADER_LEN, ptr);





}

uint16_t netChecksum_udp_tcp(uint8_t *data, uint16_t len, uint32_t xxx_x)
{
    uint32_t sum = 0,aa,bb,c;
	uint16_t lenn,a;
	uint8_t x1,x2;
	lenn=len;

    for (;;) {
        if (lenn < 2)
            break;

        x1 = *data;
		data++;
		x2 = *data;
		data++;

		a = ((unsigned short)x1)<<8 | (unsigned short)x2;

       // a = *((uint16_t *)data);
        aa = (unsigned long)a;
		sum = sum + aa;
	//	sum += *((uint16_t *)data);
		//data+=2;
        lenn -= 2;
    }


    if (lenn)
	{
        x1 = *data;
        a = ((unsigned short)x1)<<8;
        aa = (unsigned long)a;
		sum += aa;

	}


    if (xxx_x)
	    sum += xxx_x;


	//	rprintf("\r\n");
	//	rprintfu32(sum);
		//rprintf("\r\n");



    // bb =  (sum >> 16);
     //sum =  sum + bb;

	  while ((lenn = (uint16_t) (sum >> 16)) != 0)
        sum = (uint16_t) sum + lenn;



	 c=sum;



     sum = 0xFFFFFFFF - sum;

	// if( ((sum&0x0000FF00)==0x0000FF00)||((sum&0x0000FF00)==0x0000FE00) ) {debug32_t(sum);   debug_msg(" ");}


   // while ((len = (uint16_t) (sum >> 16)) != 0)
       //sum = (uint16_t) sum + len;

   // return (uint16_t) sum ^ 0xFFFF;


   // while ((lenn = (uint16_t) (sum >> 16)) != 0)
      //  sum = (uint16_t) sum + lenn;

    return sum;

}

void arpIpOut(struct netEthIpHeader* packet, uint32_t phyDstIp)
{
	int index;
	// check if destination is already present in arp table
	// use the physical dstIp if it's provided, otherwise the dstIp in packet
	if(phyDstIp)
		index = arpMatchIp(phyDstIp);
	else
		index = arpMatchIp(HTONL(packet->ip.destipaddr));
	// fill in ethernet info
	if(index != -1)
	{
		// ARP entry present, fill eth address(es)
		packet->eth.src  = ArpMyAddr.ethaddr;
		packet->eth.dest = ArpTable[index].ethaddr;
		packet->eth.type = HTONS(ETHTYPE_IP);
	}
	else
	{
		// not in table, must send ARP request
		packet->eth.src = ArpMyAddr.ethaddr;
		// MUST CHANGE, but for now, send this one broadcast
		packet->eth.dest.addr[0] = 0xFF;
		packet->eth.dest.addr[1] = 0xFF;
		packet->eth.dest.addr[2] = 0xFF;
		packet->eth.dest.addr[3] = 0xFF;
		packet->eth.dest.addr[4] = 0xFF;
		packet->eth.dest.addr[5] = 0xFF;
		packet->eth.type = HTONS(ETHTYPE_IP);
	}
}


void ipSend_mail(uint32_t srcIp, uint32_t dstIp, uint8_t protocol, uint16_t len, uint8_t* data, uint16_t id, uint8_t* bng )
{

	struct netEthIpHeader* ethIpHeader;

	data -= ETH_HEADER_LEN+IP_HEADER_LEN;
	ethIpHeader = (struct netEthIpHeader*)data;

//	debugPrintHexTable(len+ETH_HEADER_LEN+IP_HEADER_LEN, data);

	len += IP_HEADER_LEN;

	ethIpHeader->ip.vhl = 0x45;
	ethIpHeader->ip.tos = 0x00;
    ethIpHeader->ip.len = htons(len);
/*	if(ktr==0x01) ethIpHeader->ip.ipid = htons(0x0000);
	else if(ktr==0x03) ethIpHeader->ip.ipid = htons(0x0001);
	else if(ktr==0x08) ethIpHeader->ip.ipid = htons(0x0002);*/
	ethIpHeader->ip.ipid = htons(id);

	ethIpHeader->ip.ipoffset = 0;
	ethIpHeader->ip.ttl = 0x80;
    ethIpHeader->ip.proto = protocol;
	ethIpHeader->ip.ipchksum = 0x0000;
	ethIpHeader->ip.destipaddr = HTONL(dstIp);
	ethIpHeader->ip.srcipaddr = HTONL(srcIp);
	ethIpHeader->ip.ipchksum = netChecksum(&ethIpHeader->ip, IP_HEADER_LEN);


	 /*   nicGetMacAddress(&ethIpHeader->eth.src.addr[0]);

		ethIpHeader->eth.dest.addr[0] = 0xFF;
		ethIpHeader->eth.dest.addr[1] = 0xFF;
		ethIpHeader->eth.dest.addr[2] = 0xFF;
		ethIpHeader->eth.dest.addr[3] = 0xFF;
		ethIpHeader->eth.dest.addr[4] = 0xFF;
		ethIpHeader->eth.dest.addr[5] = 0xFF;
		ethIpHeader->eth.type = HTONS(ETHTYPE_IP);*/


	if( (dstIp & IpMyConfig.netmask) == (IpMyConfig.ip & IpMyConfig.netmask) )
	{
		arpIpOut(ethIpHeader,0);
	}
	else
	{
		arpIpOut(ethIpHeader,IpMyConfig.gateway);
	}


	len += ETH_HEADER_LEN;


	//debugPrintHexTable(ETH_HEADER_LEN, &data[0]);
    if(debug_==1)
    {
	    debugPrintHexTable(len, data);
    }

	bng[0]=len>>8;
	bng[1]=len;
	 for(int i=0;i<len;i++) bng[2+i]=data[i];
	nicSend(len,data);
}


void ipSend(uint32_t srcIp, uint32_t dstIp, uint8_t protocol, uint16_t len, uint8_t* data, uint16_t id)
{
    int ii;
	struct netEthIpHeader* ethIpHeader;

	data -= ETH_HEADER_LEN+IP_HEADER_LEN;
	ethIpHeader = (struct netEthIpHeader*)data;

//	debugPrintHexTable(len+ETH_HEADER_LEN+IP_HEADER_LEN, data);

	len += IP_HEADER_LEN;

	ethIpHeader->ip.vhl = 0x45;
	ethIpHeader->ip.tos = 0x00;
    ethIpHeader->ip.len = htons(len);
/*	if(ktr==0x01) ethIpHeader->ip.ipid = htons(0x0000);
	else if(ktr==0x03) ethIpHeader->ip.ipid = htons(0x0001);
	else if(ktr==0x08) ethIpHeader->ip.ipid = htons(0x0002);*/
	ethIpHeader->ip.ipid = htons(id);

	ethIpHeader->ip.ipoffset = 0;
	ethIpHeader->ip.ttl = 0x80;
    ethIpHeader->ip.proto = protocol;
	ethIpHeader->ip.ipchksum = 0x0000;
	ethIpHeader->ip.destipaddr = HTONL(dstIp);
	ethIpHeader->ip.srcipaddr = HTONL(srcIp);
	ethIpHeader->ip.ipchksum = netChecksum(&ethIpHeader->ip, IP_HEADER_LEN);


	 /*   nicGetMacAddress(&ethIpHeader->eth.src.addr[0]);

		ethIpHeader->eth.dest.addr[0] = 0xFF;
		ethIpHeader->eth.dest.addr[1] = 0xFF;
		ethIpHeader->eth.dest.addr[2] = 0xFF;
		ethIpHeader->eth.dest.addr[3] = 0xFF;
		ethIpHeader->eth.dest.addr[4] = 0xFF;
		ethIpHeader->eth.dest.addr[5] = 0xFF;
		ethIpHeader->eth.type = HTONS(ETHTYPE_IP);*/


	if( (dstIp & IpMyConfig.netmask) == (IpMyConfig.ip & IpMyConfig.netmask) )
	{
		arpIpOut(ethIpHeader,0);
	}
	else
	{
		arpIpOut(ethIpHeader,IpMyConfig.gateway);
	}


	len += ETH_HEADER_LEN;


	//debugPrintHexTable(ETH_HEADER_LEN, &data[0]);
    if(debug_==1)
    {
	    debugPrintHexTable(len, data);
    }

    if(retremail==1)
    {
    	retremail=0;
    	 buf_retrans[0]=len>>8;
    	 buf_retrans[1]=len;
    	 for(ii=0;ii<len;ii++) buf_retrans[2+ii] = data[ii]; // sprintf(buf_p," s: %d  ",len); dbg3(buf_p);
    }

        if(debug_smtp==1)
        {
        	ZapisSesjiSMTP(data, len, &adr_flash_SesjaSMTP);
        }


	nicSend(len, data);
}

void ipSend_http(uint32_t srcIp, uint32_t dstIp, uint8_t protocol, uint16_t len, uint8_t* data, uint16_t id, unsigned char ktr, unsigned char nr_retrans)
{

	int ii;
	struct netEthIpHeader* ethIpHeader;   unsigned short size_p;  unsigned char pom[3];

	data -= ETH_HEADER_LEN+IP_HEADER_LEN;
	ethIpHeader = (struct netEthIpHeader*)data;


	len += IP_HEADER_LEN;

	ethIpHeader->ip.vhl = 0x45;
	ethIpHeader->ip.tos = 0;
    ethIpHeader->ip.len = htons(len);
	ethIpHeader->ip.ipid = htons(id);

	ethIpHeader->ip.ipoffset = 0;
	ethIpHeader->ip.ttl = 0x80;
    ethIpHeader->ip.proto = protocol;
	ethIpHeader->ip.ipchksum = 0x0000;
	ethIpHeader->ip.destipaddr = HTONL(dstIp);
	ethIpHeader->ip.srcipaddr = HTONL(srcIp);
	ethIpHeader->ip.ipchksum = netChecksum(&ethIpHeader->ip, IP_HEADER_LEN);



	   if( (dstIp & IpMyConfig.netmask) == (IpMyConfig.ip & IpMyConfig.netmask) )
 	   {
		   arpIpOut(ethIpHeader,0);
	   }
	   else
	   {
		   arpIpOut(ethIpHeader,IpMyConfig.gateway);
	   }


	len += ETH_HEADER_LEN;



	if(debug_==1)
    {
	    debugPrintHexTable(len, data);
    }


	 size_p = (NETSTACK_BUFFERSIZE/2)-10;


	 if(ktr==0)
	 {
	     buf_retrans[0]=len>>8;
	     buf_retrans[1]=len;
         for(ii=0;ii<len;ii++) buf_retrans[2+ii] = data[ii];

	 }
	 else
	 {
		 buf_retrans2[0]=len>>8;
		 buf_retrans2[1]=len;
		 for(ii=0;ii<len;ii++) buf_retrans2[2+ii] = data[ii];



				//debugPrintHexTable(ii, &buf_retrans[2]);
				//HAL_Delay(30);
		 ii =  ((unsigned short)buf_retrans[0]<<8)&0xFF00  |  ((unsigned short)buf_retrans[1]<<0)&0x00FF;
				nicSend(ii,  &buf_retrans[2]);
		 ii =  ((unsigned short)buf_retrans2[0]<<8)&0xFF00  |  ((unsigned short)buf_retrans2[1]<<0)&0x00FF;
				nicSend(ii,  &buf_retrans2[2]);
	 }




}

void tcpSend_http(uint32_t srcIp, uint32_t dstIp, uint16_t srcPort, uint16_t dstPort, uint16_t len, uint8_t* data, uint16_t id, uint16_t seq, uint8_t nr_retrans )
{

    uint32_t chk_p=0,a1;
    unsigned short chek;  unsigned char fflaggg;

	uint8_t* data_p;    uint16_t len_p1,len_p2,len_p3;     uint16_t mobile;

	data_p = data;      mobile=0;



    if((*(data+0)==0x01)&&(*(data+1)==0x01)&&(*(data+2)==0x08)&&(*(data+3)==0x0A)){ mobile=12;  }  // urzadzenia mobilne
    if((*(data+0)==0x01)&&(*(data+1)==0x01)&&(*(data+2)==0x05)&&(*(data+3)==0x0A)){ mobile=12;  } //przy retransmisji

	len_p1 = mobile + ((len-mobile)/2);      len_p2 = (len-mobile)/2;       len_p3 = len_p2;







	struct netTcpHeader* tcpHeader;


	data -= TCP_HEADER_LEN;
	tcpHeader = (struct netTcpHeader*)data;

	len_p1 += TCP_HEADER_LEN;


        tcpHeader->srcport = HTONS(srcPort);
	    tcpHeader->destport = HTONS(dstPort);

		//tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + 1);
		//tcpHeader->seqno = HTONL(0x00010000);
		chk_p  = tcpHeader->ackno;
		tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + seq);
		tcpHeader->seqno = chk_p;

		//A  tcpHeader->headlen = tcpHeader->headlen;
		tcpHeader->flags |= TCP_FLAGS_ACK;      fflaggg = tcpHeader->flags;

		tcpHeader->flags = (tcpHeader->flags & (~0x01));

	    tcpHeader->wnd = HTONS(3216);//tcpHeader->wnd;
	    //A tcpHeader->urgp = tcpHeader->urgp;

		tcpHeader->tcpchksum = 0x0000;


		chk_p = ((srcIp>>16)&0x0000FFFF) + (srcIp&0x0000FFFF) + ((dstIp>>16)&0x0000FFFF) + (dstIp&0x0000FFFF) + IP_PROTO_TCP;


      chek =  netChecksum_udp_tcp(  data, len_p1, chk_p + len_p1  );



      tcpHeader->tcpchksum = HTONS(chek);

    // if(data[0x32]==0xFF) debug_msg("KURWA MAC");  if(data[0x32]==0xFE) debug_msg("KURWA MAC");


	//debugPrintHexTable(TCP_HEADER_LEN, (uint8_t*)tcpHeader);

	ipSend_http(dstIp, srcIp, IP_PROTO_TCP, len_p1, (uint8_t*)tcpHeader, id, 0, nr_retrans);








	int i;

	if((len%2)==0)
	{

	    for(i=0; i<len_p2; i++)    *(data_p+mobile+i) = *(data_p+mobile+len_p2+i);

	}
    else
    {

        for(i=0; i<(len_p2+1); i++)    *(data_p+mobile+i) = *(data_p+mobile+len_p2+i);

	    len_p2 = len_p2+1;

    }


	data_p = data_p - TCP_HEADER_LEN;
	tcpHeader = (struct netTcpHeader*)data_p;

	len_p2 += TCP_HEADER_LEN + mobile;


        tcpHeader->srcport = HTONS(srcPort);
	    tcpHeader->destport = HTONS(dstPort);

		//tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + 1);
		//tcpHeader->seqno = HTONL(0x00010000);


		//chk_p  = tcpHeader->ackno;
	    //A tcpHeader->ackno = tcpHeader->ackno;
		tcpHeader->seqno = HTONL(HTONL(tcpHeader->seqno) + len_p3);

		//A  tcpHeader->headlen = tcpHeader->headlen;
		tcpHeader->flags = fflaggg;

		//A tcpHeader->wnd = tcpHeader->wnd;
		//A tcpHeader->urgp = tcpHeader->urgp;

		tcpHeader->tcpchksum = 0x0000;


		chk_p = ((srcIp>>16)&0x0000FFFF) + (srcIp&0x0000FFFF) + ((dstIp>>16)&0x0000FFFF) + (dstIp&0x0000FFFF) + IP_PROTO_TCP;


      chek =  netChecksum_udp_tcp(  data_p, len_p2, chk_p + len_p2  );



      tcpHeader->tcpchksum = HTONS(chek);

    // if(data[0x32]==0xFF) debug_msg("KURWA MAC");  if(data[0x32]==0xFE) debug_msg("KURWA MAC");


	//debugPrintHexTable(TCP_HEADER_LEN, (uint8_t*)tcpHeader);

	ipSend_http(dstIp, srcIp, IP_PROTO_TCP, len_p2, (uint8_t*)tcpHeader, id, 1, nr_retrans);







}

void tcpSend11_2(uint32_t srcIp, uint32_t dstIp, uint16_t srcPort, uint16_t dstPort, uint16_t len, uint8_t* data, uint16_t id, uint16_t seq, uint32_t kur)
{

    uint32_t chk_p;


	struct netTcpHeader* tcpHeader;

	data -= TCP_HEADER_LEN;
	tcpHeader = (struct netTcpHeader*)data;

	len += TCP_HEADER_LEN;


        tcpHeader->srcport = HTONS(srcPort);
	    tcpHeader->destport = HTONS(dstPort);

		//tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + 1);
		tcpHeader->seqno = kur;
		//chk_p  = tcpHeader->ackno;
		tcpHeader->ackno = 0;
		tcpHeader->seqno = 0;

	    tcpHeader->headlen = 0x80;
		tcpHeader->flags = TCP_FLAGS_SYN;

	    tcpHeader->wnd = 0xffff;
		tcpHeader->urgp = 0x0000;

		tcpHeader->tcpchksum = 0x0000;


		chk_p = ((srcIp>>16)&0x0000FFFF) + (srcIp&0x0000FFFF) + ((dstIp>>16)&0x0000FFFF) + (dstIp&0x0000FFFF) + IP_PROTO_TCP;


        tcpHeader->tcpchksum = HTONS(netChecksum_udp_tcp(  (uint8_t*)tcpHeader, len, chk_p + len  ));


	//debugPrintHexTable(TCP_HEADER_LEN, (uint8_t*)tcpHeader);

	ipSend(srcIp, dstIp, IP_PROTO_TCP, len, (uint8_t*)tcpHeader, id);

}

void tcpSend22(uint32_t srcIp, uint32_t dstIp, uint16_t srcPort, uint16_t dstPort, uint16_t len, uint8_t* data, uint16_t id, uint16_t seq)
{

    uint32_t chk_p;


	struct netTcpHeader* tcpHeader;

	data -= TCP_HEADER_LEN;
	tcpHeader = (struct netTcpHeader*)data;

	len += TCP_HEADER_LEN;


        tcpHeader->srcport = HTONS(srcPort);
	    tcpHeader->destport = HTONS(dstPort);

		//tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + 1);
		//tcpHeader->seqno = HTONL(0x00010000);
		chk_p  = tcpHeader->ackno;
		tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + seq);
		tcpHeader->seqno = chk_p;

	    tcpHeader->headlen = 0x50;
		tcpHeader->flags = 0x11;

		//A  tcpHeader->wnd = tcpHeader->wnd;
		//A tcpHeader->urgp = tcpHeader->urgp;

		tcpHeader->tcpchksum = 0x0000;


		chk_p = ((srcIp>>16)&0x0000FFFF) + (srcIp&0x0000FFFF) + ((dstIp>>16)&0x0000FFFF) + (dstIp&0x0000FFFF) + IP_PROTO_TCP;


        tcpHeader->tcpchksum = HTONS(netChecksum_udp_tcp(  (uint8_t*)tcpHeader, len, chk_p + len  ));


	//debugPrintHexTable(TCP_HEADER_LEN, (uint8_t*)tcpHeader);

	ipSend(srcIp, dstIp, IP_PROTO_TCP, len, (uint8_t*)tcpHeader, id);

}

void tcpSend33(uint32_t srcIp, uint32_t dstIp, uint16_t srcPort, uint16_t dstPort, uint16_t len, uint8_t* data, uint16_t id, uint16_t seq)
{

    uint32_t chk_p;


	struct netTcpHeader* tcpHeader;

	data -= TCP_HEADER_LEN;
	tcpHeader = (struct netTcpHeader*)data;

	len += TCP_HEADER_LEN;


        tcpHeader->srcport = HTONS(srcPort);
	    tcpHeader->destport = HTONS(dstPort);

		//tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + 1);
		//tcpHeader->seqno = HTONL(0x00010000);
		chk_p  = tcpHeader->ackno;
		tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + seq);
		tcpHeader->seqno = chk_p;

	    tcpHeader->headlen = 0x50;
		tcpHeader->flags = TCP_FLAGS_ACK|TCP_FLAGS_PSH;

		//A  tcpHeader->wnd = tcpHeader->wnd;
		//A tcpHeader->urgp = tcpHeader->urgp;

		tcpHeader->tcpchksum = 0x0000;


		chk_p = ((srcIp>>16)&0x0000FFFF) + (srcIp&0x0000FFFF) + ((dstIp>>16)&0x0000FFFF) + (dstIp&0x0000FFFF) + IP_PROTO_TCP;


        tcpHeader->tcpchksum = HTONS(netChecksum_udp_tcp(  (uint8_t*)tcpHeader, len, chk_p + len  ));


	//debugPrintHexTable(TCP_HEADER_LEN, (uint8_t*)tcpHeader);

	ipSend(srcIp, dstIp, IP_PROTO_TCP, len, (uint8_t*)tcpHeader, id);

}


void tcpSend(uint32_t srcIp, uint32_t dstIp, uint16_t srcPort, uint16_t dstPort, uint16_t len, uint8_t* data, uint16_t id, uint16_t seq)
{

    uint32_t chk_p;


	struct netTcpHeader* tcpHeader;

	data -= TCP_HEADER_LEN;
	tcpHeader = (struct netTcpHeader*)data;

	len += TCP_HEADER_LEN;


        tcpHeader->srcport = HTONS(srcPort);
	    tcpHeader->destport = HTONS(dstPort);

		//tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + 1);
		//tcpHeader->seqno = HTONL(0x00010000);
		chk_p  = tcpHeader->ackno;
		tcpHeader->ackno = HTONL(HTONL(tcpHeader->seqno) + seq);
		tcpHeader->seqno = chk_p;

	   //A tcpHeader->headlen = tcpHeader->headlen;
		 tcpHeader->flags |= TCP_FLAGS_ACK;

		//A tcpHeader->wnd = tcpHeader->wnd;
		//A tcpHeader->urgp = tcpHeader->urgp;

		tcpHeader->tcpchksum = 0x0000;


		chk_p = ((srcIp>>16)&0x0000FFFF) + (srcIp&0x0000FFFF) + ((dstIp>>16)&0x0000FFFF) + (dstIp&0x0000FFFF) + IP_PROTO_TCP;


        tcpHeader->tcpchksum = HTONS(netChecksum_udp_tcp(  (uint8_t*)tcpHeader, len, chk_p + len  ));


	//debugPrintHexTable(TCP_HEADER_LEN, (uint8_t*)tcpHeader);

	ipSend(dstIp, srcIp, IP_PROTO_TCP, len, (uint8_t*)tcpHeader, id);

}

void udpSend(uint32_t srcIp, uint32_t dstIp, uint16_t srcPort, uint16_t dstPort, uint16_t len, uint8_t* data, uint16_t id)
{

    uint32_t chk_p;

	struct netUdpHeader* udpHeader;

	data -= UDP_HEADER_LEN;
	udpHeader = (struct netUdpHeader*)data;

	len += UDP_HEADER_LEN;

	udpHeader->srcport  = HTONS(srcPort);
	udpHeader->destport = HTONS(dstPort);
	udpHeader->udplen = htons(len);
	udpHeader->udpchksum = 0x0000;

    chk_p = ((srcIp>>16)&0x0000FFFF) + (srcIp&0x0000FFFF) + ((dstIp>>16)&0x0000FFFF) + (dstIp&0x0000FFFF) + IP_PROTO_UDP;

	udpHeader->udpchksum = HTONS(  netChecksum_udp_tcp( &udpHeader->srcport,len, chk_p + len  )  );

	//debugPrintHexTable(UDP_HEADER_LEN, (uint8_t*)udpPacket);

	ipSend(srcIp, dstIp, IP_PROTO_UDP, len, (uint8_t*)udpHeader, id);
}



void dns_query(uint32_t dns_ip_addr_p,  uint8_t *ptr,  uint8_t *ptr_str,  uint16_t idd)
{

    struct netDnsHeader* packet;
    packet = (struct netDnsHeader*)(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN);

	packet->id = idd;
	packet->code = HTONS(0x0100);
	packet->qdcount = HTONS(0x0001);
	packet->ancount = HTONS(0x0000);
	packet->nscount = HTONS(0x0000);
	packet->arcount = HTONS(0x0000);

    //NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 0] = 0x03;
	//NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 1] = 0x77;
	//NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 2] = 0x77;
	//NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 3] = 0x77;


    uint8_t i=0;
	uint8_t k=0;


       kaman0:
	   while(*(ptr_str + i)!='.')
	   {
	       if(*(ptr_str + i)==0)
		   {
		       *(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + i - k) = k;
			   i++;
		       goto omin234;
		   }

		   *(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 1 + i) = *(ptr_str + i);
	   	   i++;
		   k++;
       }
	   *(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + i - k) = k;
	   i++;
	   k=0;
	   goto kaman0;


    omin234:
    *(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + i) = 0x00;
    i++;
	*(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + i) = 0x00;
    i++;
	*(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + i) = 0x01;
    i++;
	*(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + i) = 0x00;
    i++;
	*(ptr + ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + i) = 0x01;
	i++;

/*	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 4] = 0x0c;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 5] = 0x6e;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 6] = 0x6f;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 7] = 0x72;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 8] = 0x74;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 9] = 0x68;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 10] = 0x65;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 11] = 0x61;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 12] = 0x73;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 13] = 0x74;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 14] = 0x65;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 15] = 0x72;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 16] = 0x6e;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 17] = 0x03;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 18] = 0x65;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 19] = 0x64;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 20] = 0x75;*/

/*	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 21] = 0x00;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 22] = 0x00;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 23] = 0x01;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 24] = 0x00;
	NetBuffer[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN+DNS_HEADER_LEN + 25] = 0x01;*/


    udpSend( IpMyConfig.ip,  dns_ip_addr_p,  DNS_CLIENT_PORT,  DNS_SERVER_PORT,  DNS_HEADER_LEN + i,  (uint8_t*)packet,  0);

}

void netstackInit(uint32_t ipaddress, uint32_t netmask, uint32_t gatewayip)
{
	nicInit();
	arpInit();
	ipSetConfig(ipaddress, netmask, gatewayip);
}

void icmpIpIn(icmpip_hdr* packet)   //ping
{
	// check ICMP type
	switch(packet->icmp.type)
	{
	case ICMP_TYPE_ECHOREQUEST:
		// echo request
		icmpEchoRequest(packet);
		break;
    case ICMP_TYPE_ECHOREPLY:
	    dbg3("ODEBRANO ICMP REPLY\r\n");


		break;
	default:
		break;
	}
}

unsigned char search_struct_HTTP_free(void)
{
   unsigned char i;
   for(i=0;i<HTTP_task_LEN;i++)
   {
       if( HTTP[i].vit == 0 )
	   {
	      return i;
	   }
   }
   return 99;
}

unsigned char if_all_HTTP_free(void)
{
   unsigned char i,j;
   j=0;
   for(i=0;i<HTTP_task_LEN;i++)
   {
       if( HTTP[i].vit == 0 )
	   {
	      j++;
	   }
   }
   if(i==j) return 1;
   else     return 0;

}

unsigned char all_HTTP_clear(void)
{
   unsigned char i,j;
   j=0;
   for(i=0;i<HTTP_task_LEN;i++)
   {
        HTTP[i].vit = 0;

   }

}

void korekcja_plus_spacja(unsigned char *in, int len)
{
   int i;
   for(i=0;i<len;i++)
   {
        if(*(in+i)=='+') *(in+i)=' ';
   }
}


unsigned long wybierz_cyfry_7(unsigned char *tx)
{
    int aa; unsigned char k,k1,k2,k3,k4,k5,k6,k7,k8,k9,k10;
	k=0;  k1=0; k2=0; k3=0; k4=0; k5=0; k6=0; k7=0;

	wybierz_cyfry_DALEJa:
	if(((*tx)>0x2F)&&((*tx)<0x3A))
	{

	        if(k==0) k1=(*tx)&0x0f;
	   else if(k==1) k2=(*tx)&0x0f;
	   else if(k==2) k3=(*tx)&0x0f;
	   else if(k==3) k4=(*tx)&0x0f;
	   else if(k==4) k5=(*tx)&0x0f;
	   else if(k==5) k6=(*tx)&0x0f;
	   else if(k==6) k7=(*tx)&0x0f;   tx++;  k++;

	   goto wybierz_cyfry_DALEJa;

	}
    else
    {
        tx++;
		if(k>0) goto wybierz_cyfry_KONIECa;
		else    goto wybierz_cyfry_DALEJa;

    }

   wybierz_cyfry_KONIECa:

        if(k==1) return k1;
   else if(k==2) return 10*k1+k2;
   else if(k==3) return 100*k1+10*k2+k3;
   else if(k==4) return 1000*k1+100*k2+10*k3+k4;
   else if(k==5) return 10000*k1+1000*k2+100*k3+10*k4+k5;
   else if(k==6) return 100000*k1+10000*k2+1000*k3+100*k4+10*k5+k6;
   else if(k==7) return 1000000*k1+100000*k2+10000*k3+1000*k4+100*k5+10*k6+k7;

}


unsigned short wybierz_cyfry(unsigned char *tx)
{
    int aa; unsigned char k,k1,k2,k3,k4,k5;
	k=0;  k1=0; k2=0; k3=0; k4=0; k5=0;

	wybierz_cyfry_DALEJ:
	if(((*tx)>0x2F)&&((*tx)<0x3A))
	{

	        if(k==0) k1=(*tx)&0x0f;
	   else if(k==1) k2=(*tx)&0x0f;
	   else if(k==2) k3=(*tx)&0x0f;
	   else if(k==3) k4=(*tx)&0x0f;
	   else if(k==4) k5=(*tx)&0x0f;   tx++;  k++;

	   goto wybierz_cyfry_DALEJ;

	}
    else
    {
        tx++;
		if(k>0) goto wybierz_cyfry_KONIEC;
		else    goto wybierz_cyfry_DALEJ;

    }

   wybierz_cyfry_KONIEC:

        if(k==1) return k1;
   else if(k==2) return 10*k1+k2;
   else if(k==3) return 100*k1+10*k2+k3;
   else if(k==4) return 1000*k1+100*k2+10*k3+k4;
   else if(k==5) return 10000*k1+1000*k2+100*k3+10*k4+k5;

}
unsigned char copy_cgiii(unsigned char *text, unsigned char *data)
{
	unsigned char var_cgi, *data2;   var_cgi=0;  int ccc,eee;
	ccc=0;  data2=data;
	while ( *text != 0x20)
    {
       if(*text=='&'){ var_cgi=1;  break; }
	   if(*text==0)  { return 2; }
	   if(*text=='+') *text=' ';
	   if((*text==0x0D)||(*text==0x0A)) break;  else *data = *text;
       text++;
       data++;
	   ccc++;
    }
	*data = 0;      data=data2;

    while(korekcja_formatu_GET(data,ccc));

    return var_cgi;


}

void CGI_callback(unsigned char *_pk_, unsigned short *wsk_html)   //cgi
{
  unsigned char *_pk2_,*_pk3_,*_pk4_,_zm_cgi_,_tab_cgi_[300],*pp;  unsigned short sf; unsigned char _zapisz_,pom1,pi;

  if(pp=strstr(_pk_,"?scroll="))
  {
  	scroll = wybierz_cyfry(pp+8);
  	//sprintf(buf_p,"\r\nScroll: %d  ",scroll);    dbg3(buf_p);
  }


  if(HTTP[ux].get_query[0]!=0)
  {
      int _i_,_j_;
	  for(_i_=0;_i_<_Len_multiple_GET_query;_i_++){ if(HTTP[ux].get_query[_i_]==0) break;  }
	  _j_=0;
	  while((*_pk_!='&')&&(*_pk_!=' ')){  HTTP[ux].get_query[_i_+_j_]=*_pk_;  _j_++; _pk_++; }   HTTP[ux].get_query[_i_+_j_]=0;
      korekcja_plus_spacja(HTTP[ux].get_query,_i_+_j_); while(korekcja_formatu_GET(HTTP[ux].get_query,_i_+_j_));    _pk4_=HTTP[ux].get_query;      _pk3_=strchr(HTTP[ux].get_query,'=')+1;    // dbg3("\r\n");  dbg3(HTTP[ux].get_query);
	  _i_=0; while(*_pk3_!=0){ _tab_cgi_[_i_++]=*_pk3_++; } _tab_cgi_[_i_]=0;  goto wykonuj_get_zalegle;
  }
 dbg3(" CGI ");
 iterix_CGI_callback2:   _pk2_=_pk_;
 iterix_CGI_callback:
 _pk3_=strchr(_pk2_,'='); if(_pk3_==0){ while((*_pk2_!='&')&&(*_pk2_!=' ')) _pk2_++; sprintf(HTTP[ux].get_query,"%s",_pk2_+1); return;} else _pk2_=_pk3_+1;  _zm_cgi_=copy_cgiii(_pk2_, _tab_cgi_);  *(_pk2_-1)=0;

         if(_zm_cgi_==2){ while((*_pk_!='&')&&(*_pk_!=' ')) _pk_++; sprintf(HTTP[ux].get_query,"%s=%s",_pk_+1,_pk2_); return;}   _pk4_=_pk_;

 unsigned char *ptr,*ptr1;   unsigned short cgi_i;   wykonuj_get_zalegle:  _zapisz_=1;

//########################### --- CGI --- ######################################################
//########################### --- CGI --- ######################################################
//########################### --- CGI --- ######################################################
pom1=0;

//dbg3("\r\n"); dbg3(_pk4_);

if(ptr1=strstr(_pk4_,"n_pwm"))   //slider PWM
{
		//dbg3("\r\n"); dbg3(_tab_cgi_);
	    for(i=0;i<_Size_pwm;i++)
		{
		  if(ptr=strstr(ptr1,"n_pwm"))
		  {
			 j=wybierz_cyfry(ptr+5);  j--;
			 Const.s_PWM[j].duty = wybierz_cyfry(_tab_cgi_);   //sprintf(buf_p,"\r\nJJ%d:   %d",j,Const.s_PWM[j].duty); dbg3(buf_p);
			 GPIO_PWM(j);

		  }
		  else{ break;}
		  ptr1=ptr+2;
		}
		//ZapiszFlash();
	    FlashWsadPrg( &Const.s_PWM[0].freq, ((2+1)*_Size__s_pwm) );
}
else if(ptr1=strstr(_pk4_,"n_115_zapisz"))
{
	//sprintf(buf_p,"\r\nn_115_zapisz= %d",wybierz_cyfry(_tab_cgi_));    dbg3(buf_p);
}

if(strstr(_pk4_,"n_s_"))
{
	 CGI_Wpis(_pk4_,_tab_cgi_);
}
else
{

	CGI_Wpis2(_pk4_,&pi);

}

/*
else
{
   if(pom1==1) _zapisz_=1;
   else        _zapisz_=0;
}
*/

//########################### --- END CGI --- ######################################################
//########################### --- END CGI --- ######################################################
//########################### --- END CGI --- ######################################################

  if(HTTP[ux].get_query[0]!=0){ HTTP[ux].get_query[0]=0; goto iterix_CGI_callback2;}

  if(_zm_cgi_==1){ _pk_=_pk2_;  goto iterix_CGI_callback; }

    //Propozycja !!!
	//     if(HTTP[ux].wsk==...) _zapisz_=...
	//else if(HTTP[ux].wsk==...) _zapisz_=...
	//else if(HTTP[ux].wsk==...) _zapisz_=...

  if(_zapisz_==1)
  {
	   // ZAPISZ_do_eeprom_x();

  }


}

unsigned char clear_struct_HTTP(void)
{
   unsigned char i;
   for(i=0;i<HTTP_task_LEN;i++)
   {
        HTTP[i].vit = 0;
   }
}

unsigned char search_struct_HTTP(unsigned short *prt)
{
   unsigned char i;
   for(i=0;i<HTTP_task_LEN;i++)
   {
       if( HTTP[i].vit != 0 )
	   {
	      if(HTTP[i].port == *prt)
		  {
		       return i;
		  }

	   }
   }
   return 99;
}
/*
void Asp_Callback(int _ss_, unsigned char *_buf_)     //asp
{
  unsigned char *_pk_, *_pk2_, *_pk3_, *_pk4_, _in_[50], _in2_[300];    unsigned short _h_,_len_,_size_v1_,_size_v2_;
  _pk3_=&_buf_[_ss_-1];   _pk4_=&_buf_[0];
  while(_pk_=strstr(_pk4_,"<%"))
  { _in2_[0]=0;  _pk2_=_pk_+2;  _size_v1_=0;  while(*_pk2_!='%'){ _in_[_size_v1_++]=*_pk2_++; }  _in_[_size_v1_]=0;  _pk2_+=2;  _pk4_=_pk2_;      if(_pk3_>_pk2_){ _len_=_pk3_-_pk2_; _len_++; } else _len_=0;



//########################### --- ASP --- ######################################################
//########################### --- ASP --- ######################################################
//########################### --- ASP --- ######################################################

  unsigned char *ptr,*ptr1;   unsigned short asp_i;

if(ptr=strstr(_in_,"v_s_Czujki_nazwa_000"))
{
	sprintf(_in2_,Const.s_Czujki[0].nazwa);
}
else if(ptr=strstr(_in_,"v_s_Czujki_prog_max1_000"))
{
  	  wartTemp_odwrotnie(_in2_,Const.s_Czujki[0].prog_max1);
}
else if(ptr=strstr(_in_,"v_s_Czujki_prog_min1_000"))
{
	wartTemp_odwrotnie(_in2_,Const.s_Czujki[0].prog_min1);
}
else if(ptr=strstr(_in_,"v_s_Czujki_hist_000"))
{
	wartTemp_odwrotnie(_in2_,Const.s_Czujki[0].hist);
}

//########################### --- END ASP --- ######################################################
//########################### --- END ASP --- ######################################################
//########################### --- END ASP --- ######################################################


    // if(_in2_[0]!=0)
    // {
	     _size_v2_=strlen(_in2_);   _pk4_=_pk_+_size_v2_;
        if(_size_v1_<(_size_v2_-4)){  for(_h_=0;_h_<(_len_+1);_h_++){  *(_pk_+2+_size_v1_+2+_len_+(_size_v2_-4-_size_v1_)-_h_)=*(_pk_+2+_size_v1_+2+_len_-_h_); }  *(_pk_+2+_size_v1_+2+_len_+(_size_v2_-4-_size_v1_)+1)=0;  _pk3_=_pk_+2+_size_v1_+2+_len_+(_size_v2_-4-_size_v1_); }
	    else{  for(_h_=0;_h_<(_len_);_h_++){  *(_pk_+_size_v2_+_h_)=*(_pk_+2+_size_v1_+2+_h_); }       *(_pk_+_size_v2_+_h_)=0;   _pk3_=_pk_+_size_v2_+_h_-1; }
	    for(_h_=0;_h_<_size_v2_;_h_++) *(_pk_+_h_)=_in2_[_h_];
     //}

  }
}
*/


void send_code_html(unsigned short *p1, unsigned char *p2, unsigned char *p3, unsigned char ku)  //sendhtml
{
    unsigned short i,j,e,f1,f2,f3,f4,f5,f6,f7;   unsigned char *pk,*pk4,*pk5,bb[3],a,end_packet=0,buf_p[20];  int Cgi_Query_p;
    int kfread;
	pk5 = &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((ku>>4)*4) + (*p1)];


	if(HTTP[ux].txt_jpg==0x08){ end_packet=1; goto omin_tu; }

	     if(HTTP[ux].txt_jpg==0x03) Cgi_Query_p=900;   //zwiekszenie wartosci: 'Cgi_Query_p' wtedy, gdy strona ma duzo i gesto umieszczone <%...%>
	//else if(HTTP[ux].txt_jpg==0x02) Cgi_Query_p=0;
	else if(HTTP[ux].txt_jpg==6) Cgi_Query_p=1200;   //potrzebne dla zaleznosci_c.htm, gdy wszystkie 200 bajty wpisane w zale¿noœciach
    else                             Cgi_Query_p=_SIZE__Cgi_Query;

    kfread= f_read(pk5, (NETSTACK_BUFFERSIZE-Cgi_Query_p), &s1);   pk5[s1]=0;   if(s1<(NETSTACK_BUFFERSIZE-Cgi_Query_p)){ end_packet=1;} else{     if(kfread==-1) end_packet=1; else end_packet=0;     }


	if((HTTP[ux].txt_jpg==0x01)||(HTTP[ux].txt_jpg==0x03)||(HTTP[ux].txt_jpg==0x07)||(HTTP[ux].txt_jpg==6)) //dla =0x07 po wyslaniu calego pliku doslanie jeszcze pomiarow w <pre>
	{

	  /* if(pk5[s1-1]=='<')      // start korekcja   (..aby odczyt nie zatrzymal sie na zmniennej asp_)
	   {
            f_read(bb, 2, &s2);    pk5[s1++]=bb[0]; pk5[s1++]=bb[1];  //uart_transmit1(bb[0]); uart_transmit1(bb[1]);
	   }
	   pk5[s1]=0;

	   pk4=&pk5[0];
	   while(pk=strstr(pk4,"<%"))
	   {
	       a=1; pk4=pk+2;
		   powtorz_e001:
		        if(*pk4=='>'){  a=0;  pk4++;       }
		   else if(*pk4==0)  {   break;                     }
		   else               {  pk4++; goto powtorz_e001; }
	   }
	   if(a==1)
	   {
	       powtorz_e002:
           f_read(bb, 1, &s2);   pk5[s1++]=bb[0];
		   if(bb[0]!='>')  goto powtorz_e002;
	   }
       pk5[s1]=0;     // stop korekcja


       pk5[s1]=0;
	   Asp_Callback(s1,pk5);*/

	   *p1 += strlen(pk5);

	}
    else if(HTTP[ux].txt_jpg==0x02)
	{
	     *p1 += s1;

	}




	    omin_tu:
    if(end_packet==1)
	{
		if(HTTP[ux].txt_jpg==0x07)
		{

		}
		else
		{
			   if(HttpPagesBuffer[WhichFragmentPage]!=0)
			   {
				    f_open();
				    *p3 = 0x08;
				  	*p2=1;
			   }
		       else
		       {
			      *p3 = 0x09;
			      *p2=4;
		       }
		}

	}
	else
	{   *p3 = 0x08;
		*p2=1;
	}








}

void copy_text(unsigned char *text, unsigned char *pbuf)
{
   while(*text!=0)
   {
      *pbuf = *text;
       text++;
       pbuf++;
   }
   *pbuf = *text;
   return;
}

Send_Header_HTTP(unsigned short *ix, unsigned char po)
{

 unsigned short kur=0;

                   copy_text("HTTP/1.0 200 OK\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;


				   copy_text("Elektronika RM\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;


                 /*  copy_text("Expires: Tue, 26 Dec 2014 11:25:11 GMT\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;


                   copy_text("Date: Tue, 26 Dec 2014 02:16:11 GMT\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;


				   copy_text("Cache-Control: max-age=21600\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;*/


				   copy_text("Content-Type: text/html\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;


                   copy_text("Connection: close\r\n\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;

}

Send_Header_HTTP_Not_Found(unsigned short *ix, unsigned char po)
{

 unsigned short kur=0;

                   copy_text("HTTP/1.0 404 Not Found\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;


				   copy_text("Content-Type: text/html\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;

                   copy_text("Connection: close\r\n\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;



}

Send_Header_AUTH_HTTP(unsigned short *ix, unsigned char po)
{

 unsigned short kur=0;

                   copy_text("HTTP/1.1 401 Authorization Required\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;


				   copy_text("WWW-Authenticate: Basic realm=\"Obszar chroniony\"\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;

				   copy_text("Content-Type: text/html\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;

                   copy_text("Connection: close\r\n\r\n", &Operation_DATA_SRAM[_Net_pam_podr]);
				   kur=0;
                   while(Operation_DATA_SRAM[_Net_pam_podr+kur]!=0)
                   {
                      Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((po>>4)*4) + (*ix) + kur] = Operation_DATA_SRAM[_Net_pam_podr+kur];
                      kur++;
				   }
				   *ix += kur;



}


void f_open2()
{
	WskPage[ux]=HttpPagesBuffer[WhichFragmentPage++];
	WskPagesFrag[ux]=0;
	HttpCount=0;
}
void f_open()
{
	int k,x, a1,a2,a3;

	if(HttpPagesBuffer[WhichFragmentPage]==HttpPagesBuffer[WhichFragmentPage-1]) HttpCount++; else HttpCount=1;



if(WhichPage==15)
{

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
		    i= HttpMainScript(&http_itx);
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
else if(WhichPage==6)
{
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
	 		sprintf(buf_p,"\r\nKONIEC: %d  ",http_itx); dbg3(buf_p);      a3=strlen(&buf_rx2[_Size_BufWifi]);  sprintf(&buf_rx2[_Size_BufWifi+a3],"\r\n</html>");
	 	}
	 	else
	 	{
	 		sprintf(buf_p,"\r\nXXX: %d  %d ",http_itx, i); dbg3(buf_p);
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
	HAL_IWDG_Refresh(&hiwdg);
    asm("nop");
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
/*
if(WhichPage==990)
{
   if(WhichFragmentPage>0)
   {   char *ptr1,*ptr2;
       if(WhichFragmentPage==1) adr_flash_Obraz_read=_Obraz_Start;
	   ptr1=&buf_rx2[_Size_BufWifi];
	   AT25SF_3_ReadArray(adr_flash_Obraz_read, ptr1, 2048 );  buf_rx2[_Size_BufWifi+2048]=0;
	   adr_flash_Obraz_read+=2048;
	    if(ptr2=strstr(ptr1,"</html>"))
		{
            *(ptr2+7)=0;
			HttpPagesBuffer[WhichFragmentPage+1]=0;
		}
		else buf_rx2[_Size_BufWifi+2048]=0;
	   HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
   }
}
*/



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
		HttpRot_1();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
	else if(WhichFragmentPage==7)
	{
		HttpRot_2();
		HttpPagesBuffer[WhichFragmentPage]= &buf_rx2[_Size_BufWifi];
	}
}




	WskPage[ux]=HttpParser( HttpPagesBuffer[WhichFragmentPage], HttpCount + HttpCountOffset[HttpCount-1]);
    WhichFragmentPage++;
    WskPagesFrag[ux]=0;


}

int f_read(unsigned char *data, int size, int *it)
{
	int i,cal,k;  cal=strlen(WskPage[ux])-WskPagesFrag[ux];

	if(size<cal) k=size;
	else         k=cal;

	for(i=0;i<k;i++)
	{
		*(data+i)=(WskPage[ux])[WskPagesFrag[ux]+i];
	}
	*(data+i)=0;

	 *it=k;
	 WskPagesFrag[ux]+=k;

	      if((WskPagesFrag[ux]==3*size)&&(strlen(WskPage[ux])==3*size)) return -1;
	 else if((WskPagesFrag[ux]==2*size)&&(strlen(WskPage[ux])==2*size)) return -1;
	 else if((WskPagesFrag[ux]==1*size)&&(strlen(WskPage[ux])==1*size)) return -1;
	 else                                                          return  2;

	 /*    if(ss3==0){  for(i=0;i<size;i++) *(data+i)='A';  *(data+i)=0;    *it=size;  ss3++; }
	else if(ss3==1){  for(i=0;i<size;i++) *(data+i)='B';  *(data+i)=0;    *it=size;  ss3++; }
	else if(ss3==2){  for(i=0;i<size;i++) *(data+i)='C';  *(data+i)=0;    *it=size;  ss3++; }
	else if(ss3==3){  for(i=0;i<100;i++) *(data+i)='X';  *(data+i)=0;    *it=100;   ss3=0;}
*/
	//data=HHHHH;      *it=strlen(HHHHH);

	//sprintf(data,"Markielowski");   *it=strlen(data);
}

void f_close(unsigned char *p1)
{

}

unsigned long zamiana_txt_na_iplong_v2(unsigned char *ptr)
{
     unsigned short a1,a2,a3,a4;

		a1 = wybierz_cyfry(ptr);   while(*ptr!='.') ptr++;     ptr++;
	    a2 = wybierz_cyfry(ptr);   while(*ptr!='.') ptr++;     ptr++;
	    a3 = wybierz_cyfry(ptr);   while(*ptr!='.') ptr++;     ptr++;
	    a4 = wybierz_cyfry(ptr);   while(*ptr!='.') ptr++;     ptr++;

     return IPDOT(a1,a2,a3,a4);
}

int IterationCycleUploadFlash(int nr_packet)
{
	int i,j;
	for(i=1;i<1000;i+=4)
	{
		if(i==nr_packet) return 1;
	}
	for(i=2;i<1000;i+=4)
	{
		if(i==nr_packet) return 2;
	}
	for(i=3;i<1000;i+=4)
	{
		if(i==nr_packet) return 3;
	}
	for(i=4;i<1000;i+=4)
	{
		if(i==nr_packet) return 4;
	}
}

/*void WriteUploadToFlash(unsigned char *pkh)
{
   int PacketSize, NrPacket, PacketSize_dec64;   int i,j,n;   unsigned char *pkh2;

   NrPacket =wybierz_cyfry(pkh);
   i=0; while(*(pkh+i)!='*') i++;   PacketSize=wybierz_cyfry(pkh+i);  sprintf(buf_p,"\r\nC: %d  %d  ",NrPacket, PacketSize); dbg3(buf_p);
   i=0; while(*(pkh+i)!='*') i++;   j=i; while(*(pkh+j)!='_') j++;    i=0; while(*(pkh+j+1+i)!=' '){ buf_p[i]=*(pkh+j+1+i); i++; } buf_p[i]=0; //dbg3(buf_p);

   j=0;
	for(i=0;i<32;i++)
	{
		buf_p[j] =(ASCII_to_hex(buf_p[i+0])<<4) | ASCII_to_hex(buf_p[i+1]);
		i++; j++;
	}  // w   buf_p   md5   od przegladarki


	if(pkh=strstr(pkh,"name=\"file\"\r\n\r\n"))
	{
      for(i=0;i<PacketSize;i++) buf_rx2[_Size_BufWifi+i]= *(pkh+15+i);   buf_rx2[_Size_BufWifi+i]=0;
	}
	hash_MD5(&buf_rx2[_Size_BufWifi], PacketSize, &buf_p[100]);  // w   &buf_p[100]   md5   od sterownika


   j=0;
	for(i=0;i<16;i++){    if(buf_p[i]==buf_p[100+i]) j++;   }
   if(j==16)
   {
   	 pkh2=&buf_rx2[_Size_BufWifi+3400];
   	 base64_decode(&buf_rx2[_Size_BufWifi], pkh2, PacketSize,&PacketSize_dec64);

   	 sprintf(buf_p,"%d  ",PacketSize_dec64); dbg3(buf_p);
   	 sprintf(&buf_p[100],"  %02x%02x%02x%02x%02x",pkh2[0],pkh2[1],pkh2[2],pkh2[3],pkh2[4]); dbg3(&buf_p[100]);


   	if(PacketSize_dec64<511){
   	   //WriteToFlash(NrPacket, PacketSize_dec64, pkh2);

   		if(PacketSize_dec64<=256)  ZapiszFlash_3_Zewn(&adr_flash_Obraz, pkh2, PacketSize_dec64);
   		else
   		{
   			ZapiszFlash_3_Zewn(&adr_flash_Obraz, pkh2, 256);
   			ZapiszFlash_3_Zewn(&adr_flash_Obraz, &pkh2[256], PacketSize_dec64-256);
   		}

   	}

   }


}*/

void WriteUploadToFlash(unsigned char *pkh)
{
   int PacketSize, NrPacket, PacketSize_dec64;   int i,j,n;   unsigned char *pkh2;

   NrPacket =wybierz_cyfry(pkh);
   i=0; while(*(pkh+i)!='*') i++;   PacketSize=wybierz_cyfry(pkh+i);  sprintf(buf_p,"\r\nC: %d  %d  ",NrPacket, PacketSize); dbg3(buf_p);
   i=0; while(*(pkh+i)!='*') i++;   j=i; while(*(pkh+j)!='_') j++;    i=0; while(*(pkh+j+1+i)!=' '){ buf_p[i]=*(pkh+j+1+i); i++; } buf_p[i]=0; //dbg3(buf_p);

   j=0;
	for(i=0;i<32;i++)
	{
		buf_p[j] =(ASCII_to_hex(buf_p[i+0])<<4) | ASCII_to_hex(buf_p[i+1]);
		i++; j++;
	}  // w   buf_p   md5   od przegladarki


	if(pkh=strstr(pkh,"name=\"file\"\r\n\r\n"))
	{
      for(i=0;i<PacketSize;i++) buf_rx2[_Size_BufWifi+i]= *(pkh+15+i);   buf_rx2[_Size_BufWifi+i]=0;
	}
	hash_MD5(&buf_rx2[_Size_BufWifi], PacketSize, &buf_p[100]);  // w   &buf_p[100]   md5   od sterownika


   j=0;
   for(i=0;i<16;i++){    if(buf_p[i]==buf_p[100+i]) j++;   }
   if(j==16)
   {
	   if(PacketSize<=680)
	   {
   	       pkh2=&buf_rx2[_Size_BufWifi];

   	   	   if(PacketSize<=256)  ZapiszFlash_3_Zewn(&adr_flash_Obraz, pkh2, PacketSize);
   	   	   else if((PacketSize>256)&&(PacketSize<=(256*2)))
   	   	   {
   	   	   	    ZapiszFlash_3_Zewn(&adr_flash_Obraz, pkh2, 256);
   	   	        ZapiszFlash_3_Zewn(&adr_flash_Obraz, &pkh2[256], PacketSize-256);
   	   	   }
   	   	   else if(PacketSize>(256*2))
   	   	   {
   	   	   	    ZapiszFlash_3_Zewn(&adr_flash_Obraz, pkh2, 256);
   	   	        ZapiszFlash_3_Zewn(&adr_flash_Obraz, &pkh2[256], 256);
   	   	        ZapiszFlash_3_Zewn(&adr_flash_Obraz, &pkh2[256*2], PacketSize-256*2);
   	   	   }

   	   	   if(PacketSize<680)
   	   	   {
   	   		   AT25SF_3_ReadArray(_Obraz_Start, &buf_rx2[_Size_BufWifi], 4000 );
   	   		   AT25SF_3_ReadArray(_Obraz_Start+4000, buf_p, 96 );
   	   	       AT25SF_3_BlockErase(_Obraz_Start); HAL_Delay(500);

   	   	       n=_Obraz_Start;
   	   	       i= adr_flash_Obraz-n-3;
   	   	       buf_rx2[_Size_BufWifi+0]=i>>16;
   	   	       buf_rx2[_Size_BufWifi+1]=i>>8;
   	   	       buf_rx2[_Size_BufWifi+2]=i>>0;

   	   	       adr_flash_Obraz=_Obraz_Start;
   	   	       for(i=0;i<15;i++)  ZapiszFlash_3_Zewn(&adr_flash_Obraz, &buf_rx2[_Size_BufWifi+i*256], 256);

   	   	       for(i=0;i<(256-96);i++)  buf_rx2[_Size_BufWifi+i]= buf_rx2[_Size_BufWifi+15*256+i];
   	   	       for(i=0;i<96;i++)        buf_rx2[_Size_BufWifi+(256-96)+i]= buf_p[i];
   	   	       AT25SF_3_PageProgram(adr_flash_Obraz, &buf_rx2[_Size_BufWifi], 256 );


   	   	       for(i=0;i<_IleLor;i++)
   	   	       {
   	   	          Const.s_Map[0].x[i]=0;
   	   	          Const.s_Map[0].y[i]=0;
   	   	       }
   	   	       Const.s_Map[0].resX=0;
   	   	       Const.s_Map[0].resY=0;

   	   	       ZapiszFlash();

   	   	   }
	   }
   }

}

void netstackTCPIPProcess(unsigned int len, tcpip_hdr* packet)   //tcpip
{
    uint32_t chk_p;   unsigned char *pkh,*pkh1,*pkh2,buf_p[150];  int port_e,is;  unsigned long IP_ser_nad;
	unsigned short asx=0,ksx=0,iterix;  char send_E;

 port_e = wybierz_cyfry(Const.s_Email[0].port);     pkh= &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN];
//################################3 --SMTP-- #############################################
if(packet->tcp.srcport ==   (  ((port_e>>8)&0x00FF)  |  ((port_e<<8)&0xFF00))   )   //SMTP
{ if(packet->tcp.destport==(  ((PORT_MAIL_S>>8)&0x00FF)  |  ((PORT_MAIL_S<<8)&0xFF00))   ){

    if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t'))
    {
    	ZapisSesjiSMTP(&Operation_DATA_SRAM[_Net_stack_start], len, &adr_flash_SesjaSMTP);
    }

	IP_ser_nad = zamiana_txt_na_iplong_v2(Const.s_Email[0].ip);

    asx = len - (ETH_HEADER_LEN+IP_HEADER_LEN+((packet->tcp.headlen>>4)*4));
	ksx=0;
    while(Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((packet->tcp.headlen>>4)*4) + ksx])
    {
        if(asx==0) break;
	    asx--;
		Operation_DATA_SRAM[_Net_pam_podr+ksx]=Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + ksx];
        ksx++;	if(ksx>(_SIZE__Net_pam_podr-1)){ dbg3("\r\nksx>_SIZE__Net_pam_podr !!!!"); break;}
    }
	Operation_DATA_SRAM[_Net_pam_podr+ksx]=0;

    if(Operation_DATA_SRAM[_Net_pam_podr]>0x0D)
	{
	      dbg3("->"); dbg3_roz(&Operation_DATA_SRAM[_Net_pam_podr]);  Operation_DATA_SRAM[_Net_pam_podr+99]=0;
	      snprintf(info_email,60,&Operation_DATA_SRAM[_Net_pam_podr]);
    }


	if(packet->tcp.flags==0x12)
    {
		if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')) debug_smtp=1;
		delay_funkc[28]=0; retremail=0;  tcpSend33( IpMyConfig.ip, IP_ser_nad, PORT_MAIL_S, port_e, 0, &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN] , 0 , 1);
		debug_smtp=0;
    }
	else if(packet->tcp.flags==0x18)
    {

	  if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"220"))
	  {
           sprintf(pkh,"EHLO STMPROJ\r\n");     if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o')) RapAddr=0;
           send_E=1;                            if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')) RapAddr=0;
           send_E=1;                            if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')) RapAddr=0;

           delay_funkc[28]=0; //koniec retr dla email

	  }
	  else if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"250-"))
	  {
	     sprintf(pkh,"AUTH LOGIN\r\n");
	     send_E=1;

	  }
	  else if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"334 VXNlcm5hbWU6"))
	  {
		base64_encode(Const.s_Email[0].nad, buf_p, strlen(Const.s_Email[0].nad));
	    sprintf(pkh,"%s\r\n",buf_p);
	    send_E=1;

	  }
	  else if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"334 UGFzc3dvcmQ6"))
	  {
	    base64_encode(Const.s_Email[0].has, buf_p, strlen(Const.s_Email[0].has));
	    sprintf(pkh,"%s\r\n",buf_p);
	    send_E=1;

	  }
	  else if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"235"))
	  {
	    sprintf(pkh,"MAIL From:<%s>\r\n",Const.s_Email[0].nad);
	    send_E=1;

	  }
	  else if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"250"))
	  {
         if(email_enc==1)
         {
  		          if(wysylka_mail==1) sprintf(buf_p,Const.s_Email[0].odb1);
  		     else if(wysylka_mail==2) sprintf(buf_p,Const.s_Email[0].odb2);
  		     else if(wysylka_mail==3) sprintf(buf_p,Const.s_Email[0].odb3);
  		     else if(wysylka_mail==4) sprintf(buf_p,Const.s_Email[0].odb4);
  		     else if(wysylka_mail==5) sprintf(buf_p,Const.s_Email[0].odb5);
  		     else if(wysylka_mail==6) sprintf(buf_p,Const.s_Email[0].odb6);
  		     else if(wysylka_mail==7) sprintf(buf_p,Const.s_Email[0].odb7);
  		     else if(wysylka_mail==8) sprintf(buf_p,Const.s_Email[0].odb8);
  		     else                     sprintf(buf_p,Const.s_Email[0].odb1);

        	sprintf(pkh,"RCPT To:<%s>\r\n",buf_p);
        	send_E=1;
            email_enc=2;

         }
         else if(email_enc==2)
         {
  		    sprintf(pkh,"DATA\r\n");
  		    send_E=1;
  		  email_enc++;
         }
         else if(email_enc==9)
         {
  		    sprintf(pkh,"quit\r\n");
  		    send_E=1;
            email_enc=0;
         }

	  }
	  else if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"221"))
	  {
		   if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')) debug_smtp=1;
		   delay_funkc[28]=0; retremail=0;   tcpSend33( IpMyConfig.ip, IP_ser_nad, PORT_MAIL_S, port_e, 0, &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN] , 0 , ksx);
		   debug_smtp=0;
		   dbg3("\r\nWyslano Email przez ENC");  delay_funkc[17]=1;     delay_funkc[28]=0; //koniec retr dla email
           wybor_bitowy&=~0x1000;  //zerujemy powtorke
           send_E=0;
           delay_funkc[13]=0;
                if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){  Const.s_Rap[0].start_po= adr_flash_read;  delay_funkc[6]=1;/*zapisz flash*/  }
           else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')){  Const.s_Rap[0].start_zd= RapAddr;         delay_funkc[6]=1;/*zapisz flash*/  }

	  }
	  else if(strstr(&Operation_DATA_SRAM[_Net_pam_podr],"354"))
	  {
		    sprintf(pkh,"From: %s\r\n",Const.s_Email[0].nad);
		    send_E=1;
		    email_enc++;

	  }

      if(send_E==1)
      {  send_E=0;
         is=0; // dbg3("    "); dbg3(pkh);  dbg3("    ");
      	 while(Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN + is]!=0) is++;
      	 if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')) debug_smtp=1;
      	delay_funkc[28]=1; retremail=1;  tcpSend33( IpMyConfig.ip, IP_ser_nad, PORT_MAIL_S, port_e, is, &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN] , 0 , ksx);
      	 debug_smtp=0;
      }


    }
	else if(packet->tcp.flags==0x10)
    {
        if(email_enc==4)
        {
 		    sprintf(pkh,"To: %s\r\n",Const.s_Email[0].odb1);
 		   send_E=1;
 		   email_enc++;
        }
        else if(email_enc==5)
        {
 		    sprintf(pkh,"Subject: %s\r\n",email_tytul);
 		   send_E=1;
 		       if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i')){ email_enc=500;  adr_flash_read= Const.s_Rap[0].start_po; }
 		  else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z')){ email_enc++;    RapAddr= Const.s_Rap[0].start_zd;    }
 		  else email_enc++;
        }


        else if(email_enc==500)
        {
 		    sprintf(pkh,"Content-Type: text/plain\r\n");
 		   send_E=1;
 		   email_enc++;
        }
        else if(email_enc==501)
        {
 		    sprintf(pkh,"Content-Disposition: attachment; filename=POMIARY.txt\r\n");
 		   send_E=1;
 		   email_enc=6;
        }


        else if(email_enc==6)
        {
 		    sprintf(pkh,"\r\n");
 		   send_E=1;
 		   email_enc++;
        }
        else if(email_enc==7)
        {
        	if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t'))  sprintf(pkh,"E`mail testowy");
        	else if((email_tytul[0]=='R')&&(email_tytul[1]=='a')&&(email_tytul[2]=='p')&&(email_tytul[3]=='o'))
        	{
        		email_enc--;
        		if(RaportStanu(&RapAddr,1)==1) email_enc++;
        		sprintf(pkh, &buf_rx2[_Size_BufWifi]);
        	}
        	else if((email_tytul[0]=='R')&&(email_tytul[1]=='e')&&(email_tytul[2]=='j')&&(email_tytul[3]=='Z'))
        	{
        		email_enc--;
        		if(OdczytZdarzenSmtp(&RapAddr)==2) email_enc++;   delay_funkc[13]=1;   delay_funkc[28]=0; //koniec retr dla email
        		sprintf(pkh, &buf_rx2[_Size_BufWifi]);
        	}
        	else if((email_tytul[0]=='P')&&(email_tytul[1]=='o')&&(email_tytul[2]=='m')&&(email_tytul[3]=='i'))
        	{
        		email_enc--;
        		if(OdczytRejTempSmtp(0)==2) email_enc++;      delay_funkc[13]=1;       delay_funkc[28]=0; //koniec retr dla email
        		sprintf(pkh, &buf_rx2[_Size_BufWifi]);
        	}
        	else sprintf(pkh,"%s\r\n",&b_mail[_Dlg_buf_Mail*(wysylka_mail-1)]);
        	//dbg3_roz(pkh);
        	sprintf(buf_p,"\r\nTTT: %d", RapAddr ); dbg3(buf_p);// dbg3("\r\nTRESC... ");
        	send_E=1;
 		   email_enc++;
        }
        else if(email_enc==8)
        {
 		    sprintf(pkh,"\r\n.\r\n");   delay_funkc[28]=0; //koniec retr dla email
 		   send_E=1;
 		   email_enc++;
        }

        if(send_E==1)
        {  send_E=0;
           is=0;
        	 while(Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN + is]!=0) is++;
        	 if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')) debug_smtp=1;

        	 delay_funkc[28]=1; retremail=1;   tcpSend33( IpMyConfig.ip, IP_ser_nad, PORT_MAIL_S, port_e, is, &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN] , 0 , ksx);
        	 debug_smtp=0;


        }



    }
	else if(packet->tcp.flags==0x11)
    {
		if((email_tytul[0]=='T')&&(email_tytul[1]=='e')&&(email_tytul[2]=='s')&&(email_tytul[3]=='t')) debug_smtp=1;
		 tcpSend22( IpMyConfig.ip, IP_ser_nad, PORT_MAIL_S, port_e, 0, &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN] , 0 , 1);
		debug_smtp=0;
    }


}}


//#############  -- MODBUS  --- ####################################################################
/*
if(packet->tcp.destport ==  (((502>>8)&0x00FF)  |  ((502<<8)&0xFF00))   )   //MODBUS
{
if(packet->tcp.flags==0x02)
{
   ux =  search_struct_HTTP_free();
   if(ux<99)
   {
	 HTTP[ux].port = packet->tcp.srcport;
	 HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + 1);
	 HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + 1);
	 HTTP[ux].vit = HTTP_CACHE_TIME_TO_LIVE;
     HTTP[ux].kup=0;
	 HTTP[ux].wsk=0;
     HTTP[ux].get_query[0]=0;
     modbus=0;

	 tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 1);
   }

}
else if( (packet->tcp.flags==0x10) )
{
  ux = search_struct_HTTP(&packet->tcp.srcport);


   if(ux<99)
   {
		iterix=0;    HTTP[ux].vit = HTTP_CACHE_TIME_TO_LIVE;


		if(modbus==1)
		{
		    HTTP[ux].seq = packet->tcp.seqno;
	        HTTP[ux].ack = packet->tcp.ackno;
			packet->tcp.flags=0x11;
			tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 0);

		}

		if(HTTP[ux].kup==0)        //*************************************************************
		{
		  if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
		  {
		    HTTP[ux].seq = packet->tcp.seqno;
	        HTTP[ux].ack = packet->tcp.ackno;  // rrprintf("kup=%d\r\n",HTTP[ux].kup); //wykomentowane dla odswiezania bez przeladowania
			HTTP[ux].kup=5;
          }
	    }
		else if(HTTP[ux].kup==1)  //*************************************************************
		{
            if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
			{
 				replay_packet[ux]=200;

				send_code_html(&iterix, &HTTP[ux].kup, &packet->tcp.flags, packet->tcp.headlen);

				HTTP[ux].seq = packet->tcp.seqno;
	            HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);


                if(iterix<200)     tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 0);
                else           {tcpSend_http( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 0, ux); }

                replay_packet[ux]=0;

				//rprintf("kup=1\r\n");

            }


		}

   }
}
else if( ((packet->tcp.flags&0x04)==0x04) )
{
   ux = search_struct_HTTP(&packet->tcp.srcport);

  // if(ux<99) HTTP[ux].vit = 0;

  HTTP[ux].vit = 0x00;
	   tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 1);


}
else if( ((packet->tcp.flags&0x01)==0x01) )
{

   ux = search_struct_HTTP(&packet->tcp.srcport);


  // if(ux<99)
  // {
     //if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
	// {
	   HTTP[ux].vit = 0x00;
	   tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 1);
       replay_packet[ux]=200;
	 //}
   //}

}
else if( (packet->tcp.flags==0x18) )
{
    ux = search_struct_HTTP(&packet->tcp.srcport);


	if(ux<99)
	{
		  // rprintf("1");


		//for(is=0;is<(2000-(ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN));is++) Buffppp[ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN + is]=0;



	  if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
	  {

		pkh1 = &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4)];

        MDB[0].Transaction_ID =   TwoPonitToInt(pkh1);//0x00ff&(((unsigned short)(*(pkh1+0)))<<0) | 0xff00&(((unsigned short)(*(pkh1+1)))<<8);
        MDB[0].Protocol_ID    =   TwoPonitToInt(pkh1+2);//0x00ff&(((unsigned short)(*(pkh1+2)))<<0) | 0xff00&(((unsigned short)(*(pkh1+3)))<<8);
		MDB[0].Lenght         =   TwoPonitToInt(pkh1+4);//0x00ff&(((unsigned short)(*(pkh1+4)))<<0) | 0xff00&(((unsigned short)(*(pkh1+5)))<<8);
		MDB[0].Unit_ID        =   *(pkh1+6);
		MDB[0].FCode          =   *(pkh1+7);
		MDB[0].Data[0]        =   *(pkh1+8);
		MDB[0].Data[1]        =   *(pkh1+9);
		MDB[0].Data[2]        =   *(pkh1+10);
		MDB[0].Data[3]        =   *(pkh1+11);
		MDB[0].Data[4]        =   *(pkh1+12);
		MDB[0].Data[5]        =   *(pkh1+13);
		MDB[0].Data[6]        =   *(pkh1+14);
		MDB[0].Data[7]        =   *(pkh1+15);
		MDB[0].Data[8]        =   *(pkh1+16);


		if(MDB[0].FCode==0x01)  //Read Coils
		{
		   MDB[0].Data[0]=0x01;
		   //MDB[0].Data[1]=0x0f&Var.value_wy[0];
		   iterix=8+2;

		}
		else if(MDB[0].FCode==0x05)  //Write Single Coil
		{
		   if(MDB[0].Data[0]==0)  //Output Address Hi (nr_pk)
		   {
		      if(((MDB[0].Data[1]+1)>0)&&((MDB[0].Data[1]+1)<5))   //Output Address Lo  (nr_pk)
			  {
			     char nrpk;  nrpk=MDB[0].Data[1];

				  if(MDB[0].Data[3]==0)       //Output Value Lo
				  {
				      if(MDB[0].Data[2]==0xff)  //Output Value Hi
					  {
					     // Var.value_wy_zal[nrpk/16] |= (1<<(nrpk-((nrpk/16)*16)));   global_pk_str|=0x01;
                          //Var.value_wy[nrpk/16] |= (1<<(nrpk-((nrpk/16)*16)));      zadaj_LAN=2;
                          iterix=8+4;
					  }
					  else if(MDB[0].Data[2]==0x00)  //Output Value Hi
					  {
					      //Var.value_wy_wyl[nrpk/16] |= (1<<(nrpk-((nrpk/16)*16)));   global_pk_str|=0x01;
                          //Var.value_wy[nrpk/16] &= ~(1<<(nrpk-((nrpk/16)*16)));     zadaj_LAN=2;
                          iterix=8+4;
					  }
					  else
					  {
		                 MDB[0].FCode|=0x80;
				         MDB[0].Data[0]=0x02;
		                 iterix=8+1;
					  }
				  }
			  }
		      else
		      {
		         MDB[0].FCode|=0x80;
				 MDB[0].Data[0]=0x02;
		         iterix=8+1;
		      }
		   }
		   else
		   {
		        MDB[0].FCode|=0x80;
				MDB[0].Data[0]=0x02;
		        iterix=8+1;
		   }





		}
		else if(MDB[0].FCode==0x02)  //Read Discrete Inputs
		{
		   MDB[0].Data[0]=0x01;
		   //MDB[0].Data[1]=0x7f&Var.value_we[0];
		   iterix=8+2;

		}



		else if(MDB[0].FCode==0x03)  //Read Holding Registers
		{

		   MDB[0].Data[0]=5*2;   //Byte Count
*/
         /*  wartTemp_odwrotnie(&buf_p[0],Var.value[0]);    to= 10*(0x0f&buf_p[1])+(0x0f&buf_p[2]);  if(buf_p[0]=='-') to|=0x80;  ta=0x0f&buf_p[4];
           MDB[0].Data[1]=to;   //Register value Hi
		   MDB[0].Data[2]=ta;   //Register value Lo

		   wartTemp_odwrotnie(&buf_p[0],Var.value[1]);    to= 10*(0x0f&buf_p[1])+(0x0f&buf_p[2]);  if(buf_p[0]=='-') to|=0x80;  ta=0x0f&buf_p[4];
           MDB[0].Data[3]=to;   //Register value Hi
		   MDB[0].Data[4]=ta;   //Register value Lo

		   wartTemp_odwrotnie(&buf_p[0],Var.value[2]);    to= 10*(0x0f&buf_p[1])+(0x0f&buf_p[2]);  if(buf_p[0]=='-') to|=0x80;  ta=0x0f&buf_p[4];
           MDB[0].Data[5]=to;   //Register value Hi
		   MDB[0].Data[6]=ta;   //Register value Lo

		   wartTemp_odwrotnie(&buf_p[0],Var.value[3]);    to= 10*(0x0f&buf_p[1])+(0x0f&buf_p[2]);  if(buf_p[0]=='-') to|=0x80;  ta=0x0f&buf_p[4];
           MDB[0].Data[7]=to;   //Register value Hi
		   MDB[0].Data[8]=ta;   //Register value Lo

		   wartTemp_odwrotnie(&buf_p[0],Var.value[4]);    to= 10*(0x0f&buf_p[1])+(0x0f&buf_p[2]);  if(buf_p[0]=='-') to|=0x80;  ta=0x0f&buf_p[4];
           MDB[0].Data[9]=to;   //Register value Hi
		   MDB[0].Data[10]=ta;   //Register value Lo
*/
/*
            iterix=8+11;



		}
		else
		{
		    MDB[0].FCode|=0x80;
			MDB[0].Data[0]=0x01;
		    iterix=8+1;
		}


	      composition(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4)], &MDB[0].start+1,iterix, 0,0,  0,0,  0,0);

		       packet->tcp.flags=0x18;
			   HTTP[ux].kup=0;

			   HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + 12);
               HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

			    tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 12);
                modbus=1;


      }
    }
}

}*/


//################################3 --HTTP-- #############################################
 if(packet->tcp.destport ==  (((PORT>>8)&0x00FF)  |  ((PORT<<8)&0xFF00))   )   //HTTP
 {

	 if(packet->tcp.flags==0x02)
	 {
		 if(http_wstrz==0);// all_HTTP_clear();//To powofduje ze na moim kompie z routerm cyfr. polsatu nie ma przerwy ¿adania GET TME.txt  za to blokada odsiwezania jesli szybko klikniemy kilka razy na zalacz/wylacz przekaznik
		 else return;
		// if(if_all_HTTP_free()==0) return;

		   ux =  search_struct_HTTP_free();
		   if(ux<99)
		   {
			 HTTP[ux].port = packet->tcp.srcport;
			 HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + 1);
			 HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + 1);
			 HTTP[ux].vit = HTTP_CACHE_TIME_TO_LIVE;
             HTTP[ux].kup=0;
			 HTTP[ux].wsk=0;
             HTTP[ux].get_query[0]=0;
             ss3=0;

			 tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 1);
	       }

	 }
	   else if( (packet->tcp.flags==0x10) )
	   {
          ux = search_struct_HTTP(&packet->tcp.srcport);


		   if(ux<99)
		   {
				iterix=0;    HTTP[ux].vit = HTTP_CACHE_TIME_TO_LIVE;

				if(HTTP[ux].kup==0)        //*************************************************************
				{
				  if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
				  {
				    HTTP[ux].seq = packet->tcp.seqno;
			        HTTP[ux].ack = packet->tcp.ackno;   //sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup);  dbg3(buf_p);//wykomentowane dla odswiezania bez przeladowania
					HTTP[ux].kup=5;
					 if(http_wstrz==0);  else return;

                  }
			    }
				else if(HTTP[ux].kup==1)  //*************************************************************
				{
	                if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
					{
		 				replay_packet[ux]=200;
		 			    http_wstrz=1; delay_funkc[22]=1;  port_ux=ux; dbg3(".");
						send_code_html(&iterix, &HTTP[ux].kup, &packet->tcp.flags, packet->tcp.headlen);

						HTTP[ux].seq = packet->tcp.seqno;
			            HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

			           // dbg3("x");
                      if(iterix<200)  tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 0);
                      else{           tcpSend_http( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 0, ux);
                                      replay_packet[ux]=0;
                      }




	                    //rprintf("kup=1\r\n");

                  }


				}
				else if(HTTP[ux].kup > 4)  //*************************************************************
				{
	                if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
					{
						if(http_wstrz==0);  else return;

						asx = len - (ETH_HEADER_LEN+IP_HEADER_LEN+((packet->tcp.headlen>>4)*4));
				        ksx=0;
			            while(Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((packet->tcp.headlen>>4)*4) + ksx])
			            {
                         if(asx==0) break;
				           asx--;
                         ksx++;
			            }
				        Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + ksx]=0;
				        pkh1 = &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4)];

						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
			            HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) );


						if(HTTP[ux].kup==5)
					    {
							if(pkh=strstr(pkh1,"GET /lpc.cgi")){ CGI_callback(pkh,&select_HTML); /*sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup); dbg3(buf_p);*/ }
						}
						else
						{
			              if((HTTP[ux].kup%2)==0)
                          {
							   	CGI_callback(pkh1,&select_HTML);
                             iterix=0;
						       tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);
						      // sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup);  dbg3(buf_p);
                          }
                          else
                          {
                              CGI_callback(pkh1,&select_HTML);
							  //  sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup);  dbg3(buf_p);

                          }

						}

						//rrprintf("kup=%d\r\n",HTTP[ux].kup);
                        HTTP[ux].kup++;


                  }


				}
				else if(HTTP[ux].kup==4)  //*************************************************************
				{
				      HTTP[ux].seq = packet->tcp.seqno;
			          HTTP[ux].ack = packet->tcp.ackno;
				}

         }
	   }
	   else if( ((packet->tcp.flags&0x04)==0x04) )
	   {
           ux = search_struct_HTTP(&packet->tcp.srcport);

          // if(ux<99) HTTP[ux].vit = 0;

		  HTTP[ux].vit = 0x00;
			   tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 1);


	   }
	  // else if( ((packet->tcp.flags&0x01)==0x01) )
	   else if(packet->tcp.flags==0x11)
	   {

           ux = search_struct_HTTP(&packet->tcp.srcport);

            // dbg3(" Close...");
		  // if(ux<99)
		   //{
		    // if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
			 //{
			   HTTP[ux].vit = 0x00;
			   tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , 1);
               replay_packet[ux]=200;
              // dbg3("ok ");

      	    	if(WhichPage==2)
      	    	{
      	    	   dbg3("\r\n[WWDG]");    //Jump to BOOT

      	      	  hwwdg.Instance = WWDG;
      	      	  hwwdg.Init.Prescaler = WWDG_PRESCALER_1;
      	      	  hwwdg.Init.Window = 64;
      	      	  hwwdg.Init.Counter = 64;
      	      	  hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
      	      	  HAL_WWDG_Init(&hwwdg);

      	      	   while(1);
      	    	}
      	    	if(http_wstrz==1){  if(port_ux==ux){ port_ux=-1; http_wstrz=0;  delay_funkc[22]=0; } } //  all_HTTP_clear();

			// }
		   //}

		}
		else if( (packet->tcp.flags==0x18) )
		{
           ux = search_struct_HTTP(&packet->tcp.srcport);

		   if(ux<99)
		   {
			  HTTP[ux].vit = HTTP_CACHE_TIME_TO_LIVE;

			  if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
			  {
					asx = len - (ETH_HEADER_LEN+IP_HEADER_LEN+((packet->tcp.headlen>>4)*4));
					ksx=0;
				    while(Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN+((packet->tcp.headlen>>4)*4) + ksx])
				    {
	                    if(asx==0) break;
					    asx--;
	                    ksx++;
				    }
					Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + ksx]=0;
					pkh1 = &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4)];

					//sprintf(buf_p,"ZZZ: %d:%d", ksx,asx); dbg3(buf_p);

					if(pkh=strstr(pkh1,"GET /lpc.cgi"))
					{
	                   //rprintf("\r\n");  rprintf(pkh1);  rprintf("\r\n");

						if(strstr(pkh1,"GET /lpc.cgi/testemail")) delay_funkc[14]=1;
						else  CGI_callback(pkh,&select_HTML);

							NrPageToString(WhichPage, buf_p);

					        iterix=0;
							Send_Header_HTTP(&iterix, packet->tcp.headlen);
							if((WhichPage==3)||(WhichPage==997)) sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],"<html><head><meta http-equiv=\"refresh\" content=\"0;url=../%s\"></head><body></body></html>",buf_p);
							else if(WhichPage==82)               sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],"<html><head><meta http-equiv=\"refresh\" content=\"0;url=../%s\"></head><body bgcolor=\"bbbbbb\"></body></html>",buf_p);
							else                                 sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],"<html><head><meta http-equiv=\"refresh\" content=\"0;url=../%s\"></head><body bgcolor=\"000000\"></body></html>",buf_p);

		                       iterix += strlen( &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

						       packet->tcp.flags=0x19;
							   HTTP[ux].kup=0;

							   HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				               HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

							   tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);


					}
					else if((pkh=strstr(pkh1,"GET /favic"))||(pkh=strstr(pkh1,"GET /theme")))
					{
						     iterix=0;
					         Send_Header_HTTP_Not_Found(&iterix,packet->tcp.headlen);

							 sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],"<html><head><title>404 Not Found</title></head><body>404 Not Found</body></html>");



		                       iterix += strlen( &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

						       packet->tcp.flags=0x19;
							   HTTP[ux].kup=0;

							   HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				               HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

							    tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);

							    dbg3("\r\nfavic/theme ");


					}
					else if(pkh=strstr(pkh1,"POST /uploader.php"))
					{


						if(pkh=strstr(pkh1,"POST /uploader.php_Start"))
						{
							adr_flash_Obraz=_Obraz_Start;
							AT25SF_3_BlockErase(adr_flash_Obraz);   HAL_Delay(200);
							AT25SF_3_PageProgram(adr_flash_Obraz, "000", 3 );   adr_flash_Obraz+=3;
							//dbg3("\r\nPOST /uploader.php_Start");
							WriteUploadToFlash(pkh);

						}
						else if(pkh=strstr(pkh1,"POST /uploader.php_End"))
						{
							WriteUploadToFlash(pkh); // dbg3("\r\nPOST /uploader.php_End");

						}
						else if(pkh=strstr(pkh1,"POST /uploader.php_"))
						{
							WriteUploadToFlash(pkh);  //dbg3("\r\nPOST /uploader.php_");

						}

						iterix=0;
					    Send_Header_HTTP(&iterix,packet->tcp.headlen);
				        sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "Dziekuje");
						iterix += strlen(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

						packet->tcp.flags=0x19;
						HTTP[ux].kup=0;

						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				        HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

						tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), ((packet->tcp.headlen>>4)*4) + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);


					}
/*#ifdef _LoRa
					else if(pkh=strstr(pkh1,"GET /Sensagh.txt"))
					{

					   iterix=0;
					    Send_Header_HTTP(&iterix,packet->tcp.headlen);

					   // Lora[0]&=~0x01;   Lora[0]&=~0x02;   Lora[0]&=~0x04;   lifeLora[0]=0;
					   //Lora[1]|=0x01;  Lora[1]|=0x02;   Lora[1]&=~0x04;   lifeLora[1]=10;



for(i=0;i<_Ile_Czujek;i++)
{
	if(lifeLora[i]==0) ni=0;
    else if(lifeLora[i]==_Czas_Zycia_Czujek+100) ni=2;
    else               ni=1;

    sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "%d%d%d%d ", (Lora[i]>>0)&0x01, (Lora[i]>>1)&0x01, (Lora[i]>>2)&0x01, ni );
    iterix += strlen(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);
}

						packet->tcp.flags=0x19;
						HTTP[ux].kup=0;

						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				        HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);  //rprintf(ptxt);

						tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), ((packet->tcp.headlen>>4)*4) + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);
						 dbg3(" Sensagh.txt");


					}
					else if(pkh=strstr(pkh1,"GET /mobile/aghpk"))
					{
						ni =  100*(0x0f&(*(pkh+17)))+10*(0x0f&(*(pkh+18)))+(0x0f&(*(pkh+19)));

					   iterix=0;
					    Send_Header_HTTP(&iterix,packet->tcp.headlen);

				        sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "<img src='data:image/png;base64,'>");

						iterix += strlen(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

						packet->tcp.flags=0x19;
						HTTP[ux].kup=0;

						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				        HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

						tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), ((packet->tcp.headlen>>4)*4) + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);
						//dbg3("\r\nmobile/pk ");
                       sprintf(buf_p,"\r\nAGH PK %d ",ni); dbg3(buf_p);

                       i_pwr[ni-1]=4;  Lora_Send_PK(ni);   //max power TX
                       Lora[ni-1]=0;
                       ZapisZdarzenia(_ZdaLoRa,_LoObiektOdw,ni);
                       lifeLora[ni-1]=0;

                       if(ni==5)
                       {
                           Lora[22-1]=0;
                           lifeLora[22-1]=_Czas_Zycia_Czujek;

                           Lora[23-1]=0;
                           lifeLora[23-1]=_Czas_Zycia_Czujek;
                       }


					}
#endif*/
//					else if(pkh=strstr(pkh1,"GET /SensLora.txt"))
//					{
//                        unsigned short ppm;
//						pkh+=18;  j=0;
//                        for(i=0;i<_IleLor;i++)
//                        {
//                        	ppm= wybierz_cyfry(pkh);
//                        	if(ppm!=Const.s_Map[0].x[i]) j=1;
//                        	Const.s_Map[0].x[i]=ppm;
//                        	while(1){ pkh++; if(*pkh=='_'){ pkh++; break; } }
//
//                        	ppm= wybierz_cyfry(pkh);
//                        	if(ppm!=Const.s_Map[0].y[i]) j=1;
//                        	Const.s_Map[0].y[i]=ppm;
//                        	while(1){ pkh++; if(*pkh=='_'){ pkh++; break; } }
//                        }
//                        if(j==1) ZapiszFlash();
//
//
//					   iterix=0;
//					    Send_Header_HTTP(&iterix,packet->tcp.headlen);
//
//					    if((wybor2_bitowy&0x0040)>0){ wybor2_bitowy&=~0x0040; sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "+13.40+22.51-30.20+43.40+52.51-10.20+23.40+32.51-10.20+23.40+32.51-10.20"); }
//					    else                        { wybor2_bitowy|=0x0040; sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],  "+24.51+33.60-09.11+24.51+33.60-09.11+24.51+33.60-09.91+24.51+45.60-09.71"); }
//					    iterix += strlen(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);
//
//						packet->tcp.flags=0x19;
//						HTTP[ux].kup=0;
//
//						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
//				        HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);  //rprintf(ptxt);
//
//						tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), ((packet->tcp.headlen>>4)*4) + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);
//      //  sprintf(buf_p,"\r\n%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,",Const.s_Map[0].x[0],Const.s_Map[0].y[0],Const.s_Map[0].x[1],Const.s_Map[0].y[1],Const.s_Map[0].x[2],Const.s_Map[0].y[2],Const.s_Map[0].x[3],Const.s_Map[0].y[3],Const.s_Map[0].x[4],Const.s_Map[0].y[4],Const.s_Map[0].x[5],Const.s_Map[0].y[5],Const.s_Map[0].x[6],Const.s_Map[0].y[6],Const.s_Map[0].x[7],Const.s_Map[0].y[7],Const.s_Map[0].x[8],Const.s_Map[0].y[8],Const.s_Map[0].x[9],Const.s_Map[0].y[9]); dbg3(buf_p);
//                     dbg3("Mapa.txt ");
//
//					}
					else if(pkh=strstr(pkh1,"GET /SensLora.txt"))
					{
                        unsigned short ppm;
						pkh+=18;  j=0;
                        for(i=0;i<_IleLor;i++)
                        {
                        	ppm= wybierz_cyfry(pkh);
                        	if(ppm!=Const.s_Map[0].x[i]) j=1;
                        	Const.s_Map[0].x[i]=ppm;
                        	while(1){ pkh++; if(*pkh=='_'){ pkh++; break; } }

                        	ppm= wybierz_cyfry(pkh);
                        	if(ppm!=Const.s_Map[0].y[i]) j=1;
                        	Const.s_Map[0].y[i]=ppm;
                        	while(1){ pkh++; if(*pkh=='_'){ pkh++; break; } }
                        }
                        if(j==1) ZapiszFlash();


					   iterix=0;
					    Send_Header_HTTP(&iterix,packet->tcp.headlen);
					    pkh= &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix];

					    for(int iv=0;iv<12;iv++)
					    {

					    	     if(iv==0) i=5;
					    	else if(iv==1) i=6;
					    	else if(iv==2) i=7;
					    	else if(iv==3) i=8;
					    	else if(iv==4) i=9;//17;
					    	else if(iv==5) i=10;//18;
					    	else if(iv==6) i=28;
					    	else if(iv==7) i=21;
					    	else if(iv==8) i=22;//15;
					    	else if(iv==9) i=23;
					    	else if(iv==10) i=24;
					    	else if(iv==11) i=33;

					    	     i--;

					    		 if(Const.s_GPIO[i].val==2)  //DS
					    		 {
					    			 j= WyszukajNrCzujkiDlaPortu(i+1);
					    			 if(j!=-1)
					    			 {
						    			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';
						    			 if((Var.value[j]>0x063f)&&(Var.value[j]<0x7FFF)){ buf_p[50+4]=' '; buf_p[50+5]=0; }
						    		 	  buf_p[60]='-';
						    		 	  buf_p[61]='-';
						    		 	  buf_p[62]='%';
						    		 	  buf_p[63]=0;
						    		 	  if(lifeLora[i-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
						    		     sprintf(pkh,"1%s%c%c%s0", &buf_p[50], KolorTemp(j), buf_p[70] ,&buf_p[60]);  pkh+=strlen(pkh);
					    			 }
					    		 }
					    		 else if(Const.s_GPIO[i].val==9)  //DHT
					       	     {
					    			 j= WyszukajNrCzujkiDlaPortu(i+1);
					    			 if(j!=-1)
					    			 {
						    			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';  tt=KolorTemp(j);
						       	         j= WyszukajNrCzujkiDlaPortu_2(i+1);
						       	         if(j!=-1)
						       	         {
						       	    	     wartTemp_odwrotnie(&buf_p[60],Var.value[j]); if(buf_p[60+1]=='0') buf_p[60+1]=' ';  ww=KolorTemp(j);
						       	         }
						    		 	 if(lifeLora[i-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
						       	         sprintf(pkh,"2%s%c%c%c%c",&buf_p[50],tt, buf_p[70],buf_p[61],buf_p[62]);
						       	         a=strlen(pkh); pkh[a++]='%'; pkh[a++]=ww; pkh[a]=0;   pkh+=strlen(pkh);
					    			 }
					       	     }
					    		 else if(Const.s_GPIO[i].val==3)  //Pt
					       	     {
					    			 j= WyszukujPortElement(t_pt,_Size_pt, i+1);
					    			 if(j!=-1)
					    			 {
						       	         if(Const.s_ADC_pt[j].val==0x7FFF){ sprintf(pkh,"3  --- 0 --0"); pkh+=strlen(pkh); }
						       	         else
						       	         {
						       	        	 sprintf(buf_p,"          ");
						       	        	 if((int16_t)Const.s_ADC_pt[j].val<0)
						       	        	 {
						       	        		 buf_p[0]='-';
						       	        		 sprintf(&buf_p[1],"%d",(int16_t)Const.s_ADC_pt[j].val);   buf_p[1]=' '; buf_p[1+strlen(&buf_p[1])]=' ';
						       	        	 }
						       	        	 else
						       	        	 {
						       	        		 buf_p[0]='+';
						       	        		 sprintf(&buf_p[2],"%d",(int16_t)Const.s_ADC_pt[j].val);   buf_p[1]=' '; buf_p[2+strlen(&buf_p[2])]=' ';
						       	        	 }
						       	        	 tt=KolorTemp( WyszukajNrCzujkiDlaPortu(i+1));
						       	        	 buf_p[5]=tt;
						       	        	 buf_p[6]=0;
						       	        	 sprintf(pkh,"3%s --- ",buf_p);  pkh+=strlen(pkh);
						       	         }

					    			 }
					       	     }
					    		 else if(Const.s_GPIO[i].val==4)  //WE
					    		 {
					    			 j= WyszukujPortElement(t_we,_Size_we, i+1);
									 if(j!=-1)
									 {
						    			 if((Var.we[j/16]&(1<<(j-16*(j/16))))>0){ if(lifeLora[i-1]>0) sprintf(pkh,"4-----3 ---0"); else sprintf(pkh,"4-----1 ---0");  }
						    			 else                                   { if(lifeLora[i-1]>0) sprintf(pkh,"4-----2 ---0"); else sprintf(pkh,"4-----0 ---0");  }
						    			 pkh+=strlen(pkh);
									 }

					    		 }
					    		 else if(Const.s_GPIO[i].val==0)  //PK
					    		 {
					    			 j= WyszukujPortElement(t_wy,_Size_wy, i+1);
									 if(j!=-1)
									 {
										 if((Const.s_WY[j/16].val&(1<<(j-16*(j/16))))>0){ if(lifeLora[i-1]>0) sprintf(pkh,"5-----3 ---0"); else sprintf(pkh,"5-----1 ---0");  }
						    			 else                                           { if(lifeLora[i-1]>0) sprintf(pkh,"5-----2 ---0"); else sprintf(pkh,"5-----0 ---0");  }
						    			 pkh+=strlen(pkh);
									 }

					    		 }
					    		 else
					    		 {
					    	 	    buf_p[60]='-';
					    		    buf_p[61]='-';
					    		    buf_p[62]='%';
					    		    buf_p[63]=0;
					    		    sprintf(pkh,"0 --.-0 %s0",&buf_p[60]);   pkh+=strlen(pkh);
					    		 }



					    }
					    *(pkh+0)=' '; *(pkh+1)=' '; *(pkh+2)=0;   pkh+=strlen(pkh);

//
//					    if((wybor2_bitowy&0x0040)>0){ wybor2_bitowy&=~0x0040; sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "+13.40+22.51-30.20+43.40+52.51-10.20+23.40+32.51-10.20+23.40+32.51-10.20"); }
//					    else                        { wybor2_bitowy|=0x0040; sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],  "+24.51+33.60-09.11+24.51+33.60-09.11+24.51+33.60-09.91+24.51+45.60-09.71"); }

					    iterix += strlen(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

						packet->tcp.flags=0x19;
						HTTP[ux].kup=0;

						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				        HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);  //rprintf(ptxt);

						tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), ((packet->tcp.headlen>>4)*4) + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);
      //  sprintf(buf_p,"\r\n%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,%d-%d,",Const.s_Map[0].x[0],Const.s_Map[0].y[0],Const.s_Map[0].x[1],Const.s_Map[0].y[1],Const.s_Map[0].x[2],Const.s_Map[0].y[2],Const.s_Map[0].x[3],Const.s_Map[0].y[3],Const.s_Map[0].x[4],Const.s_Map[0].y[4],Const.s_Map[0].x[5],Const.s_Map[0].y[5],Const.s_Map[0].x[6],Const.s_Map[0].y[6],Const.s_Map[0].x[7],Const.s_Map[0].y[7],Const.s_Map[0].x[8],Const.s_Map[0].y[8],Const.s_Map[0].x[9],Const.s_Map[0].y[9]); dbg3(buf_p);
                     dbg3("Mapa.txt ");

					}
					else if(pkh=strstr(pkh1,"GET /mobile/pk"))
					{
						ni =  100*(0x0f&(*(pkh+14)))+10*(0x0f&(*(pkh+15)))+(0x0f&(*(pkh+16)));
						ni--;
					   iterix=0;
					    Send_Header_HTTP(&iterix,packet->tcp.headlen);
					     if((Const.s_WY[ni/16].val&(1<<(ni-16*(ni/16))))>0)
				         {	 Const.s_WY[ni/16].val&=~(1<<(ni-16*(ni/16)));
				             GPIO_PK(ni);
				             sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "<font color=\"#000\">%s</font>",Const.s_PK[ni].nazwa_of);
				             ZapisZdarzenia(_ZdaPKwww,_PKoff,ni);
                           #ifdef _LoRa
				            if(lifeLora[t_wy[ni]-1]>0){ Lora_Send_Przek(t_wy[ni],0);  Lora_wstrz_synch=t_wy[ni];  Lora_wstrz_synch2=0; }
                           #endif
				         }
				         else
				         {	 Const.s_WY[ni/16].val|=(1<<(ni-16*(ni/16)));
			                 GPIO_PK(ni);
				             sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "<font color=\"#eee\">%s</font>",Const.s_PK[ni].nazwa_on);
				             ZapisZdarzenia(_ZdaPKwww,_PKon,ni);
                            #ifdef _LoRa
				             if(lifeLora[t_wy[ni]-1]>0){ Lora_Send_Przek(t_wy[ni],1);  Lora_wstrz_synch=t_wy[ni];  Lora_wstrz_synch2=1; }
                            #endif
				         }
						iterix += strlen(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

						packet->tcp.flags=0x19;
						HTTP[ux].kup=0;

						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				        HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

						tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), ((packet->tcp.headlen>>4)*4) + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);

						//ZapiszFlash();
						FlashWsadPrg( &Const.s_WY[0].val, (2)*(1+(_Size_wy-1)/16) );



					}
					else if(pkh=strstr(pkh1,"GET /TME.txt"))
					{

                        iterix=0;
					    Send_Header_HTTP(&iterix,packet->tcp.headlen);
						//j=HttpDynTME();

						//for(i=0;i<j;i++)
						//{
							//Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix + i] = buf_TME[i];
						//}

pkh= &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix];
PobierzCzas(buf_p);
sprintf(pkh, "%02d:%02d:%02d &nbsp;%02d/%02d/%02d%s",buf_p[4],buf_p[5],buf_p[6] ,buf_p[0],buf_p[1],buf_p[2],zasieg_gsm);     pkh+=strlen(pkh);
//sprintf(pkh, "10:35:09 &nbsp;01/01/0900");     pkh+=strlen(pkh);

for(i=0;i<_Size_wy;i++)   //WY
{
	if((Const.s_WY[i/16].val&(1<<(i-16*(i/16))))>0){ if(lifeLora[t_wy[i]-1]>0) *(pkh+i)='3'; else *(pkh+i)='1';  }
	else                                           { if(lifeLora[t_wy[i]-1]>0) *(pkh+i)='2'; else *(pkh+i)='0';  }
}
*(pkh+i)=' '; *(pkh+i+1)=0;   pkh+=strlen(pkh);


for(i=0;i<_Size_we;i++)   //WE
{
	if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){ if(lifeLora[t_we[i]-1]>0) *(pkh+i)='3'; else *(pkh+i)='1';  }
	else                                   { if(lifeLora[t_we[i]-1]>0) *(pkh+i)='2'; else *(pkh+i)='0';  }
}
*(pkh+i)=' ';  *(pkh+i+1)=0;   pkh+=strlen(pkh);

for(i=0;i<_Size_eol;i++)   //WE EOL
{
	if((Var.eol[i/16]&(1<<(i-16*(i/16))))>0)
	{
		if(lifeLora[t_eol[i]-1]>0) *(pkh+i)='6'; else *(pkh+i)='5';
	}
	else
	{
		if((Var.we[i/16]&(1<<(i-16*(i/16))))>0){ if(lifeLora[t_eol[i]-1]>0) *(pkh+i)='3'; else *(pkh+i)='1';  }
		else                                   { if(lifeLora[t_eol[i]-1]>0) *(pkh+i)='2'; else *(pkh+i)='0';  }

	}
}
*(pkh+i)=0;   pkh+=strlen(pkh);



for(i=0;i<_Size_te;i++)   //Te
{
	 j= WyszukajNrCzujkiDlaPortu(t_te[i]);
	 if(j!=-1)
	 {
		 if(Const.s_GPIO[t_te[i]-1].val==2)
		 {
			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';
			 if((Var.value[j]>0x063f)&&(Var.value[j]<0x7FFF)){ buf_p[50+4]=' '; buf_p[50+5]=0; }
		 	  buf_p[60]='-';
		 	  buf_p[61]='-';
		 	  buf_p[62]='%';
		 	  buf_p[63]=0;
		 	  if(lifeLora[t_te[i]-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
		     sprintf(pkh," %s%c%c%s0", &buf_p[50], KolorTemp(j), buf_p[70] ,&buf_p[60]);  pkh+=strlen(pkh);
		 }
		 else if(Const.s_GPIO[t_te[i]-1].val==9)
   	     {
			 wartTemp_odwrotnie(&buf_p[50],Var.value[j]); if(buf_p[50+1]=='0') buf_p[50+1]=' ';  tt=KolorTemp(j);
   	         j= WyszukajNrCzujkiDlaPortu_2(t_te[i]);
   	         if(j!=-1)
   	         {
   	    	     wartTemp_odwrotnie(&buf_p[60],Var.value[j]); if(buf_p[60+1]=='0') buf_p[60+1]=' ';  ww=KolorTemp(j);
   	         }
		 	 if(lifeLora[t_te[i]-1]>0) buf_p[70]='x'; else buf_p[70]=' ';
   	         sprintf(pkh," %s%c%c%c%c",&buf_p[50],tt, buf_p[70],buf_p[61],buf_p[62]);
   	         a=strlen(pkh); pkh[a++]='%'; pkh[a++]=ww; pkh[a]=0;   pkh+=strlen(pkh);
   	     }
		 else
		 {
	 	    buf_p[60]='-';
		    buf_p[61]='-';
		    buf_p[62]='%';
		    buf_p[63]=0;
		    sprintf(pkh,"  --.-0 %s0",&buf_p[60]);   pkh+=strlen(pkh);
		 }
	 }
	 else
	 {
	    buf_p[60]='-';
	    buf_p[61]='-';
	    buf_p[62]='%';
	    buf_p[63]=0;
	    sprintf(pkh,"  --.-0 %s0",&buf_p[60]);   pkh+=strlen(pkh);
	 }
}
*(pkh+0)=' '; *(pkh+1)=' '; *(pkh+2)=0;   pkh+=strlen(pkh);




for(i=0;i<_Size_TT;i++)
{
	ZmianaTTnaTME(i,buf_p);
	sprintf(pkh,"%s",buf_p);  pkh+=strlen(pkh);
}
*(pkh+0)=' '; *(pkh+1)=0;   pkh+=strlen(pkh);



for(i=0;i<_Size_pt;i++)
{
	 if(Const.s_ADC_pt[i].val==0x7FFF){ sprintf(pkh,"   ---   0");  pkh+=strlen(pkh); }
	 else
	 {
	     sprintf(buf_p,"          ");
	     if((int16_t)Const.s_ADC_pt[i].val<0)
	     {
	    	 buf_p[0]='-';
	    	 sprintf(&buf_p[1],"%d",(int16_t)Const.s_ADC_pt[i].val);   buf_p[1]=' '; buf_p[1+strlen(&buf_p[1])]=' ';
	     }
	     else
	     {
	    	 buf_p[0]='+';
	    	 sprintf(&buf_p[2],"%d",(int16_t)Const.s_ADC_pt[i].val);   buf_p[1]=' '; buf_p[2+strlen(&buf_p[2])]=' ';
	     }
	     tt=KolorTemp( WyszukajNrCzujkiDlaPortu(t_pt[i]));
	     buf_p[9]=tt;
	     buf_p[10]=0;
	     sprintf(pkh,"%s",buf_p);  pkh+=strlen(pkh);
	 }
}
*(pkh+0)=' '; *(pkh+1)=0;   pkh+=strlen(pkh);



for(i=0;i<_Size_TT;i++)   //nazwy jdn.adc
{
 	sprintf(pkh,"%s",Const.s_TT[i].jm);  pkh+=strlen(pkh);
 	for(j=0;j<(5-strlen(Const.s_TT[i].jm));j++) *(pkh+j)=' ';
 	*(pkh+j+0)=' ';
 	*(pkh+j+1)=0;
 	pkh+=strlen(pkh);
}

for(i=0;i<_Size_imp;i++)
{
	for(j=0;j<_Imp_IleVal;j++)
	{
		    PrzeliczLicznikKolejny(i,j,pkh);
		    if(j==0)
		    {
	 	        *(pkh+17+5+0)=' ';
	 	        *(pkh+17+5+1)=0;
		    }
		    else
		    {
		    	if((Var.etrwsk[i][j]>0)||(Var.etrwsk2[i][j]>0)) *(pkh+16+0+0)='1';
		    	else                                            *(pkh+16+0+0)='0';
		    	*(pkh+16+1+0)=' ';
 	            *(pkh+16+1+1)=0;
		    }
 	        pkh+=strlen(pkh);
    }
}







						//sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix], "00:00:30 &nbsp;00/01/01--1000000010 0010000110 +11.10 11A0 +22.20 22A0 +33.31 33A1 +44.40 44A0 +55.50 55A0 +66.60 66A0 +77.70 77A0 +88.80 88A0 +99.90 99A1 +00.00 00A1     98.1  0+ 98.2   0   98.3  1   98.4  0   98.5  0   98.6  0   98.7  0   98.8  0   98.9  1   99.0  1 V     V     V     V     Pa    V     V     V     Twoja MPa   ");
						iterix += strlen(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);
						//iterix += j;
						packet->tcp.flags=0x19;
						HTTP[ux].kup=0;

						HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
				        HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);  //rprintf(ptxt);

						tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), ((packet->tcp.headlen>>4)*4) + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);
						 dbg3(">");


					}

else if(pkh=strstr(pkh1,"GET"))    //get
{
  int i,j,ni;   j=0;  char *ptr,*ptr1;

  sprintf(buf_p,"%s:%s",Const.s_Log[0].login, Const.s_Log[0].haslo);
  base64_encode(buf_p, &buf_p[50], strlen(buf_p));   // rrprintf("\r\n%s\r\n",buf_p);
  sprintf(&buf_p[100],"Authorization: Basic %s", &buf_p[50]);
  if(strstr(pkh1,&buf_p[100]))
  {
	  j=1;
  }

  if(j==1)  //tu sprawdzamy haslo nastrone  "Authorization: Basic YWE6YmI="
  {
	  if(http_wstrz==0){  http_wstrz=1; delay_funkc[22]=1; port_ux=ux; dbg3(".");  } else return;

	 HTTP[ux].txt_jpg = 0x01;

/*
	      if(ptr=strstr(pkh1,"GET /net")) WhichPage=4;
	 else if(ptr=strstr(pkh1,"GET /wielotemp")) WhichPage=1;
	 else if(ptr=strstr(pkh1,"GET /tim")) WhichPage=12;
	 else if(ptr=strstr(pkh1,"GET /term")) WhichPage=10;

	 else WhichPage=0;
	 */
	 WhichPage_p = WhichPage;
	 WhichPage = WhichPageForGET(pkh1);
	 if((WhichPage!=6)&&(WhichPage!=8)) scroll=0;

	 //if((WhichPage==15)||(WhichPage==6)) wybor_bitowy&=~0x0200;
	 wybor_bitowy&=~0x0200;

	     // if(WhichPage==6) MainPanel();
	// else if(WhichPage==16) HttpDynTME();

	 zmiana_ekranu();

	     if(WhichPage==900)  //IRprg
		 {
			WhichPage=WhichPage_p;
			i= WyszukujPortElement(t_ir, _Size_ir, t_wy[WhichPage_p-500-1]);
            i=2*i;
			 sprintf(buf_p,"\r\nPrg IR:%d  ",i+1);  dbg3(buf_p);
			 while(do_buf_kod_pilota()==0);
			 Flash_in_progr_16( i*FLASH_PAGE_SIZE+_AdrFlashToIR, &buf_irda[0], (int)buf_irda[0]+1);
			 sprintf(buf_p," %d  ",buf_irda[0]); dbg3(buf_p);
		     dbg3("OK  ");

		 }
	     else if(WhichPage==901)  //IRprg
		 {
			WhichPage=WhichPage_p;
			i= WyszukujPortElement(t_ir, _Size_ir, t_wy[WhichPage_p-500-1]);
            i=2*i+1;
			 sprintf(buf_p,"\r\nPrg IR:%d  ",i+1);  dbg3(buf_p);
			 while(do_buf_kod_pilota()==0);
			 Flash_in_progr_16( i*FLASH_PAGE_SIZE+_AdrFlashToIR, &buf_irda[0], (int)buf_irda[0]+1);
			 sprintf(buf_p," %d  ",buf_irda[0]); dbg3(buf_p);
		     dbg3("OK  ");

		 }

	 load_page(WhichPage);
	 WhichFragmentPage=0;
	 f_open2();

     iterix=0;
	 //if(WhichPage!=81) Send_Header_HTTP(&iterix,packet->tcp.headlen);

     send_code_html(&iterix, &HTTP[ux].kup, &packet->tcp.flags, packet->tcp.headlen);

      HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
	  HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

	  replay_packet[ux]=200;
	 if(iterix<200){ tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);   }
	 else{           tcpSend_http( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4  + iterix - TCP_HEADER_LEN, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx, ux);
	                  replay_packet[ux]=0;
	 }


	 HTTP[ux].kup=1;
	 //sprintf(buf_p,"\r\nSS: %d %d  %d  ",ksx, ux, iterix); dbg3(buf_p); dbg3("\r\nGET\r\n");
	 no_querry_http:
	 asm("nop");

	// HAL_Delay(50);   // dbg3("\r\nGET\r\n");

  }
  else
  {
	 niepoprwane_auth:
	 iterix=0;
	 Send_Header_AUTH_HTTP(&iterix,packet->tcp.headlen);
	 sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],"<html><head><title>404 Not Found</title></head><body>404 Not Found</body></html>");
     iterix += strlen( &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

	 packet->tcp.flags=0x19;
	 HTTP[ux].kup=0;

	 HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
	 HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

	 tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);

  }

}
else
{
  if(HTTP[ux].kup > 4)  //*************************************************************
  {
	if(http_wstrz==0);  else return;

	if( (packet->tcp.seqno==HTTP[ux].seq)&&(packet->tcp.ackno==HTTP[ux].ack) )
    {

	  if(len<1513)
	  {
	     CGI_callback(pkh1,&select_HTML);
	     NrPageToString(WhichPage, buf_p);

		 iterix=0;
		 Send_Header_HTTP(&iterix, packet->tcp.headlen);
		 sprintf(&Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix],"<html><head><meta http-equiv=\"refresh\" content=\"0;url=../%s\"></head><body bgcolor=\"000000\"></body></html>",buf_p);

         iterix += strlen( &Operation_DATA_SRAM[_Net_stack_start+ETH_HEADER_LEN+IP_HEADER_LEN + ((packet->tcp.headlen>>4)*4) + iterix]);

		 packet->tcp.flags=0x19;
		 HTTP[ux].kup=0;

		 HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
		 HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) + iterix);

		 tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);


		 //sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup); dbg3(buf_p);
	     HTTP[ux].kup++;




	  }
	  else
	  {
		  HTTP[ux].seq = HTONL(HTONL(packet->tcp.seqno) + ksx);
		  HTTP[ux].ack = HTONL(HTONL(packet->tcp.ackno) );

		  if(HTTP[ux].kup==5)
		  {
			 if(pkh=strstr(pkh1,"GET /lpc.cgi")){ CGI_callback(pkh,&select_HTML); /*sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup);  dbg3(buf_p);*/ }
		  }
		  else
		  {
			 if((HTTP[ux].kup%2)==0)
	         {
				 CGI_callback(pkh1,&select_HTML);
	             iterix=0;
				 tcpSend( HTONL(packet->ip.srcipaddr), HTONL(packet->ip.destipaddr), HTONS(packet->tcp.destport), HTONS(packet->tcp.srcport), (packet->tcp.headlen>>4)*4 - TCP_HEADER_LEN + iterix, (uint8_t*)(&packet->tcp.urgp + 1) , 0 , ksx);
				 sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup); dbg3(buf_p);
	         }
	         else
	         {
	             CGI_callback(pkh1,&select_HTML);
	             //sprintf(buf_p,"kup=%d\r\n",HTTP[ux].kup); dbg3(buf_p);

	         }

		  }
	      HTTP[ux].kup++;
	  }

	}

  }













}




			  }
		   }
	    }
 }

}
/*
void netstackUDPIPProcess(unsigned int len, udpip_hdr* packet)
{
   int k;

	   if( (packet->udp.srcport == 0x4300)&&(packet->udp.destport == 0x4400) )     //DHCP query
	   {
	        struct netDhcpHeader* packet1;
		    packet1 = (struct netDhcpHeader*)(&packet->udp.udpchksum + 1);

			if( packet1->bootp.xid == DhcpTransactID )
	        {
			   if( packet1->cookie == 0x63538263 )
	           {
	               if( (packet1->options[0] == 0x35)||(packet1->options[1] == 0x01) )
				   {
				       if( packet1->options[2] == 0x02 )
					   {

							 IPADDRESS = HTONL(packet1->bootp.yiaddr);

	                         i=3;
							 k=0;
							 dale_szu:
							 if(packet1->options[i]==0x36)
							 {   GATEWAY = ((unsigned long)packet1->options[i+2]<<24)|((unsigned long)packet1->options[i+3]<<16)|((unsigned long)packet1->options[i+4]<<8)|((unsigned long)packet1->options[i+5]<<0);
							     k++;
							 }
							 else if(packet1->options[i]==0x01)
							 {   NETMASK = ((unsigned long)packet1->options[i+2]<<24)|((unsigned long)packet1->options[i+3]<<16)|((unsigned long)packet1->options[i+4]<<8)|((unsigned long)packet1->options[i+5]<<0);
							     k++;
							 }
							 else
							 {  i = packet1->options[i+1] + i + 2;
	                            goto dale_szu;
							 }
							 if(k==1)
							 {
							    i = packet1->options[i+1] + i + 2;
							    goto dale_szu;
							 }

							  dhcp_query_stan=2;
							  dbg3("35_01_02\r\n");

					   }
					   else if( packet1->options[2] == 0x05 )
					   {

							  //eeprom_write_byte1(10,IPADDRESS>>24);
			                 // eeprom_write_byte1(11,IPADDRESS>>16);
			                 // eeprom_write_byte1(12,IPADDRESS>>8);
			                 // eeprom_write_byte1(13,IPADDRESS>>0);

							 // eeprom_write_byte1(20,NETMASK>>24);
			                 // eeprom_write_byte1(21,NETMASK>>16);
			                  //eeprom_write_byte1(22,NETMASK>>8);
			                  //eeprom_write_byte1(23,NETMASK>>0);

							 // eeprom_write_byte1(30,GATEWAY>>24);
			                  //eeprom_write_byte1(31,GATEWAY>>16);
			                 // eeprom_write_byte1(32,GATEWAY>>8);
			                 // eeprom_write_byte1(33,GATEWAY>>0);

							  ip_addr_dest = GATEWAY;
							  netstackInit(IPADDRESS, NETMASK, GATEWAY);
							  dhcpInit();
							 dhcp_query_stan=5;
							 dbg3("35_01_05\r\n");   // wsk_delay_funkc|=0x0001;

					   }
					   else if( packet1->options[2] == 0x06 )
					   {
							 dhcp_query_stan=6;
							 dbg3("35_01_06\r\n");


					   }


				   }

	           }
	        }
	   }

}*/

void netstackIPProcess(unsigned int len, struct netEthIpHeader* packet)
{

	if( (htonl(packet->ip.destipaddr) != ipGetConfig()->ip) && (htonl(packet->ip.destipaddr) != 0xFFFFFFFF) ) return;


	if( htonl(packet->ip.destipaddr) == ipGetConfig()->ip ) Update_table_for_IP(packet);


	if( packet->ip.proto == IP_PROTO_ICMP )
	{

		dbg3("NET Rx: ICMP/IP packet\r\n");
		//icmpPrintHeader((icmpip_hdr*)packet);
		icmpIpIn((icmpip_hdr*)&packet->ip);
	}
	else if( packet->ip.proto == IP_PROTO_UDP )
	{

		dbg3("NET Rx: UDP/IP packet\r\n");
		//debugPrintHexTable(NetBufferLen-14, &NetBuffer[14]);
		//netstackUDPIPProcess(len, ((udpip_hdr*)&packet->ip) );
	}
	else if( packet->ip.proto == IP_PROTO_TCP )
	{
        dbg3("*");
		//dbg3("NET Rx: TCP/IP packet\r\n");
		netstackTCPIPProcess(len, ((tcpip_hdr*)&packet->ip) );
	}
	else
	{
		dbg3("NET Rx: OTHER packet\r\n");
	}
}

void SEND_arp_request(unsigned long ip_addr, unsigned char *ptr)
{
   struct netEthArpHeader* packet1;
    packet1 = (struct netEthArpHeader*)ptr;

           packet1->eth.src = ArpMyAddr.ethaddr;  // header ETH
		   packet1->eth.dest.addr[0] = 0xFF;
	       packet1->eth.dest.addr[1] = 0xFF;
		   packet1->eth.dest.addr[2] = 0xFF;
		   packet1->eth.dest.addr[3] = 0xFF;
		   packet1->eth.dest.addr[4] = 0xFF;
		   packet1->eth.dest.addr[5] = 0xFF;
		   packet1->eth.type = htons(ETHTYPE_ARP);

		   packet1->arp.dipaddr = htonl(ip_addr);                // header ARP
           packet1->arp.opcode = htons(ARP_OPCODE_REQUEST);
		   packet1->arp.dhwaddr.addr[0] = 0x00;
	       packet1->arp.dhwaddr.addr[1] = 0x00;
		   packet1->arp.dhwaddr.addr[2] = 0x00;
		   packet1->arp.dhwaddr.addr[3] = 0x00;
		   packet1->arp.dhwaddr.addr[4] = 0x00;
		   packet1->arp.dhwaddr.addr[5] = 0x00;
		   packet1->arp.shwaddr = ArpMyAddr.ethaddr;
		   packet1->arp.sipaddr = htonl(ArpMyAddr.ipaddr);
		   packet1->arp.hwtype = 0x0100;
		   packet1->arp.protocol = 0x0008;
		   packet1->arp.hwlen = 0x06;
		   packet1->arp.protolen = 0x04;
		   packet1->arp.opcode = 0x0100;

		  dbg3("Sending ARP Request\r\n");
		  // debugPrintHexTable(60, (unsigned char*)packet1);
         // arpPrintHeader( &packet1->arp );

		   nicSend(ETHERNET_MIN_PACKET_LENGTH, (unsigned char*)packet1);

}

void netstackARPProcess(unsigned int len, struct netEthArpHeader* packet)  //arp
{
    int i,j;

	if(	(packet->arp.dipaddr == HTONL(ArpMyAddr.ipaddr)) && (packet->arp.opcode == htons(ARP_OPCODE_REQUEST)) )
	{

		   Update_table_for_ARP(packet);

		   dbg3("Received ARP Request\r\n");
	       //arpPrintHeader( &packet->arp );

		packet->arp.dhwaddr = packet->arp.shwaddr;
		packet->arp.dipaddr = packet->arp.sipaddr;

		packet->arp.shwaddr = ArpMyAddr.ethaddr;
		packet->arp.sipaddr = HTONL(ArpMyAddr.ipaddr);

		packet->arp.opcode = htons(ARP_OPCODE_REPLY);

		packet->eth.dest = packet->eth.src;
		packet->eth.src  = ArpMyAddr.ethaddr;


		dbg3("Sending ARP Reply\r\n");
		  // arpPrintHeader( &packet->arp );

		nicSend(len-4, (unsigned char*)packet);

	}
	else if( (packet->arp.dipaddr == HTONL(ArpMyAddr.ipaddr)) && (packet->arp.opcode == htons(ARP_OPCODE_REPLY)) )
	{

		   Update_table_for_ARP(packet);     //arpPrintHeader( &packet->arp );
		   dbg3("Received ARP Reply\r\n");

		  /* if(packet->arp.sipaddr == HTONL(ip_addr_dest))
		   {
		       wybor5_bitowy|=0x0200;	//flaga info ze odebrano arp od bramy sieci
               if((wybor5_bitowy&0x0400)>0){ wsk_delay_funkc2|=0x0040; wybor5_bitowy&=~0x0400; }

		   }


	   for(j=0;j<__Krot_Ping;j++)
       {
          char_p =SRAM_read_char(__Adr_Ping_wybor, j,__Len_Ping);
	      if(char_p!=4)
	      {
		     long_p =SRAM_read_long(__Adr_Ping_ip, j,__Len_Ping);
		     if(long_p==HTONL(packet->arp.sipaddr))  SEND_echo_request(long_p,&Operation_DATA_SRAM[_Net_stack_start+0]);
		  }

       }*/

	}

}

void netstackService(void)
{
	char buf_p[50];   int i;
	unsigned short a,b,c;
    unsigned long len;
    struct netEthHeader* ethPacket;

   // for(i=0;i<NETSTACK_BUFFERSIZE;i++) Operation_DATA_SRAM[_Net_stack_start+i]=0;

     len=0;
	 len = enc28j60PacketReceive(MAX_FRAMELEN, &Operation_DATA_SRAM[_Net_stack_start]);

	  if(len>0)
	  {
	        ethPacket = (struct netEthHeader*)&Operation_DATA_SRAM[_Net_stack_start];

			if(debug_==1)
			{
				 sprintf(buf_p,"\r\nReceived packet len: %d, type:", len); dbg3(buf_p);
				 dbg3u16(htons(ethPacket->type));

			     dbg3("\r\nPacket Contents\r\n");
			     debugPrintHexTable(len, &Operation_DATA_SRAM[_Net_stack_start]);
			}


			a=ethPacket->type;
			b=htons(ETHTYPE_IP);
			c=htons(ETHTYPE_ARP);

			if(a == b)
			{

				//dbg3("NET Rx: IP packet\r\n");

				netstackIPProcess( len, ((struct netEthIpHeader*)&Operation_DATA_SRAM[_Net_stack_start]) );

			}
			else if(a == c)
			{
				netstackARPProcess(len, ((struct netEthArpHeader*)&Operation_DATA_SRAM[_Net_stack_start]) );
			}


	  }


}



