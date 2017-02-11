#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <util/delay.h>
#include "SSD1289.h"
#include "XPT2046.h"
#include "adc.h"
#include "button.h"
#include "myusart.h"

uint32_t i_water=0, i_water_hold = 0, i_time=0, compare_time=0;

unsigned int EEMEM temp_divider_hold=100, water_flow_divider_hold=330;  //should be keep 100 and 330
unsigned int temp_divider=0, water_flow_divider=0;

ISR(INT0_vect)
{
	i_water = i_water + 1;
}
ISR(TIMER1_COMPA_vect)
{
	i_time = i_time + 1;
}

int main(void)
{
	#define back_color blue
	uint16_t TPval[2]={0};
	uint16_t mxpos=13;
	uint32_t kp_val=0, water=0, temp=0, time=0;
	uint32_t water_read=0, temp_read=0, time_read=0;
	uint8_t input_f=0, usart_data=0;
	
	PORTA=0x00;
	PORTB=0x40;
	PORTC=0x00;
	PORTD=0x00; //0x0C;
	
	DDRA=0xFE;
	DDRB=0xBF;
	DDRC=0xFF;
	DDRD=0xF0;
	usart_init(600);
	adc_init(7);
	TFT_Init();
	TP_init();
	//show_button();
	TCCR1A=0x00;
	TCCR1B=0x0C;		//Timer counter 1 initialization with CTC and interrupt mode with Compare match OCR1A 
	OCR1AH=0x7A;
	OCR1AL=0x12;

	GICR |= (1<<INT0);				// Enable INT2
	MCUCSR |= 0x00;					// Trigger INT2 settings
	MCUCR |= (1<<ISC01)|(1<<ISC00);	//trigger int0 on rising edge
	GIFR = (1<<INTF0);				//Interrupt flag 2 is cleared by writing a logical one into INTF2
	TIMSK=0x10;						//Timer/Counter1, Output Compare A Match Interrupt Enable
	sei();							//Global Interrupt enable
    //END Setup and initialization
	
	//**************************************************//
	//Start Application from here***********************//
	//**************************************************//
	
	
	
start_pgm:
	
	temp_divider = eeprom_read_word(&temp_divider_hold);
	water_flow_divider = eeprom_read_word(&water_flow_divider_hold);

	//Welcome message and startup
	TFT_Fill(back_color);
	//TFT_Text("Mark Wash & Dyeing Ltd.",23,30,8,yellow,back_color);
	TFT_Text("ASTEX",75,84,16,white,back_color);
	TFT_Box(35,150,195,174,yellow);
	TFT_Text("Start Task",45,155,16,blue,yellow);
	TFT_Box(35,274,109,292,yellow);
	TFT_Text("SETTINGS",40,280,8,blue,yellow);
	TFT_Text("Temperature:",96,5,8,white,blue);
	TFT_Image(180,265,50,50,image_data_astex_logo);
	TFT_Rectangle(179,264,230,315,white);

	while (1)
	{
		if (compare_time != i_time)
		{
			TFT_PutsInt((Read_ADC(0,0x00) * 100UL)/(uint32_t)temp_divider,4,200,5,8,white,back_color);
			compare_time = i_time;
		}
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)
			{
				usart_data = 0;
				break;
			}
			else if (usart_data == 11)
			{
				usart_data = 0;
				goto settings_calibration;
			}
			else usart_data = 0;
		}
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[0]>120 && TPval[0]<150 && TPval[1]>51 && TPval[1]<201)  // button for start task
			{
				break;
			}
			else if (TPval[0]>35 && TPval[0]<56 && TPval[1]>40 && TPval[1]<120)  // button for settings
			{
				goto settings_calibration;
			}
			_delay_ms(100);
		}
		
	}
	//End welcome message and startup
	
	//Start task input/selection system
