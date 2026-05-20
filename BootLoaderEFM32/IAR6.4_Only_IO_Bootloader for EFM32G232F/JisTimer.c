#include "JisTimer.h"

#include  <stdarg.h>
#include  <math.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#define T_CHANNEL_MAX 32

static UINT32 w_Timer_Flag;
static UINT16 aw_Timer_Data[T_CHANNEL_MAX];


void Timer_Init(void)
{
    w_Timer_Flag = 0;
    memset(aw_Timer_Data,0,sizeof(aw_Timer_Data));
}

void Timer_100ms_Int(void)
{
    w_Timer_Flag = 0xffffffff;
}

void Timer_Clear(UCHAR by_Index)
{
    aw_Timer_Data[by_Index] = 0;
}

UCHAR Timer_Counter(UCHAR by_Index,UINT16 w_Dat)
{
    UCHAR by_Channel;
    UINT32 w_Mask;
    
    by_Channel = by_Index & 0x7f;
    //如超过最大32个定时器，则返回0
    if(by_Channel>T_CHANNEL_MAX) return(0);
    
    w_Mask = 0x01 << by_Channel;
    
    //每过100ms执行一次当前定时器.
    if((w_Timer_Flag & w_Mask) == 0) return(0);  
    
    //相应标志位被清零
    w_Timer_Flag &= ~w_Mask;
    
    //每执行一次相应的定时器就加1
    if(aw_Timer_Data[by_Channel] < 0xFFFF)
    {
        aw_Timer_Data[by_Channel]++;
    }
    
    //等于目标值返回1,保证每计数一次定时器，只会触发一次
    if(aw_Timer_Data[by_Channel] == w_Dat)
    {
        if(by_Index & T_LOOP) aw_Timer_Data[by_Channel] = 0;   //循环计数
        return(1);
    }
    
    //默认返回0
    return(0);
}
