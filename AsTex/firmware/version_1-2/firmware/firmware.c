#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "SSD1289.h"
#include "XPT2046.h"
#include "adc.h"
#include "button.h"

uint32_t i_water=0, i_time=0, compare_time=0;

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
	uint8_t input_f=0;
	
	PORTA=0x00;
	PORTB=0x40;
	PORTC=0x00;
	PORTD=0x00; //0x0C;
	
	DDRA=0xFE;
	DDRB=0xBF;
	DDRC=0xFF;
	DDRD=0xF3;
	
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
	//Welcome message and startup
	TFT_Fill(back_color);
	//TFT_Text("Mark Wash & Dyeing Ltd.",23,30,8,yellow,back_color);
	TFT_Text("ASTEX",75,84,16,white,back_color);
	TFT_Box(35,150,195,174,yellow);
	TFT_Text("Start Task",45,155,16,blue,yellow);
	TFT_Text("Temperature:",96,5,8,white,blue);
	TFT_Image(180,265,50,50,image_data_astex_logo);
	TFT_Rectangle(179,264,230,315,white);

	while (1)
	{
		if (compare_time != i_time)
		{
			TFT_PutsInt(Read_ADC(0,0x00),4,200,5,8,blue,white);
			compare_time = i_time;
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
	while (!(PIND & (1<<PD3)));  // to eliminate the unwanted button press
	while (1)
	{
		if (!(PIND & (1<<PD3)))
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
					water = 70;
					temp = 25;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>175 && TPval[0]<190)
				{
					water = 70;
					temp = 50;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>148 && TPval[0]<167)
				{
					water = 50;
					temp = 50;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>127 && TPval[0]<144)
				{
					water = 60;
					temp = 50;
					time = 2;
					goto confirmation;
				}
				else if (TPval[0]>83 && TPval[0]<116)  //button for manual input
				{
					break;
				}
			}
			_delay_ms(100);
		}
	}
		//for manual input show the button
	TFT_Fill(back_color);
	show_button();
	input_f = 1;
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
		
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[0]>36 && TPval[0]<72)  // 9 0 DEL >> 
			{
				if (TPval[1]>23 && TPval[1]<73)
				{
					TFT_Text("9",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 9;
				}
				else if (TPval[1]>77 && TPval[1]<126)
				{
					TFT_Text("0",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10);
				}
				else if (TPval[1]>129 && TPval[1]<181)
				{
					mxpos-=14;
					TFT_Text(" ",mxpos,77,16,black,white);
					kp_val = (kp_val / 10);
				}
				else if (TPval[1]>183 && TPval[1]<236)
				{
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
				}
				while (!(PIND & (1<<PD3)));
			}
			else if (TPval[0]>75 && TPval[0]<112)  // 5 6 7 8 
			{
				if (TPval[1]>23 && TPval[1]<73)
				{
					TFT_Text("5",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 5;
				}
				else if (TPval[1]>77 && TPval[1]<126)
				{
					TFT_Text("6",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 6;
				}
				else if (TPval[1]>129 && TPval[1]<181)
				{
					TFT_Text("7",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 7;
				}
				else if (TPval[1]>183 && TPval[1]<236)
				{
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
					TFT_Text("1",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 1;
				}
				else if (TPval[1]>77 && TPval[1]<126)
				{
					TFT_Text("2",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 2;
				}
				else if (TPval[1]>129 && TPval[1]<181)
				{
					TFT_Text("3",mxpos,77,16,black,white);
					mxpos+=14;
					kp_val = (kp_val * 10) + 3;
				}
				else if (TPval[1]>183 && TPval[1]<236)
				{
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
	TFT_Text("Water :      Liture",20,50,8,white,back_color);
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
			PORTD |= (1<<0);  //This port pin is for water valve on
			TFT_Text("On ",171,60,8,white,red);
			break;
		}
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	
	while (1) //This while loop is for water calculation
	{
		water_read = (i_water*10)/33; //This is the assumption to calculate water 1 liter = 2 rotation of propeller
		if (water_read >= water)
		{
			break;
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
	
	PORTD &= ~(1<<0); //This port pin is for water valve off
	TFT_Text("Off",171,60,8,white,back_color);
	
	//Updating last status for water
	TFT_PutsInt((water_read*100)/water,-1,43,72,8,white,back_color);
	TFT_PutsInt(water_read,-1,123,72,8,white,back_color);
	TFT_PutsInt(water-water_read,4,123,102,8,white,back_color);
	
	i_time = 0;
	while (1) //This while loop is for steam valve open after 1 second
	{
		if (i_time > 0)
		{
			PORTD |= (1<<1);  //This port pin is for steam valve on
			TFT_Text("On ",171,148,8,white,red);
			break;
		}
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	while (1) //This while loop is for temperature calculation
	{
		_delay_ms(20);
		temp_read = (Read_ADC(0,0x00)*6)/5; //This is the divider to adjust the temperature now 4 for experiment
		if (temp_read >= temp)
		{
			break;
		}
		else if (!(PIND & (1<<PD3)))
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
	
	PORTD &= ~(1<<1); //This port pin is for steam valve off
	TFT_Text("Off",171,148,8,white,back_color);
	
	// Updating last status for temperature
	TFT_PutsInt((temp_read*100)/temp,3,43,160,8,white,back_color);
	TFT_PutsInt(temp_read,3,123,160,8,white,back_color);
	TFT_PutsInt(temp-temp_read,3,123,188,8,white,back_color);	
	
	i_time = 0;
	TFT_Text("Counting",123,236,8,white,red);
	while (1) //This while loop is for time calculation
	{
		time_read = i_time / 60;
		if (time_read >= time)
		{
skip_time:
			PORTD |= (1<<6); //This port pin is for alarm on
			TFT_Text("Idle    ",123,236,8,white,back_color);
			break;
		}
		TFT_PutsInt(time_read,-1,123,260,8,white,back_color);
		TFT_Text(":",155,260,8,white,back_color);
		TFT_PutsInt(i_time % 60,2,163,260,8,white,back_color);
		TFT_PutsInt(time - time_read,4,123,276,8,white,back_color);
		if (!(PIND & (1<<PD3)))
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
		if (!(PIND & (1<<PD3)))
		{
			Read_TPval(TPval);
			_delay_ms(10);
			Read_TPval(TPval);
			//TFT_PutsInt(TPval[0],4,0,0,8,blue,white);
			//TFT_PutsInt(TPval[1],4,40,0,8,blue,white);
			if (TPval[0]>120 && TPval[0]<150 && TPval[1]>51 && TPval[1]<201)
			{
				PORTD &= ~(1<<6); //This port pin is programmed for alarm Off
				goto start_pgm;
			}
			_delay_ms(100);
		}
		TFT_PutsInt(i_time,4,5,5,8,blue,white);
	}
	
	//End progress page

}