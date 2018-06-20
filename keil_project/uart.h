#ifndef UART_H_H
#define UART_H_H

#include "stdio.h"
#include "stm32f10x.h"

typedef void (*Uart_RecvByteCb)(u8 data);
void Uart_Init(Uart_RecvByteCb fun);
void BspUsart1Send(u8 *buf, u16 len);
u16 BspUsart1Receive(u8 *buf);
#endif
