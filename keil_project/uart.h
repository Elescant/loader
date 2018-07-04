#ifndef UART_H_H
#define UART_H_H

#include "stdio.h"
#include "stm32f10x.h"

#define  USART1_BUFF_LANGTH 1048

typedef void (*Uart_RecvByteCb)(u8 data);
void Uart1_Init(Uart_RecvByteCb fun);
void Uart2_Init(Uart_RecvByteCb fun);
void BspUsart1Send(u8 *buf, u16 len);
u16 BspUsart1Receive(u8 *buf);

void DMA_Again(uint16_t count);
void DMA_Configuration(uint8_t *bufaddr);
#endif
