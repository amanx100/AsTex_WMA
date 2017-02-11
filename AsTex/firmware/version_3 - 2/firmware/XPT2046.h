/*
 * XPT2046.h
 *
 * Created: 12/28/2014 2:48:39 AM
 *  Author: Aman
 */ 

#ifndef XPT2046_H_
#define XPT2046_H_

//Define for TP_CS
#define TP_CS_1 (PORTB |= (1<<PB4))
#define TP_CS_0 (PORTB &= ~(1<<PB4))

#include <avr/io.h>
#include "my_spi.h"
#include <util/delay.h>

void TP_init(void)
{
	SPI_MasterInit();
}

uint16_t TPReadX(void)
{
	uint16_t x=0;
	TP_CS_0;
	_delay_us(10);
	SPI_MasterTransmit_Receive(0x90);
	_delay_us(10);
	x=SPI_MasterTransmit_Receive(0x00);
	x<<=8;
	x+=SPI_MasterTransmit_Receive(0x00);
	TP_CS_1;
	x=x>>7;
	x  = x & 0x0FFF;
	return x;
}

uint16_t TPReadY(void)
{
	uint16_t y=0;
	TP_CS_0;
	_delay_us(10);
	SPI_MasterTransmit_Receive(0xd0);
	_delay_us(10);
	y=SPI_MasterTransmit_Receive(0x00);
	y<<=8;
	y+=SPI_MasterTransmit_Receive(0x00);
	TP_CS_1;
	y = y>>7;
	y = y & 0x00FF;
	return y;
}

uint16_t Read_TPval(unsigned int *tpxyval)
{
	tpxyval[0]=TPReadX();
	tpxyval[1]=TPReadY();
	return 1;
}

#endif /* XPT2046_H_ */