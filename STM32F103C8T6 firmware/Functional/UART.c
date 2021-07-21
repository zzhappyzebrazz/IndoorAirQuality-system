#include "UART.h"
#include "delay.h"

typedef struct {
	int8_t *Buffer;
	uint16_t Size;
	uint16_t Num;
	uint16_t In;
	uint16_t Out;
} USART_t;


struct __FILE {
    int dummy;
};

//priavite variable declare
int8_t USART1_Buffer[TxBufferSize];
USART_t F1_USART1 = {USART1_Buffer, TxBufferSize, 0, 0, 0};
int answer;

//Init struct delare
USART_InitTypeDef USART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

void USART_Configuration(void){
	
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Configure USARTy */
  USART_Init(USART1, &USART_InitStructure);
	//enable RX it uart1
//	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);
	
}

void DMA_Transfer(void){
	/* enable DMA*/
	DMA1_Channel4->CCR |= DMA_CCR4_EN;
	/* Wait for transfer complete*/
	while((DMA1-> ISR & DMA_ISR_TCIF4) ==0){};
	/* clear the transfer comp*/
	DMA1->IFCR = DMA_IFCR_CTCIF4;
		/* disable dma1 channel4*/
	DMA1_Channel4 -> CCR &= ~DMA_CCR4_EN;
}

//
//{
//  //Test on DMA1 Channel1 Transfer Complete interrupt
////  if(DMA_GetITStatus(DMA1_IT_TC4))
////  {
//		for(i = 0; i<5 ; i++){
//    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
//		Delay_Ms(1000);
//		GPIO_SetBits(GPIOB, GPIO_Pin_12);
//		Delay_Ms(1000);
//		}
////   //Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits
//		DMA_Cmd(DMA1_Channel4, DISABLE);
////  }
//	
////  DMA_ClearFlag(DMA1_FLAG_TC4|DMA1_FLAG_GL4|DMA1_FLAG_HT4);
//  DMA_ClearITPendingBit(DMA1_IT_GL4);
////	DMA1->IFCR = DMA_IFCR_CTCIF4;
////	DMA1_Channel4 -> CCR &= ~DMA_CCR4_EN;
////	
////	while((DMA1-> ISR & DMA_ISR_TCIF4) ==0){};
////	/* clear the transfer comp*/
////	DMA1->IFCR = DMA_IFCR_CTCIF4;
////		/* disable dma1 channel4*/
////	DMA1_Channel4 -> CCR &= ~DMA_CCR4_EN;
//}


void RCC_Configuration(void)
{    
  /* DMA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable GPIO USART1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

}

void GPIO_Configuration(void)
{

  /* Configure USART1 Rx = PA10 as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  /* Configure USART1 Tx = PA9 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
 /* Configure ESP wake up pin PA4 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	
	/* Configure ESP task pin PA5 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOA, GPIO_Pin_5);
}


void NVIC_Configuration(void)
{
  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel =  USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	//Enable the DMA UART TX interrupt
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);	
}

void DMA_Configuration(void)
{

	/*enable dma1 clock*/
//	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	/*use dma when usart transmit*/
	USART1->CR3 |= USART_CR3_DMAT;
	/* length of dma counter = txbuffer size*/
	DMA1_Channel4-> CNDTR = TxBufferSize;
	/* enable memory increase | enable reading from memory to peripheral register | enable transfer interrupt | enable circular mode*/
	DMA1_Channel4-> CCR |= DMA_CCR4_MINC | DMA_CCR7_DIR | DMA_CCR7_TCIE | DMA_CCR7_CIRC;
	/* fill in USART1 DR register address*/
	DMA1_Channel4 -> CPAR = (uint32_t)&USART1->DR;
	/* fill in memory buffer*/
	DMA1_Channel4 -> CMAR = (uint32_t)USART1_Buffer;
	//disable DMA force to not trasnfer
	DMA_Cmd(DMA1_Channel4, DISABLE);

}

