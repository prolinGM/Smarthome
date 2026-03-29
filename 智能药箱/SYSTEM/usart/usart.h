#ifndef _USART_H_
#define _USART_H_


#include "stm32f10x.h"
#include <stdio.h>

#define USART_DEBUG		USART1		//调试打印所使用的串口组


void Usart1_Init(unsigned int baud);

void Usart3_Init(unsigned int baud);

void Usart_SendString(USART_TypeDef *USARTx, unsigned char *str, unsigned short len);

void UsartPrintf(USART_TypeDef *USARTx, char *fmt,...);

int fputc(int ch, FILE *f);
void USART3_printf(USART_TypeDef* USARTx, uint8_t *Data,...);
void UART3SendByte(unsigned char SendData);

#endif
