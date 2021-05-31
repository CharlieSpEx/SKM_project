/*
 * DS18B20.h
 *
 *  Created on: 31 maj 2021
 *      Author: cerws
 */

#ifndef INC_DS18B20_H_
#define INC_DS18B20_H_

#define portW1 GPIOA
#define wire1 GPIO_Pin_7


#endif /* INC_DS18B20_H_ */

#include "stm32f1xx_hal.h"


uint16_t RESET_PULSE(void);

void delay_us (uint16_t us);

void SendBit(uint16_t bit);

uint16_t ReadBit(void);

void SendByte(uint16_t value);

uint16_t ReadByte(void);

float ReadTemp(void);
