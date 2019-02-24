///////////////////////////////////////////////////////////
//  Widget.h
//  Implementation of the Class Widget
//  Created on:      02-Mar-2015 9:17:00 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#if !defined(EA_B6B323D4_C08E_4473_BE8D_AEA9F30B3DA6__INCLUDED_)
#define EA_B6B323D4_C08E_4473_BE8D_AEA9F30B3DA6__INCLUDED_

#ifndef WIDGET_H
#define WIDGET_H

//#pragma GCC optimize ("-O2")

//Length of characters to use for labels and texts in general
#define DISPLAY_SIZE 6

//Font Dimensions (based on the standard 5x7 font included in Adafruit_GFX
#define FONT_SPACE 6
#define FONT_X 6
#define FONT_Y 8


#include <Arduino.h>

// Forward declarations
class Canvas;
class Point;

/**
 * A widget represents a button, a field box, a drop-down list, a list, etc. Any
 * GUI element that will be part of the GUI. There are input widgets and display
 * widgets. Input widgets are buttons, entry box, selection lists (dropdown and
 * normal). An output widget could be a text display, numeric display, color-
 * changing button or indicator.
 */
class Widget
{

public:
	Widget();
	virtual ~Widget();
	
	char type = 0x00;

	// Attributes
	int x;
	int y;	
	int h;
	int w;	
	int borderWidth;	
	int bgColor;
	int fgColor;	
	int borderColor;
	
	// defaults
	bool visible = true;
	bool isButton = false;
	unsigned char fontSize = 2;

	// Methods
	void hide();
	void setCanvas(Canvas* c);
	void setColors(int bgColor,int fgColor, int borderColor);
	void setSize(int width, int height);
	int getCenterTextX(int xPos, int width, int length){
		return xPos + (width - (length*FONT_X*this->fontSize))/2 + 1;
	};
	int getCenterTextY(int y, int h){
		return y+(h-FONT_Y*this->fontSize)/2 + 1;	
	};
	
	// static methods
	/*
	 * Widgets types:
	 * 
	 * Widget - 		0x00
	 * 	Display - 		0x10
	 * 	Terminal -		0x15
	 * 	Indicator - 	0x20
	 * 		Dial -		0x21
	 * 		Gauge -		0x22
	 * 		Trend -		0x23
	 * 	Button -		0x30
	 * 		Buttongrid-	0x31
	 * 		Numkey -	0x32
	 * 		Popup - 	0x33
	 * 		Option - 	0x34
	 *		IconButton - 0x35
	 */ 
	static char* getType(Widget* w){
		if((w->type & 0xFF) == 0x00){ 
			return "widget";
		}else if ((w->type & 0xFF) == 0x10){
			return "display";
		}else if ((w->type & 0xFF) == 0x15){
			return "terminal";
		}else if ((w->type & 0xFF) == 0x20){
			return "indicator";
		}else if ((w->type & 0xFF) == 0x21){
			return "dial";
		}else if ((w->type & 0xFF) == 0x22){
			return "gauge";
		}else if ((w->type & 0xFF) == 0x23){
			return "trend";
		}else if ((w->type & 0xFF) == 0x30){
			return "button";
		}else if ((w->type & 0xFF) == 0x31){
			return "buttongrid";
		}else if ((w->type & 0xFF) == 0x32){
			return "numkey";
		}else if ((w->type & 0xFF) == 0x33){
			return "popup";
		}else if ((w->type & 0xFF) == 0x34){
			return "option";
		}else if ((w->type & 0xFF) == 0x35){
			return "iconbutton";
		}		
	};
	
	static uint8_t getTextLength(char* c){
		uint8_t size = 0;
		if(*c){
			char* chars = c;
			while(*chars){
				*chars++; // equivalent to *(chars++), returns the value of *chars and then it increments the pointer
				size++;
			}
	  }
	  return size;
	}
	
	// Pure Virtual methods - To force inheritance (makes this an abstract class)
	virtual bool checkTouch(Point* p)=0;
	virtual void show() =0;
	virtual void update() =0;
	
protected:
	Canvas* myCanvas;
	
};
#endif // WIDGET_H
#endif // !defined(EA_B6B323D4_C08E_4473_BE8D_AEA9F30B3DA6__INCLUDED_)
