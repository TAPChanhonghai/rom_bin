
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "HW.h"
#ifdef DEBUG_MS_REPRINTF
#include <stdio.h>
#endif
extern uint32_t systimeMs;
extern uint32_t debug_flag;
void System_Io_Init(void);
#endif