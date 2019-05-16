///////////////////////////////////////////////////////////
//  Trend.h
//  Implementation of the Class Trend
//	The Trend class implements the Trend widget 
//	inheriting from the Indicator class instead of the Widget. 
//	The Indicator class is an abstract Widget which serves to
//  provide common functionality for all indicator widgets
//	(dial, gauge, etc). 
//
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef TREND_H
#define TREND_H

#ifndef MAX_TREND_VALUES
#if defined(__STM32F1__) || defined(ESP32)
#define MAX_TREND_VALUES 32
#else
#define MAX_TREND_VALUES 16
#endif
#endif

#include "Canvas.h"
#include "Indicator.h"

class Trend : public Indicator{
	public:
		//Constructor
		Trend();
		Trend(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit);
		~Trend();
		
		//Methods
		void init();
		void clear();
		void addValue(uint8_t val);
		void forceUpdate(){
			forcedUpdate = true;
			update();
			forcedUpdate = false;
		}
		int getXVal(int index);
		int getYVal(int value);
		int getMin();
		int getMax();
		void setMaxX(int maxX);
		void autoFit(bool scale);
		void setWindow(int min, int max);
		
		//Overriden methods
		void show();
		void update();
		
		//Attributes
		uint8_t* values;  // Array of values to plot
		uint8_t vals;			// number of values in the array
		//uint8_t maxValues = MAX_TREND_VALUES;	// To define max values in the trend
		uint8_t maxValues;
		bool forcedUpdate = false;
		bool enableAutoFit = false;
		
		struct TrendWindow {
			int minValue;
			int maxValue;
		} trendWindow;
		
	private:
		//Attributes
		byte updates = 0;
		byte yScaleWidth = 4 * FONT_X + 10; // four characters and a line of 10 pixels
		byte xScaleHeight = 2 * FONT_Y + 10 + borderWidth;
		//int maxX = MAX_TREND_VALUES;
		//Methods
		void drawYScale();
		void drawXScale();
		void drawBorder();
		void drawThresholdLines(bool colors);
		void drawValues(uint16_t color);
};

#endif // TREND_H
