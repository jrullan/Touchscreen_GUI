///////////////////////////////////////////////////////////
//  Canvas.h
//  Implementation of the Class Canvas
//  Created on:      02-Mar-2015 9:16:46 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#if !defined(EA_207A98B8_9829_4028_BF00_047398821F6C__INCLUDED_)
#define EA_207A98B8_9829_4028_BF00_047398821F6C__INCLUDED_

#ifndef CANVAS_H
#define CANVAS_H

#if defined(ARDUINO) && ARDUINO >= 100
#define SEEEDUINO
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <avr/pgmspace.h>

// Required includes: StackArray for container
#include "StackArray.h"
#include "Point.h"
#include "SeeedTouchScreen.h"
#include "TFTv2.h"

#define DEBOUNCE 0

//#include "Widget.h"
// Forward declaration of class Widget to solve issue with mutual 
// includes
class Widget;

class Canvas
{

public:
	Canvas();
	virtual ~Canvas();

	void add(Widget* widget, int x, int y);
	Point getTouchedPoint();
	void init();
	void init(int mode);
	//void remove(Widget* widget);
	void portrait();
	Widget* pop();
	void landscape();
	void touchWidgets(Point* p);
	void scan();
	void setDebounce(unsigned int d);
	void updateTouch(Point* p);
	void redraw();
	
	Point* touchedPoint;
	TouchScreen ts = TouchScreen(XP, YP, XM, YM);
	StackArray<Widget*> widgets;

private:
	unsigned long lastMillis;
	unsigned int debounceTime;
	int bgColor;
};

#endif //CANVAS_H
#endif // !defined(EA_207A98B8_9829_4028_BF00_047398821F6C__INCLUDED_)
