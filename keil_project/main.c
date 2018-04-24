#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stdio.h"
#include "stm32f10x_flash.h"

#define FLASH_APP_ADDR (0x08010000)

__IO uint32_t sys_cnt_ms=0;

void Delay_ms(__IO u32 nMs);
void SysTick_Init(void);
u32 get_sys_ms(void);
void test_flash_bank(void);
void SysTick_Cmd(FunctionalState newstate);
//u16 Flash_WriteBank(u32 addr,void *pdat,u16 nbytes);
u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size);

typedef void (*iapfun)(void);
iapfun jump2app;

__asm void MSR_MSP(u32 addr)
{
	MSR MSP,r0
	BX r14
}

//__set_MSP(*(__IO uint32_t *)1234);


int main(void)
{
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpio);
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
	//GPIO_SetBits(GPIOC,GPIO_Pin_13);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA,&gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_10;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&gpio);
	
	usart.USART_BaudRate = 115200;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1, &usart);
	USART_Cmd(USART1,ENABLE);

	SysTick_Init();
	SysTick_Cmd(ENABLE);
	
	test_flash_bank();
	while(1)
	{
//		while(!USART_GetFlagStatus(USART1,USART_FLAG_RXNE));
//		c = USART_ReceiveData(USART1);
		Delay_ms(1000);
		printf("%ud",sys_cnt_ms);
	}
}

void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000) == 0x20000000)
	{
		jump2app = (iapfun)*(vu32 *)(appxaddr + 4);
		MSR_MSP(*(vu32*)appxaddr);
		jump2app();
	}
}

int fputc(int ch,FILE *f)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	USART_SendData(USART1,(unsigned char)ch);
	return (ch);
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

void Delay_ms(__IO u32 nMs)
{
	uint32_t cur = get_sys_ms();
	while(get_sys_ms() - cur < nMs);
}

void SysTick_Handler(void)
{
	sys_cnt_ms++;
}

u32 get_sys_ms(void)
{
	return sys_cnt_ms;
}

void test_flash_bank(void)
{
	u16 ret;
	u8 data[11]={0x12,0x34,0xFF,0xFF,0xFF,4,3,2,1,0,0xAA};
		
	FLASH_Unlock();
	FLASH_ErasePage(FLASH_APP_ADDR);
	ret = FLASH_WriteBank(data,FLASH_APP_ADDR,11);
	FLASH_Lock();
	printf("ret %d\n",ret);
}


u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size)
{
    vu16 *pDataTemp = (vu16 *)pData;
    vu32 temp = addr;

    for (; temp < (addr + size); pDataTemp++, temp += 2)
    {
        FLASH_ProgramHalfWord(temp, *pDataTemp);
        if (*pDataTemp != *(vu16 *)temp)
        {
            return FALSE;
        }
    }

    return TRUE;
}

