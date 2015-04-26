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
#include "Widget.h"

#define DEBOUNCE 100

/**
 * This class is a subclass of TFT to represent the Canvas. The canvas is
 * responsible for organizing the widgets and refreshing the contents of the
 * screen.
 */
class Canvas
{

public:
	Canvas();
	virtual ~Canvas();
	//Widget *m_Widget;

	void add(Widget* widget, int x, int y);
	Point getTouchedPoint();
	void init();
	void init(int mode);
	void remove(Widget* widget);
	void portrait();
	Widget* pop();
	void landscape();
	void touchWidgets(Point* p);
	void scan();
	void setDebounce(unsigned int d);
	void updateTouch(Point* p);
	//char getWidgetsQty();
	void redraw();
	
	Point* touchedPoint;
	TouchScreen ts = TouchScreen(XP, YP, XM, YM);

private:
	unsigned long lastMillis;
	unsigned int debounceTime;
	/**
	 * Background Color
	 */
	int bgColor;
	/**
	 * Collection of widgets in the Canvas
	 */
	//std::vector<Widget*> widgets;	
	StackArray<Widget*> widgets;

};
#endif //CANVAS_H
#endif // !defined(EA_207A98B8_9829_4028_BF00_047398821F6C__INCLUDED_)
