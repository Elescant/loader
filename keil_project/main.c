#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdbool.h"
#include "flash.h"

#define FLASH_APP_ADDR (0x08010000)

__IO uint32_t sys_cnt_ms=0;

void Delay_ms(__IO u32 nMs);
void SysTick_Init(void);
u32 get_sys_ms(void);
void test_flash_bank(void);
void SysTick_Cmd(FunctionalState newstate);
//u16 Flash_WriteBank(u32 addr,void *pdat,u16 nbytes);
u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size);
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize);
void iap_load_app(u32 appxaddr);

typedef void (*iapfun)(void);
iapfun jump2app;

__asm void MSR_MSP(u32 addr)
{
	MSR MSP,r0
	BX r14
}

//__set_MSP(*(__IO uint32_t *)1234);

u8 RxBuf[4096];
u16 bufcnt=0;
bool update=false;
u16 applen=0;


int main(void)
{

	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpio);
	
	//GPIO_ResetBits(GPIOC,GPIO_Pin_13);
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
	
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
	
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = USART1_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&nvic);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	USART_Init(USART1, &usart);
	USART_Cmd(USART1,ENABLE);

	SysTick_Init();
	SysTick_Cmd(ENABLE);
	
//	test_flash_bank();
	
	u16 oldcnt=0;

//	__disable_irq();
//	 vu32 temp = *(vu32*)(FLASH_APP_ADDR+4);
//	if((temp&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
//	{	 
//		iap_load_app(FLASH_APP_ADDR);//执行FLASH APP代码
//	}


	while(1)
	{
		Delay_ms(10);
		if(bufcnt)
		{
			if(bufcnt == oldcnt)
			{
				printf("接收完成,%d 字节\n",bufcnt);
				applen = bufcnt;
				update = true;
				bufcnt = 0;
				oldcnt = 0;
			}else
			{
				oldcnt = bufcnt;
			}
		}
		if(update)
		{
			update = false;
			printf("开始更新\n");
			__disable_irq();
			iap_write_appbin(FLASH_APP_ADDR,RxBuf,applen);
			iap_load_app(FLASH_APP_ADDR);
		}
	}
}

void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000) == 0x20000000)
	{
		jump2app = (iapfun)*(vu32 *)(appxaddr + 4);
		//MSR_MSP(*(vu32*)appxaddr);
		__set_MSP(*(__IO uint32_t*) appxaddr);
		jump2app();
	}
}

u16 iapbuf[1024];
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u16 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];	  
		dfu+=2;//偏移2个字节
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
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
	u8 data2[11]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB};
		
	FLASH_Unlock();
	FLASH_ErasePage(FLASH_APP_ADDR-2);
	FLASH_ErasePage(FLASH_APP_ADDR);
	FLASH_ProgramHalfWord(FLASH_APP_ADDR - 2,0xAABB);
	FLASH_ProgramHalfWord(FLASH_APP_ADDR,0xCCDD);
//	FLASH_ErasePage(FLASH_APP_ADDR-1024);
//	ret = FLASH_WriteBank(data,FLASH_APP_ADDR-11,5);
//	ret = FLASH_WriteBank(data,FLASH_APP_ADDR,11);
	FLASH_Lock();
	if(ret)
	{
			printf("write success\n");
	}else
	{
		  printf("write failed\n");
	}

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
            return false;
        }
    }

    return true;
}

void USART1_IRQHandler(void)
{
	u8 res;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
	{
		res = USART_ReceiveData(USART1);
		if(bufcnt<4096)
		{
			RxBuf[bufcnt] = res;
			bufcnt++;
		}
	}
}


