#include "main.h"
#include "JIStypes.h"
#include "system.h"
#include "JisTimer.h"
#include "JISFlash.h"
#include "boot.h"
#include "Bootload_Cfg.h"
#define TIMEOUT                     100
#define BUFFER_SIZE                 512
#define FLASH_ERASE_BLOCK_SIZE      512
#define SOFT_VERSION                31

#define PROG_SEG1               0xF800 //information area     2K
#define BOOTLOAD_CONFIG         0xF600

#define HAS_FIRMWARE_FLASHED    0xF602
#define ENTER_BOOTLOAD_MODE_ADDR  0xF600
#define D_ENTER_UPDATEMODE_CODE 0x55aa

#define D_PASSWORD              0x55aa
#define IM_POWER                 62



//#define DEBUG_MS_REPRINTF

#ifdef DEBUG_MS_REPRINTF
#include "stdio.h"
#endif 



#if defined( __ICCAVR__ )
//__root const unsigned char DeviceModel[32] @ ".ModelAddr"           = THIS_MODEL;
__root const unsigned char DeviceSoftwareSVersion[16] @ ".SVerAddr" = THIS_S_VER;
#endif

//#elseif was not supported by IAR so changed the elseif block to two #if block
#if defined (__GNUC__ )
__attribute__ ((section (".ModelAddr"))) const unsigned char DeviceModel[32] = THIS_MODEL;
__attribute__ ((section (".SVerAddr"))) const unsigned char DeviceSoftwareSVersion[16] = THIS_S_VER;
#endif
  
//IN CASE OF IAR IDE, THE COMPILATION FAILED WHEN EXTERN KEYWORD WAS NOT USED
//MAYBE THE EXTERN KEYWORD WAS REMOVED TO MAKE IT COMPILE IN ECLIPSE
//IF SO THEN WE NEED TO ADD A CONDITIONAL IF TO DEFINE DIFFERENTLY FOR THE TWO SYSTEMS
//unsigned char __checksum;
#if defined( __ICCAVR__ )
	extern unsigned short __checksum;
#else
	unsigned short __checksum;
#endif 
        unsigned short __checksum;
        
void Main_Initial_IO(void)
{
    __disable_irq();
    System_Io_Init();
    __enable_irq();
}

//erase the AP config
bool Erase_AP_Info_Config(void)
{
    static uint8_t Fail_Cnt = 0;
    
    

    for(uint8_t i = 0; i < (PROG_SEG1_SIZE / FLASH_ERASE_BLOCK_SIZE); i++)
    {
        __disable_irq();  
        MSC_Init();
        
        if(MSC_ErasePage((uint32_t *)(PROG_SEG1 + 512 * i)) != mscReturnOk)
        {  
            Fail_Cnt |= 1 << i;
        }
        
         MSC_Deinit();
         __enable_irq();
    }
    
    return ((0 == Fail_Cnt) ? true : false);
}

void updateFlash(unsigned char* w_buffer, int address, unsigned short value)
{
  uint32_t* ptr = (uint32_t*)w_buffer;
  uint32_t* data = ((uint32_t *)BOOTLOAD_CONFIG);
  
  //read the data from flash before erasing the block
  for(int i = 0; i < FLASH_ERASE_BLOCK_SIZE; i++)
  {
    *ptr = *data;
    ptr++;
    data++;
  }
  
  __disable_irq();  
  MSC_Init();
  
  //erase the boot config
  if(MSC_ErasePage((uint32_t *)BOOTLOAD_CONFIG) == mscReturnOk)
  {  
    //update the value
    w_buffer[address - BOOTLOAD_CONFIG] = value;
    w_buffer[address - BOOTLOAD_CONFIG+1] = value >> 8;
    
    //write back the data
    MSC_WriteWord((uint32_t *)BOOTLOAD_CONFIG, (UINT32*)w_buffer, FLASH_ERASE_BLOCK_SIZE);
  }
  MSC_Deinit();
  __enable_irq();
}

//this function can only be used to read 16 bit value
int ReadFlashAddress(int read_address)
{
  return (unsigned short)*((uint32_t *)read_address);
}

