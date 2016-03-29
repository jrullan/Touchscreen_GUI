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

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif

#define BUF_SIZE 8

#ifndef MAX_TREND_VALUES
#define MAX_TREND_VALUES 8
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
		
		//Overriden methods
		void show();
		void update();
		
		//Attributes
		uint8_t* values;  // Array of values to plot
		uint8_t vals;			// number of values in the array
		bool forcedUpdate = false;

	private:
		//Methods
		void drawScale();
		void drawValues();
};

#endif // TREND_H
