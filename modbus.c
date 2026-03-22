/*
 * modbus.c
 *
 *  Created on: 9 wrz 2017
 *      Author: Elektronika RM
 */



#include "modbus.h"

void composition(unsigned char *out,  unsigned char *p1,int l1, unsigned char *p2,int l2, unsigned char *p3,int l3, unsigned char *p4,int l4)
{
    int i;

    for (i=0; i<l1; i++) out[i] = p1[i];
	for (i=0; i<l2; i++) out[l1+i] = p2[i];
	for (i=0; i<l3; i++) out[l1+l2+i] = p3[i];
    for (i=0; i<l4; i++) out[l1+l2+l3+i] = p4[i];
}

int TwoPonitToInt(char *point)
{
   return  (  0x00ff&(((unsigned short)(*(point+0)))<<0) | 0xff00&(((unsigned short)(*(point+1)))<<8)  );
}
