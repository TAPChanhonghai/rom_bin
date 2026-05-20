#include "HW.h"
#include "system.h"



#if  defined(_WDOG_ENABLE_)
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
        .perSel     = wdogPeriod_32k,//wdogPeriod_8k,//wdogPeriod_8k,      256--8秒 32--1秒   16--0.5秒  8--0.2秒 
    };
    
    /* Initializing watchdog with choosen settings */
    WDOG_Init(&init);
    while (WDOG->SYNCBUSY & WDOG_SYNCBUSY_CTRL);
    WDOG_Feed();
}
#endif

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
    
#ifdef _SP100W_
    CMU_ClockEnable(cmuClock_USART0, true);// 
#else
    CMU_ClockEnable(COM_TO_APP_CMU_CLOCK, true);//  cmuClock_USART1
#endif
    //CMU_ClockEnable(cmuClock_ADC0, true);
    //CMU_ClockEnable(cmuClock_DMA, true);
    //CMU_ClockEnable(cmuClock_TIMER1, true);
    //CMU_ClockEnable(cmuClock_TIMER0, true);
        
    
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_CORELEDIV2);//cmuSelect_LFXO
    //CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2);

    #ifdef _SP100W_
     /*
    LEUART0的时钟有三个选择，cmuSelect_LFXO是外部32.768k(254---16384)，cmuSelect_CORELEDIV2是选择系统6M(46511----3000000)时钟，第三个时钟基本不选择，
    再通过CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_2); 来设置LEUART0的时钟
    cmuClkDiv_2(23255--1500000)
    cmuClkDiv_4(11627--750000)
    cmuClkDiv_8(2906--187000)
    分给LEUART0的时钟决定了它能用的波特率   最大波特率是时钟的二分之一 最小波特率是时钟的129分之一 cmuClkDiv_2时 961200
    */

    //if(LeuatBautdata<=4800){
            //CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);//外部32.768k(254---16384)      
    //}else{
            CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2);//cmuSelect_CORELEDIV2是选择系统6M
            CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_8);//cmuClkDiv_2(23255--1500000)  115200选这个      
    //}    
    CMU_ClockEnable(cmuClock_LEUART0, true);                    // LEUART0

    #endif
    

    CMU_ClockEnable(COM_USART2_CLOCK, true);	//非电源板上的调试串口
    SystemHFXOClockSet(24000000);  //24MHz
    
    SystemCoreClockUpdate();
//    MSC->READCTRL = (MSC->READCTRL & ~_MSC_READCTRL_MODE_MASK) | MSC_READCTRL_MODE_WS1;
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    #if  defined(_WDOG_ENABLE_)
        HW_WDT_Initial_IO();
    #endif
     
    SysTick_Config(SystemCoreClock / 1000);//  运行完此函数，SysTick中断就打开了  hhhan 2021 04 09

}

#ifndef _SP100W_
//------------------------- com interface TO APP -------------------------
void HW_COM_Initial_IO(void)
{
    USART_InitAsync_TypeDef UART_init = USART_INITASYNC_DEFAULT;
    UART_init.baudrate = BAUTDDATA_TO_APP;
    USART_InitAsync(COM_TO_APP_UART, &UART_init);
    
    COM_TO_APP_UART->IFC = _USART_IFC_MASK;
    NVIC_ClearPendingIRQ(COM_TO_APP_UART_Rx_IRQn);
    NVIC_EnableIRQ(COM_TO_APP_UART_Rx_IRQn);
    COM_TO_APP_UART->IEN = USART_IEN_RXDATAV;
    
    GPIO_PinModeSet(COM_TO_APP_UART_TX_PORT,COM_TO_APP_UART_TX_PIN,gpioModePushPull,1); //tx
    GPIO_PinModeSet(COM_TO_APP_UART_RX_PORT,COM_TO_APP_UART_RX_PIN,gpioModeInput,0); //rx
    
    COM_TO_APP_UART->ROUTE |= (COM_TO_APP_UART_ROUTE_EN | COM_TO_APP_UART_LOCATION);
    HW_Com_TX_INT_disable();
}

