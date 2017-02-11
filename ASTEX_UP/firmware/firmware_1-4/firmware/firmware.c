/*
 * firmware.c
 *
 * Created: 3/7/2015 10:47:00 PM
 *  Author: Aman
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <math.h>
#include "my_lcd.h"
#include "MAX6675.h"
#include "keypad.h"


/////////// variable for time management
uint32_t i_time=0, compare_time=0;

//////////// variable for temperature management
uint32_t i_water=0, i_water_hold = 0;
uint32_t  water_flow_divider=0;
uint32_t EEMEM  water_flow_divider_hold = 330;  //should be keep 330


//////////// variable for temperature measurement and it's stability
uint8_t first_time_check = 0x00;
uint16_t temp_array[4];
char div4=0;
float start_temp=0.0, end_temp=0.0, ratio_start=0.0, ratio_end=0.0; //
float temp_buffer=0.0, Global_temperature=0.0, final_temp=0.0, temp_calc_nominator=0.0, temp_calc_denominator=0.0;
float EEMEM start_temp_hold=30.0, end_temp_hold=50.0, ratio_start_hold=93.0, ratio_end_hold=130.0;


ISR(INT0_vect)
{
	i_water = i_water + 1;
}


uint16_t stable_temp(uint16_t temp_val)
{
	int i=0, avg_temp=0;
	if (first_time_check == 0)
	{
		for (i=0;i<=3;i++)	temp_array[i] = temp_val;
		first_time_check = 1;
		return temp_val;
	}
	else
	{
		for(i=0;i<=2;i++)	
		{
			temp_array[i] = temp_array[i+1];
		}
		temp_array[3] = temp_val;
		
		if ((temp_array[3] + 1) == temp_array[2]) temp_array[3] = temp_array[2];
		//for (i=0;i<=3;i++)	avg_temp = avg_temp + temp_array[i];	//for average
		return temp_array[3];//round(avg_temp/4);
	}
}


ISR(TIMER1_COMPA_vect)
{
	wdt_reset();
	div4 = div4 + 1;
	temp_buffer = temp_buffer + Read_temperature();
	if (div4>=4)
	{
		i_time = i_time + 1;
		Global_temperature = round(temp_buffer / 4);
		Global_temperature = stable_temp(Global_temperature);
		temp_calc_nominator = (ratio_end - ratio_start) * (Global_temperature - start_temp);
		temp_calc_denominator = (end_temp - start_temp);
		final_temp = Global_temperature * (((temp_calc_nominator/temp_calc_denominator) + ratio_start) / 100.0);
		
		temp_buffer = 0.0;
		div4 = 0x00;
	}
}


int main(void)
{
	uint32_t kp_val=0, water=0, temp=0, time=0;
	uint32_t water_read=0, temp_read=0, time_read=0;
	uint8_t input_f=0, mxpos=6, kp_input=0, y_pos=0;
	
	DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2);
	DDRC = 0xFE;
	
	max6675_init();
	lcd_init();
	keypad_init();
	
	TCCR1A=0x00;
	TCCR1B=0x0C;		//Timer counter 1 initialization with CTC and interrupt mode with Compare match OCR1A
	OCR1AH=0x0F;  //3d09
	OCR1AL=0x42;

	GICR |= (1<<INT0);				// Enable INT2
	MCUCSR |= 0x00;					// Trigger INT2 settings
	MCUCR |= (1<<ISC01)|(1<<ISC00);	//trigger int0 on rising edge
	GIFR = (1<<INTF0);				//Interrupt flag 2 is cleared by writing a logical one into INTF2
	TIMSK=0x10;						//Timer/Counter1, Output Compare A Match Interrupt Enable
	sei();							//Global Interrupt enable

	wdt_enable(WDTO_500MS);
	//END Setup and initialization


/*
***************************************************************************************************************
***********************************************************************************************
***********************************>> Start Application from here <<*********************
**************************************************************************************************
***************************************************************************************************************
*/
start_pgm:
	// keep off all relay at the start point 
	PORTB &= ~((1<<PB0)|(1<<PB1)|(1<<PB2));	
	
	//updating the variable from eeprom
	start_temp = eeprom_read_float(&start_temp_hold);
	end_temp = eeprom_read_float(&end_temp_hold);
	ratio_start = eeprom_read_float(&ratio_start_hold);
	ratio_end = eeprom_read_float(&ratio_end_hold);
	water_flow_divider = eeprom_read_dword(&water_flow_divider_hold);

	//Welcome message and startup
	lcd_clear();
	lcd_gotoxy(0,15);
	lcd_puts("ASTEX");
	lcd_gotoxy(1,0);
	lcd_puts("TEMPERATURE:    ßC");
	lcd_gotoxy(2,0);
	lcd_puts("*> SETTINGS");
	lcd_gotoxy(3,0);
	lcd_puts("#> START TASK");
	
	while(scan_kpd_4x4());
	_delay_ms(100);
    while(1)
    {
		if (compare_time != i_time)
		{
			lcd_gotoxy(1,12);
			if (Global_temperature == 2000)
			{
				lcd_puts("TCNC!!");
			}
			else
			{
				
				lcd_putsint(final_temp,4);
				lcd_puts("ßC");
			}
			compare_time = i_time;
		}
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 12)
			{
				break;
			}
			else if (kp_input == 11)
			{
				goto settings_calibration;
			}
		}
    }
