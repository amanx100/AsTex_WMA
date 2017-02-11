/*
 * button.h
 *
 * Created: 1/11/2015 1:15:53 AM
 *  Author: Aman
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_

#include <avr/io.h>
#include "SSD1289.h"
//#include "XPT2046.h"

#define text_color black
#define bg_color white

void show_button()
{
	/****************************************************/
	//______Start Button Creation for calculator________//
	//***************************************************//
	/*
	TFT_Box(10,76,60,126,bg_color);
	TFT_Text("1",27,93,16,text_color,bg_color);
	TFT_Box(67,76,117,126,bg_color);
	TFT_Text("2",84,93,16,text_color,bg_color);
	TFT_Box(124,76,174,126,bg_color);
	TFT_Text("3",141,93,16,text_color,bg_color);
	TFT_Box(181,76,231,126,bg_color);
	TFT_Text("/",198,93,16,text_color,bg_color);
	/////////////////////////////////////////////
	*/
	TFT_Box(10,133,60,183,bg_color);
	TFT_Text("1",27,150,16,text_color,bg_color);
	TFT_Box(67,133,117,183,bg_color);
	TFT_Text("2",84,150,16,text_color,bg_color);
	TFT_Box(124,133,174,183,bg_color);
	TFT_Text("3",141,150,16,text_color,bg_color);
	TFT_Box(181,133,231,183,bg_color);
	TFT_Text("4",198,150,16,text_color,bg_color);
	/////////////////////////////////////////////
	TFT_Box(10,190,60,240,bg_color);
	TFT_Text("5",27,207,16,text_color,bg_color);
	TFT_Box(67,190,117,240,bg_color);
	TFT_Text("6",84,207,16,text_color,bg_color);
	TFT_Box(124,190,174,240,bg_color);
	TFT_Text("7",141,207,16,text_color,bg_color);
	TFT_Box(181,190,231,240,bg_color);
	TFT_Text("8",198,207,16,text_color,bg_color);
	/////////////////////////////////////////////
	TFT_Box(10,247,60,297,bg_color);
	TFT_Text("9",27,264,16,text_color,bg_color);
	TFT_Box(67,247,117,297,bg_color);
	TFT_Text("0",84,264,16,text_color,bg_color);
	TFT_Box(124,247,174,297,bg_color);
	TFT_Text("DEL",128,264,16,text_color,bg_color);
	TFT_Box(181,247,231,297,bg_color);
	TFT_Text("OK",191,264,16,text_color,bg_color);
	/*********************************************/
	//_________End Button Creatoin_______________//
	/*********************************************/
}


#endif /* BUTTON_H_ */