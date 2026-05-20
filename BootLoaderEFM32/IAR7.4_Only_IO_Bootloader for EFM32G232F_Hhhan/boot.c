/**************************************************************************//**
 * @file
 * @brief Boot Loader
 * @author Energy Micro AS
* @version 1.50
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include "efm32.h"
#include "boot.h"
#include "Bootload_Cfg.h"


/**************************************************************************//**
 * @brief This function sets up the Cortex M-3 with a new SP and PC.
 *****************************************************************************/
#if defined ( __CC_ARM   )
__asm void BOOT_jump(uint32_t sp, uint32_t pc)
{
  /* Set new MSP, PSP based on SP (r0)*/
  msr msp, r0
  msr psp, r0

  /* Jump to PC (r1)*/
  mov pc, r1
}
#else
__ramfunc void BOOT_jump(uint32_t sp, uint32_t pc)
{
  (void) sp;
  (void) pc;
  /* Set new MSP, PSP based on SP (r0)*/
  __asm("msr msp, r0");
  __asm("msr psp, r0");

  /* Jump to PC (r1)*/
  __asm("mov pc, r1");
}
#endif

/**************************************************************************//**
 * @brief Boots the application
 *****************************************************************************/
__ramfunc void BOOT_boot(void)
{
  uint32_t pc, sp;

  /* Reset registers */
  /* Clear all interrupts set. */
  NVIC->ICER[0] = 0xFFFFFFFF;
  NVIC->ICER[1] = 0xFFFFFFFF;
  
  /* Set new vector table */
  SCB->VTOR = (uint32_t) APPLICATION_START_ADDRESS;
  /* Read new SP and PC from vector table */
  sp = *((uint32_t *) APPLICATION_START_ADDRESS);
  pc = *((uint32_t *) APPLICATION_START_ADDRESS + 1);

  BOOT_jump(sp, pc);
}
/**************************************************************************//**
 * @brief Boots the application
 *****************************************************************************/
__ramfunc __noreturn void BOOT_boot_(void)
{
  uint32_t pc, sp;

  /* Reset registers */

#if defined(USART_OVERLAPS_WITH_BOOTLOADER) \
    && !defined(_SILICON_LABS_32B_PLATFORM_2)
  CMU->LFBCLKEN0    = _CMU_LFBCLKEN0_RESETVALUE;
  GPIO->ROUTE       = _GPIO_ROUTE_RESETVALUE;
  GPIO->EXTIPSELL   = _GPIO_EXTIPSELL_RESETVALUE;
  GPIO->EXTIFALL    = _GPIO_EXTIFALL_RESETVALUE;
  GPIO->IEN         = _GPIO_IEN_RESETVALUE;
  GPIO->IFC         = 0xFFFFFFFF;
#endif

  /* Reset GPIO settings */
  GPIO->P[5].MODEL  = _GPIO_P_MODEL_RESETVALUE;
  /* Clear all interrupts set. */
  NVIC->ICER[0]     = 0xFFFFFFFF;
#if ( __CORTEX_M != 0 )
  NVIC->ICER[1]     = 0xFFFFFFFF;
#endif

#if defined(_SILICON_LABS_32B_PLATFORM_2)
  BOOTLOADER_USART->CLKDIV    = _USART_CLKDIV_RESETVALUE;
  BOOTLOADER_USART->ROUTEPEN  = _USART_ROUTEPEN_RESETVALUE;
  BOOTLOADER_USART->ROUTELOC0 = _USART_ROUTELOC0_RESETVALUE;

  LDMA->CTRL = 0x80000000;                    // Reset LDMA peripheral.
  LDMA->CTRL = _LDMA_CTRL_RESETVALUE;

  RTCC->IEN        = _RTCC_IEN_RESETVALUE;
  RTCC->IFC        = 0xFFFFFFFF;
  RTCC->CC[1].CCV  = _RTCC_CC_CCV_RESETVALUE;
  RTCC->CC[1].CTRL = _RTCC_CC_CTRL_RESETVALUE;
  RTCC->CTRL       = _RTCC_CTRL_RESETVALUE;

  MSC->WDATA     = _MSC_WDATA_RESETVALUE;
  MSC->ADDRB     = _MSC_ADDRB_RESETVALUE;
  MSC->WRITECTRL = _MSC_WRITECTRL_RESETVALUE;
  MSC->LOCK      = MSC_LOCK_LOCKKEY_LOCK;

  CMU->LFECLKSEL   = _CMU_LFECLKSEL_RESETVALUE;
  CMU->LFECLKEN0   = _CMU_LFECLKEN0_RESETVALUE;
  CMU->HFBUSCLKEN0 = _CMU_HFBUSCLKEN0_RESETVALUE;
  CMU->HFPERCLKEN0 = _CMU_HFPERCLKEN0_RESETVALUE;
  CMU->OSCENCMD    = CMU_OSCENCMD_LFRCODIS;
  CMU->CTRL        = _CMU_CTRL_RESETVALUE;
  // Change to default clock frequency.
  CMU->HFRCOCTRL   = *(uint32_t *)(&DEVINFO->HFRCOCAL8);

#else
  RTC->CTRL         = _RTC_CTRL_RESETVALUE;
  RTC->COMP0        = _RTC_COMP0_RESETVALUE;
  RTC->IEN          = _RTC_IEN_RESETVALUE;
  /* Disable RTC clock */
  CMU->LFACLKEN0    = _CMU_LFACLKEN0_RESETVALUE;
  CMU->LFCLKSEL     = _CMU_LFCLKSEL_RESETVALUE;
  /* Disable LFRCO */
  CMU->OSCENCMD     = CMU_OSCENCMD_LFRCODIS;
  /* Disable LE interface */
  CMU->HFCORECLKEN0 = _CMU_HFCORECLKEN0_RESETVALUE;
  /* Reset clocks */
  CMU->HFPERCLKDIV  = _CMU_HFPERCLKDIV_RESETVALUE;
  CMU->HFPERCLKEN0  = _CMU_HFPERCLKEN0_RESETVALUE;
#endif

  /* Set new vector table */
  SCB->VTOR = (uint32_t)APPLICATION_START_ADDRESS;

  /* Read new SP and PC from vector table */
  sp = *((uint32_t *)APPLICATION_START_ADDRESS);
  pc = *((uint32_t *)APPLICATION_START_ADDRESS + 1);

#if defined(_SILICON_LABS_32B_PLATFORM_2)
  /* Invalidate I-cache before executing from main flash. */
  MSC->CACHECMD = MSC_CACHECMD_INVCACHE;
#endif

  BOOT_jump(sp, pc);
}
/**************************************************************************//**
 * @brief Boots the application
 *****************************************************************************/
__ramfunc void BOOT_boot2(void)
{
  uint32_t pc, sp;

  /* Reset registers */
  /* Clear all interrupts set. */
  NVIC->ICER[0] = 0xFFFFFFFF;
  NVIC->ICER[1] = 0xFFFFFFFF;
  
  /* Set new vector table */
  SCB->VTOR = (uint32_t) APPLICATION_START_ADDRESS_2;
  /* Read new SP and PC from vector table */
  sp = *((uint32_t *) APPLICATION_START_ADDRESS_2);
  pc = *((uint32_t *) APPLICATION_START_ADDRESS_2 + 1);

  BOOT_jump(sp, pc);
}