#include <stdio.h>

/* Stm32 includes -----------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "misc.h"

/* Private includes -----------------------------------------------*/
#include "UART.h"
#include "ADC.h"
#include "CCS811.h"
#include "kalman.h"
#include "delay.h"

/* FreeRTOS includes -----------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stack_macros.h"
#include "semphr.h"
#include "queue.h"

/* Private variables -----------------------------------------------*/

/* Task priorities -----------------------------------------------*/
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainCOM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainINTEGER_TASK_PRIORITY   

/* FreeRTOS variavles -----------------------------------------------*/
static xSemaphoreHandle xDustmeasureSemaphore = NULL;
static xSemaphoreHandle xIAQmeasureSemaphore = NULL;
static xQueueHandle xDataQueue = NULL;

/* Private function prototypes -----------------------------------------------*/

void Transfer_frame(char* GP2Y_buffer, char* CCS811_buffer, char* Data_frame);
void task2(void);
void vStartupTask(void *pvParameter);
void vDustmeasureTask(void *pvParameter);
void vIAQmeasureTask(void *pvParameter);
void vTransferTask(void *pvParameter);

/* FreeRTOS main -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

void vStartupTask(void *pvParameter)
{
	
	/* initialize CCS811 with start parameters*/
	CCS811_start();	
	
	xSemaphoreGive(xDustmeasureSemaphore);
	xSemaphoreGive(xIAQmeasureSemaphore);
	vTaskDelay(portMAX_DELAY);
	
}

	/* GP2Y variables -----------------------------------------------*/
	extern float dustDensity;
	extern float volMeasured;
	extern float calVoltage;
	float val, sum;
	extern uint32_t ADC_convertedValue;// for DMA ADC value
	float GP2Y_est = 0;
	uint8_t dust_buff[4];
void vDustmeasureTask(void *pvParameter)
{

	while(1)
	{
		if(xSemaphoreTake(xDustmeasureSemaphore, portMAX_DELAY)){
			//measure dust density
			dustDensity = getDustDensity();
			//get adc voltage value
			volMeasured = getVoltage();
			//kalman filter for ADC
			GP2Y_est = updateEstimate_GP2Y(dustDensity);
			//convert to array from, 2 digits after "." -> (float)/100
			dust_convert(&dust_buff[0], GP2Y_est);
			if(xQueueSend(xDataQueue, &dust_buff,portMAX_DELAY)){
				//GPIO_ResetBits(GPIOB, GPIO_Pin_1);
			}
					for(int i = 0; i<5; i++){
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		Delay_Ms(100);
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		Delay_Ms(100);
	}
		}
	}
}

/* CCS811 varialbels -----------------------------------------------*/
	uint8_t I2C_BUFF[8], I2C_TX_BUFF[8], I2C_RX_BUFF[8], IAQ_BUFF[4];
	uint16_t eCO2, eTVOC;
	float eCO2_est = 0, eTVOC_est = 0;
void vIAQmeasureTask(void *pvParameter)
{

	while(1)
	{
		if(xSemaphoreTake(xIAQmeasureSemaphore, portMAX_DELAY)){
			CCS811_measure(&I2C_RX_BUFF[0]);
			//wait for measurement
			Delay_Ms(250);
			//get value
			eCO2 = get_eCO2(&I2C_RX_BUFF[0], eCO2);
			eTVOC = get_eTVOC(&I2C_RX_BUFF[0], eTVOC);
			//Filter
			eCO2_est = updateEstimate_eCO2(eCO2);
			eTVOC_est = updateEstimate_eTVOC(eTVOC);
			IAQ_BUFF[0] = eCO2_est/16;
			IAQ_BUFF[1] = (uint8_t)(eCO2_est)%16;
			IAQ_BUFF[2] = eTVOC_est/16;
			IAQ_BUFF[3] = (uint8_t)(eTVOC_est)%16;
			if(xQueueSend(xDataQueue, &IAQ_BUFF, portMAX_DELAY)){
				//GPIO_ResetBits(GPIOB, GPIO_Pin_1);				
			}
//					for(int i = 0; i<10; i++){
//		GPIO_SetBits(GPIOB, GPIO_Pin_1);
//		Delay_Ms(100);
//		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
//		Delay_Ms(100);
//	}
		}
	}
}