//unsigned char HW_Com_Uart_Get_Rx(void)
//{
//    return USART_Rx(COM_UART);
//}

void HW_Com_Uart_Send_Tx(unsigned char ucData)
{
    USART_Tx(COM_TO_APP_UART, ucData);
  #ifdef DEBUG_MS_REPRINTF  
    if(debug_flag)
    printf_h("%02X ",ucData);  
  #endif    
}

void HW_Com_TX_INT_enable(void)
{
    NVIC_EnableIRQ(COM_TO_APP_UART_Tx_IRQn);
    COM_TO_APP_UART->IEN |= USART_IEN_TXC ;
    COM_TO_APP_UART->IFS = USART_IFS_TXC ;
}

void HW_Com_TX_INT_disable(void)
{
    UINT16 TimeOut = 0;
    while (!(USART1->STATUS & USART_IF_TXC) && TimeOut < 10000)
    {
        TimeOut++;
    }
    NVIC_DisableIRQ(COM_TO_APP_UART_Tx_IRQn);
    COM_TO_APP_UART->IEN  &= (~USART_IEN_TXC) ;
    COM_TO_APP_UART->IFS  &= (~USART_IFS_TXC) ;
}
#endif
/****************************************************************************/

//void HW_Initial_IO()
//{
//   HW_Initial_Clock(); 
//   HW_COM_Initial_IO();
//   HW_WDT_Initial_IO();
//   USART2_Initial();
//}
#ifndef _SP100W_
//------------------------- USART2 -------------------------

//unsigned char USART2_Get_Rx(void)
//{
//    return USART_Rx(COM_USART2);
//}

//void USART2_Send_Tx(unsigned char ucData)
//{
//    USART_Tx(COM_USART2, ucData);
//}


#ifdef DEBUG_MS_REPRINTF  
    #ifndef _SP100W_
    int fputc(int ucData, FILE* f){ 
      USART_Tx(COM_USART2, (char)ucData);
      return ucData;
    }
    #endif
#endif


//void USART2_TX_INT_Enable(void)
//{
//    NVIC_EnableIRQ(COM_USART2_Tx_IRQn);
//    COM_USART2->IEN |= USART_IEN_TXC ;
//    COM_USART2->IFS = USART_IFS_TXC ;
//}


//void USART2_TX_INT_Disable(void)
//{
//    UINT16 TimeOut = 0;
//    while (!(COM_USART2->STATUS & USART_IF_TXC) && TimeOut < 10000)
//    {
//        TimeOut++;
//    }
//    NVIC_DisableIRQ(COM_USART2_Tx_IRQn);
//    COM_USART2->IEN  &= (~USART_IEN_TXC) ;
//    COM_USART2->IFS  &= (~USART_IFS_TXC) ;
//}
void USART2_Initial(void)
{
    USART_InitAsync_TypeDef UART_init = USART_INITASYNC_DEFAULT;
    //UART_init.parity = usartEvenParity;
    //UART_init.stopbits =usartStopbits0p5;
    //UART_init.baudrate = 9600;
    UART_init.baudrate = 1382400;
    USART_InitAsync(COM_USART2, &UART_init);

    COM_USART2->IFC = _USART_IFC_MASK;
    NVIC_ClearPendingIRQ(COM_USART2_Rx_IRQn);
    NVIC_EnableIRQ(COM_USART2_Rx_IRQn);
    COM_USART2->IEN = USART_IEN_RXDATAV;
//gpioModeWiredAndFilter    gpioModeWiredAndFilter   gpioModePushPull   gpioModeWiredOrPullDown 
   // gpioModeWiredAndPullUp    gpioModeWiredAndDrive
    GPIO_PinModeSet(COM_USART2_TX_PORT, COM_USART2_TX_PIN, gpioModePushPull, 1); //tx   
    GPIO_PinModeSet(COM_USART2_RX_PORT, COM_USART2_RX_PIN, gpioModeInput, 0); //rx

    COM_USART2->ROUTE |= (COM_USART2_ROUTE_EN | COM_USART2_LOCATION);
    //HW_Blu_TX_INT_Disable();
    //USART2_TX_INT_Disable();
}
void USART2_DeInitial(void)
{

    NVIC_ClearPendingIRQ(COM_USART2_Rx_IRQn);
    NVIC_DisableIRQ(COM_USART2_Rx_IRQn);


    COM_USART2->ROUTE &= ~(COM_USART2_ROUTE_EN | COM_USART2_LOCATION);

}
#endif
//------------------------- USART2 -------------------------end

