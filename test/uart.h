#ifndef UART_H_H
#define UART_H_H

#include "stdio.h"
#include "stm32f10x.h"

typedef void (*Uart_RecvByteCb)(u8 data);
void Uart_Init(Uart_RecvByteCb fun);

#endif