optin_select:
	lcd_clear();
	lcd_gotoxy(0,15);
	lcd_puts("ASTEX");
	lcd_gotoxy(1,0);
	lcd_puts("C> CHOOSE INPUT");
	lcd_gotoxy(2,0);
	lcd_puts("#> MANUAL INPUT");
	lcd_gotoxy(3,0);
	lcd_puts("*> BACK");
	
	water = 0;
	temp = 0;
	time = 0;
	
	while(scan_kpd_4x4());
	_delay_ms(100);
	while(1)
	{
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 11)
			{
				goto start_pgm;
			}
			else if (kp_input == 15)
			{
				goto selection;
			}
			else if (kp_input == 12)
			{
				goto pre_manual_operation;
			}
		}
	}
	
selection:
	lcd_clear();
	lcd_gotoxy(0,0);
	lcd_puts("A:70Lit._25ßC_2min");
	lcd_gotoxy(1,0);
	lcd_puts("B:70Lit._50ßC_2min");
	lcd_gotoxy(2,0);
	lcd_puts("C:50Lit._50ßC_2min");
	lcd_gotoxy(3,0);
	lcd_puts("D:60Lit._50ßC_2min");
	
	while(scan_kpd_4x4());
	_delay_ms(100);
	while(1)
	{
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 13)
			{
				water = 70;
				temp = 25;
				time = 2;
				goto confirmation;
			}
			else if (kp_input == 14)
			{
				water = 70;
				temp = 50;
				time = 2;
				goto confirmation;
			}
			else if (kp_input == 15)
			{
				water = 50;
				temp = 50;
				time = 2;
				goto confirmation;
			}
			else if (kp_input == 16)
			{
				water = 60;
				temp = 50;
				time = 2;
				goto confirmation;
			}
			else if (kp_input == 11 || kp_input == 12)
			{
				goto optin_select;
			}
		}
	}
	
//for manual input show the button

pre_manual_operation:

input_f = 1;
	lcd_clear();
	lcd_gotoxy(0,0);
	lcd_puts("WATER:    Liter");
	lcd_gotoxy(1,0);
	lcd_puts("TEMP.:    ßC");
	lcd_gotoxy(2,0);
	lcd_puts("TIME :    minute");
	lcd_gotoxy(3,0);
	lcd_puts("*>BACK D>DEL. #>NEXT");
	
