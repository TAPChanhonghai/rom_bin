#include "main.h"
#include "JIStypes.h"
#include "system.h"
//#include "JisTimer.h"
#include "JISFlash.h"
#include "boot.h"
#include "Bootload_Cfg.h"
#include "system.h"
#define TIMEOUT                     5000
#define BUFFER_SIZE                 512
#define FLASH_ERASE_BLOCK_SIZE      512
#define SOFT_VERSION                31

#define PROG_SEG1                   0xF800 //information area     2K
#define BOOTLOAD_CONFIG             0xF600

#define HAS_FIRMWARE_FLASHED        0xF602
#define ENTER_BOOTLOAD_MODE_ADDR    0xF600
#define D_ENTER_UPDATEMODE_CODE     0x55aa

#define D_PASSWORD                  0x55aa
#define IM_POWER                    62



UCHAR Version_Ay[6] = {21,7,15,15,8,0};

#if defined( __ICCAVR__ )
//__root const unsigned char DeviceModel[32] @ ".ModelAddr"           = THIS_MODEL;
//__root const unsigned char DeviceSoftwareSVersion[16] @ ".SVerAddr" = THIS_S_VER;
#endif

//#elseif was not supported by IAR so changed the elseif block to two #if block
#if defined (__GNUC__ )
//__attribute__ ((section (".ModelAddr"))) const unsigned char DeviceModel[32] = THIS_MODEL;
//__attribute__ ((section (".SVerAddr"))) const unsigned char DeviceSoftwareSVersion[16] = THIS_S_VER;
#endif
  

#if defined( __ICCAVR__ )
	extern unsigned short __checksum;
#else
	unsigned short __checksum;
#endif 
unsigned short __checksum;
        
//void Main_Initial_IO(void)
//{
//    __disable_irq();
//    System_Io_Init();
//    __enable_irq();
//}

