#ifndef __HW_H__
#define __HW_H__

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
//#include "em_timer.h"
//#include "em_letimer.h"
#include "em_leuart.h"
//#include "em_adc.h"
#include "em_wdog.h"
//#include "em_acmp.h"
#include "JIStypes.h"

//#define printf_h(expr,...)    ((void)0);
#define printf_h    printf

//#define   _WDOG_ENABLE_


#define BAUTDDATA_TO_APP            115200 
#define SPEED_KM_MIN	    8
#define SPEED_KM_MAX	    180

#define SPEED_MILE_MIN	    5
#define SPEED_MILE_MAX	    112


#define THIS_MODEL          "EBWhite"
#define THIS_S_VER          "10101"

#define C_PASSWORD          0x55aa5a5a

#define C_MILE	            0           
#define C_KM	            1

#define INT_FALLING_EDGE	0
#define INT_RISING_EDGE		1

//-------------------------Digital communication interface----------------------
//THIS UART IS USED TO CONNECT TO THE CORE CPU
//   和APP通信的串口号
#define COM_TO_APP_CMU_CLOCK               cmuClock_USART1
#define COM_TO_APP_UART_STATUS             USART1->STATUS
#define COM_TO_APP_UART_UARTTXC            USART_STATUS_TXC
#define COM_TO_APP_UART                    USART1
#define COM_TO_APP_UART_Rx_IRQn            USART1_RX_IRQn
#define COM_TO_APP_UART_Tx_IRQn            USART1_TX_IRQn
#define COM_TO_APP_UART_TX_PORT            gpioPortC
#define COM_TO_APP_UART_TX_PIN             0
#define COM_TO_APP_UART_RX_PORT            gpioPortC
#define COM_TO_APP_UART_RX_PIN             1
#define COM_TO_APP_UART_LOCATION           USART_ROUTE_LOCATION_LOC0
#define COM_TO_APP_UART_ROUTE_EN           USART_ROUTE_TXPEN | USART_ROUTE_RXPEN


//////////////////////////////DEBUG/////////////////////////////////////////////////////////
#define COM_USART2_CLOCK               	cmuClock_USART2
#define COM_USART2_STATUS          	    USART2->STATUS
#define COM_USART2_UARTTXC            	USART_STATUS_TXC
#define COM_USART2                    	USART2
#define COM_USART2_Rx_IRQn            	USART2_RX_IRQn
#define COM_USART2_Tx_IRQn            	USART2_TX_IRQn
#define COM_USART2_TX_PORT            	gpioPortC
#define COM_USART2_TX_PIN             	2
#define COM_USART2_RX_PORT            	gpioPortC
#define COM_USART2_RX_PIN             	3
#define COM_USART2_LOCATION           	USART_ROUTE_LOCATION_LOC0
#define COM_USART2_ROUTE_EN           	USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
////////////////////////////////////////////////////////////////////////////////////////
//************************************Leuart0 TO APP  for  100W Poer ***********************************

#define COMMU_MCU0_TX_PORT                      gpioPortD
#define COMMU_MCU0_TX_BIT                       4
#define COMMU_MCU0_TX_MODE                      gpioModePushPull

#define COMMU_MCU0_RX_PORT                      gpioPortD
#define COMMU_MCU0_RX_BIT                       5
#define COMMU_MCU0_RX_MODE                      gpioModeInputPull

#define DIGITAL_DIR_PORT            			gpioPortD
#define DIGITAL_DIR_BIT             			6
#define DIGITAL_DIR_MODE            			gpioModePushPull

#define LINK_COMMU_MCU0_UART                    cmuClock_LEUART0

#define COMMU_MCU0_UART                         LEUART0
#define COMMU_MCU0_ROUTE_EN                     LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN
#define COMMU_MCU0_ROUTE_LOCATION               LEUART_ROUTE_LOCATION_LOC0

#define LINK_REF_FREQUENCY0                     CMU_ClockFreqGet(LINK_COMMU_MCU0_UART)
#define COMMU_MCU0_IRQ                          LEUART0_IRQn
#define LEUART0_STATUS                          LEUART0->STATUS
#define UARTTXC                                 LEUART_STATUS_TXC
/**************************************debug for 100W **************************************************************/
#define COM_CMU_CLOCK_OXYGENERATOR               cmuClock_USART0
#define COM_UART_STATUS_OXYGENERATOR  	         USART0->STATUS
#define COM_UART_UARTTXC_OXYGENERATOR            USART_STATUS_TXC
#define COM_UART_OXYGENERATOR                    USART0
#define COM_UART_Rx_IRQn_OXYGENERATOR            USART0_RX_IRQn
#define COM_UART_Tx_IRQn_OXYGENERATOR            USART0_TX_IRQn
#define COM_UART_TX_PORT_OXYGENERATOR            gpioPortE
#define COM_UART_TX_PIN_OXYGENERATOR             10
#define COM_UART_RX_PORT_OXYGENERATOR            gpioPortE
#define COM_UART_RX_PIN_OXYGENERATOR             11
#define COM_UART_LOCATION_OXYGENERATOR           USART_ROUTE_LOCATION_LOC0
#define COM_UART_ROUTE_EN_OXYGENERATOR           USART_ROUTE_TXPEN | USART_ROUTE_RXPEN
/*****************************************************************************************************/
//************************************SAFE KEY***********************************
#define SAFEKEY_INT_PORT            gpioPortF
#define SAFEKEY_INT_PIN             3

