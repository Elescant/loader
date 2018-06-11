#include "stdio.h"
#include "stm32f10x_gpio.h"

//SCB->VTOR = FLASH_BASE | 0x10000;

int main(void)
{
	SCB->VTOR = FLASH_BASE | 0x10000;
	
	GPIO_InitTypeDef gpio;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpio);
	
	while(1);
	return 0;
}

