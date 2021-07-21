#include "CCS811.h"

void I2C_Configuration(void){
	//init GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE);
	//release PB3 from JTAG defaut pin
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//PB1 for blink
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, 0);
	
	//init I2C
		GPIO_InitTypeDef init_gpio;
	I2C_InitTypeDef init_i2c;
	//enable gpio clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//enable I2C clock
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_I2C1EN, ENABLE);
	//PB6 = SCL, PB7 = SDA
	init_gpio.GPIO_Pin = GPIO_Pin_6;
	init_gpio.GPIO_Mode = GPIO_Mode_AF_OD;
	init_gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &init_gpio);
	init_gpio.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &init_gpio);
	//reset state
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1 , ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1 , DISABLE);
	//I2C1 configuration
	init_i2c.I2C_ClockSpeed = 100000;
	init_i2c.I2C_Mode = I2C_Mode_I2C;
	init_i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	init_i2c.I2C_OwnAddress1 = 0;
	init_i2c.I2C_Ack = I2C_Ack_Enable;
	init_i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1 , &init_i2c);
	//turn on I2C1
	I2C_Cmd(I2C1, ENABLE);
}

void CCS811_write(uint8_t address, uint8_t reg, uint8_t *tx_data, uint8_t length)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, 0);
	
	address = (address<<1) + 0;
	
	uint16_t timeOut, temp, i;
	//start
	I2C_GenerateSTART(I2C1, ENABLE);
	//wait for timeOut
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_SB) == RESET)
	{
		if(timeOut-- == 0) timeOut = 0xffff;
	}
	//send slave address
	I2C_SendData(I2C1, address);
	Delay_Us(5);
	timeOut= 0xffff;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET)
	{
		if(timeOut-- == 0) timeOut = 0xffff;
	}
	//clear addr flag
	temp = I2C1 -> SR2;	
	Delay_Us(5);
	
	//send register address
	I2C_SendData(I2C1, reg);
	Delay_Us(5);
	while((I2C1 -> SR1 & 0x00004) != 0x000004);

	Delay_Us(5);
	
	//write data to register address
	for (i = 0; i<length; i++)
	{
			I2C_SendData(I2C1, tx_data[i]);
			Delay_Us(5);
			while((I2C1 -> SR1 & 0x00004) != 0x000004);
			Delay_Us(5);		
	}
	Delay_Us(5);
	I2C_GenerateSTOP(I2C1, ENABLE);
}	

void CCS811_read(uint8_t address, uint8_t *rx_data, uint8_t length)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, 0);
	
	address = (address<<1) + 1;
	
	uint16_t timeOut, temp, i;
	//start
	I2C_GenerateSTART(I2C1, ENABLE);
	//wait for Start Bit
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_SB) == RESET)
	{
		if(timeOut-- == 0) timeOut = 0xffff;
	}
	//send slave address
	I2C_SendData(I2C1, address);
	Delay_Us(5);
	timeOut= 0xffff;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET)
	{
		if(timeOut-- == 0) timeOut = 0xffff;
	}
	//clear ack
	
	I2C_AcknowledgeConfig(I2C1,ENABLE);
	temp = I2C1 -> SR2;
//	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
	rx_data[2] = I2C_ReceiveData(I2C1);
	while((I2C1 -> CR1&0x200) == 0x200);
//	I2C_AcknowledgeConfig(I2C1, ENABLE);	
	
	I2C_AcknowledgeConfig(I2C1,ENABLE);
	temp = I2C1 -> SR2;
//	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
	rx_data[3] = I2C_ReceiveData(I2C1);
	while((I2C1 -> CR1&0x200) == 0x200);
//	I2C_AcknowledgeConfig(I2C1, ENABLE);

	I2C_AcknowledgeConfig(I2C1,ENABLE);
	temp = I2C1 -> SR2;
//	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
	rx_data[4] = I2C_ReceiveData(I2C1);
	while((I2C1 -> CR1&0x200) == 0x200);
//	I2C_AcknowledgeConfig(I2C1, ENABLE);

	I2C_AcknowledgeConfig(I2C1,ENABLE);
	temp = I2C1 -> SR2;
//	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
	rx_data[5] = I2C_ReceiveData(I2C1);
	while((I2C1 -> CR1&0x200) == 0x200);
//	I2C_AcknowledgeConfig(I2C1, ENABLE);	

	I2C_AcknowledgeConfig(I2C1,ENABLE);
	temp = I2C1 -> SR2;
//	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
	rx_data[6] = I2C_ReceiveData(I2C1);
	while((I2C1 -> CR1&0x200) == 0x200);
//	I2C_AcknowledgeConfig(I2C1, ENABLE);
	
	I2C_AcknowledgeConfig(I2C1,DISABLE);
	temp = I2C1 -> SR2;
	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
	rx_data[7] = I2C_ReceiveData(I2C1);
	while((I2C1 -> CR1&0x200) == 0x200);
	I2C_AcknowledgeConfig(I2C1, ENABLE);

}	


void CCS811_start(void){
	uint8_t i2c_buff[8];
	i2c_buff[0] = 0x40;
	
	CCS811_write(CCS811_ADDR, MEAS_MODE_REG, &i2c_buff[0], 1);
	I2C_GenerateSTOP(I2C1, ENABLE);
	Delay_Ms(500);
	CCS811_write(CCS811_ADDR, APP_START_REG, &i2c_buff[0], 0);
	Delay_Us(50);
	CCS811_write(CCS811_ADDR, STATUS_REG, &i2c_buff[0],0);
	CCS811_read(CCS811_ADDR, &i2c_buff[1] ,0);
}

void CCS811_measure(uint8_t *rx_data){
	uint8_t temp;
	uint8_t i2c_buff[8];
	CCS811_write(CCS811_ADDR, ALG_RESULT_DATA, &i2c_buff[0],0);		
	CCS811_read(CCS811_ADDR, &rx_data[0],4);
	Delay_Ms(250);
}

int get_eCO2(uint8_t *rx_data, uint16_t eCO2){
	return 	eCO2 = rx_data[2]*16*16 + rx_data[3];
}
int get_eTVOC(uint8_t *rx_data, uint16_t eTVOC){
	return 	eTVOC = rx_data[4]*16*16 + rx_data[5];
}