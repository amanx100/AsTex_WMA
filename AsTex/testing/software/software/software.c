/*
 * software.c
 *
 * Created: 2/2/2015 11:33:53 PM
 *  Author: Aman
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include "myusart.h"

int main(void)
{
	DDRA = 0xFF;
	usart_init(9600);
	
    while(1)
    {
		if (usart_dthit())
		{
			PORTA = usart_getchar();
		}
    }
}