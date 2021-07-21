#ifndef _CCS811_h
#define _CCS811_h

#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_gpio.h"
#include "delay.h"


#define STATUS_REG 0x00
#define MEAS_MODE_REG 0x01
#define ALG_RESULT_DATA 0x02
#define ENV_DATA 0x05
#define THRESHHOLD 0x10
#define BASELINE 0x11
#define HW_ID_REG 0x20
#define ERROR_ID_REG 0xE0
#define APP_START_REG 0xF4
#define SW_RESET 0xFF
#define CCS811_ADDR 0x5A
#define MODE_IDLE 0x0
#define MODE_1S 0x10
#define MODE_10S 0x20
#define MODE_60S 0x30
#define INT_DRIVE 0x80
#define THRESH_ENA 0x4

void I2C_Configuration(void);
void CCS811_write(uint8_t address, uint8_t reg, uint8_t *tx_data, uint8_t length);
void CCS811_read(uint8_t address, uint8_t *rx_data, uint8_t length);
void CCS811_start(void);
void CCS811_measure(uint8_t *rx_data);
int get_eCO2(uint8_t *rx_data, uint16_t eCO2);
int get_eTVOC(uint8_t *rx_data, uint16_t eTVOC);

#endif
