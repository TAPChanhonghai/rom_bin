#include "system.h"
#include "HW.h"
#include "Com_APP.h"
#include "JisTimer.h"
#include <stdlib.h>  

uint32_t debug_flag=1;
uint32_t systimeMs=0;
uint32_t FlagToBUZZER=0; 
uint32_t TimeToBUZZER=0;
void SysTick_Handler(void)
{    
  #if  defined(_WDOG_ENABLE_)
    WDOG_Feed();
  #endif
    systimeMs++;
    
    
  if(FlagToBUZZER)
  {
      if( systimeMs%3000 == 0){
          GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 1);//´ò¿ª·äÃùÆ÷£¬
          TimeToBUZZER=50;
      }else if(--TimeToBUZZER==0) {
          GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 0);//¹Ø±Õ·äÃùÆ÷£¬            
      } 
  }
     
}


#ifdef _SP100W_
    void LEUART0_IRQHandler(void)                                   //COMMUNICATE WITH APP
    {
        unsigned long state;
        state = LEUART_IntGet(COMMU_MCU0_UART);

        LEUART_IntClear(COMMU_MCU0_UART, ~_LEUART_IFC_RESETVALUE);

        if (state & LEUART_IF_TXC)
        {
                Com_Tx_Int();

        }
        if (state & LEUART_IF_RXDATAV)
        {
                Com_Rx_Int(LEUART_Rx(COMMU_MCU0_UART));
                //Com_Rx_Int_TimeoutMode(LEUART_Rx(COMMU_MCU0_UART));
        }
    }
    #define SizeOfUsart0RxBuff			   30
    char usart0RxDataBuf[SizeOfUsart0RxBuff];
    char p_usart0RxDataLong=0;
    void USART0_RX_IRQHandler(void)
    {
      uint8_t Data;
        if (USART0->STATUS & USART_STATUS_RXDATAV)
        {

                    Data=USART_Rx(USART0);
                    if((Data =='\n')||(Data =='\r')||(p_usart0RxDataLong>=SizeOfUsart0RxBuff)){
                        p_usart0RxDataLong=0;
                        if(strncmp(usart0RxDataBuf,"at+uart",7)==0){//at+SetupOff,0728                      
                                                  
                            //debug_flag ^= 1<< (atoi((char const *)&usart0RxDataBuf[8]));					  					  
                              debug_flag =  !debug_flag;
                        }else if(strncmp(usart0RxDataBuf,"at+help",7)==0){
                              #ifdef DEBUG_MS_REPRINTF  
                              //printf_h("at+uart \"open or close uart data\"\r\n");
                              #endif
                        }else if(strncmp(usart0RxDataBuf,"at+reset",8)==0){
                                NVIC_SystemReset();
                        }
                        memset(usart0RxDataBuf,0,SizeOfUsart0RxBuff);
                    }else{
                        usart0RxDataBuf[p_usart0RxDataLong++]=Data;
                    }
        }
    }
#else
    void USART1_RX_IRQHandler(void)
    {
        if (USART1->STATUS & USART_STATUS_RXDATAV)
        {
            Com_Rx_Int(USART_Rx(COM_TO_APP_UART));
            //Com_Rx_Int_TimeoutMode(USART_Rx(COM_TO_APP_UART));
        }
    }

    void USART1_TX_IRQHandler(void)
    {
        USART_IntClear(USART1, ~_USART_IFC_RESETVALUE);
        if (USART1->STATUS & USART_IF_TXC )
        {
            Com_Tx_Int();
        }
    }

    // ----------------------------- USART2 interrupt handler -----------------------------------
    #define SizeOfUsart2RxBuff			   30
    char usart2RxDataBuf[SizeOfUsart2RxBuff];
    char p_usart2RxDataLong=0;
    void USART2_RX_IRQHandler(void)
    {
      uint8_t Data;
        if (USART2->STATUS & USART_STATUS_RXDATAV)
        {

                    Data=USART_Rx(COM_USART2);
                    if((Data =='\n')||(Data =='\r')||(p_usart2RxDataLong>=SizeOfUsart2RxBuff)){
                        p_usart2RxDataLong=0;
                        if(strncmp(usart2RxDataBuf,"at+uart",7)==0){//at+SetupOff,0728                      
                                                  
                            //debug_flag ^= 1<< (atoi((char const *)&usart2RxDataBuf[8]));					  					  
                              debug_flag =  !debug_flag;
                        }else if(strncmp(usart2RxDataBuf,"at+help",7)==0){
                              #ifdef DEBUG_MS_REPRINTF  
                              printf_h("at+uart \"open or close uart data\"\r\n");
                              #endif
                        }else if(strncmp(usart2RxDataBuf,"at+reset",8)==0){
                                NVIC_SystemReset();
                        }
                        memset(usart2RxDataBuf,0,SizeOfUsart2RxBuff);
                    }else{
                        usart2RxDataBuf[p_usart2RxDataLong++]=Data;
                    }
        }
    }
    void USART2_TX_IRQHandler(void)
    {
        USART_IntClear(USART2, ~_USART_IFC_RESETVALUE);
        if (USART2->STATUS & USART_IF_TXC)
        {
            //Digital_Drve_Int();
         }
    }
#endif

//---------------------------------------------------------------------------