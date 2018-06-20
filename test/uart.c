#include "uart.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"

static Uart_RecvByteCb uart_handler = NULL;

void Uart_Init(Uart_RecvByteCb fun)
{
	USART_InitTypeDef usart;
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	
	if(fun)
	{
		uart_handler = fun;
	}
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
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
	
	nvic.NVIC_IRQChannel = USART1_IRQn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&nvic);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	USART_Init(USART1, &usart);
	USART_Cmd(USART1,ENABLE);
}

int fputc(int ch,FILE *f)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	USART_SendData(USART1,(unsigned char)ch);
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
		if(uart_handler)
		{
			uart_handler(res);
		}
		USART_ClearFlag(USART1,USART_IT_RXNE);
	}
}












