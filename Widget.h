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

#if defined(ARDUINO) && ARDUINO >= 100
#define SEEEDUINO
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <avr/pgmspace.h>
#include <TFTv2.h>


#ifndef INT8U
#define INT8U unsigned char
#endif

#ifndef INT16U
#define INT16U unsigned short
#endif

extern INT8U simpleFont[][8];
//extern TFT Tft;

//#include "./SeeedTouchScreen/SeeedTouchScreen.h"
//#include <SeeedTouchScreen.h>
#include "Point.h"
#include "SeeedTouchScreen.h"
//#include "Canvas.h"
//class Canvas;
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
	
	int bgColor;
	int borderColor;
	int borderWidth;
	int fgColor;
	int h;
	int w;
	int x;
	int y;

	//template <typename T> void update(T* c);
	void hide();
	void setColors(int bgColor,int fgColor, int borderColor);
	void setSize(int width, int height);
	
	//Pure Virtual methods
	virtual bool checkTouch(Point* p)=0;
	virtual bool isButton() =0;
	virtual void show() =0;
	virtual void update() =0;
	
private:

	
};
#endif // WIDGET_H
#endif // !defined(EA_B6B323D4_C08E_4473_BE8D_AEA9F30B3DA6__INCLUDED_)
