#ifndef __COM_APP_H__
#define __COM_APP_H__

//MACRO FOR STATING THIS RUN IS FOR TEST PURPOSE
//COMMENT THIS OUR FOR REAL USE/OR WHILE RUNNING THIS FIRMWARE IN THE HARDWARE
//#define __FOR_TEST

#include "Comm.h"
#include "HW.h"

#define ERROR  0
#define RIGHT  1

void Com_Initial(void);
WORD Com_Write(unsigned char readwrite, unsigned short Command, unsigned char Length, unsigned char *pData);
DATA_PACKAGE* Com_GetCommand(DATA_PACKAGE *stData);
void Com_10msInt(void);
void Com_Tx_Int(void);
void Com_Rx_Int(UCHAR by_Data);
void Com_RxProcess(UCHAR receivedPacketLength);
UCHAR Com_CRC8_Check(UCHAR *pby_Buffer, UCHAR by_Length);

#endif