selection:
	TFT_Fill(back_color);
	TFT_Text("Choose an option...",23,20,8,white,back_color);
	TFT_Box(27,45,203,62,yellow);
	TFT_Text("70Lit. _ 25'C _ 2min",35,50,8,black,yellow);
	TFT_Box(27,80,203,97,yellow);
	TFT_Text("70Lit. _ 50'C _ 2min",35,85,8,black,yellow);
	TFT_Box(27,115,203,132,yellow);
	TFT_Text("50Lit. _ 50'C _ 2min",35,120,8,black,yellow);
	TFT_Box(27,150,203,167,yellow);
	TFT_Text("60Lit. _ 50'C _ 2min",35,155,8,black,yellow);
	TFT_Box(27,188,203,228,red);
	TFT_Text("Manual Input",31,199,16,white,red);
	TFT_Box(27,260,203,277,yellow);
	TFT_Text("<<  BACK  <<",35,265,8,black,yellow);
	
	while (!(PIND & (1<<PD3)));  // to eliminate the unwanted button press
	while (1)
	{
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 13)
			{
				usart_data = 0;
				goto task_a;
			}
			else if (usart_data == 14)
			{
				usart_data = 0;
				goto task_b;
			}
			else if (usart_data == 15)
			{
				usart_data = 0;
				goto task_c;
			}
			else if (usart_data == 16)
			{
				usart_data = 0;
				goto task_d;
			}
			else if (usart_data == 12)
			{
				usart_data = 0;
				break;
			}
			else if (usart_data == 11)
			{
				usart_data = 0;
				goto start_pgm;
			}
			else usart_data = 0;
		}
		
		if (!(PIND & (1<<PD3)))  //scan for touch data
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[1]>35 && TPval[1]<211)  // for taking predefined or manual input
			{
				if (TPval[0]>197 && TPval[0]<216)
				{
task_a:
					water = 70;
					temp = 25;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>175 && TPval[0]<190)
				{
task_b:
					water = 70;
					temp = 50;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>148 && TPval[0]<167)
				{
task_c:
					water = 50;
					temp = 50;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>127 && TPval[0]<144)
				{
task_d:
					water = 60;
					temp = 50;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>83 && TPval[0]<116)  //button for manual input
				{
					break;
				}
				else if (TPval[0]>43 && TPval[0]<67)  //button for manual input
				{
					goto start_pgm;
				}
			}
			_delay_ms(100);
		}
	}
		//for manual input show the button
		
	input_f = 1;

