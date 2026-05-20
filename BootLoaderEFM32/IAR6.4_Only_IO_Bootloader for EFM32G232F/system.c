#include "system.h"
#include "HW.h"
#include "Com_APP.h"
#include "JisTimer.h"

void System_Io_Init(void)
{
  HW_Initial_IO();
}

void SysTick_Handler(void)
{    
    static char counter_100ms = 0;
    WDOG_Feed();
    
    if((++counter_100ms) % 100 == 0)
    {
      Timer_100ms_Int();
    }
}

void USART1_RX_IRQHandler(void)
{
    if (USART1->STATUS & USART_STATUS_RXDATAV)
    {
        Com_Rx_Int(USART_Rx(COM_UART));
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