//note this function is not a generic one, it assumes that 
//this function is called 
char Flash_Write(int* write_address, UCHAR* w_buffer, int size)
{
  char result = 0;
  __disable_irq();  
  MSC_Init();
  int tmpsize = size;
  while(tmpsize > 0)
  {
    if (MSC_ErasePage((uint32_t *)*write_address) != mscReturnOk)
      return 1;
    
    tmpsize -= FLASH_ERASE_BLOCK_SIZE;
  }
  result = (MSC_WriteWord((uint32_t *)*write_address, (UINT32 *)w_buffer, size) == mscReturnOk);

  MSC_Deinit();
  __enable_irq();
  *write_address = *write_address + size;  
  return result;
}

        
int main(void)
{  
    if(__checksum == 0) __checksum = 1;

    for(UINT delay_run=0; delay_run<25000; delay_run++); //延时，让FLASH电压更稳定，读写不会出差错。
    //Main_Initial_IO(); 

    //IF THERE IS FIRMWARE FLASHED IN THE ROM AND IT ASKED BOOTLOAD TO ENTER UPLOAD MODE OR
    //THERE IS NO ANY FIRMWARE FLASHED, IN BOTH THESE CASE ENTER UPDATE MODE
  
    #ifdef DEBUG_MS_REPRINTF
    printf("Boot system started...\n");
    #endif 
  
    if( (ReadFlashAddress(HAS_FIRMWARE_FLASHED) == D_PASSWORD && 
        ReadFlashAddress(ENTER_BOOTLOAD_MODE_ADDR) == D_PASSWORD) || //there is firmware and it asked to enter update mode
        ReadFlashAddress(HAS_FIRMWARE_FLASHED) != D_PASSWORD) //no firmware in the flash or for the first time
    {  
      
        Main_Initial_IO();
      
        #ifdef DEBUG_MS_REPRINTF
        printf("enter Pre BSL mode...\n");
        #endif 
      
        Timer_Init();
    
        Timer_Clear(0);
        
        int w_next_write_address = APPLICATION_START_ADDRESS;//0x00000000;
        
        UCHAR w_buffer[BUFFER_SIZE];
        
        DATA_PACKAGE stData;
        
        UINT w_flash_buffer_index = 0;
        
        UINT w_PacketSize = sizeof(DATA_PACKAGE);
        
        UINT w_Expected_Packet_Number = 1;
        
        UINT w_data_size = 0, w_copy_size = 0, w_buffer_empty_space=0;
        
        //don't allow to update until get the command REGISTER_ENTER_UPDATE_MODE
        char allow_update = 0;
        
        //clear the flag for enter update mode as it already entered update mode
        updateFlash(w_buffer, ENTER_BOOTLOAD_MODE_ADDR, 0);
        
        //since 2 bytes of data field is used for packet sequence number
        unsigned short dataProcessedIndex = 2;
        memset(&stData, 0, w_PacketSize);
        
        
        //WAIT FOR TIMEOUT
        while(!Timer_Counter(0, TIMEOUT))
        {
            //get the packet, if no packet received continue counting timeout
            if(Com_GetCommand(&stData) == 0)
            {        
                continue;
            }
     
            //clear the idle counter timer
            Timer_Clear(0);
            
            if(stData.Command == REGISTER_ENTER_UPDATE_MODE)
            {
                Com_Write(WRITE_REGISTER, REGISTER_ENTER_UPDATE_MODE, 0, 0);
                        
                allow_update = 1;

                #ifdef DEBUG_MS_REPRINTF
                printf("BSL mode started CMD comfirmed...\n");
                #endif 
            }
            //CHECK THE COMMANDID IN THE RECEIVED COMMAND IF IT'S UPDATE_DATA
            //CHECK IF THE PACKET HAS THE EXPECTING SEQUENCE NUMBER
            else if(allow_update && REGISTER_UPDATE_DATA == stData.Command && stData.WR == WRITE_REGISTER)
            {
                //UPDATE DATA TRANSFER COMPLETE
                //DURING UPDATE, IF EMPTY PACKET IS SENT, THAT IMPLIES UPDATE COMPLETE
                if(stData.Length == 0)
                {
                    //send back the acknowledgement
                    Com_Write(WRITE_REGISTER, REGISTER_UPDATE_DATA, 1, (UCHAR*)&w_Expected_Packet_Number);
                    
                    //flush the buffer data to flash
                    Flash_Write(&w_next_write_address, w_buffer, BUFFER_SIZE);
                    
                    //write down the version of the IO currently flashed
                    
                    //erase the AP config
                    Erase_AP_Info_Config();
                    
                    //update the flag that firmware is flashed
                    updateFlash(w_buffer, HAS_FIRMWARE_FLASHED, D_PASSWORD);
                    
                    #ifdef DEBUG_MS_REPRINTF
                    printf("BSL Flinsh, Reset system...\n");
                    #endif 
                    
                    //reset the system
                    NVIC_SystemReset();
                }
                //if the received packet is not same as the expected one then return 
                //the acknowledgement with the expected packet number
                else if(w_Expected_Packet_Number != *((UCHAR*)stData.Data))
                {
                    #ifdef DEBUG_MS_REPRINTF
                    printf("Worng Packet Num, need %d -th Num\n", w_Expected_Packet_Number);
                    #endif 
      
                    Com_Write(WRITE_REGISTER, REGISTER_UPDATE_DATA, 1, (UCHAR*)&w_Expected_Packet_Number);
                    continue;
                }
                
                
                #ifdef DEBUG_MS_REPRINTF
                printf("%d -th Packet Num got.\n", w_Expected_Packet_Number);
                #endif 
                
                w_Expected_Packet_Number = (w_Expected_Packet_Number + 1)%256;
                Com_Write(WRITE_REGISTER, REGISTER_UPDATE_DATA, 1, (UCHAR*)&w_Expected_Packet_Number);
                 
                #ifdef DEBUG_MS_REPRINTF
                printf("%d -th Packet Num next need\n", w_Expected_Packet_Number);
                #endif 
                
                //if the expected packet received, clear the timeout counter
                //and increment the expected packet number for the next one
                Timer_Clear(0);
                
                w_data_size = (stData.Length - 1);
                dataProcessedIndex = 1;
                
                //check if already into the application flashing area
                //if so then further process the flash writing part below
                if(w_next_write_address >= APPLICATION_START_ADDRESS)
                {
                    //do nothing, don't go to process next else if statements
                }
                //if not reached application area, then skip writing it to flash
                else if(w_next_write_address + w_data_size < APPLICATION_START_ADDRESS)
                {
                    w_next_write_address += w_data_size;
                    continue;
                }
                //this is boundary case where on part of this block of data 
                //needs to be written to flash and part of it discarded
                else
                {
                    dataProcessedIndex += APPLICATION_START_ADDRESS - w_next_write_address;
                    w_data_size -= APPLICATION_START_ADDRESS - w_next_write_address;
                    w_next_write_address = APPLICATION_START_ADDRESS;
                }
        
                while(w_data_size != 0)
                {
                    w_buffer_empty_space = BUFFER_SIZE - w_flash_buffer_index;
                    w_copy_size = w_data_size;
              
                    //if not enough space to accomodate all data in the buffer 
                    //then 
                    if(w_buffer_empty_space < w_copy_size)
                      w_copy_size = w_buffer_empty_space;
                    
                    //copy the received data to the buffer
                    memcpy(&w_buffer[w_flash_buffer_index], &stData.Data[dataProcessedIndex], w_copy_size);
                    w_flash_buffer_index += w_copy_size;
                    dataProcessedIndex += w_copy_size;
                    w_data_size -= w_copy_size;
                    
                    //if the receiving buffer is full then write it to the flash
                    if(w_flash_buffer_index == BUFFER_SIZE)
                    {                   
                        //write to flash
                        Flash_Write(&w_next_write_address, w_buffer, BUFFER_SIZE);                  
                        //update the flag that firmware is not available
                        if(ReadFlashAddress(HAS_FIRMWARE_FLASHED) == D_PASSWORD)
                          updateFlash(w_buffer, HAS_FIRMWARE_FLASHED, 0);
                        
                        //clear the buffer
                        memset(w_buffer, 0, BUFFER_SIZE);
                        w_flash_buffer_index = 0;
                        
                        #ifdef DEBUG_MS_REPRINTF
                        printf("write to flash\n");
                        #endif 
                    }
                }
            }
            else if(stData.WR == READ_REGISTER && REGISTER_VERSION_DB == stData.Command)
            {
                UCHAR Version_Ay[6] = {1, 1, 1, 0, 0, 1};
                
                //Version_Ay[0] = 1; //CUSTOMER_DEF;
                //Version_Ay[1] = 1; //CONTROL_TYPE_DEF;
                //Version_Ay[2] = 1; //CONTROLER_DEF;
                //Version_Ay[3] = 0; //KEY_BD_DEF;
                //Version_Ay[4] = 0; //MAIN_SOFT_VERSION; 
                //Version_Ay[5] = 1; //SUB_SOFT_VERSION;   //Sub-version should plus one point, when project code has been changed.
                
                Com_Write(READ_REGISTER, REGISTER_VERSION_DB, 6, Version_Ay);
            }
            else if(stData.WR == READ_REGISTER && stData.Command == REGISTER_VERSION)
            {
                if(ReadFlashAddress(HAS_FIRMWARE_FLASHED) == D_PASSWORD)
                {
                    Com_Write(READ_REGISTER, REGISTER_VERSION, 2, (UCHAR*)&w_Expected_Packet_Number);
                }
            }
        }
        
        //TIMEOUT OCCURRED, SO EXIT THE UPLOAD MODE
        #ifdef DEBUG_MS_REPRINTF
        printf("TIMEOUT OCCURRED, SO EXIT THE UPLOAD MODE\n");
        #endif 
    }
    
    #ifdef DEBUG_MS_REPRINTF
    printf("BSL mode out...\n");
    #endif 
    
    //IF THE FIRMWARE PRESENT THEN BOOT THE FIRMWARE OTHERWISE RESET
    if(ReadFlashAddress(HAS_FIRMWARE_FLASHED) == D_PASSWORD)
    {
       #ifdef DEBUG_MS_REPRINTF
       printf("Jump to APPLICATION_START_ADDRESS...\n");
       #endif 
       BOOT_boot();
    }
    else
    {
       #ifdef DEBUG_MS_REPRINTF
       printf("Reset system...\n");
       #endif

       NVIC_SystemReset();
    }
}