void ESP_trigger(void){
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	Delay_Ms(10);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

int fputc(int ch, FILE *f){
	
	USART_Putc(USART1, ch);
	
	
	return ch;

}

//Send char func
void USART_Putc(USART_TypeDef* USARTx, volatile char c){
	/* wait for tx ready*/
	while(!USART_GetFlagStatus(USARTx, USART_FLAG_TXE));
	/* Begin send data*/
	USARTx->DR = (uint16_t)(c&0x01FF);
}

//Send string func
void USART_Puts(USART_TypeDef* USARTx, char* str){
	while(*str){
		/* Send data by each char*/
		USART_Putc(USARTx, *str++);
	}
}


USART_t* USART_INT_GetUsart(USART_TypeDef* USARTx){
	USART_t* u;
	return u = &F1_USART1;
}

// Insert char to buffer
void USART_int_InsertToBuffer(USART_t* u, uint8_t c){
	/* check for available space in buffer*/
	if(u->Num < u->Size){
		/*Check overflow*/
		if(u->In == u->Size){
			u->In = 0;
		}
		/* begin add ti buffer*/
		u->Buffer[u->In] = c;
		u->In++;
		u->Num++;
	}
}

//check for buffer empty
uint8_t USART_BufferEmpty(USART_TypeDef* USARTx){
	USART_t* u = USART_INT_GetUsart(USART1);
	return (u->Num == 0);
}

//check for buffer full
uint8_t USART_BufferFull(USART_TypeDef USARTx){
	USART_t* u = USART_INT_GetUsart(USART1);
	return (u->Num = u->Size);
}

//Clear USART1 buffer
void USART_ClearBuffer(USART_TypeDef* USARTx){
	USART_t* u = USART_INT_GetUsart(USART1);
	
	
	u->Num = 0; u->In = 0; u->Out = 0;
}

//find the char already in the buffer, ex: find '/n' to know an end of a string
uint8_t USART_FindCharacter(USART_TypeDef* USARTx, volatile char c) {
	uint16_t num, out;
	USART_t* u = USART_INT_GetUsart(USART1);
	
	/* Temp variables */
	num = u->Num;
	out = u->Out;
	
	while (num > 0) {
		/* Check overflow */
		if (out == u->Size) {
			out = 0;
		}
		if (u->Buffer[out] == c) {
			/* Character found */
			return 1;
		}
		out++;
		num--;
	}
	
	/* Character is not in buffer */
	return 0;
}

//Get char form USART1
uint8_t USART_Getc(USART_TypeDef* USARTx){
	uint8_t c = 0;
	USART_t* u = USART_INT_GetUsart(USARTx);
	
	/* Check if we have any data in buffer*/
	if(u->Num > 0){
		if(u->Out == u->Size){
			u->Out = 0;
		}
		c = *(u->Buffer + u->Out);
		u->Out++;
		u->Num--;
	}
	return c;
}


//Get string from USART1
uint16_t USART_Gets(USART_TypeDef USARTx, char* buffer, uint16_t buffersize){
	uint16_t i = 0;
	/* Check for any data in USART RX reg*/
	if(USART_BufferEmpty(USART1) || (!USART_FindCharacter(USART1, '\n') && !USART_BufferFull(USARTx))){
		return 0;		
	}
		
	/* If available buffer size is more than 0 characters */
	while (i < (buffersize - 1)) {
		/* We have available data */
		buffer[i] = (char) USART_Getc(USART1);
		/* Check for end of string */
		if (buffer[i] == '\n') {
			i++;
			/* Done */
			break;
		} else {
			i++;
		}
	}
	
	/* Add zero to the end of string */
	buffer[i] = 0;               

	return (i);
}
	


//int sendAT(char* ATcmd, char* rec_answer, unsigned int timeout){
//	int x = 0, answer = 0;
//	uint8_t c = 0;
//	
//	char response[100];
//	//unsigned long previous string;
//	memset(response, '\0', 100);//Initialize the response string memory set
//	Delay_Ms(100); //wait for memset
//	
//	while(USART_Getc(USART1) > 0);
//	printf("%s\r\n",ATcmd);
//	x = 0;
//	//previous = mills(); arduino function to wait
//	do{
//		Delay_Ms(1);
//		c = USART_Getc(USART1);
//		if(c){
//			response[x] = c;
//			x++;
//			if(strstr(response, rec_answer) != NULL)
//			{
//				answer = 1;//check for response ok
//			}
//		}
//	}
//	while((answer == 0)&&(--timeout > 0));//wait for end of conversation
//	return answer;
//	
//}

//void init_ESP8266(void){
//	do{ answer = sendAT("AT","OK",1000);} while(answer == 0); //check ATcmd firmware
//	
//	
//}
void USART1_IRQHandler(void){
	if(USART_GetITStatus(USART1, USART_IT_RXNE)){
		//get RX by register
			USART_int_InsertToBuffer(&F1_USART1, USART1->DR);
	}
}
