#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"

/*
ADC pinout
	CHANNEL			ADC1		ADC2		ADC3
	0						PA0			PA0			PA0
	1						PA1			PA1			PA1
	2						PA2			PA2			PA2
	3						PA3			PA3			PA3
	4						PA4			PA4			PF6
	5						PA5			PA5			PF7	
	6						PA6			PA6			PF8
	7						PA7			PA7			PF9
	8						PB0			PB0			PF10
	9						PB1			PB1			PF3
	10					PC0			PC0			PC0
	11					PC1			PC1			PC1
	12					PC2			PC2			PC2
	13					PC3			PC3			PC3
	14					PC4			PC4			PF4
	15					PC5			PC5			PF5
*/
//#define SAMPLINGTIME 280
//#define DELTATIME 19

#define SAMPLINGTIME 280
#define DELTATIME 40
#define SLEEPING_TIME 9680

float getDustDensity(void);
float getVoltage(void);
int getADC(void);

uint16_t get_adc(void);
uint16_t get_adc_avg(uint8_t times);

void init_GPIO(void);
void init_DMA(void);
void init_ADC(void);
void calc(void);

void dust_convert(uint8_t *dust_buff, float dust);

#endif