/* UART variables -----------------------------------------------*/
extern  int8_t USART1_Buffer[TxBufferSize];
	uint32_t i = 0, temp = 0;
	char Data_frame[14];
void vTransferTask(void *pvParameter)
{

	while(1)
	{
		if(xQueueReceive(xDataQueue, &Data_frame, portMAX_DELAY)){
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
		}
		else{
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		}
		//Dust data
		USART1_Buffer[0] = 0xDD;
		for(int i = 0; i < 4; i++){
			USART1_Buffer[i+1] = Data_frame[i];		
		}
		//PM Dust CRC byte
		USART1_Buffer[5] = USART1_Buffer[4] + USART1_Buffer[3] + USART1_Buffer[2] + USART1_Buffer[1];
		//IAQ data
		USART1_Buffer[6] = 0xCC;
		for(int i = 0; i < 4; i++){
			USART1_Buffer[i+7] = Data_frame[i+4];		
		}
		//IAQ CRC byte
		USART1_Buffer[11] = USART1_Buffer[10] + USART1_Buffer[9] + USART1_Buffer[8] + USART1_Buffer[7];
		
		//end of frame
		USART1_Buffer[12] = 0x0D;
		USART1_Buffer[13] = 0x0A;
		DMA_Transfer();		
//		Delay_Ms(4500);
//		GPIO_SetBits(GPIOB, GPIO_Pin_1);
//		Delay_Ms(100);
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		Delay_Ms(100);
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		vTaskDelay(400000);
		xSemaphoreGive(xDustmeasureSemaphore);
		xSemaphoreGive(xIAQmeasureSemaphore);
	}
}

void vLedTask(void * pvParameter){
	for(int i = 0; i<10; i++){
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
		vTaskDelay(10000);
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		vTaskDelay(10000);
	}
}

int main(){
 
	/* System Clocks Configuration */
	RCC_Configuration();      
	/* NVIC configuration */
	NVIC_Configuration();
	/* Configure the GPIO ports */
	GPIO_Configuration();	
	/* Configure USART*/
	USART_Configuration();
	/* Configure the DMA */
	DMA_Configuration();
	/* initialize TIM4 delay application*/
	TIM4_Configuration();	
	/* initialize ADC1 channel 1*/
	ADC_Configuration();	
	/* initialize I2C1 and start conversation on CCS811*/
	I2C_Configuration();

	//blink led PB1
	GPIO_InitTypeDef blink_struct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	blink_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	blink_struct.GPIO_Pin = GPIO_Pin_1;
	blink_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &blink_struct);	
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
 
	/* Initialize Kalman filter for 3 measurements*/
	KalmanFilter_GP2Y(1, 1, 0.001);
	KalmanFilter_eCO2(1, 1, 0.1);
	KalmanFilter_eTVOC(1, 1, 0.1);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
	
	vSemaphoreCreateBinary(xDustmeasureSemaphore);
	vSemaphoreCreateBinary(xIAQmeasureSemaphore);
	xDataQueue = xQueueCreate(14, sizeof(char) );
	if(xDustmeasureSemaphore != NULL)
	{
		xSemaphoreTake(xDustmeasureSemaphore,(portTickType)1);
	}
	if(xIAQmeasureSemaphore != NULL)
	{
		xSemaphoreTake(xIAQmeasureSemaphore,(portTickType)1);
	}
	
	xTaskCreate(vStartupTask, (const char*) "Startup Task", configMINIMAL_STACK_SIZE + 128 , NULL //the parameter to put in the task = pvParameters
	,  3, (xTaskHandle *) NULL);
	xTaskCreate(vDustmeasureTask, (const char*) "Dust measure Task", configMINIMAL_STACK_SIZE , NULL 	,  1, (xTaskHandle *) NULL);
	xTaskCreate(vIAQmeasureTask, (const char*) "IAQ measure Task", configMINIMAL_STACK_SIZE , NULL, 1, (xTaskHandle *) NULL);
	xTaskCreate(vTransferTask, (const char*) "Transfer Task", configMINIMAL_STACK_SIZE , NULL, 2, (xTaskHandle *) NULL);
//	xTaskCreate(vLedTask, (const char*) "Led Task", configMINIMAL_STACK_SIZE , NULL, 1, (xTaskHandle *) NULL);
	vTaskStartScheduler();
	while(1){
		//no code here
	}
}

