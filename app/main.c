#include "stm32f10x_gpio.h"
#include "stm32f10x.h"
#include "sys.h"


int main(void)
{
	__enable_irq();
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOC, &gpio);

	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	
	SysTick_Init();
	SysTick_Cmd(ENABLE);
	while(1)
	{
		Delay_ms(500);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		Delay_ms(500);
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}
}
