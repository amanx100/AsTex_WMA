/*
 * adc.h
 *
 * Created: 1/11/2015 12:30:58 AM
 *  Author: Aman
 */ 


/*
REFS1 REFS0 Voltage Reference Selection
0x00	0 0 AREF, Internal Vref turned off
0x40	0 1 AVCC with external capacitor at AREF pin
0x80	1 0 Reserved
0xC0	1 1 Internal 2.56V Voltage Reference with external capacitor at AREF pin
*/

/*
Table 85. ADC Prescaler Selections
	ADPS2 ADPS1 ADPS0 Division Factor
0	0		0		0		2
1	0		0		1		2
2	0		1		0		4
3	0		1		1		8
4	1		0		0		16
5	1		0		1		32
6	1		1		0		64
7	1		1		1		128
*/

#ifndef ADC_H_
#define ADC_H_


#include <avr/io.h>
#include <util/delay.h>

void adc_init(unsigned char AD_PRSCL)
{
	ADCSRA=0x80 | AD_PRSCL;
}

unsigned int Read_ADC(unsigned char adc_input, unsigned char ADC_VREF)
{
	char i;
	int temp=0;
	for (i=0;i<5;i++)
	{
		ADMUX=adc_input | ADC_VREF; //MUX setup page 215
		_delay_us(10);
		ADCSRA|=0x40;
		while ((ADCSRA & 0x10)==0);
		ADCSRA|=(1<<4);
		temp = temp + ADCW;
	}
	return (temp/i);	
}


#endif /* ADC_H_ */