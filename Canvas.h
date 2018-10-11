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
#if defined (__AVR__) //|| defined(TEENSYDUINO) || defined(ESP8266) || defined (ESP32) || defined(__arm__)
	#include <avr/pgmspace.h>
#endif

#if defined(__STM32F1__)
	#define YP 17//A2   // must be an analog pin, use "An" notation!
	#define XM 16//A1   // must be an analog pin, use "An" notation!
	#define YM 15//A0   // can be a digital pin, this is A0
	#define XP 18//A3   // can be a digital pin, this is A3
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	#define YP A2   // must be an analog pin, use "An" notation!
	#define XM A1   // must be an analog pin, use "An" notation!
	#define YM 54   // can be a digital pin, this is A0
	#define XP 57   // can be a digital pin, this is A3
#elif defined(MQX_CPU) //Added support for Udoo Neo MCU!!!! Yay!!!
	#define YP A2   // must be an analog pin, use "An" notation!
	#define XM A1   // must be an analog pin, use "An" notation!
	#define YM A0   // can be a digital pin, this is A0
	#define XP A3   // can be a digital pin, this is A3
#elif defined(__AVR_ATmega32U4__)
	#define YP A2   // must be an analog pin, use "An" notation!
	#define XM A1   // must be an analog pin, use "An" notation!
	#define YM 18   // can be a digital pin, this is A0
	#define XP 21   // can be a digital pin, this is A3
#elif defined(ESP32)// For resistive touchscreens...
	#define YP 34   // must be an analog pin, use "An" notation!
	#define XM 33   // must be an analog pin, use "An" notation!
	#define YM 36   // can be a digital pin, this is A0
	#define XP 35   // can be a digital pin, this is A3
#else 
	#define YP A2   // must be an analog pin, use "An" notation!
	#define XM A1   // must be an analog pin, use "An" notation!
	#define YM 14   // can be a digital pin, this is A0
	#define XP 17   // can be a digital pin, this is A3
#endif

#define DEBOUNCE 0
#define TOUCH_SAMPLING_TIME 25
#define SCAN_SAMPLING_TIME 100
#define TOUCH_BUFFER_SIZE 5


// Required includes:
#include "StackArray.h"
#include "Point.h"
#include "GUI_TFT.h"
#include "Touch.h"

// Forward declaration of class Widget to solve issue with mutual 
// includes
class Widget;
class Screen;

class Canvas
{
public:

	// Constructors and Destructor
	Canvas(int mode, int color, int tft_cs = -1, int tft_ds = -1, int ts_cs = -1);
	virtual ~Canvas();
	
	// Methods
	void portrait();
	void landscape();
	void add(Widget* widget, int x, int y, uint8_t show = true);
	Widget* pop();
	void showWidgets();
	bool touchWidgets(Point* p);
	bool scan();
	void redraw();
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
	int touchBufferIndex = 0;
	int xTouchBuffer[TOUCH_BUFFER_SIZE];
	int yTouchBuffer[TOUCH_BUFFER_SIZE];
	int touchType;
	int ts_cs;
	Guitft* tft;
	Touch* ts;
	Point touchedPoint;	
	Screen* currentScreen = NULL;
	StackArray<Widget*> widgets;
	unsigned int _mode;	
	
protected:
	unsigned long lastMillis;
	unsigned long touchSampling;
	unsigned int scanSampling;
	unsigned int scanSampleDelay;
};

#endif //CANVAS_H
#endif // !defined(EA_207A98B8_9829_4028_BF00_047398821F6C__INCLUDED_)
