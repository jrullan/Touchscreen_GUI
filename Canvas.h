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

//#pragma GCC optimize ("-O2")

#if defined(ARDUINO) && ARDUINO >= 100
	#define SEEEDUINO
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif
#if defined (__AVR__) //|| defined(TEENSYDUINO) || defined(ESP8266) || defined (ESP32) || defined(__arm__)
	#include <avr/pgmspace.h>
#endif

#define DEBOUNCE 0
#define TOUCH_SAMPLING_TIME 0
#define SCAN_SAMPLING_TIME 20


// Required includes:
#include "StackArray.h"
#include "Point.h"
#include "GUI_TFT.h"
#include "Touch.h"

// Forward declarations
class Widget;
class Screen;
class TouchMapper;

class Canvas
{
public:

	// Constructors and Destructor
	Canvas(int mode, int color, int tft_cs = -1, int tft_ds = -1, int ts_cs = -1);
	virtual ~Canvas();
	
	// Methods
	void portrait();
	void landscape();
	void portrait2();
	void landscape2();
	void add(Widget* widget, int x, int y, uint8_t show = true);
	Widget* pop();
	void showWidgets();
	bool touchWidgets(Point* p);
	bool scan();
	void redraw();
	void redrawRegion(int rx, int ry, int rw, int rh);
	void setScanSampling(unsigned int d);
	void setScreen(Screen* screen, uint8_t show = true);
	void updateTouch(Point* p);
	bool inBounds(Point* p);
	
	// Converted this class to abstract by making these
	// methods virtual. 
	virtual	void init()=0;
	virtual Point* getTouchedPoint()=0;
	
	// Attributes
	int x;
	int y;
	int w;
	int h;
	int bgColor;
	int touchType;
	int ts_cs;
	Guitft* tft;
	Touch* ts;
	Point touchedPoint;	
	Screen* currentScreen = NULL;
	StackArray<Widget*> widgets;
	unsigned int _mode;	
	
	TouchMapper* _mapper = NULL;

protected:
	unsigned long lastMillis;
	unsigned long touchSampling;
	unsigned int scanSampling;
	unsigned int scanSampleDelay;
};

#endif //CANVAS_H
#endif // !defined(EA_207A98B8_9829_4028_BF00_047398821F6C__INCLUDED_)
