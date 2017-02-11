/*
 * myusart.h
 *
 * Created: 11/6/2014 4:15:34 PM
 *  Author: Aman
 */ 


#ifndef INCFILE_USART_H_
#define INCFILE_USART_H_

#include <avr/io.h>
#include <math.h>
#include <string.h>

void usart_init(uint32_t baud)
{
	// Calculation for baud rate value into ubrr value
	uint16_t ubrr = lrint(( F_CPU / (16L * baud) ) - 1);
	// Set baud rate
	UBRRH = (uint8_t)(ubrr>>8);
	UBRRL = (uint8_t)ubrr;
	// Enable receiver and transmitter
	UCSRB = (1<<RXEN)|(1<<TXEN);
	// Set frame format: 8data, 1stop bit
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
}

void usart_putchar(char data)
{
	// Wait for empty transmit buffer
	while ( !(UCSRA & (_BV(UDRE))) );
	// Start transmission
	UDR = data;
}

char usart_getchar(void)
{
	// Wait for incoming data
	while ( !(UCSRA & (_BV(RXC))) );
	// Return the data
	return UDR;
}

void usart_putstr(char *s)
{
	// loop through entire string
	while (*s) {
		usart_putchar(*s);
		s++;
	}
}
//function to send a string through tx pin
unsigned char usart_dthit(void) {
	unsigned char b;
	b=0;
	if(UCSRA & (1<<RXC)) b=1;
	return b;
}
// help and function to get a string from rx pin
/*
\r = chr(13)
\t = chr(9)
\n = vbCrLf  ( chr(13) & chr(10)
\v = chr(10)
*/
void usart_getstr(char *ch)
{
	while(1)
	{
		*ch=usart_getchar();
		if (*ch!='\r') ch++;
		else break;
	}
}

/*
void lcd_putsf(char *a)
{
	int i;
	for(i=0;a[i]!='\0';i++)
	lcd_putchar(a[i]);
}

// write the string located in SRAM
void lcd_puts(char *str)
{
	char k;
	while((k=*str++)!=0)
	{
		lcd_putchar(k);
	}
}

*/


/* Need stdio header file to use this function
// this function is called by printf as a stream handler
int usart_putchar_printf(char var, FILE *stream) {
	// translate \n to \r for br@y++ terminal
	if (var == '\n') usart_putchar('\r');
	usart_putchar(var);
	return 0;
	
	
	if(UCSRA & (1<<RXC))
	{
		//ch[0]=UDR;
		scanf("%s",&ch);
		srl=atoi(ch);
	}
}
*/




#endif /* USART_H_ */