//erase the AP config  0xF800 --0x10000
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
/*
把BOOTLOAD_CONFIG地址开始0x200大小的空间数据全读出来，然后改写要更改的数据，最后再写回去
*/
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
int getCheckSumCRC16ForROMBIN(unsigned char *data, int len) {
    int CRC = 0x0000;
    int POLYNOMIAL = 0x1021;
    for (int i = 0; i < len; i++) {
        CRC ^= data[i]<<8;
        for (int j = 0; j < 8; j++) {
            if ((CRC & 0x8000) != 0) {
                CRC <<= 1;
                CRC ^= POLYNOMIAL;
            } else {
                CRC <<= 1;
            }
        }
    }
    return CRC&0xffff;
}
uint32_t TotalPackageNumber=0x1ac;//应用程序总包数
uint32_t TimeToAppHeart=0;        
extern   uint32_t FlagToBUZZER;        
int main(void)
{  
    if(__checksum == 0) __checksum = 1;
    for(UINT delay_run=0; delay_run<25000; delay_run++); //延时，让FLASH电压更稳定，读写不会出差错。
    
    uint32_t resetFlag=RMU_ResetCauseGet();
    //RMU_ResetCauseClear();//不要清除复位标志，清除以后在应用程序里就看不到真正的复位原因了
     
//    Main_Initial_IO();
//    Buzzer_start(NOR_B); 
//    printf_h("\r\nResetFlag:0x%02x\r\n",resetFlag);
//    if(resetFlag==0x01)// 上电复位 
//    {
//        printf_h("A Power-on Reset has been performed. X bits are don't care.\r\n");
//    }else if(resetFlag==0x02)// 
//    {
//          printf_h("A Brown-out has been detected on the unregulated power.\r\n");
//    }else if(resetFlag==0x04)// 软件复位 
//    {
//          printf_h("A Brown-out has been detected on the regulated power.\r\n");
//    }else if(resetFlag==0x08)
//    {
//          printf_h("An external reset has been applied.\r\n");
//    }else if(resetFlag==0x10)//
//    {
//          printf_h("A watchdog reset has occurred.\r\n");
//    }else if(resetFlag==0x20)//
//    {
//          printf_h("A lockup reset has occurred.\r\n");
//    }else if(resetFlag==0x40)//
//    {
//          printf_h("A system request reset has occurred.\r\n");
//    }


    
    HW_Initial_Clock();//打开了systick中断 
    //__disable_irq();    
    //GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 1);//打开蜂鸣器，升级的时候蜂鸣器会响，升级完跳转成功关掉蜂鸣器，     运行此函数要在HW_Initial_Clock();之后才有效

 
    /*SP100W的电源在设计时，硬件工程师完全没有考虑到软件的需求，用的的串口和在模组上的MCU不一样，
     模组的MCU和APP通信用USART1，和下控用LEUART0,
     而SP100W电源反了过来，这就让BOOTLOAD程序不能和原来兼容，要单独生成*/    
    #ifdef _SP100W_         
        HW_LEUART0_Initial();//LEUART0  to APP for  100W power  D4 D5
        #ifdef DEBUG_MS_REPRINTF
            HW_USART0_Initial(); // USART0 debug1 for 100W power  E10 E11        
            printf_h("\r\nRetFlag:0x%x\r\n",resetFlag);
        #endif
    #else
        HW_COM_Initial_IO();//  Usart1  to APP C0 C1 
        #ifdef DEBUG_MS_REPRINTF
            USART2_Initial(); //debug Usart2 to debug  C2 C3             
            printf_h("\r\nRetFlag:0x%x\r\n",resetFlag);
        #endif
    #endif  
    
//    __enable_irq();  不用在此处打开中断，因为下面的updateFlash函数运行完会打开全局中断
    Com_Initial();

    #ifdef DEBUG_MS_REPRINTF
    for(int i=0;i<6;i++)
      printf_h("%d ",Version_Ay[i]);
    printf_h("\r\nFro_boot 0x%x,0x%x,0xF3F0:0x%x\r\n",ReadFlashAddress(ENTER_BOOTLOAD_MODE_ADDR),ReadFlashAddress(HAS_FIRMWARE_FLASHED),*((uint8_t *)0xf3f0));
    #endif  
    
    TimeToAppHeart =  systimeMs;
    
    if(ReadFlashAddress(HAS_FIRMWARE_FLASHED) != D_PASSWORD){  //no firmware in the flash or for the first time
         goto UPDATA;
    }else if(ReadFlashAddress(ENTER_BOOTLOAD_MODE_ADDR) == D_PASSWORD){ //there is firmware but  user  asked to enter update mode
         goto UPDATA;
    }else{// run app,
         goto APP;
    }
   
UPDATA:    
    {  
        //Main_Initial_IO();

        #ifdef DEBUG_MS_REPRINTF
        //printf_h("Updata.\r\n");
        #endif     
        
        int NextWriteAddress = APPLICATION_START_ADDRESS;//0x00000000;
            
        UCHAR w_buffer[BUFFER_SIZE];
        
        DATA_PACKAGE stData;
        
        UINT AdressToCopyInBuff= 0;        
        UINT ErrorPackNumTimes =0;
        UINT ExpectedPacketNumberLowByte = 1;
        UINT ExpectedPacketNumberTotal = 1;
        
        FlagToBUZZER =1;  //让蜂鸣器间隔的响
        //UINT PacketNumberFromAPP;
        UINT ReceiveDataSize = 0, CopySizeToBuff = 0, EmptySpaceOf_BUFFER_SIZE=0;
        
        //don't allow to update until get the command REGISTER_ENTER_UPDATE_MODE
        char allow_update = 0;
            
        //since 2 bytes of data field is used for packet sequence number
        unsigned short dataProcessedIndex = 2;
        memset(&stData, 0, sizeof(DATA_PACKAGE));
        
        //clear the flag for enter update mode as it already entered update mode
        updateFlash(w_buffer, ENTER_BOOTLOAD_MODE_ADDR, 0);//下发一次升级指令，执行一下，没有成功的话，需要再次下发,此函数运行完毕会打开全局中断        
        
        //while(!Timer_Counter(0, TIMEOUT))
        while(systimeMs < TimeToAppHeart + TIMEOUT)
        {
            //get the packet, if no packet received continue counting timeout
                       
            if(Com_GetCommand(&stData) == 0)
            {        
                continue;
            }                        
                                    
            TimeToAppHeart =  systimeMs;
            
            
            if(stData.Command == REGISTER_ENTER_UPDATE_MODE){// 0x0090
                Com_Write(WRITE_REGISTER, REGISTER_ENTER_UPDATE_MODE, 0, 0);
                        
                allow_update = 1;
                /////////////////////////////验证总大小不要超过ROM可用分配空间////////////////////////////////////////////
//                TotalPackageNumber  = stData.Data[0] + stData.Data[1]<<8;
//                if(TotalPackageNumber==0){
//                    TotalPackageNumber  = 0x1ac;
//                }
//                TotalPackageNumber  = 0x1ac;
//                if(TotalPackageNumber*(0x80-1) > (0xf600-APPLICATION_START_ADDRESS)){
//                    #ifdef DEBUG_MS_REPRINTF
//                     printf_h("ROM is morethan %d\r\n",(0xf600-APPLICATION_START_ADDRESS));
//                     NVIC_SystemReset();//hhhan
//                    #endif 
//                }            
                //////////////////////////////////////////////////////////////////////////
                #ifdef DEBUG_MS_REPRINTF
                printf_h("BSLstar..TtalNumPak:%d\r\n",TotalPackageNumber);
                #endif 
            }else if(allow_update &&  stData.Command == REGISTER_UPDATE_DATA && stData.WR == WRITE_REGISTER){  //0x0091
                if(stData.Length == 0){
                    unsigned short CRC;
                    //send back the acknowledgement
                    Com_Write(WRITE_REGISTER, REGISTER_UPDATE_DATA, 1, (UCHAR*)&ExpectedPacketNumberLowByte);
                    
                    //flush the buffer data to flash
                    Flash_Write(&NextWriteAddress, w_buffer, BUFFER_SIZE);
                    
                    //write down the version of the IO currently flashed
                    
                    //erase the SYSPAR  config
                    Erase_AP_Info_Config();//erase the SYSPAR config (0xF800 --0x10000)
                    
                    //update the flag that firmware is flashed
                    CRC=getCheckSumCRC16ForROMBIN((unsigned char *)APPLICATION_START_ADDRESS,0xf3f0-APPLICATION_START_ADDRESS);
                    if((CRC&0xFF) == *((unsigned char *)0xF3F0)){
                        updateFlash(w_buffer, HAS_FIRMWARE_FLASHED, D_PASSWORD);
                    }else{
                        #ifdef DEBUG_MS_REPRINTF
                        printf_h("CRC:0x%x 0x%x\r\n",CRC&&0xff,*((unsigned char *)0xF3F0));
                        #endif                         
                    }
                    #ifdef DEBUG_MS_REPRINTF
                    printf_h("Res_system\r\n");
                    #endif 

                    NVIC_SystemReset();
                }else if(ExpectedPacketNumberLowByte != *((UCHAR*)stData.Data)){
                    #ifdef DEBUG_MS_REPRINTF
                    printf_h("Wrong  0x%x\r\n", ExpectedPacketNumberLowByte);
                    #endif 
                    if(ErrorPackNumTimes++ > 1)
                    NVIC_SystemReset();//hhhan
                    Com_Write(WRITE_REGISTER, REGISTER_UPDATE_DATA, 1, (UCHAR*)&ExpectedPacketNumberLowByte);
                    continue;
                }                             
                ErrorPackNumTimes=0;
                #ifdef DEBUG_MS_REPRINTF
                printf_h("Rec 0x%x Pack", ExpectedPacketNumberTotal);
                #endif
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if(ExpectedPacketNumberTotal > TotalPackageNumber){  //如果收到的数据里包序号大于总包数，则出错，
                    #ifdef DEBUG_MS_REPRINTF
                    printf_h("Pakag numbe more than .\r\n");
                    NVIC_SystemReset();//hhhan
                    #endif                   
                }
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                ExpectedPacketNumberTotal++;
                ExpectedPacketNumberLowByte = ExpectedPacketNumberTotal%256;
                //ExpectedPacketNumber = (ExpectedPacketNumber + 1)%256;
                Com_Write(WRITE_REGISTER, REGISTER_UPDATE_DATA, 1, (UCHAR*)&ExpectedPacketNumberLowByte);
                 
                #ifdef DEBUG_MS_REPRINTF
                //printf_h("Get 0x%x Pack", ExpectedPacketNumberTotal);
                #endif 
                
                TimeToAppHeart =  systimeMs;
                
                ReceiveDataSize = (stData.Length - 1);//  0x80 -1 = 0x7f   127 
/*                
55 AA 01 91 00 80 00 
1B 
13 0F 04 DB 5F F0 12 0A 2E 48 
80 F8 00 A0 5F F0 00 09 5F FA 
8A FA D1 45 0C D2 09 20 FF F7 
CF FE 00 99 09 F8 01 00 28 49 
09 F8 01 00 19 F1 01 09 EF E7 
01 25 80 21 0C 20 FF F7 CE FE 
20 21 01 20 FF F7 C3 FE 28 00 
C0 B2 BD E8 F2 8F 38 B5 00 24 
00 25 00 E0 64 1C 40 F6 FF 71 
8C 42 0C D2 43 F2 F8 41 01 FB 
00 F1 49 1E 62 00 52 1C B1 FB 
F2 F5 4F F6 FF 71 8D 42 ED D2 
15 48 05 60 15 48 04 01 0A CRC OK -->                
*/
                dataProcessedIndex = 1;
                                
                while(ReceiveDataSize != 0){
                    EmptySpaceOf_BUFFER_SIZE = BUFFER_SIZE - AdressToCopyInBuff;// buff 剩余量
                    CopySizeToBuff = ReceiveDataSize;  //需要拷贝的字节数
              

                    if(EmptySpaceOf_BUFFER_SIZE < CopySizeToBuff) //如果剩余量装不下要拷贝的字节数 ，则只拷贝部分数据
                      CopySizeToBuff = EmptySpaceOf_BUFFER_SIZE;
                    
                    //copy the received data to the buffer
                    memcpy(&w_buffer[AdressToCopyInBuff], &stData.Data[dataProcessedIndex], CopySizeToBuff);
                    AdressToCopyInBuff += CopySizeToBuff;
                    dataProcessedIndex += CopySizeToBuff;
                    ReceiveDataSize -= CopySizeToBuff;
                    
                    //if the receiving buffer is full then write it to the flash
                    if(AdressToCopyInBuff == BUFFER_SIZE){                   
                        //write to flash
                        Flash_Write(&NextWriteAddress, w_buffer, BUFFER_SIZE);                  
                        //update the flag that firmware is not available
                        if(ReadFlashAddress(HAS_FIRMWARE_FLASHED) == D_PASSWORD)
                          updateFlash(w_buffer, HAS_FIRMWARE_FLASHED, 0);
                        
                        //clear the buffer
                        //memset(w_buffer, 0, BUFFER_SIZE);
                        AdressToCopyInBuff = 0;
                        
                        #ifdef DEBUG_MS_REPRINTF
                        printf_h("Writ Fah");
                        #endif 
                    }
                }
                #ifdef DEBUG_MS_REPRINTF
                     printf_h("\r\n");
                #endif 
            }else if(stData.WR == READ_REGISTER &&  stData.Command == REGISTER_VERSION_DB ){
                //UCHAR Version_Ay[6] = {1, 1, 1, 0, 0, 1}; 
                  
                Com_Write(READ_REGISTER, REGISTER_VERSION_DB, 6, Version_Ay);
                
            }else if(stData.WR == READ_REGISTER && stData.Command == REGISTER_VERSION){
                if(ReadFlashAddress(HAS_FIRMWARE_FLASHED) == D_PASSWORD){
                    Com_Write(READ_REGISTER, REGISTER_VERSION, 2, (UCHAR*)&ExpectedPacketNumberLowByte);
                }
            }else{
                //printf_h("stData.Command:0x%x\r\n",stData.Command);
            }
        }       
        #ifdef DEBUG_MS_REPRINTF
        printf_h("Timout\r\n");
        #endif 
    } 
APP:  
    __disable_irq(); // 进入以下的程序跳转前一定要先关闭中断，不然在某些不确定情况下，会跳转失败，应用程序运行不起来，而且热启动会失败  韩红海  20210408
    //IF THE FIRMWARE PRESENT THEN BOOT THE FIRMWARE OTHERWISE RESET
    FlagToBUZZER =0;
    if(ReadFlashAddress(HAS_FIRMWARE_FLASHED) == D_PASSWORD)
    {
       #ifdef DEBUG_MS_REPRINTF
       printf_h("RunApp\r\n");
       #endif
              
        #ifdef _SP100W_         
            #ifdef DEBUG_MS_REPRINTF
            HW_USART0_DeInitial();//应用程序可能把这个串口当IO使用，所以要恢复成IO
            #endif
        #else
            #ifdef DEBUG_MS_REPRINTF
             USART2_DeInitial();//应用程序可能把这个串口当IO使用，所以要恢复成IO
            #endif
        #endif  
                        
       GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 0);////关闭蜂鸣器，
       BOOT_boot();
    }
    #ifdef _SP100W_   //100W电源如果升级中途数据中断，下次上电要运行0x13000处的出厂备份程序，要用备份程序和模组建立通信才能为再次升级做准备，
                      //因为升级失败后如果没有备份程序那么上电主板的MCU会检测不到100W电源的存在，会运行和下控直接通信的程序，这样就和100W电源通信不上了。也升级不了，
        else
        {
           #ifdef DEBUG_MS_REPRINTF
           printf_h("RunApp2\r\n");
           HW_USART0_DeInitial();   //应用程序可能把这个串口当IO使用，所以要恢复成IO      
           #endif 
           GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 0);////关闭蜂鸣器，
           BOOT_boot2();        
        }
    #else
        else
        {
           #ifdef DEBUG_MS_REPRINTF
           printf_h("Reset\r\n");
           printf_h("RunApp2\r\n");
            
           #endif
           BOOT_boot2();// 如果boot2地址处有有效的应用程序则运行，没有则重启
           NVIC_SystemReset();
        }
    #endif    
}