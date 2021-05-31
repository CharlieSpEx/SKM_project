/*
 * DS18B20.c
 *
 *  Created on: 31 maj 2021
 *      Author: cerws
 */
#include "DS18B20.h"


void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
}

uint16_t RESET_PULSE(void)
{
	uint16_t PRESENCE=0;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);				//set low
	delay_us(480); //wait 480us
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); 				//set high
	delay_us(70); //wait 70 us

	//check for response
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7)==GPIO_PIN_RESET)
		PRESENCE++;
	delay_us(410);

	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7)==GPIO_PIN_SET)
		PRESENCE++;

	if (PRESENCE == 2)
		return 1;
	else
		return 0;
}

void SendBit(uint16_t bit)
{
	if (bit==0)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);				//set low
		delay_us(65);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); 				//set high
		delay_us(10);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);				//set low
		delay_us(10);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); 				//set high
		delay_us(65);
	}
}

uint16_t ReadBit(void)
{
	uint16_t bit=0;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);					//set low
	delay_us(5);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); 					//set high

						//check what response
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7)==GPIO_PIN_SET)
		bit=1;
	else
		bit=0;

	delay_us(55);  		//delay between bits
	return bit;
}

void SendByte(uint16_t value)
{
	uint16_t i,tmp;
	for (i = 0; i < 8; i++)
	{
		tmp = value >> i;
		tmp &= 0x01;
		SendBit(tmp);
	}
}

uint16_t ReadByte(void)
{
	uint16_t i,value=0;
	for (i = 0; i < 8; i++)
	{
		if(ReadBit()) 														//read data line
			value |= 0x01 << i;
	}
	return value;
}

float ReadTemp(void)
{
	uint16_t i, presence=0, memory[3];
	int temp=0;

	presence=RESET_PULSE();

	if (presence==1)
	{
		SendByte(0xCC); //Skip ROM
		SendByte(0x44); //Convert T

		for (i=0; i < 100; i++) //odczekanie 750ms na odczyt i konwersję
			delay_us(7500); //temperatury
	}

	presence = RESET_PULSE();

	if (presence==1)
	{
		SendByte(0xCC);	//Skip ROM
		SendByte(0xBE);	//Read Scratchpad

		for (i=0;i<2;i++)
			memory[i] = ReadByte(); 		//odczyt 2 bajtów Scratchpad

		memory[2] = (240U & memory[1]) >>  7;
		memory[1] = (15U & memory[2] ) <<  8;

		if (memory[2] == 0)		//jeśli dodatnia temperatura
			temp = (memory[0] + memory[1])/16;
		if (memory[2] == 1)		//jeśli ujemna temperatura
			temp = (memory[0] + memory[1]-4095)/16;
	}

	presence = RESET_PULSE();
	return temp;
}
