/*
USART1: TX		RX
				PA9		PA10
				PB6		PB7
				
USART2:	TX		RX
				PA2		PA3
				
USART3:	TX		RX
				PB10	PB11				
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H


#define TxBufferSize   14
#define RxBufferSize   13

#define USART1_BUFFER_SIZE TxBufferSize

#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "delay.h"

//USART config
void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void DMA_Configuration(void);
void USART_Configuration(void);
void DMA_Transfer(void);

void ESP_trigger(void);

//USART function
void USART_Putc(USART_TypeDef* USARTx, volatile char c);
void USART_Puts(USART_TypeDef* USARTx, char* str);
uint16_t USART_Gets(USART_TypeDef USARTx, char* buffer, uint16_t buffersize);
uint8_t USART_Getc(USART_TypeDef* USARTx);
uint8_t USART_FindCharacter(USART_TypeDef* USARTx, volatile char c);
void USART_ClearBuffer(USART_TypeDef* USARTx);
uint8_t USART_BufferFull(USART_TypeDef USARTx);
uint8_t USART_BufferEmpty(USART_TypeDef* USARTx);
//void USART_int_InsertToBuffer(USART_t* u, uint8_t c);
//USART_t* USART_INT_GetUsart(USART_TypeDef* USARTx;

//AT cmd function
int sendAT(char* ATcmd, char* rec_answer, unsigned int timeout);
void init_ESP8266(void);


#endif 
