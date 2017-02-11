/*
 * keypad.h
 *
 * Created: 3/11/2015 1:38:45 AM
 *  Author: Aman
 */ 

#ifndef KEYPAD_H_
#define KEYPAD_H_

	#define KEYPAD_PORT PORTC //User configurable
	#define KEYPAD_DDR DDRC //User configurable
	#define KEYPAD_PIN PINC //User configurable
	
	void keypad_init(void)
	{
		KEYPAD_PORT = 0xFF;
		KEYPAD_DDR = 0xF0;
	}

	uint16_t scan_kpd_4x4(void)
	{
		KEYPAD_PORT = 0xEF;    //start first column
		_delay_ms(1);
		if (!(KEYPAD_PIN & 0x01)) return 1;
		else if (!(KEYPAD_PIN & 0x02)) return 4;
		else if (!(KEYPAD_PIN & 0x04)) return 7;
		else if (!(KEYPAD_PIN & 0x08)) return 11;
		else
		{
			KEYPAD_PORT = 0xDF;    //start second column
			_delay_ms(1);
			if (!(KEYPAD_PIN & 0x01)) return 2;
			else if (!(KEYPAD_PIN & 0x02)) return 5;
			else if (!(KEYPAD_PIN & 0x04)) return 8;
			else if (!(KEYPAD_PIN & 0x08)) return 10;
			else 
			{
				KEYPAD_PORT = 0xBF;   //start third column
				_delay_ms(1);
				if (!(KEYPAD_PIN & 0x01)) return 3;
				else if (!(KEYPAD_PIN & 0x02)) return 6;
				else if (!(KEYPAD_PIN & 0x04)) return 9;
				else if (!(KEYPAD_PIN & 0x08)) return 12;
				else
				{
					KEYPAD_PORT = 0x7F;   //start fourth column
					_delay_ms(1);
					if (!(KEYPAD_PIN & 0x01)) return 13;
					else if (!(KEYPAD_PIN & 0x02)) return 14;
					else if (!(KEYPAD_PIN & 0x04)) return 15;
					else if (!(KEYPAD_PIN & 0x08)) return 16;
					else
					{
						KEYPAD_PORT = 0xFF;
						return 0;
					}
				}
			}
		}
	}


#endif /* KEYPAD_H_ */