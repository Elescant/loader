#include "sys.h"
#include "stm32f10x.h"

__IO uint32_t sys_cnt_ms=0;

vu32 TimerCnt[MaxTimeCnt]={0,};

void SysTick_Init(void)
{
	if(SysTick_Config(SystemCoreClock/1000))
	{
		while(1);
	}
	SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Cmd(FunctionalState newstate)
{
	if(newstate)
	{
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	}
	else
	{
		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	}
	sys_cnt_ms = 0;
}

u32 get_sys_ms(void)
{
	return sys_cnt_ms;
}

void Delay_ms(__IO u32 nMs)
{
	uint32_t cur = get_sys_ms();
	while(get_sys_ms() - cur < nMs);
}

void SysTick_Handler(void)
{
	sys_cnt_ms++;
}
