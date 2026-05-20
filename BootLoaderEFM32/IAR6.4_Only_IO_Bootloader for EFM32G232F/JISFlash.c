#include "JISFlash.h"
#include "hw.h"

#define FLASH_BUFFER_LEN 64  //VALUE IN WORD SIZE AND NOT IN BYTES
__no_init static UINT16 pw_Info_Buffer[FLASH_BUFFER_LEN]; //IN TOTAL 128 BYTES
__no_init static UCHAR by_Mass1_Block;

void Flash_Info_Init(void)
{    
  signed char by_Loop;

  for(by_Loop = PROG_SEG1_BLOCK_COUNT - 1; by_Loop >= 0; by_Loop--)
  {
      by_Mass1_Block = by_Loop;
      
      //READ LAST TWO BYTES OF THE BLOCK
      if(Flash_Read(0, PROG_SEG1_BLOCK_SIZE - 2) == D_FLASH_FLAG)
        break;
  }
}

UINT16 Flash_Read_Buffer_Value(UCHAR by_Num)
{
    return(*(pw_Info_Buffer + by_Num));
}

void Flash_Write_Buffer_Value(UCHAR by_Num, UINT16 w_Value)
{
    *(pw_Info_Buffer + by_Num) = w_Value;
}

void Flash_Buffer_Read_Mass(UCHAR by_Page)
{
  UINT16 w_Loop;
  UINT32 *Ptr;
  UINT32 *Dist = (UINT32 *)pw_Info_Buffer;
  
  if(by_Page == PROG_SEG1_ID)
  {
    Ptr = (UINT32 *)(PROG_SEG1 + by_Mass1_Block * PROG_SEG1_BLOCK_SIZE);
    
    //READ 4 BYTES EACH TIME
    for(w_Loop = 0; w_Loop < PROG_SEG1_BLOCK_SIZE / 4; w_Loop++)
      *(Dist + w_Loop) = *(Ptr + w_Loop);
  }
}

//EVERY WRITE OPERATION WRITES IN THE NEW BLOCK IN THE SEGMENT
UINT32 Flash_Buffer_Write_Mass(UCHAR by_Page)
{
    UINT32 Address;
    
    if(by_Page == PROG_SEG1_ID)
    {
      //WHILE WRITING WRITE TO THE NEXT BLOCK, THIS WAY NO NEED TO ERASE
      //THE PAGE WRITTEN BEFORE
      ++by_Mass1_Block;
      by_Mass1_Block = ((by_Mass1_Block) %  PROG_SEG1_BLOCK_COUNT);
      
      //WRITE A MARKER AT THE LAST TWO BYTES OF THE BLOCK TO SIGNIFY THIS IS THE
      //LATEST BLOCK WHICH CONTAINS THE VALID DATA
      pw_Info_Buffer[PROG_SEG1_BLOCK_SIZE/2-1] = D_FLASH_FLAG;
      
      //NUMBER OF PROGRAM SEGMENT BLOCKS THAT CAN BE ACCOMODATED IN ONE FLASH 
      //ERASE BLOCK
      UCHAR by_blocks_per_flash_erase_block = 
        FLASH_ERASE_BLOCK_SIZE / PROG_SEG1_BLOCK_SIZE;
      
      __disable_irq();
      WDOG_Feed();
      while(WDOG->SYNCBUSY & WDOG_SYNCBUSY_CMD);
      WDOG_Enable(false);
      while (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CTRL);
      MSC_Init();
      
      //IF THE CURRENT BLOCK MOVED TO THE NEXT FLASH_ERASE_BLOCK_SIZE BLOCK
      //THEN ERASE THE NEXT BLOCK FIRST SO THAT THE WRITE IS CORRECT
      if(by_Mass1_Block % by_blocks_per_flash_erase_block == 0)
      {
        Address = PROG_SEG1 + 512 * (by_Mass1_Block / by_blocks_per_flash_erase_block);
        if(MSC_ErasePage((uint32_t *)Address) != mscReturnOk)
          return ERROR;
      }
      
      //ADDRESS TO THE CURRENT BLOCK
      Address = PROG_SEG1 + by_Mass1_Block * PROG_SEG1_BLOCK_SIZE;
      
      //WRITE TO FLASH, 4 BYTES AT A TIME
      if(MSC_WriteWord((uint32_t *)Address, 
                       (UINT32 *)pw_Info_Buffer, 
                       PROG_SEG1_BLOCK_SIZE) != mscReturnOk)
      {
          return ERROR;
      }
      WDOG_Enable(true);
      while (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CTRL);
      WDOG_Feed();
      MSC_Deinit();
      __enable_irq();
      return RIGHT;   
    }
    return ERROR;
}

//可mass中数据直读
UINT16 Flash_Read(UCHAR by_Page, UINT16 by_Address)
{    
  union
  {
      UINT16 BYTE[2];
      UINT32 WORD;
  } Target;
  
  if(by_Page == PROG_SEG1_ID)
  {
    //GET THE CURRENT BLOCK POINTER
    UINT32 *Ptr = (UINT32 *)(PROG_SEG1 + by_Mass1_Block * PROG_SEG1_BLOCK_SIZE);
    
    //READ 4 BYTES
    Target.WORD = *(Ptr + ((by_Address * 2) / 4));
  }
  
  return (!(by_Address % 2)) ? Target.BYTE[0] :  Target.BYTE[1];    
}