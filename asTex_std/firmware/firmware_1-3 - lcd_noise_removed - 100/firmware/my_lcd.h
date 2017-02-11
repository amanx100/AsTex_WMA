/*
 * my_lcd.h
 *
 * Created: 3/11/2015 1:51:29 AM
 *  Author: Aman
 */ 

/*
Warning:
To use this header file please define belows name in your main c file

//Start LCD display connections
#define D4_0 (PORT &= ~(1<<P))
#define D4_1 (PORT |= (1<<P))
#define D5_0 (PORT &= ~(1<<P))
#define D5_1 (PORT |= (1<<P))
#define D6_0 (PORT &= ~(1<<P))
#define D6_1 (PORT |= (1<<P))
#define D7_0 (PORT &= ~(1<<P))
#define D7_1 (PORT |= (1<<P))

#define RS_0 (PORT &= ~(1<<P))
#define RS_1 (PORT |= (1<<P))

#define EN_0 (PORT &= ~(1<<P))
#define EN_1 (PORT |= (1<<P))
//End of LCD display connections

*/

//Start LCD display connections
#define D4_0 (PORTA &= ~(1<<PC4))
#define D4_1 (PORTA |= (1<<PC4))
#define D5_0 (PORTA &= ~(1<<PC5))
#define D5_1 (PORTA |= (1<<PC5))
#define D6_0 (PORTA &= ~(1<<PC6))
#define D6_1 (PORTA |= (1<<PC6))
#define D7_0 (PORTA &= ~(1<<PC7))
#define D7_1 (PORTA |= (1<<PC7))

#define RS_0 (PORTA &= ~(1<<PC1))
#define RS_1 (PORTA |= (1<<PC1))

#define EN_0 (PORTA &= ~(1<<PC3))
#define EN_1 (PORTA |= (1<<PC3))
//End of LCD display connections

#ifndef MY_LCD_H_
#define MY_LCD_H_


#include<util/delay.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//Set Data to the particular pin out
void lcd_port(char data)
{
	if(data & (1<<0)) D4_1;
	else D4_0;
	if(data & (1<<1)) D5_1;
	else D5_0;
	if(data & (1<<2)) D6_1;
	else D6_0;
	if(data & (1<<3)) D7_1;
	else D7_0;
}

//Write a command to the Display
void lcd_cmd(uint8_t cmd)
{
	RS_0;
	EN_0;
	lcd_port(cmd >> 4);
	EN_1;
	_delay_ms(1);
	EN_0;
	_delay_ms(1);
	lcd_port(cmd & 0x0F);
	EN_1;
	_delay_ms(1);
	EN_0;
	_delay_ms(2);
}

void lcd_clear()
{
	lcd_cmd(1);
}

//Initialization of Display
void lcd_init()
{
	lcd_port(0x00);
	_delay_ms(18);
	lcd_cmd(0x33);
	lcd_cmd(0x32); //see the datasheet flowchart on page 46.
	lcd_cmd(0x28);
	lcd_cmd(0x28);
	lcd_cmd(0x06);
	lcd_cmd(0x0C);
	lcd_cmd(0x01);
}

//Set a location at Display RAM
void lcd_gotoxy(char row, char col)
{
	if(row == 0)		lcd_cmd(0x80 + col);
	else if(row == 1)	lcd_cmd(0xC0 + col);
	else if(row == 2)	lcd_cmd(0x94 + col);
	else if(row == 3)	lcd_cmd(0xD4 + col);
}

//Write a Character/Data to the Display
void lcd_putchar(char ch)
{
	RS_1;
	EN_0;
	lcd_port(ch >> 4);
	EN_1;
	_delay_us(100);
	EN_0;
	_delay_us(100);
	lcd_port(ch & 0x0F);
	EN_1;
	_delay_us(100);
	EN_0;
	_delay_us(100);
}

// write the string located in FLASH Memory
void lcd_putsf(char *a)
{
	int i;
	for(i=0;a[i]!='\0';i++)
	lcd_putchar(a[i]);
}

// write the string located in SRAM Memory
void lcd_puts(char *str)
{
	char k;
	while((k=*str++)!=0)
	lcd_putchar(k);
}

//Function to write a long integer type value to the LCD Display
void lcd_putsint_z(int32_t val_m, signed int field_length)
{
	int32_t val = fabs(val_m);
	char str[10]={0};
	int i=9,j=0;
	while(val)
	{
		str[i]=val%10;
		val=val/10;
		i--;
	}
	if(field_length==-1)
	while(str[j]==0) j++;
	else
	j=10-field_length;
	if(val_m<0) lcd_putchar('-');
	for(i=j;i<10;i++)
	lcd_putchar(48+str[i]);
}

void lcd_putsint(int32_t val, int8_t length)
{
	char str[10];
	int i = 0;
	if(val<0) lcd_putchar('-');
	val = fabs(val);
	for (i=0;val>0;val=val/10)
	str[i++] = 48+(val % 10);
	str[i] = '\0';
	strrev(str);
	lcd_puts(str);
	if ((length = length - strlen(str)) >= 0)
	for (i=0;length>i;i++)
	lcd_putchar(' ');
	else
	lcd_puts("E");
}

//Shift LCD String to right direction
void lcd_shiftr()
{
	lcd_cmd(0x1C);
}

//Shift LCD String to left direction
void lcd_shiftl()
{
	lcd_cmd(0x18);
}



#endif /* MY_LCD_H_ */