button_operation:
	TFT_Fill(back_color);
	show_button();
	while (!(PIND & (1<<PD3)));  // to eliminate the unwanted button press
	while(1)
	{
		if (input_f == 1)
		{
			TFT_Box(10,69,231,101,white);
			TFT_Text("Input Water ...",10,20,16,white,back_color);
			TFT_Text("(Liter)  ",10,38,16,white,back_color);
			input_f = 2;
		}
		else if (input_f == 3)
		{
			TFT_Box(10,69,231,101,white);
			TFT_Box(10,20,231,54,back_color);
			TFT_Text("Input Temp ... ",10,20,16,white,back_color);
			TFT_Text("(Deg. C) ",10,38,16,white,back_color);
			input_f = 4;
		}
		else if (input_f == 5)
		{
			TFT_Box(10,69,231,101,white);
			TFT_Box(10,20,231,54,back_color);
			TFT_Text("Input Time ... ",10,20,16,white,back_color);
			TFT_Text("(Minute)",10,38,16,white,back_color);
			input_f = 6;
		}
		else if (input_f == 7)  //this section is to update the calubration of temperature value
		{
			TFT_Box(10,69,231,101,white);
			//TFT_Box(10,20,231,54,back_color);
			//TFT_Text("Thermal Calibration",45,140,8,white,back_color);
			TFT_Text("Thermal ADC value:",10,15,8,white,back_color);
			TFT_Text("Enter the real value...",10,38,8,white,back_color);
			input_f = 8;
		}
		else if (input_f == 8)
		{
			if (compare_time != i_time)
			{
				TFT_PutsInt(Read_ADC(0,0x00),4,162,15,8,white,back_color);
				compare_time = i_time;
			}
		}
		else if (input_f == 9)  //this section is to update the calibration of water
		{
			TFT_Box(10,69,231,101,white);
			//TFT_Box(10,20,231,54,back_color);
			//TFT_Text("Thermal Calibration",45,140,8,white,back_color);
			TFT_Text("Total pulse counted:",10,15,8,white,back_color);
			TFT_PutsInt(i_water_hold,-1,178,15,8,white,back_color);
			TFT_Text("Enter the real value...",10,38,8,white,back_color);
			input_f = 10;
		}
		
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 9)
			{
				usart_data = 0;
				goto btn_9;
			}
			else if (usart_data == 10)
			{
				usart_data = 0;
				goto btn_0;
			}
			else if (usart_data == 11)
			{
				usart_data = 0;
				goto btn_del;
			}
			else if (usart_data == 12)
			{
				usart_data = 0;
				goto btn_ok;
			}
			else if (usart_data == 5)
			{
				usart_data = 0;
				goto btn_5;
			}
			else if (usart_data == 6)
			{
				usart_data = 0;
				goto btn_6;
			}
			else if (usart_data == 7)
			{
				usart_data = 0;
				goto btn_7;
			}
			else if (usart_data == 8)
			{
				usart_data = 0;
				goto btn_8;
			}
			else if (usart_data == 1)
			{
				usart_data = 0;
				goto btn_1;
			}
			else if (usart_data == 2)
			{
				usart_data = 0;
				goto btn_2;
			}
			else if (usart_data == 3)
			{
				usart_data = 0;
				goto btn_3;
			}
			else if (usart_data == 4)
			{
				usart_data = 0;
				goto btn_4;
			}
			else usart_data = 0;
		}
		
		
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[0]>36 && TPval[0]<72)  // 9 0 DEL OK 
			{
				if (TPval[1]>23 && TPval[1]<73)
				{
btn_9:
					TFT_Text("9",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 9;
				}
				else if (TPval[1]>77 && TPval[1]<126)
				{
btn_0:
					TFT_Text("0",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10);
				}
				else if (TPval[1]>129 && TPval[1]<181)
				{
btn_del:
					if (mxpos>13)
					{
						mxpos-=14;
						TFT_Text(" ",mxpos,77,16,black,white);
						kp_val = (kp_val / 10);
					}
					else
					{
						if (input_f == 2)
						{
							input_f = 0;
							goto selection;
						}
						else if (input_f == 4)
						{
							input_f = 1;
							goto button_operation;
						}
						else if (input_f == 6)
						{
							input_f = 3;
							goto button_operation;
						}
						else if (input_f == 8 || input_f == 10)
						{
							input_f = 0;
							goto settings_calibration;
						}
					}				
				}
				else if (TPval[1]>183 && TPval[1]<236)
				{
btn_ok:
					if (input_f == 2)
					{
						water = kp_val;
						kp_val = 0;
						mxpos = 13;
						input_f = 3;
					}
					else if (input_f == 4)
					{
						temp = kp_val;
						kp_val = 0;
						mxpos = 13;
						input_f = 5;
					}
					else if (input_f == 6)
					{
						time = kp_val;
						kp_val = 0;
						mxpos = 13;
						input_f = 0;
						break;
					}
					else if (input_f == 8)
					{
						eeprom_update_word(&temp_divider_hold,(Read_ADC(0,0x00) * 100UL) / (uint32_t)kp_val);
						kp_val = 0;
						mxpos = 13;
						input_f = 0;
						goto settings_calibration;
					}
					else if (input_f == 10)
					{
						eeprom_update_word(&water_flow_divider_hold,(i_water_hold * 100UL) / (uint32_t)kp_val);
						kp_val = 0;
						mxpos = 13;
						input_f = 0;
						goto settings_calibration;
					}
				}
				while (!(PIND & (1<<PD3)));
			}
			else if (TPval[0]>75 && TPval[0]<112)  // 5 6 7 8 
			{
				if (TPval[1]>23 && TPval[1]<73)
				{
btn_5:
					TFT_Text("5",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 5;
				}
				else if (TPval[1]>77 && TPval[1]<126)
				{
btn_6:
					TFT_Text("6",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 6;
				}
				else if (TPval[1]>129 && TPval[1]<181)
				{
btn_7:
					TFT_Text("7",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 7;
				}
				else if (TPval[1]>183 && TPval[1]<236)
				{
btn_8:
					TFT_Text("8",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 8;
				}
				while (!(PIND & (1<<PD3)));
			}
			else if (TPval[0]>115 && TPval[0]<150)  // 1 2 3 4 
			{
				if (TPval[1]>23 && TPval[1]<73)
				{
btn_1:
					TFT_Text("1",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 1;
				}
				else if (TPval[1]>77 && TPval[1]<126)
				{
btn_2:
					TFT_Text("2",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 2;
				}
				else if (TPval[1]>129 && TPval[1]<181)
				{
btn_3:
					TFT_Text("3",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 3;
				}
				else if (TPval[1]>183 && TPval[1]<236)
				{
btn_4:
					TFT_Text("4",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 4;
				}
				while (!(PIND & (1<<PD3)));
			}
			_delay_ms(100);
		}
	}
	
	//End task Input system
	
	//Start confirmation input
confirmation:
	TFT_Fill(back_color);
	TFT_Text("Water :      Liter",20,50,8,white,back_color);
	TFT_PutsInt(water,-1,84,50,8,white,back_color);
	TFT_Text("Temp. :      Deg. C",20,70,8,white,back_color);
	TFT_PutsInt(temp,-1,84,70,8,white,back_color);
	TFT_Text("Time  :      minute",20,90,8,white,back_color);
	TFT_PutsInt(time,-1,84,90,8,white,back_color);
	TFT_Box(50,140,190,180,white);
	TFT_Box(50,200,190,240,white);
	TFT_Text("CORRECT !",57,152,16,black,white);
	TFT_Text("INCORRECT",57,212,16,red,white);
	while (!(PIND & (1<<PD3)));  // to eliminate the unwanted button press
	while(1)
	{
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)
			{
				usart_data = 0;
				break;
			}
			else if (usart_data == 11)
			{
				usart_data = 0;
				goto selection;
			}
			else usart_data = 0;
		}
		
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[1]>60 && TPval[1]<195)
			{
				if (TPval[0]>120 && TPval[0]<150)  //Correct button
				{
					break;
				}
				else if (TPval[0]>75 && TPval[0]<105)  // Incorrect button
				{
					goto selection;
				}
				_delay_ms(100);
			}
		}
	}
	//End confirmation input
	
	//Start progress page
	TFT_Fill(back_color);
	TFT_Text("Progress monitor...",20,20,8,white,back_color);
	
	TFT_Box(30,40,210,56,white);
	TFT_Rectangle(30,56,210,120,white);
	TFT_Text("Water:",35,44,8,black,white);
	TFT_Text("Status   : Valve Off",35,60,8,white,back_color); // Idle, Loading, Complete//On
	TFT_Text("0   %     0   Liter",43,72,8,white,back_color);
	TFT_Text("Target   :     Liter",35,84,8,white,back_color);
	TFT_PutsInt(water,-1,123,84,8,white,back_color);
	TFT_Text("Remaining:     Liter",35,102,8,white,back_color);
	TFT_PutsInt(water,4,123,102,8,white,back_color);
	
	TFT_Box(30,128,210,144,white);
	TFT_Rectangle(30,144,210,208,white);
	TFT_Text("Temperature:",35,132,8,black,white);
	TFT_Text("Status   : Valve Off",35,148,8,white,back_color);  //NC, Rising, Complete
	TFT_Text("0   %     N   Deg.C",43,160,8,white,back_color);
	TFT_Text("Target   :     Deg.C",35,172,8,white,back_color);
	TFT_PutsInt(temp,-1,123,172,8,white,back_color);
	TFT_Text("Remaining:     Deg.C",35,188,8,white,back_color);
	TFT_PutsInt(temp,3,123,188,8,white,back_color);
	
	TFT_Box(30,216,210,232,white);
	TFT_Rectangle(30,232,210,296,white);
	TFT_Text("Time:",35,220,8,black,white);
	TFT_Text("Status   : Idle",35,236,8,white,back_color);   //Idle , Counting, Complete
	TFT_Text("Target   :     min",35,248,8,white,back_color);
	TFT_PutsInt(time,-1,123,248,8,white,back_color);
	TFT_Text("Counted  : 0      min",35,260,8,white,back_color);
	TFT_Text("Remaining:       min",35,276,8,white,back_color);
	TFT_PutsInt(time,4,123,276,8,white,back_color);
	
	//Applying one by one method to calculate water, temperature and time measurement
	
	i_water = 0;
	i_time = 0;
	while (1) //This while loop is for water valve open after 1 second
	{
		if (i_time > 0)
		{
			//PORTD |= (1<<0);  //This port pin is for water valve on
			usart_putchar(97);
			_delay_ms(100);
			usart_putchar(97);
			TFT_Text("On ",171,60,8,white,red);
			break;
		}
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	
	while (1) //This while loop is for water calculation
	{
		water_read = (i_water * 100UL)/(uint32_t)water_flow_divider; //This is the assumption to calculate water 1 liter = 2 rotation of propeller
		if (water_read >= water)
		{
			break;
		}
		
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)
			{
				usart_data = 0;
				break;
			}
			else usart_data = 0;
		}
		
		else if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			if (TPval[0]>190 && TPval[0]<222 && TPval[1]>37 && TPval[1]<216)
			{
				break;
			}
		}
		TFT_PutsInt((water_read*100)/water,-1,43,72,8,white,back_color);
		TFT_PutsInt(water_read,-1,123,72,8,white,back_color);
		TFT_PutsInt(water-water_read,4,123,102,8,white,back_color);
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	
	//PORTD &= ~(1<<0); //This port pin is for water valve off
	usart_putchar(98);
	_delay_ms(100);
	usart_putchar(98);
	TFT_Text("Off",171,60,8,white,back_color);
	
	//Updating last status for water
	TFT_PutsInt(100,-1,43,72,8,white,back_color);
	TFT_PutsInt(water,-1,123,72,8,white,back_color);
	TFT_PutsInt(0,4,123,102,8,white,back_color);
	
	i_time = 0;
	while (1) //This while loop is for steam valve open after 1 second
	{
		if (i_time > 0)
		{
			//PORTD |= (1<<1);  //This port pin is for steam valve on
			usart_putchar(99);
			_delay_ms(100);
			usart_putchar(99);
			TFT_Text("On ",171,148,8,white,red);
			break;
		}
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	while (1) //This while loop is for temperature calculation
	{
		_delay_ms(5);
		temp_read = (Read_ADC(0,0x00) * 100UL)/(uint32_t)temp_divider; //This is the divider to adjust the temperature now 4 for experiment
		if (temp_read >= temp)
		{
			break;
		}
		
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)
			{
				usart_data = 0;
				break;
			}
			else usart_data = 0;
		}
		
		else if (!(PIND & (1<<PD3)))  //scan for touch data
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			if (TPval[0]>130 && TPval[0]<165 && TPval[1]>37 && TPval[1]<216)
			{
				break;
			}
		}
		if (compare_time != i_time)
		{
			TFT_PutsInt((temp_read*100)/temp,3,43,160,8,white,back_color);
			TFT_PutsInt(temp_read,3,123,160,8,white,back_color);
			TFT_PutsInt(temp-temp_read,3,123,188,8,white,back_color);
			compare_time = i_time;
		}
		
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	
	//PORTD &= ~(1<<1); //This port pin is for steam valve off
	usart_putchar(100);  //command for steam valve off
	_delay_ms(100);
	usart_putchar(100);
	TFT_Text("Off",171,148,8,white,back_color);
	
	// Updating last status for temperature
	TFT_PutsInt(100,3,43,160,8,white,back_color);
	TFT_PutsInt(temp,3,123,160,8,white,back_color);
	TFT_PutsInt(0,3,123,188,8,white,back_color);	
	
	i_time = 0;
	TFT_Text("Counting",123,236,8,white,red);
	while (1) //This while loop is for time calculation
	{
		time_read = i_time / 60;
		if (time_read >= time)
		{
skip_time:
			//PORTD |= (1<<6); //This port pin is for alarm on
			usart_putchar(101);
			_delay_ms(100);
			usart_putchar(101);
			TFT_Text("Idle    ",123,236,8,white,back_color);
			break;
		}
		TFT_PutsInt(time_read,-1,123,260,8,white,back_color);
		TFT_Text(":",155,260,8,white,back_color);
		TFT_PutsInt(i_time % 60,2,163,260,8,white,back_color);
		TFT_PutsInt(time - time_read,4,123,276,8,white,back_color);
		
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)
			{
				usart_data = 0;
				goto skip_time;
			}
			else usart_data = 0;
		}
		
		if (!(PIND & (1<<PD3)))  //scan touch pad data
		{
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[0]>77 && TPval[0]<103 && TPval[1]>37 && TPval[1]<216)
			{
				goto skip_time;
			}
		}
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	 //this section is to wait for alarm off command
	 i_time = 0;
	TFT_Fill(back_color);
	TFT_Text("ALARMING!",51,84,16,white,back_color);
	TFT_Box(35,150,195,174,white);
	TFT_Text("ALARM OFF",53,155,16,red,white);	
	while (1)
	{
		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)
			{
				usart_data = 0;
				goto alrm_off;
			}
			else usart_data = 0;
		}
		
		if (!(PIND & (1<<PD3)))  //scan for touch pad data
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[0]>120 && TPval[0]<150 && TPval[1]>51 && TPval[1]<201)
			{
alrm_off:
				//PORTD &= ~(1<<6); //This port pin is programmed for alarm Off
				usart_putchar(102);
				_delay_ms(100);
				usart_putchar(102);
				goto start_pgm;
			}
			_delay_ms(100);
		}
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	//End progress page
	/*
	*******************************************
	**  Start settings and Calibration part  **
	*******************************************
	*/