#ifdef _SP100W_
/*****************************专门给100W电源用的和APP通信串口******************************************************************/
void HW_Leuart0_SendData(UCHAR by_Dat)
{
    LEUART_Tx(COMMU_MCU0_UART, by_Dat);
    #ifdef DEBUG_MS_REPRINTF
    if(debug_flag) 
        printf_h("%02X ",by_Dat);      
    #endif
}
void HW_Leuart0_EnableTx(UCHAR by_Dat)
{
    UINT16 TimeOut = 0;
    if (by_Dat == 0)
    {
        while (!(LEUART0_STATUS & UARTTXC) && TimeOut < 10000)
        {
            TimeOut++;
        }
        LEUART_IntDisable(COMMU_MCU0_UART,LEUART_IEN_TXC);
        //GPIO_PinOutClear(DIGITAL_DIR_PORT, DIGITAL_DIR_BIT);
    }
    else
    {
        //GPIO_PinOutSet(DIGITAL_DIR_PORT, DIGITAL_DIR_BIT);
        LEUART_IntEnable(COMMU_MCU0_UART, LEUART_IEN_TXC);
        LEUART_IntSet(COMMU_MCU0_UART, LEUART_IFS_TXC);
    }
}
void HW_LEUART0_Initial(void)
{
    LEUART_Init_TypeDef leuart0Init =
    {
        .enable   = leuartEnable,                           /* Activate data reception on LEUn_TX pin. */
        .refFreq  = LINK_REF_FREQUENCY0,                    /* Inherit the clock frequenzy from the LEUART clock source */
        .baudrate = BAUTDDATA_TO_APP,                       /* Baudrate = 9600 bps */
        .databits = leuartDatabits8,                        /* Each LEUART frame containes 8 databits */
        .parity   = leuartNoParity,                         /* No parity bits in use */
        .stopbits = leuartStopbits1,                        /* Setting the number of stop bits in a frame to 2 bitperiods */
    };

    /* Reseting and initializing LEUART0 */
    LEUART_Reset(COMMU_MCU0_UART);

    /* Route LEUART0 TX pin to DMA location 0 */
    COMMU_MCU0_UART->ROUTE = COMMU_MCU0_ROUTE_EN | COMMU_MCU0_ROUTE_LOCATION;

    LEUART_Init(COMMU_MCU0_UART, &leuart0Init);

    LEUART_IntEnable(COMMU_MCU0_UART, LEUART_IEN_RXDATAV);

    NVIC_SetPriority(COMMU_MCU0_IRQ,7);
    /* Enable LEUART0 interrupt vector */
    NVIC_EnableIRQ(COMMU_MCU0_IRQ);

    /* Enable GPIO for LEUART0.  */
    GPIO_PinModeSet(COMMU_MCU0_TX_PORT, COMMU_MCU0_TX_BIT, COMMU_MCU0_TX_MODE, 1);
    GPIO_PinModeSet(COMMU_MCU0_RX_PORT, COMMU_MCU0_RX_BIT, COMMU_MCU0_RX_MODE, 1);
    GPIO_PinModeSet(DIGITAL_DIR_PORT, DIGITAL_DIR_BIT, DIGITAL_DIR_MODE, 0);
}

