#include "uart.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "stdbool.h"

static Uart_RecvByteCb uart1_handler = NULL;
static Uart_RecvByteCb uart2_handler = NULL;

void Uart1_Init(Uart_RecvByteCb fun)
{
	USART_InitTypeDef usart;
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	
	if(fun)
	{
		uart1_handler = fun;
	}
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = GPIO_Pin_9;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_10;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&gpio);
	
	usart.USART_BaudRate = 115200;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	
	if(fun)
	{
		nvic.NVIC_IRQChannel = USART1_IRQn;
		nvic.NVIC_IRQChannelCmd = ENABLE;
		nvic.NVIC_IRQChannelPreemptionPriority = 3;
		nvic.NVIC_IRQChannelSubPriority = 3;
		NVIC_Init(&nvic);
		
		USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	}
	USART_Init(USART1, &usart);

	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
}

void Uart2_Init(Uart_RecvByteCb fun)
{
	USART_InitTypeDef usart;
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	
	if(fun)
	{
		uart2_handler = fun;
	}
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&gpio);
	
	usart.USART_BaudRate = 115200;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_WordLength = USART_WordLength_8b;
	
	nvic.NVIC_IRQChannel = USART2_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&nvic);
	
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	
	USART_Init(USART2, &usart);
	USART_Cmd(USART2,ENABLE);
}


int fputc(int ch,FILE *f)
{
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
	USART_SendData(USART2,(unsigned char)ch);
	return (ch);
}

void test(u8 byte)
{
	printf("%c\n",byte);
}

void USART1_IRQHandler(void)
{
	u8 res;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
	{
		res = USART_ReceiveData(USART1);
		if(uart1_handler)
		{
			uart1_handler(res);
		}
		USART_ClearFlag(USART1,USART_IT_RXNE);
	}else
	{
		res = 0;
	}
}

void USART2_IRQHandler(void)
{
	u8 res;
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET)
	{
		res = USART_ReceiveData(USART2);
		if(uart2_handler)
		{
			uart2_handler(res);
		}
		USART_ClearFlag(USART2,USART_IT_RXNE);
	}else
	{
		res = 0;
	}
}

void BspUsart1Send(u8 *buf, u16 len)
{
	while(len>0)
	{
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		USART_SendData(USART1,(unsigned char)*buf);
		len--;
		buf++;
	}
}

void DMA_Configuration(uint8_t *bufaddr)
{
	DMA_InitTypeDef dma;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	DMA_DeInit(DMA1_Channel5);

	dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
	dma.DMA_MemoryBaseAddr =(uint32_t)bufaddr;
	dma.DMA_DIR = DMA_DIR_PeripheralSRC;
	dma.DMA_BufferSize = USART1_BUFF_LANGTH;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_Priority = DMA_Priority_VeryHigh;
	dma.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5,&dma);

	USART_Cmd(USART1,ENABLE);
	DMA_Cmd(DMA1_Channel5,ENABLE);
}

void DMA_Again(uint16_t count)
{
	DMA_Cmd(DMA1_Channel5,DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel5,count);
	DMA_Cmd(DMA1_Channel5,ENABLE);
}

bool Uart1_Rec_Complish(void)
{
	return DMA_GetFlagStatus(DMA1_FLAG_TC5)==SET;
}

uint16_t Uart1_Get_RecLen(void)
{
	return USART1_BUFF_LANGTH - DMA_GetCurrDataCounter(DMA1_Channel5);
}



