/*
 * adc.h
 *
 * Created: 1/11/2015 12:30:58 AM
 *  Author: Aman
 */ 

/* ADC Datasheet reference: ATmega32 page 2001 */

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

#define DF_02 0
#define DF_2 1
#define DF_4 2
#define DF_8 3
#define DF_16 4
#define DF_32 5
#define DF_64 6
#define DF_128 7

#define AREF_PIN 0X00
#define AVCC_PIN 0x40
#define INT_2P56V 0xC0

	// Start  Header settings
	#define ADC_VREF AVCC_PIN  // change this line to change the ADC reference
	// End Header settings

#include <avr/io.h>
#include <util/delay.h>

void adc_init(unsigned char AD_PRSCL)
{
	ADCSRA=0x80 | AD_PRSCL;
}

float Read_ADC(unsigned char adc_input)
{
	char i;
	uint16_t temp=0;
	for (i=0;i<40;i++)  // make 25 sample and then average it
	{
		ADMUX = adc_input | ADC_VREF; //MUX setup page 215
		_delay_us(10);
		ADCSRA |= 0x40;
		while ((ADCSRA & 0x10) == 0);
		ADCSRA |= (1<<4);
		temp = temp + ADCW;
	}
	return (temp/i);
}


#endif /* ADC_H_ */