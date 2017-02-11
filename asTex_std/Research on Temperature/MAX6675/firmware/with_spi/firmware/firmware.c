/*
 * firmware.c
 *
 * Created: 2/25/2015 8:18:57 PM
 *  Author: Aman
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "myusart.h"
#include "MAX6675.h"


int main(void)
{
	char ch[20];
	signed int temp;
	
	DDRB |= (1<<0)|(1<<1)|(1<<2);
	
	max6675_init();
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