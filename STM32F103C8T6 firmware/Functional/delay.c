#include "delay.h"

void init_TIM4(void){
	TIM_TimeBaseInitTypeDef time4_init;
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM4EN, ENABLE);
	time4_init.TIM_Prescaler = 72;//clock = 72mHz/2 = 36MHz;
	time4_init.TIM_CounterMode = TIM_CounterMode_Up;
	time4_init.TIM_Period = 0xffff;
	time4_init.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &time4_init);
	TIM_Cmd(TIM4, ENABLE);

}

void Delay_Ms(uint32_t u32Ms){
	while(u32Ms){
	TIM_SetCounter(TIM2, 0);
	while(TIM_GetCounter(TIM2) < 1000){			
	}
	u32Ms--;
}
}

void Delay_Us(uint32_t u32Us){
	while(u32Us){
	TIM_SetCounter(TIM2, 0);
	while(TIM_GetCounter(TIM2) < 1){			
	}
	u32Us--;
}
}