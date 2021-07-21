#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "misc.h"
#include "UART.h"
#include "ADC.h"
#include "CCS811.h"
#include "kalman.h"
#include "delay.h"


/* Private variables ---------------------------------------------------------*/

/*UART variables*/
extern int8_t USART1_Buffer[TxBufferSize];
#define FRAME_SIZE 14
//extern uint8_t RxBuffer[13];
uint32_t i = 0, temp = 0;
char Data_frame[FRAME_SIZE];

/*GP2Y variables*/
extern float dustDensity;
extern float volMeasured;
extern float calVoltage;
float val, sum;
extern uint32_t ADC_convertedValue;// for DMA ADC value
float GP2Y_est = 0;
uint8_t dust_buff[4];

/*CCS811 varialbels*/
#define I2C_BUFF_SIZE 8
uint8_t I2C_BUFF[I2C_BUFF_SIZE], I2C_TX_BUFF[I2C_BUFF_SIZE], I2C_RX_BUFF[I2C_BUFF_SIZE];
uint16_t eCO2, eTVOC;
float eCO2_est = 0, eTVOC_est = 0;

/* Private function prototypes -----------------------------------------------*/

void Start_Up(void);
void Dust_measure_Task(void);
void IAQ_measure_Task(void);
void Transfer_frame(char* GP2Y_buffer, char* CCS811_buffer, char* Data_frame);
void task2(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
	
void Start_Up(void){
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
	init_TIM4();	
	/* initialize ADC1 channel 1*/
	init_ADC();	
	/* initialize I2C1 and start conversation on CCS811*/
	I2C_config();
	CCS811_start();	
	//blink led PB12
	GPIO_InitTypeDef blink_struct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	blink_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	blink_struct.GPIO_Pin = GPIO_Pin_12;
	blink_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &blink_struct);	
	/* Initialize Kalman filter for 3 sensor*/
	KalmanFilter_GP2Y(1, 1, 0.001);
	KalmanFilter_eCO2(1, 1, 0.1);
	KalmanFilter_eTVOC(1, 1, 0.1);
}

void Dust_measure_Task(void){
	//measure dust density
	dustDensity = getDustDensity();
	//get adc voltage value
	volMeasured = getVoltage();
	//kalman filter for ADC
	GP2Y_est = updateEstimate_GP2Y(dustDensity);
	//convert to array from, 2 digits after "." -> (float)/100
	dust_convert(&dust_buff[0], GP2Y_est);
}

void IAQ_measure_Task(void){
	CCS811_measure(&I2C_RX_BUFF[0]);
	//wait for measurement
	Delay_Ms(250);
	//get value
	eCO2 = get_eCO2(&I2C_RX_BUFF[0], eCO2);
	eTVOC = get_eTVOC(&I2C_RX_BUFF[0], eTVOC);
	//Filter
	eCO2_est = updateEstimate_eCO2(eCO2);
	eTVOC_est = updateEstimate_eTVOC(eTVOC);
}

void Transfer_frame(char* GP2Y_buffer, char* CCS811_buffer, char* Data_frame){
	//Dust data
	Data_frame[0] = 0xDD;
	for(int i = 0; i < 4; i++){
		Data_frame[i+1] = GP2Y_buffer[i];		
	}
	//PM Dust CRC byte
	Data_frame[5] = Data_frame[4] + Data_frame[3] + Data_frame[2] + Data_frame[1];
	//IAQ data
	Data_frame[6] = 0xCC;
	Data_frame[7] = eCO2_est/16;
	Data_frame[8] = (uint8_t)(eCO2_est)%16;
	Data_frame[9] = eTVOC_est/16;
	Data_frame[10] = (uint8_t)(eTVOC_est)%16;
	//IAQ CRC byte
	Data_frame[11] = Data_frame[10] + Data_frame[9] + Data_frame[8] + Data_frame[7];
	
	//end of frame
	Data_frame[12] = 0x0D;
	Data_frame[13] = 0x0A;
	
}

void task2(void){
//		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
//		Delay_Ms(255);
		Transfer_frame(&dust_buff[0], &I2C_RX_BUFF[2], &Data_frame[0]);
		for(i = 0; i <14; i++){
			USART1_Buffer[i] = Data_frame[i];	
		}			
		DMA_Transfer();		
//		USART_Puts(USART1, "string \n\r"); 

//		GPIO_WriteBit(GPIOB, GPIO_Pin_12, 1);
//		GPIO_SetBits(GPIOB, GPIO_Pin_12);
//		Delay_Ms(255);
}

int main(void)
{
	init_TIM4();
	Delay_Ms(3000);
	Start_Up();
	GPIO_InitTypeDef init_structB;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	init_structB.GPIO_Mode = GPIO_Mode_Out_PP;
	init_structB.GPIO_Pin = GPIO_Pin_1;
	init_structB.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &init_structB);
	//init_ESP8266();
  while (1)
  {		
		GPIO_WriteBit(GPIOB, GPIO_Pin_1,0);
		//wake esp
//		ESP_trigger();
		//pull PA5 high for TASK_PIN begin
		GPIO_SetBits(GPIOA, GPIO_Pin_5);
		//do task 5 time
//		for(temp =0 ; temp < 5; temp++){
//			task2();
//		}
		//end of task
		Dust_measure_Task();
		IAQ_measure_Task();
		task2();
		Delay_Ms(1);
		GPIO_ResetBits(GPIOA, GPIO_Pin_5);
		//let ESP sleep for 100ms
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, 1);
		Delay_Ms(4500);

  }
}

//int main(){
//	init_TIM4();
//	GPIO_InitTypeDef init_structB;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	
//	init_structB.GPIO_Mode = GPIO_Mode_Out_PP;
//	init_structB.GPIO_Pin = GPIO_Pin_1;
//	init_structB.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &init_structB);
//	while(1){
//		GPIO_WriteBit(GPIOB, GPIO_Pin_1, 0);
//		Delay_Ms(250);
//		GPIO_WriteBit(GPIOB, GPIO_Pin_1, 1);
//		Delay_Ms(250);
//	
//	}
//}