//************************************HEART & POLAR***********************************
#define HEART_PORT                  gpioPortC
#define HEART_PIN                   5//4
#define HEART_IRQn                  GPIO_ODD_IRQn

#define POLAR_PORT                  gpioPortC
#define POLAR_PIN                   2//2
//#define POLAR_IRQn                  GPIO_EVEN_IRQn

//************************************FAN***********************************

#define FAN_IO_PWM_PORT             gpioPortA 
#define FAN_IO_PWM_PIN              2
#define FAN_TIMER                   TIMER0
#define FAN_CCP                     2
#define FAN_TIMER_ROUTE_CCPEN       TIMER_ROUTE_CC2PEN
#define FAM_TIMER_ROUTE_LOCATION    TIMER_ROUTE_LOCATION_LOC1
#define FAN_IO_Sel_Low()            FAN_TIMER->ROUTE &= ~(FAN_TIMER_ROUTE_CCPEN | FAM_TIMER_ROUTE_LOCATION)
#define FAN_IO_Sel_High()           FAN_TIMER->ROUTE |=  (FAN_TIMER_ROUTE_CCPEN | FAM_TIMER_ROUTE_LOCATION)
#define FAN_TIMER_TOP               11764
#define FAN_SPEED_MIN			    6000//8000
#define FAN_SPEED_MID			    7000//9500
#define FAN_SPEED_MAX			    10587//10943


//************************************RF*****************************************
#define I2C_WakeUp_Key_PORT   gpioPortA
#define I2C_WakeUp_Key_PIN    8

//************************************BOOTLOAD***********************************
#define BOOTLOAD_INT_PORT           gpioPortB
#define BOOTLOAD_INT_PIN            7


//************************************POWER ON***********************************

#define SYS_POWER_ON_PORT           gpioPortC
#define SYS_POWER_ON_PIN            3
//************************************POWER ON***********************************




void HW_Initial_IO();
void HW_Initial_Clock(void);

void HW_COM_Initial_IO(void);
unsigned char HW_Com_Uart_Get_Rx(void);
void HW_Com_Uart_Send_Tx(unsigned char ucData);
void HW_Com_TX_INT_enable(void);
void HW_Com_TX_INT_disable(void);

void HW_Digital_Initial_IO(void);
void HW_Digital_EnableTx(UCHAR by_Dat);
void HW_Digital_SendData(UCHAR by_Dat);
void HW_Bootload_Initial_IO(void);
unsigned char HW_Bootload_Check_Int_IO(void);

void HW_SAFEKEY_IO(void);
unsigned char HW_Speed_Get_Safetykey(void);
void delay_ms(int t);
void delay_us(unsigned long dat);

void HW_Heart_Initial_IO(void);
void HW_Fan_Initial_IO(void);
void HW_WDT_Initial_IO(void);
void HW_Set_IO_Output(GPIO_Port_TypeDef by_Port, UINT w_Pin);
void HW_Set_IO_Input(GPIO_Port_TypeDef by_Port, UINT w_Pin);
void HW_Set_IO_High(GPIO_Port_TypeDef by_Port, UINT w_Pin);
void HW_Set_IO_Low(GPIO_Port_TypeDef by_Port, UINT w_Pin);
UCHAR HW_Get_IO(GPIO_Port_TypeDef by_Port, UINT w_Pin);


//added by rupesh
void HW_Send_MainMCU_ERP_Signal(unsigned char data);
void HW_Digital_Set_DIR(UCHAR dir);
unsigned char HW_KeyPad_Check_Int_IO(void);
UINT HW_Ditital_Get_TX_Idle(void);
UINT HW_Digital_Get_DIR(void);

void HW_LEUART0_Initial(void);
void HW_USART0_Initial(void);
void HW_Initial_Clock(void);
void USART2_Initial(void);
void HW_Leuart0_SendData(UCHAR by_Dat);
void HW_Leuart0_EnableTx(UCHAR by_Dat);
void USART2_DeInitial(void);
void HW_USART0_DeInitial(void);
#endif
