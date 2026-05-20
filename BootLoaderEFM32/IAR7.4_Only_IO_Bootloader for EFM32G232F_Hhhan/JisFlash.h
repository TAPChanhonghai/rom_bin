#ifndef __JISFLASH_H__
#define __JISFLASH_H__

#include "JIStypes.h"
#include "efm32.h"
#include "em_chip.h"
#include "em_msc.h"

//IN EFM, SINGLE ERASE ERASES 512 BYTES AT A TIME
#define FLASH_ERASE_BLOCK_SIZE    512

//PROGRAM SEGMENT 1 IS TOTAL OF 2k AND EACH BLOCK IS OF SIZE 128 BYTES, 
//SO IN 2K THERE CAN BE 16 BLOCKS IN TOTAL.
#define PROG_SEG1_ID            0
#define PROG_SEG1               0xF800 //information area     2K
#define PROG_SEG1_SIZE          2048
#define PROG_SEG1_BLOCK_SIZE    128 //128 bytes
#define PROG_SEG1_BLOCK_COUNT   (PROG_SEG1_SIZE/PROG_SEG1_BLOCK_SIZE) //16 blocks

#define D_FLASH_FLAG    0xa5a5

#define READ_LONG  0
#define READ_INT   1
#define READ_CHAR  2

#define ERROR  0
#define RIGHT  1

void Flash_Info_Init(void);
UINT16 Flash_Read_Buffer_Value(UCHAR by_Num);
UINT32 Flash_Buffer_Write_Mass(UCHAR by_Page);
void Flash_Buffer_Read_Mass(UCHAR by_Page);
UINT16 Flash_Read(UCHAR by_Page, UINT16 by_Address);
void Flash_Write_Buffer_Value(UCHAR by_Num, UINT16 w_Value);
#endif