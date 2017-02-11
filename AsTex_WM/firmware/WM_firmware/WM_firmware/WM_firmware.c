/*
 * WM_firmware.c
 *
 * Created: 10/24/2015 10:36:01 AM
 *  Author: Aman
 */ 

#define True 1
#define Flse 0
#define Button_1 (PIND & (1<<PIND0))
#define Button_2 (PIND & (1<<PIND1))
#define Button_3 (PIND & (1<<PIND2))
#define Button_4 (PIND & (1<<PIND3))

#define F_CPU 4000000UL
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "adc.h"
//#include "seven_seg.h"

const uint8_t SS_DATA[10] PROGMEM={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x27,0x7F,0x6F};
uint8_t button_input=0;
uint16_t adc_value=0, adc_value_temp=0, adc_value_prev=0, check=0;

void set_data_ss(uint16_t val, uint8_t dimmer, uint8_t high_step, uint8_t adc_active, int dot_pnum)
{
	int i;
	uint8_t select = 0x10, p1=0, p2=0;
	float adc_buffer=0.0;
	for (i=0;i<4;i++)
	{
		p1 = (PORTA & 0x0F) | select;
		p2= ~pgm_read_byte(&SS_DATA[val%10]);
		if (i==(dot_pnum-1)) p2 &= ~(1<<7); // make it zero for inverse output (CA display)
		PORTA = p1;
		PORTC = p2;
		if (dimmer)
		{
			if (!(select & (1<<(3+high_step))))
			{
				_delay_us(150);
				PORTA &= 0x0F;
			}
		}
		else if (!val)
		{
			PORTA &= 0x1F;
		}
		select = select<<1;
		val=val/10;
		if (adc_active) 
		{
			if (i==3)
			{
				adc_value_temp = (uint16_t)(adc_buffer/3.0);
				if (adc_value_temp < adc_value-4 || adc_value_temp > adc_value+4)
				{
					adc_value = adc_value_temp;
				}
				else
				{
					if (adc_value_prev==adc_value_temp)
					{
						check=check+1;
						if (check>=30)
						{
							adc_value = adc_value_temp;
						}
					}
					else
					{
						adc_value_prev=adc_value_temp;
						check = 0;
					}
				}
				_delay_ms(4.5);
			}
			else
			{
				adc_buffer = adc_buffer + Read_ADC(0);
			}
		}
		else 
		{
			_delay_ms(4.7);
		}
	}
	if (!Button_1) button_input = 1;
	else if (!Button_2) button_input = 2;
	else if (!Button_2) button_input = 3;
	else if (!Button_2) button_input = 4;
	else button_input = 0;
}

int main(void)
{
	int i=0,data=0;
	
	DDRA = 0xF0;
	DDRB = 0x0F;
	DDRC = 0xFF;
	DDRD = 0x3F;
	
	adc_init(DF_32);
	
	while(1)
	{
		if (i==10)
		{
			data = data +1;
			i=0;
		}
		PORTB |= (1<<0);
		set_data_ss(adc_value,Flse,2,True,4);
		PORTB &= ~(1<<0);
		i++;
    }
}