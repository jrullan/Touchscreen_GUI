///////////////////////////////////////////////////////////
//  Gauge.h
//  Implementation of the Class Gauge
//	The Gauge class implements the Gauge widget 
//	inheriting from the Indicator class instead of the Widget. 
//	The Indicator class is an abstract Widget which serves to
//  provide common functionality for all indicator widgets
//	(dial, gauge, etc). 
//
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef GAUGE_H
#define GAUGE_H

#include "Canvas.h"
#include "Indicator.h"

class Gauge : public Indicator{
	public:
		//Constructor
		Gauge();
		Gauge(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit);
		~Gauge();
		
		//Methods
		void init();
		void clear();
		void drawBorder();
		void drawFill();
		int getYVal(int value);
		
		//Overriden methods
		void show();
		void update();
		
		//Attributes
		byte yScaleWidth = 3 * FONT_X + 10; // four characters and a line of 10 pixels
		
	private:
		//Methods
		void drawLimits();
		void drawScale();
};

#endif // GAUGE_H