manual_operation:

	while(scan_kpd_4x4());  // to eliminate the unwanted button press
	_delay_ms(100);
	while(1)
	{
		if (input_f == 1)
		{
			y_pos = 0;
			mxpos = 6;
			lcd_gotoxy(0,6);
			lcd_puts("    ");
			lcd_gotoxy(0,6);
			lcd_cmd(0x0D);
			kp_val = 0;
			input_f = 2;
		}
		else if (input_f == 3)
		{
			y_pos = 1;
			mxpos = 6;
			lcd_gotoxy(1,6);
			lcd_puts("    ");
			lcd_gotoxy(1,6);
			lcd_cmd(0x0D);
			kp_val = 0;
			input_f = 4;
		}
		else if (input_f == 5)
		{
			y_pos = 2;
			mxpos = 6;
			lcd_gotoxy(2,6);
			lcd_puts("    ");
			lcd_gotoxy(2,6);
			lcd_cmd(0x0D);
			kp_val = 0;
			input_f = 6;
		}
		
		else if (input_f == 7)  //this section is to update the calibration of temperature value
		{
			lcd_clear();
			lcd_gotoxy(0,0);
			lcd_puts("__Temp. Calibration_");
			lcd_gotoxy(1,0);
			lcd_puts("Converted value:");
			lcd_gotoxy(2,0);
			lcd_puts("ERVL :    ßC/STRT.P");
			lcd_gotoxy(3,0);
			lcd_puts("*>BACK D>DEL. #>SAVE");
			
			y_pos = 2;
			mxpos = 6;
			lcd_cmd(0x0D);
			kp_val = 0;

			input_f = 8;
		}
		else if (input_f == 8)
		{
			if (compare_time != i_time)
			{
				lcd_cmd(0x0C);
				lcd_gotoxy(1,16);
				lcd_putsint(Global_temperature,4);
				compare_time = i_time;
				lcd_gotoxy(y_pos,mxpos);
				if (mxpos == 10)
				{
					lcd_cmd(0x0C);
				}
				else
				{
					lcd_cmd(0x0D);
				}
			}
		}
		else if (input_f == 9)  //this section is to update the calibration of water
		{
			lcd_clear();
			lcd_gotoxy(0,0);
			lcd_puts("__Water Calibration_");
			lcd_gotoxy(1,0);
			lcd_puts("Pulse Counted:");
			lcd_gotoxy(1,14);
			lcd_putsint(i_water_hold,6);
			lcd_gotoxy(2,0);
			lcd_puts("ERVL :    Liter");
			lcd_gotoxy(3,0);
			lcd_puts("*>BACK D>DEL. #>SAVE");
			
			y_pos = 2;
			mxpos = 6;
			lcd_gotoxy(2,6);
			lcd_cmd(0x0D);
			kp_val = 0;
			input_f = 10;
		}
		else if (input_f == 11)  //this section is to update the calibration of temperature value
		{
			lcd_clear();
			lcd_gotoxy(0,0);
			lcd_puts("__Temp. Calibration_");
			lcd_gotoxy(1,0);
			lcd_puts("Converted value:");
			lcd_gotoxy(2,0);
			lcd_puts("ERVL :    ßC/END.P");
			lcd_gotoxy(3,0);
			lcd_puts("*>BACK D>DEL. #>SAVE");
			
			y_pos = 2;
			mxpos = 6;
			lcd_cmd(0x0D);
			kp_val = 0;

			input_f = 12;
		}
		else if (input_f == 12)
		{
			if (compare_time != i_time)
			{
				lcd_cmd(0x0C);
				lcd_gotoxy(1,16);
				lcd_putsint(Global_temperature,4);
				compare_time = i_time;
				lcd_gotoxy(y_pos,mxpos);
				if (mxpos == 10)
				{
					lcd_cmd(0x0C);
				}
				else
				{
					lcd_cmd(0x0D);
				}
			}
		}
		
		if ((kp_input = scan_kpd_4x4()))
		{
			if (kp_input == 11)
			{
				lcd_cmd(0x0C);
				if (input_f == 8 || input_f == 10 || input_f == 12) 
				{
					input_f = 0;
					goto settings_calibration;
				}
				else 
				{
					input_f = 0;
					goto optin_select;
				}
			}
			
			else if (kp_input == 16) // D
			{
				if (mxpos>6)
				{
					mxpos= mxpos - 1;
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar(' ');
					lcd_gotoxy(y_pos,mxpos);
					lcd_cmd(0x0D);
					kp_val = (kp_val / 10);
				}
				else
				{
					if (input_f == 2)
					{
						input_f = 0;
						lcd_cmd(0x0C);
						goto optin_select;
					}
					else if (input_f == 4)
					{
						input_f = 1;
						goto manual_operation;
					}
					else if (input_f == 6)
					{
						input_f = 3;
						goto manual_operation;
					}
					else if (input_f == 8)
					{
						input_f = 0;
						lcd_cmd(0x0C);
						goto settings_calibration;
					}
					else if (input_f == 10)
					{
						input_f = 0;
						goto settings_calibration;
					}
					else if (input_f == 12)
					{
						input_f = 0;
						lcd_cmd(0x0C);
						goto settings_calibration;
					}
				}
			}
			else if (kp_input == 12) // #
			{
				if (input_f == 2)
				{
					water = kp_val;
					kp_val = 0;
					mxpos = 6;
					input_f = 3;
				}
				else if (input_f == 4)
				{
					temp = kp_val;
					kp_val = 0;
					mxpos = 6;
					input_f = 5;
				}
				else if (input_f == 6)
				{
					time = kp_val;
					kp_val = 0;
					mxpos = 6;
					input_f = 0;
					lcd_cmd(0x0C);
					break;
				}
				else if (input_f == 8)
				{
					start_temp = Global_temperature;
					eeprom_update_float(&start_temp_hold,start_temp);
					eeprom_update_float(&ratio_start_hold,(kp_val * 100.0)/start_temp);
					kp_val = 0;
					mxpos = 6;
					input_f = 0;
					lcd_cmd(0x0C);
					goto temp_calib;
				}
				else if (input_f == 10)
				{
					//water_flow_divider = (i_water_hold * 100UL) / kp_val;
					eeprom_update_dword(&water_flow_divider_hold,(i_water_hold * 100UL) / kp_val);
					kp_val = 0;
					mxpos = 6;
					input_f = 7;
					lcd_cmd(0x0C);
					goto manual_operation;
				}
				else if (input_f == 12)
				{
					end_temp = Global_temperature;
					eeprom_update_float(&end_temp_hold,end_temp);
					eeprom_update_float(&ratio_end_hold,(kp_val * 100.0) / end_temp);
					kp_val = 0;
					mxpos = 6;
					input_f = 0;
					lcd_cmd(0x0C);
					goto start_pgm;
				}
			}
			
			if (mxpos <= 9) // limit the excess input
			{
				if (kp_input == 9) // 9
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('9');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 9;
				}
				else if (kp_input == 10) //0
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('0');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10);
				}
				else if (kp_input == 5) // 5
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('5');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 5;
				}
				else if (kp_input == 6) //6
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('6');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 6;
				}
				else if (kp_input == 7) //7
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('7');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 7;
				}
				else if (kp_input == 8) //8
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('8');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 8;
				}

				else if (kp_input == 1) //1
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('1');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 1;
				}
				else if (kp_input == 2) //2
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('2');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 2;
				}
				else if (kp_input == 3) //3
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('3');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 3;
				}
				else if (kp_input == 4) //4
				{
					lcd_gotoxy(y_pos,mxpos);
					lcd_putchar('4');
					mxpos= mxpos + 1;
					kp_val = (kp_val * 10) + 4;
				}
				if (mxpos == 10)
				{
					lcd_cmd(0x0C);
				}
			}
			while (scan_kpd_4x4());
			_delay_ms(100);
		}
		
		

	}

