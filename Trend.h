///////////////////////////////////////////////////////////
//  Trend.h
//  Implementation of the Class Trend
//	The Trend class implements a single-series trend widget
//	inheriting from TrendBase (shared chrome/geometry) which in
//	turn inherits from Indicator. The Indicator class is an
//	abstract Widget which serves to provide common functionality
//	for all indicator widgets (dial, gauge, etc).
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

#include "TrendBase.h"

class Trend : public TrendBase{
	public:
		//Constructor
		Trend();
		Trend(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit, uint8_t max_trend_values=MAX_TREND_VALUES);
		~Trend();
		
		//Methods
		void init();
		void clear();
		void addValue(uint8_t val, bool updateTrend=true);
		void drawTrend();
		void eraseTrend();
		void forceUpdate(){
			forcedUpdate = true;
			update();
			forcedUpdate = false;
		}
		int getMin();
		int getMax();
		void autoFit(bool scale);
		
		//Overriden methods
		void show();
		void update();
		
		//Attributes
		uint8_t* values = nullptr;  // Array of values to plot
		uint8_t vals = 0;     // number of values in the array
		bool forcedUpdate = false;
		
	private:
		void fitScale(bool scale);
		//Methods
		void drawValues(uint16_t color);
};

#endif // TREND_H
