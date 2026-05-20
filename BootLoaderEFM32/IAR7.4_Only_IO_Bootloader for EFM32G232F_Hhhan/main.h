#ifndef __MIAN_H__
#define __MAIN_H__

#include "HW.h"
#include "em_dbg.h"
#include "Com_APP.h"
#include "Comm.h"

#define M_WORK_WORKING     1
#define M_WORK_PAUSE       2
#define M_WORK_OVER        3


#define M_NONE   0
#define M_RESET  6
#define M_START  7
#define M_ENG    4

#define A_POWER         0
#define A_RESET         1
#define A_START      30
#define A_WORKING    31

void Main_Initial_IO(void);
void Main_Initial_Data(void);
void Main_Initial_Informatiom(void);
void Main_Initial_Power_On(void);
void Main_Initialize_LCB(void);
void Main_Get_MCB_Version(void);
void Main_Decode_RecData(void);
void Main_Work(void);
#endif
