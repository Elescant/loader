#include "stm32f10x_gpio.h"

void SysTick_Init(void);
__IO uint32_t sys_cnt_ms=0;

int main()
{
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpio);
	
	SysTick_Init();
	SysTick_Cmd(ENABLE);
	__enable_irq();
	while(1)
	{
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		Delay_ms(500);
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		Delay_ms(500);
	}
}



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


