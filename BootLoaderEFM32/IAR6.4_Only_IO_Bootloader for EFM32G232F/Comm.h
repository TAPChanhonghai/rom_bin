#ifndef __COMM_H__
#define __COMM_H__
////---------------- Command Type -----------------------//
#define WRITE_REGISTER			        0x01
#define READ_REGISTER			        0x02

#define REGISTER_VERSION			0x0005
#define REGISTER_ENTER_UPDATE_MODE              0x0090
#define REGISTER_UPDATE_DATA                    0x0091

#define REGISTER_VERSION_DB			0x0020

typedef struct tag_DATA_PACKAGE
{
    unsigned short PacketHead;
    unsigned char  WR;
    unsigned short Command;
    unsigned short  Length;
    unsigned char  Data[128];
} DATA_PACKAGE;

typedef struct _T_COMSTATUS
{
    unsigned bRec : 1;
    unsigned bRecWrite : 1;
    unsigned bRecRead : 1;
    unsigned bRecError : 1;
    unsigned bSendRead : 1;
    unsigned bSendWrite : 1;
    volatile unsigned bSendReady : 1;
    unsigned bInting  : 1;
}T_COMSTATUS, *PT_COMSTATUS;


void Com_Io_Init(void);
void Com_Data_Init(void);
void Com_1ms_Int(void);
void Com_Set_1msValue(unsigned int by_Dat);
unsigned int Com_Get_1msValue(void);
void Com_Write_Data(unsigned char Command, unsigned int Parameter, unsigned int Length, void * pData);
void Comm_Proc(void);
void Comm_HandleArm(void);
#endif
