#include "HW.h"
void HW_Initial_Clock(void);


void HW_SAFEKEY_IO(void)
{
    GPIO_PinModeSet(SAFEKEY_INT_PORT, SAFEKEY_INT_PIN, gpioModeInput, 0);
}

unsigned char HW_Speed_Get_Safetykey(void)
{
    return GPIO_PinInGet(SAFEKEY_INT_PORT, SAFEKEY_INT_PIN);
}

//--------------------About System clock intial Function-----------------------
void HW_Initial_Clock(void)
{
    CHIP_Init();
    CMU_TypeDef *cmu = CMU;
    
    /* Turning on HFXO to increase frequency accuracy. */
    /* Waiting until oscillator is stable */
    cmu->OSCENCMD = CMU_OSCENCMD_HFXOEN;
    while (!(cmu->STATUS && CMU_STATUS_HFXORDY));
    
    /* Switching the CPU clock source to HFXO */
    cmu->CMD = CMU_CMD_HFCLKSEL_HFXO;
    
    CMU_ClockDivSet(cmuClock_CORE, cmuClkDiv_2);
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_CORELE, true);
    CMU_ClockEnable(COM_CMU_CLOCK, true);
    CMU_ClockEnable(cmuClock_ADC0, true);
    CMU_ClockEnable(cmuClock_DMA, true);
    CMU_ClockEnable(cmuClock_TIMER1, true);
    CMU_ClockEnable(cmuClock_TIMER0, true);
    
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_CORELEDIV2);//cmuSelect_LFXO
    //CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2);
    //CMU_ClockEnable(cmuClock_LEUART0, true);
    //CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_8);
    
    SystemHFXOClockSet(24000000);  //24MHz
    SystemCoreClockUpdate();
//    MSC->READCTRL = (MSC->READCTRL & ~_MSC_READCTRL_MODE_MASK) | MSC_READCTRL_MODE_WS1;
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    HW_WDT_Initial_IO();
    SysTick_Config(SystemCoreClock / 1000);
}




//------------------------- com interface Function-------------------------
void HW_COM_Initial_IO(void)
{
    USART_InitAsync_TypeDef UART_init = USART_INITASYNC_DEFAULT;
    UART_init.baudrate = 115200;
    USART_InitAsync(COM_UART, &UART_init);
    
    COM_UART->IFC = _USART_IFC_MASK;
    NVIC_ClearPendingIRQ(COM_UART_Rx_IRQn);
    NVIC_EnableIRQ(COM_UART_Rx_IRQn);
    COM_UART->IEN = USART_IEN_RXDATAV;
    
    GPIO_PinModeSet(COM_UART_TX_PORT,COM_UART_TX_PIN,gpioModePushPull,1); //tx
    GPIO_PinModeSet(COM_UART_RX_PORT,COM_UART_RX_PIN,gpioModeInput,0); //rx
    
    COM_UART->ROUTE |= (COM_UART_ROUTE_EN | COM_UART_LOCATION);
    HW_Com_TX_INT_disable();
}


unsigned char HW_Com_Uart_Get_Rx(void)
{
    return USART_Rx(COM_UART);
}

void HW_Com_Uart_Send_Tx(unsigned char ucData)
{
    USART_Tx(COM_UART, ucData);
}


void HW_Com_TX_INT_enable(void)
{
    NVIC_EnableIRQ(COM_UART_Tx_IRQn);
    COM_UART->IEN |= USART_IEN_TXC ;
    COM_UART->IFS = USART_IFS_TXC ;
}


void HW_Com_TX_INT_disable(void)
{
    UINT16 TimeOut = 0;
    while (!(USART1->STATUS & USART_IF_TXC) && TimeOut < 10000)
    {
        TimeOut++;
    }
    NVIC_DisableIRQ(COM_UART_Tx_IRQn);
    COM_UART->IEN  &= (~USART_IEN_TXC) ;
    COM_UART->IFS  &= (~USART_IFS_TXC) ;
}


/****************************************************************************/

void HW_WDT_Initial_IO(void)
{
    /* Defining the watchdog initialization data */
    WDOG_Init_TypeDef init =
    {
        .enable     = true,               /* Start watchdog when init done */
        .debugRun   = false,              /* WDOG not counting during debug halt */
        .em2Run     = false,               /* WDOG counting when in EM2 */
        .em3Run     = false,               /* WDOG counting when in EM3 */
        .em4Block   = false,              /* EM4 can be entered */
        .swoscBlock = false,              /* Do not block disabling LFRCO/LFXO in CMU */
        .lock       = false,              /* Do not lock WDOG configuration (if locked, reset needed to unlock) */
        .clkSel     = wdogClkSelLFRCO,   /* Select 1kHZ WDOG oscillator */
        .perSel     = wdogPeriod_8k,     /* Set the watchdog period to 2049 clock periods (ie ~2 seconds)*/
    };
    
    /* Initializing watchdog with choosen settings */
    WDOG_Init(&init);
    while (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CTRL);
    WDOG_Feed();
}


void HW_Initial_IO()
{
   HW_Initial_Clock(); 
   HW_COM_Initial_IO();
   HW_WDT_Initial_IO();
}


//--------------------------Local function-----------------------------//
//needn't change as usual by project engineer
void HW_Set_IO_Output(GPIO_Port_TypeDef by_Port, UINT w_Pin)
{
    GPIO_PinModeSet(by_Port, w_Pin, gpioModePushPull, 0);
}

void HW_Set_IO_Input(GPIO_Port_TypeDef by_Port, UINT w_Pin)
{
    GPIO_PinModeSet(by_Port, w_Pin, gpioModeInput, 1);
}

void HW_Set_IO_High(GPIO_Port_TypeDef by_Port, UINT w_Pin)
{
    GPIO_PinOutSet(by_Port,w_Pin);
}

void HW_Set_IO_Low(GPIO_Port_TypeDef by_Port, UINT w_Pin)
{
    GPIO_PinOutClear(by_Port,w_Pin);
}

UCHAR HW_Get_IO(GPIO_Port_TypeDef by_Port, UINT w_Pin)
{
  int data;
  if (w_Pin < 8)
  {
          data= (GPIO->P[by_Port].MODEL & (0xF << (w_Pin * 4)))>>(w_Pin * 4);                     
  }
  else
  {
          data= (GPIO->P[by_Port].MODEH & (0xF << ((w_Pin - 8) * 4)))>>((w_Pin - 8) * 4);                 
  }

  if(data<4)//当前GPIO为输入状态，否则为输出状态
    return( (UCHAR)GPIO_PinInGet(by_Port,w_Pin) );
  else
    return( (UCHAR)GPIO_PinOutGet(by_Port,w_Pin) );
}