//End task Input system	


confirmation:
	
	lcd_clear();
	lcd_gotoxy(0,0);
	lcd_puts("WATER:    Liter");
	lcd_gotoxy(0,6);
	lcd_putsint(water,4);
	lcd_gotoxy(1,0);
	lcd_puts("TEMP.:    ßC");
	lcd_gotoxy(1,6);
	lcd_putsint(temp,4);
	lcd_gotoxy(2,0);
	lcd_puts("TIME :    minute");
	lcd_gotoxy(2,6);
	lcd_putsint(time,4);
	lcd_gotoxy(3,0);
	lcd_puts("*>WRONG    #>CORRECT");
	
	while(scan_kpd_4x4());
	_delay_ms(100);
	while(1)
	{
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 11)
			{
				goto optin_select;
			}
			else if (kp_input == 12)
			{
				break;
			}
		}
	}
	
	//*********************************************************************************************************************
	//Operation and progress mode ***************************************
	//*******************************************************************************************************************
	
		lcd_clear();
		lcd_gotoxy(0,0);
		lcd_puts("WATERÿ0000/    LITER");
		lcd_gotoxy(0,11);
		lcd_putsint(water,4);
		lcd_gotoxy(1,0);
		lcd_puts("TEMP ÿ..../    ßC");
		lcd_gotoxy(1,11);
		lcd_putsint(temp,4);
		lcd_gotoxy(2,0);
		lcd_puts("TIME ÿ00:00/   Min.");
		lcd_gotoxy(2,12);
		lcd_putsint(time,3);
		lcd_gotoxy(3,0);
		lcd_puts("C>CANCEL      #>SKIP");
		
		//Applying one by one method to calculate water, temperature and time measurement
	
	
	_delay_ms(100);
	i_water = 0;
	i_time = 0;
	while (1) //This while loop is for water valve open after 1 second
	{
		if (i_time > 0)
		{
			PORTB |= (1<<2);  //This port pin is for water valve on
			lcd_gotoxy(0,5);
			lcd_putchar('~');
			break;
		}
		while(scan_kpd_4x4());
		//TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	_delay_ms(100);
	while (1) //This while loop is for water calculation
	{
		water_read = (i_water * 100UL)/water_flow_divider; //This is the assumption to calculate water 1 liter = 2 rotation of propeller
		if (water_read >= water)
		{
			break;
		}
		
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data 
		{
			if (kp_input == 12)
			{
				break;
			}
			else if (kp_input == 15)
			{
				goto start_pgm;
			}
		}
		lcd_gotoxy(0,6);
		lcd_putsint_z(water_read,4);
		lcd_gotoxy(1,6);
		lcd_putsint_z(final_temp,4);
		_delay_ms(10);
	}
	
	PORTB &= ~(1<<2); //This port pin is for water valve off
	lcd_gotoxy(0,5);
	lcd_putchar('ÿ');
	
	//Updating last status for water
	
	lcd_gotoxy(0,6);
	lcd_putsint_z(water,4);
	
	_delay_ms(100);
	i_time = 0;
	while (1) //This while loop is for steam valve open after 1 second
	{
		if (i_time > 0)
		{
			PORTB |= (1<<1);  //This port pin is for steam valve on
			lcd_gotoxy(1,5);
			lcd_putchar('~');
			break;
		}
		while(scan_kpd_4x4());
	}
	_delay_ms(100);
	while (1) //This while loop is for temperature calculation
	{
		if (compare_time != i_time)
		{
			temp_read = (final_temp);//(uint32_t)temp_divider; //This is the divider to adjust the temperature now 4 for experiment
			lcd_gotoxy(1,6);
			lcd_putsint_z(temp_read,4);			
			compare_time = i_time;
		}
		if (temp_read >= temp)
		{
			break;
		}
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 12)
			{
				break;
			}
			else if (kp_input == 15)
			{
				goto start_pgm;
			}
		}
	}
	
	PORTB &= ~(1<<1); //This port pin is for steam valve off
	
	lcd_gotoxy(1,5);
	lcd_putchar('ÿ');
	
	// Updating last status for temperature
	lcd_gotoxy(1,6);
	lcd_putsint_z(temp,4);
	
	i_time = 0;
	lcd_gotoxy(2,5);
	lcd_putchar('~');
	while(scan_kpd_4x4());
	_delay_ms(100);
	while (1) //This while loop is for time calculation
	{
		time_read = i_time / 60;
		if (time_read >= time)
		{
skip_time:
			PORTB |= (1<<0); //This port pin is for alarm on
			lcd_gotoxy(2,5);
			lcd_putchar('ÿ');
			break;
		}
		lcd_gotoxy(2,6);
		lcd_putsint_z(time_read,2);
		lcd_gotoxy(2,9);
		lcd_putsint_z(i_time % 60,2);
		//lcd_gotoxy(1,6);
		//lcd_putsint_z(Global_temperature-temp_divider,4);
		
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 12)
			{
				goto skip_time;
			}
			if (kp_input == 15)
			{
				goto start_pgm;
			}
		}
	}
	 //this section is to wait for alarm off command
	 i_time = 0;
	 lcd_clear();
	 lcd_puts("Time:   :");

	 lcd_gotoxy(0,15);
	 lcd_puts("ASTEX");
	 lcd_gotoxy(1,6);
	 lcd_puts("ALARMING!");
	 lcd_gotoxy(3,0);
	 lcd_puts("#> ALARM OFF");
	 
	while(scan_kpd_4x4());
	_delay_ms(100);
	while (1)
	{
		lcd_gotoxy(0,6);
		lcd_putsint_z(i_time/60,2);
		lcd_gotoxy(0,9);
		lcd_putsint_z(i_time%60,2);
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data via usart
		{
			if (kp_input == 12)
			{
				PORTB &= ~(1<<0); //alarm off
				cli();
				while(1);
			}
		}
		_delay_ms(10);
	}
	//End progress page
	/*
	************************************************************************************************************
	**  Start settings and Calibration part  *******************************************************************
	************************************************************************************************************
	*/
	
	settings_calibration:
	
	lcd_clear();
	lcd_gotoxy(0,0);
	lcd_puts("______SETTINGS______");
	lcd_gotoxy(1,0);
	lcd_puts("A> System Calibration");
	//lcd_gotoxy(2,0);
	//lcd_puts("B> Water Calibration");
	lcd_gotoxy(3,0);
	lcd_puts("*>BACK        #>NEXT");

	while(scan_kpd_4x4());
	_delay_ms(100);
	while (1)
	{
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data via usart
		{
			if (kp_input == 13)  // option a for system calibration
			{
				break;
				//input_f = 7;
				//goto manual_operation;
			}
			//else if (kp_input == 14)  // option b for water flow calibration
			//{
			//	break;
			//}
			else if (kp_input == 11 || kp_input == 12)
			{
				goto start_pgm; // can be use cli() then wdt reset
			}
		}
		_delay_ms(10);
	}
	
	lcd_clear();
	lcd_cmd(0x0C);
	lcd_gotoxy(0,0);
	lcd_puts("__Water Calibration_");
	//lcd_gotoxy(1,0);
	//lcd_puts("Counted Pulse:0");
	lcd_gotoxy(2,0);
	lcd_puts("#> Load Water");
	lcd_gotoxy(3,0);
	lcd_puts("C> CANCEL");
	
	while (scan_kpd_4x4());  // to eliminate the unwanted button press
	_delay_ms(100);
	while (1)
	{
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 12)  // press # to load water
			{
				break;
			}
			else if (kp_input == 15)  // c for cancel
			{
				goto settings_calibration;
			}
		}
		_delay_ms(10);
	}

	i_water = 0;
	i_water_hold =0;
	PORTB |= (1<<2);  //This port pin is for water valve on
	
	lcd_gotoxy(1,0);
	lcd_puts("Counted Pulse:");
	lcd_gotoxy(2,0);
	lcd_puts("#> Stop Loading");
	lcd_gotoxy(3,0);
	lcd_puts("         ");
	
	while (scan_kpd_4x4());  // to eliminate the unwanted button press
	_delay_ms(100);
	while (1)
	{

		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 12)  // press # to stop loading
			{
				PORTB &= ~(1<<2); //water valve off
				_delay_ms(500);
				i_water_hold = i_water;
				input_f = 9;
				goto manual_operation;
			}
		}
		
		lcd_gotoxy(1,14);
		lcd_putsint(i_water,6);	
	}

temp_calib:
	
	lcd_clear();
	lcd_cmd(0x0C);
	lcd_gotoxy(0,0);
	lcd_puts("__Temp. Calibration_");
	lcd_gotoxy(1,0);
	lcd_puts("Counted Value:    ßC");
	lcd_gotoxy(2,0);
	lcd_puts("#> Stop Steam");

	while(scan_kpd_4x4());
	_delay_ms(100);
	
	PORTB |= (1<<1); //steam valve on
	
	while(1)
	{
		if (compare_time != i_time)
		{
			lcd_gotoxy(1,14);
			lcd_putsint(Global_temperature,4);
			compare_time = i_time;
		}
		if ((kp_input = scan_kpd_4x4()))  //scan for keypad data
		{
			if (kp_input == 12)  // press # to stop loading
			{
				PORTB &= ~(1<<1); //steam valve off
				_delay_ms(500);
				input_f = 11;
				goto manual_operation;
			}
		}
	}
	
	/////////////	
}