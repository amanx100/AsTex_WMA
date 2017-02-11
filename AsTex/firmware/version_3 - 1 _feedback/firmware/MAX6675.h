/*
 * MAX6675.h
 *
 * Created: 2/28/2015 2:15:27 AM
 *  Author: Aman
 */ 

/*
// *** Port pin setup for MAX6675 interfacing****************
#define MAX_SO (PIN & (1<<P))
#define MAX_SCK_0 (PORT &= ~(1<<P))
#define MAX_SCK_1 (PORT |= (1<<P))
#define MAX_CS_0 (PORT &= ~(1<<P))
#define MAX_CS_1 (PORT |= (1<<P))
// ********* Set properly io pin also ***********************
*/


#ifndef MAX6675_H_
#define MAX6675_H_

#include <avr/io.h>
#include <util/delay.h>

void TCK_Init(void)
{
	MAX_CS_1;
	MAX_SCK_1;//0
}

uint16_t Read_TCK(void)
{
	uint16_t num=0;
	MAX_CS_0;
	_delay_us(1);
	for(char count=0;count<16;count++)
	{
		MAX_SCK_1;
		_delay_us(2);
		num = num<<1;
		MAX_SCK_0;
		_delay_us(2);
		if(MAX_SO) num++;
	}
	MAX_SCK_1;
	MAX_CS_1;
	return num;
}

int16_t Read_temperature(void)
{
	int16_t temp= Read_TCK();
	if (temp & 0b0000000000000100)
	{
		return 1111;
	}
	else
	{
		return (((temp & 0b0111111111111000)>>5));
	}
}


#endif /* MAX6675_H_ */