/*****************************调试串口 专门给100W电源用的******************************************************************/
unsigned char HW_Oxygenerator_Uart_Get_Rx(void)
{
    return USART_Rx(COM_UART_OXYGENERATOR);
}
#ifdef DEBUG_MS_REPRINTF  
int fputc(int ucData, FILE* f){ 
    USART_Tx(COM_UART_OXYGENERATOR, (char)ucData);
    //USART_Tx(COM_UART_MCU2, (char)ucData);
  return ucData;
}
#endif
void HW_Oxygenerator_Uart_Send_Tx(unsigned char ucData)
{
    USART_Tx(COM_UART_OXYGENERATOR, ucData);
}

void HW_Oxygenerator_TX_INT_Enable(void)
{
    NVIC_EnableIRQ(COM_UART_Tx_IRQn_OXYGENERATOR);
    COM_UART_OXYGENERATOR->IEN |= USART_IEN_TXC ;
    COM_UART_OXYGENERATOR->IFS = USART_IFS_TXC ;
}

void HW_Oxygenerator_TX_INT_Disable(void)
{
    UINT16 TimeOut = 0;
    while (!(USART0->STATUS & USART_IF_TXC) && TimeOut < 10000)
    {
        TimeOut++;
    }
    NVIC_DisableIRQ(COM_UART_Tx_IRQn_OXYGENERATOR);
    COM_UART_OXYGENERATOR->IEN  &= (~USART_IEN_TXC) ;
    COM_UART_OXYGENERATOR->IFS  &= (~USART_IFS_TXC) ;
}
void HW_USART0_Initial(void)
{
    USART_InitAsync_TypeDef UART_init = USART_INITASYNC_DEFAULT;
    //UART_init.parity = usartEvenParity;
    //UART_init.stopbits =usartStopbits0p5;
    UART_init.baudrate = 1382400;
    USART_InitAsync(COM_UART_OXYGENERATOR, &UART_init);

    COM_UART_OXYGENERATOR->IFC = _USART_IFC_MASK;
    NVIC_ClearPendingIRQ(COM_UART_Rx_IRQn_OXYGENERATOR);
    NVIC_EnableIRQ(COM_UART_Rx_IRQn_OXYGENERATOR);//此串口是临时焊接出来作为调试用的，没有外接上拉和下拉电阻，打开接收中断后，如果没有和电脑串口通信，RX悬空，容易产生"干扰",
    //把下面的串口输入模式设置成gpioModeInputPull上拉输入，则会减小干扰，最好的办法就是外接上拉电阻，因为100W电源是开关电源，有几百K的频率存在
    //我在调试时焊接了2条线出来，更加剧了干扰。
    COM_UART_OXYGENERATOR->IEN = USART_IEN_RXDATAV;

    GPIO_PinModeSet(COM_UART_TX_PORT_OXYGENERATOR, COM_UART_TX_PIN_OXYGENERATOR, gpioModePushPull, 1); //tx
    GPIO_PinModeSet(COM_UART_RX_PORT_OXYGENERATOR, COM_UART_RX_PIN_OXYGENERATOR, gpioModeInputPull, 0); //rx  gpioModeInputPull(上拉输入)  gpioModeInput

    COM_UART_OXYGENERATOR->ROUTE |= (COM_UART_ROUTE_EN_OXYGENERATOR | COM_UART_LOCATION_OXYGENERATOR);
    //HW_Blu_TX_INT_Disable();

    HW_Oxygenerator_TX_INT_Disable();
}
void HW_USART0_DeInitial(void)
{
    COM_UART_OXYGENERATOR->ROUTE &= ~(COM_UART_ROUTE_EN_OXYGENERATOR | COM_UART_LOCATION_OXYGENERATOR);
}
#endif
/*****************************调试串口 专门给100W电源用的 end ******************************************************************/