settings_calibration:

	TFT_Fill(back_color);
	TFT_Text("SETTINGS",64,70,16,white,back_color);
	TFT_Box(40,132,201,156,yellow);
	TFT_Text("Thermal Calibration",45,140,8,blue,yellow);
	TFT_Box(40,182,201,206,yellow);
	TFT_Text("Water Calibration",53,190,8,blue,yellow);
	TFT_Box(40,232,201,256,yellow);
	TFT_Text("<<  BACK  <<",45,240,8,blue,yellow);

	while (1)
	{

		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 13)  // option a for thermal calibration
			{
				usart_data = 0;
				goto thermal_calib;
			}
			else if (usart_data == 14)  // option b for water flow calibration
			{
				usart_data = 0;
				break;
			}
			else if (usart_data == 11 || usart_data == 15)
			{
				usart_data = 0;
				goto start_pgm;
			}
			else usart_data = 0;
		}
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[1]>45 && TPval[1]<215)
			{
				if (TPval[0]>130 && TPval[0]<160)  //Thermal calibration button
				{
thermal_calib:
					input_f = 7;
					goto button_operation;
				}
				else if (TPval[0]>97 && TPval[0]<122)  // water calibration button
				{
					break;
				}
				else if (TPval[0]>61 && TPval[0]<88)  // back button
				{
					goto start_pgm;
				}
				_delay_ms(100);
			}
		}
		
	}
	
	TFT_Fill(back_color);
	TFT_Text("Water Calibration",53,20,8,white,back_color);
	TFT_Text("Pulse Count: 0",64,80,8,white,back_color);
	TFT_Box(40,132,201,156,yellow);
	TFT_Text("    Load Water",45,140,8,blue,yellow);
	
	while (!(PIND & (1<<PD3)));  // to eliminate the unwanted button press
	while (1)
	{

		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)  // press Ok to load water
			{
				usart_data = 0;
				break;
			}
			else usart_data = 0;
		}
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[1]>45 && TPval[1]<215)
			{
				if (TPval[0]>130 && TPval[0]<160)  //load water
				{
					break;
				}
				_delay_ms(100);
			}
		}
		
	}

	i_water = 0;
	i_water_hold =0;
	usart_putchar(97); //water valve on
	_delay_ms(100);
	usart_putchar(97);
	TFT_Box(40,132,201,156,yellow);
	TFT_Text("   Stop Loading",45,140,8,blue,yellow);
	
	while (!(PIND & (1<<PD3)));  // to eliminate the unwanted button press
	while (1)
	{

		if (usart_dthit())  //scan for keypad data via usart
		{
			usart_data = usart_getchar();
			if (usart_data == 12)  // press Ok to load water
			{
				usart_data = 0;
				goto water_calib;
			}
			else usart_data = 0;
		}
		
		TFT_PutsInt(i_water,-1,168,80,8,white,back_color);
		
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[1]>45 && TPval[1]<215)
			{
				if (TPval[0]>130 && TPval[0]<160)  //load water
				{
water_calib:
					usart_putchar(98); //water valve off
					_delay_ms(100);
					usart_putchar(98);
					_delay_ms(500);
					i_water_hold = i_water;
					input_f = 9;
					goto button_operation;
				}
				_delay_ms(100);
			}
		}
		
	}
	
	/////////////
}