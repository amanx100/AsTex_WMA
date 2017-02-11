/*
 * firmware.c
 *
 * Created: 2/25/2015 8:18:57 PM
 *  Author: Aman
 */ 

//*** Port pin setup for MAX6675 interfacing****************
#define MAX_SO (PINA & (1<<PA0))
#define MAX_SCK_0 (PORTA &= ~(1<<PA1))
#define MAX_SCK_1 (PORTA |= (1<<PA1))
#define MAX_CS_0 (PORTA &= ~(1<<PA2))
#define MAX_CS_1 (PORTA |= (1<<PA2))
//**********************************************************

#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "myusart.h"
#include "MAX6675.h"


int main(void)
{
	char ch[20];
	signed int temp;
	
	DDRA = (1<<PA1)|(1<<PA2);
	//PORTA |= (1<<PA0);
	
	TCK_Init();
	usart_init(9600);
	
    while(1)
    {
        if (PINB & (1<<PB0))
        {
			temp = Read_temperature();
			if (temp == 2000)
			{
				usart_putstr("Sensor disconnected\r");
			}
			else
			{
				sprintf(ch,"Temp: %d\r",temp);
				usart_putstr(ch);
			}
			while(PINB & (1<<PB0));
        }
    }
}