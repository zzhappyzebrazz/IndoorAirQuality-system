#ifndef __DELAY_h
#define __DELAY_h
#include "stm32f10x.h"
//sysclock = 72MHz
#include "stm32f10x_tim.h"

void Delay_Ms(uint32_t u32Ms);
void Delay_Us(uint32_t u32Us);
void TIM4_Configuration(void);

#endif
