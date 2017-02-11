/*
 * firmware_ext.c
 *
 * Created: 2/6/2015 10:46:38 PM
 *  Author: Aman
 */

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "myusart.h"

void data_port(unsigned char data_buffer)
{
	if (data_buffer == 97)
	{
		PORTD |= (1<<PD5);
	}
	else if (data_buffer == 98)
	{
		PORTD &= ~(1<<PD5);
	}
	else if (data_buffer == 99)
	{
		PORTD |= (1<<PD6);
	}
	else if (data_buffer == 100)
	{
		PORTD &= ~(1<<PD6);
	}
	else if (data_buffer == 101)
	{
		PORTD |= (1<<PD7);
	}
	else if (data_buffer == 102)
	{
		PORTD &= ~(1<<PD7);
	}
}

int main(void)
{
	PORTB = 0x0F;
	DDRB = 0xF0;
	
	PORTD = 0x00;
	DDRD = (1<<PD5)|(1<<PD6)|(1<<PD7);
	
	usart_init(600);
	
	wdt_enable(WDTO_2S);
	
    while(1)
    {
		wdt_reset();
		
		if (usart_dthit())  // first check for usart data and do the operation
		{
			data_port(usart_getchar());
		}
        
        PORTB = 0xEF;    //start first column
        _delay_us(1);
        if (!(PINB & 0x01))
        {
			usart_putchar(1);
			_delay_ms(250);
	        while(!(PINB & 0x01));
			_delay_ms(100);
        }
        else if (!(PINB & 0x02))
        {
			usart_putchar(4);
			_delay_ms(250);
	        while(!(PINB & 0x02));
			_delay_ms(100);
        }
        else if (!(PINB & 0x04))
        {
			usart_putchar(7);
			_delay_ms(250);
	        while(!(PINB & 0x04));
			_delay_ms(100);
        }
        else if (!(PINB & 0x08))
        {
			usart_putchar(11);
			_delay_ms(250);
	        while(!(PINB & 0x08));
			_delay_ms(100);
        }


        
        PORTB = 0xDF;    //start second column
        _delay_us(1);
        if (!(PINB & 0x01))
        {
			usart_putchar(2);
			_delay_ms(250);
	        while(!(PINB & 0x01));
			_delay_ms(100);
        }
        else if (!(PINB & 0x02))
        {
			usart_putchar(5);
			_delay_ms(250);
	        while(!(PINB & 0x02));
			_delay_ms(100);
        }

        else if (!(PINB & 0x04))
        {
			usart_putchar(8);
			_delay_ms(250);
	        while(!(PINB & 0x04));
			_delay_ms(100);
        }

        else if (!(PINB & 0x08))
        {
			usart_putchar(10);
			_delay_ms(250);
	        while(!(PINB & 0x08));
			_delay_ms(100);
        }

		if (usart_dthit())  //second check for usart data and do the operation
		{
			data_port(usart_getchar());
		}

        PORTB = 0xBF;   //start third column
        _delay_us(1);
        if (!(PINB & 0x01))
        {
			usart_putchar(3);
			_delay_ms(250);
	        while(!(PINB & 0x01));
			_delay_ms(100);
        }
        else if (!(PINB & 0x02))
        {
			usart_putchar(6);
			_delay_ms(250);
	        while(!(PINB & 0x02));
			_delay_ms(100);
        }
        else if (!(PINB & 0x04))
        {
			usart_putchar(9);
			_delay_ms(250);
	        while(!(PINB & 0x04));
			_delay_ms(100);
        }
        else if (!(PINB & 0x08))
        {
			usart_putchar(12);
			_delay_ms(250);
	        while(!(PINB & 0x08));
			_delay_ms(100);
        }
		
		
        
        PORTB = 0x7F;   //start fourth column
        _delay_us(1);
        if (!(PINB & 0x01))
        {
			usart_putchar(13);
			_delay_ms(250);
	        while(!(PINB & 0x01));
			_delay_ms(100);
        }
        else if (!(PINB & 0x02))
        {
			usart_putchar(14);
			_delay_ms(250);
	        while(!(PINB & 0x02));
			_delay_ms(100);
        }
        else if (!(PINB & 0x04))
        {
			usart_putchar(15);
			_delay_ms(250);
	        while(!(PINB & 0x04));
			_delay_ms(100);
        }
        else if (!(PINB & 0x08))
        {
			usart_putchar(16);
			_delay_ms(250);
	        while(!(PINB & 0x08));
			_delay_ms(100);
        } 
    } //end main while loop
}