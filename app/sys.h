#ifndef SYS_H_H
#define SYS_H_H

#include "stm32f10x.h"
#include "stdbool.h"

void Delay_ms(__IO u32 nMs);
void SysTick_Init(void);
void SysTick_Cmd(FunctionalState newstate);
u32 get_sys_ms(void);

enum {eTimYModem,FrameTimeOver,MaxTimeCnt};
extern vu32 TimerCnt[MaxTimeCnt];

#define IS_TIME_OUT(num,timeMs) (get_sys_ms()-TimerCnt[(num)]>(timeMs)?true:false)
#define CLR_TIME_OUT(num) (TimerCnt[num]=get_sys_ms())

//#define IS_TIMEOUT_1MS(index,count) ( (get_sys_ms() - TimerCnt[index])>count ? ((TimerCnt[index] = get_sys_ms())>0):false)

#define IS_TIMEOUT_1MS(index, count)    (get_sys_ms()-(TimerCnt[(u16)(index)]) >= (count)? true:false)
                                 


#endif
