/*
 * MAX6675.h
 *
 * Created: 2/28/2015 2:15:27 AM
 *  Author: Aman
 */ 

#ifndef MAX6675_H_
#define MAX6675_H_


//Define for max6675_CS
#define DD_CS_1 (PORTB |= (1<<PB4))
#define DD_CS_0 (PORTB &= ~(1<<PB4))

#include <avr/io.h>
#include "my_spi.h"
#include <util/delay.h>


void max6675_init(void)
{
	DDR_SPI |= (1<<PB4);
	DD_CS_1;
	SPI_MasterInit();
}


uint16_t Read_TCK(void)
{
	uint16_t num=0;
	DD_CS_0;
	_delay_us(10);
	num = SPI_MasterTransmit_Receive(0x00);
	num <<= 8;
	num += SPI_MasterTransmit_Receive(0x00);
	DD_CS_1;
	return num;
}

int16_t Read_temperature(void)
{
	int16_t temp= Read_TCK();
	if (temp & 0b0000000000000100)
	{
		return 2000;
	}
	else
	{
		return ((temp & 0b0111111111111000)>>5);
	}
}


#endif /* MAX6675_H_ */