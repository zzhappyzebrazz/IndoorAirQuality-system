#include "ADC.h"

float dustDensity;
float volMeasured;
float calVoltage;
uint32_t ADC_convertedValue = 0;


void init_GPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructureB;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructureB.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructureB.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructureB.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructureB);
	
	GPIO_InitTypeDef GPIO_InitStructureA;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructureA.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructureA.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructureA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructureA);
	
//	GPIO_InitStructureA.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructureA.GPIO_Pin = GPIO_Pin_2;
//	GPIO_InitStructureA.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructureA);
	
}

void init_ADC(void)
{
	ADC_InitTypeDef ADC_initstr;
	GPIO_InitTypeDef GPIO_initstr;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //72m/6 = 12mHz
	
	GPIO_initstr.GPIO_Pin = GPIO_Pin_2;//adc PIN A2
	GPIO_initstr.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_initstr);
	
	ADC_DeInit(ADC1);
	
	ADC_initstr.ADC_Mode = ADC_Mode_Independent;
	ADC_initstr.ADC_ScanConvMode = DISABLE;
	ADC_initstr.ADC_ContinuousConvMode = DISABLE;
	ADC_initstr.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_initstr.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_initstr.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_initstr);
	
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
		
	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	GPIO_WriteBit(GPIOA, GPIO_Pin_3, 1);
}

void init_DMA(void)
{

	DMA_InitTypeDef	DMA_initstr;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	//DMA channel 1 config
	DMA_DeInit(DMA1_Channel1);
	DMA_initstr.DMA_PeripheralBaseAddr = ((uint32_t) 0x4001244C);//0x4001244C DR_address cua adc1 xem trong file stm32f10x_adc
	DMA_initstr.DMA_MemoryBaseAddr = (uint32_t)&ADC_convertedValue;//dia chi chua gia tri do duoc cua adc
	DMA_initstr.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_initstr.DMA_BufferSize = 1;
	DMA_initstr.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_initstr.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_initstr.DMA_Mode = DMA_Mode_Normal;
	DMA_initstr.DMA_Priority = DMA_Priority_Medium;
	DMA_initstr.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_initstr);
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

uint16_t get_adc(void)
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5);
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	
	return ADC_GetConversionValue(ADC1);
}

uint16_t	get_adc_avg(uint8_t times)
{
	uint32_t temp_val = 0;
	uint8_t t;
	for(t = 0; t<times; t++)
	{
		temp_val += get_adc();
		Delay_Us(5);
	}
	return temp_val/times;
}

void calc()
{
	uint16_t temp;
	GPIO_WriteBit(GPIOA, GPIO_Pin_3, 1);
	Delay_Us(SAMPLINGTIME);
	
	temp = get_adc_avg(5);
	volMeasured = (float)((2*3.3*temp)/4095);
	dustDensity = 170.0*volMeasured - 0.099;
	
	Delay_Us(DELTATIME);
	GPIO_WriteBit(GPIOA, GPIO_Pin_3, 0);
	
	Delay_Us(SLEEPING_TIME);
}

float getDustDensity(){
	calc();
	return dustDensity;
}

float getVoltage(){
	calc();
	return volMeasured;
}

void dust_convert(uint8_t *dust_buff, float dust){
	int temp = dust*100;
	dust_buff[3] = (temp%16);
	dust_buff[2] = (temp/16)%16;
	dust_buff[1] = ((temp)/(16*16))%16;
	dust_buff[0] = temp/(16*16*16);
	
}