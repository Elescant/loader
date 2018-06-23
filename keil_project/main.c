#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdbool.h"
#include "flash.h"
#include "uart.h"
#include "sys.h"
#include "ymodem.h"
#include "update.h"

#define FLASH_APP_ADDR (0x08010000)

void test_flash_bank(void);

u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size);
void iap_write_appbin(u32 appxaddr, u8 *appbuf, u32 appsize);
void iap_load_app(u32 appxaddr);

typedef void (*iapfun)(void);
iapfun jump2app;
void USART1_UserHandler(u8 data);
void USART2_UserHandler(u8 data);

//__set_MSP(*(__IO uint32_t *)1234);

int main(void)
{

	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOC, &gpio);

	GPIO_ResetBits(GPIOC, GPIO_Pin_13);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_7;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio);

	SysTick_Init();
	SysTick_Cmd(ENABLE);

	Uart1_Init(USART1_UserHandler);
	Uart2_Init(USART2_UserHandler);

	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7) != RESET)
	{
		__disable_irq();
		vu32 temp = *(vu32 *)(FLASH_APP_ADDR + 4);
		if ((temp & 0xFF000000) == 0x08000000) //判断是否为0X08XXXXXX.
		{
			iap_load_app(FLASH_APP_ADDR); //执行FLASH APP代码
		}
	}
	else
	{
		printf("请通过Ymodem传输更新文件\n");
		file_get_by_ymodem();
		__disable_irq();
		iap_load_app(FLASH_APP_ADDR);
	}
	while (1)
		; //don't reach here
}

void iap_load_app(u32 appxaddr)
{
	if (((*(vu32 *)appxaddr) & 0x2FFE0000) == 0x20000000)
	{
		jump2app = (iapfun) * (vu32 *)(appxaddr + 4);
		__set_MSP(*(__IO uint32_t *)appxaddr);
		jump2app();
	}
}



void USART2_UserHandler(u8 data)
{
	printf("%c", data);
}
