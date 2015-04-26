///////////////////////////////////////////////////////////
//  NewGauge.h
//  Implementation of the Class NewGauge
//	The NewGauge class implements the Gauge widget but 
//	inheriting from the Indicator class instead of the Widget. 
//	The Indicator class is an abstract Widget which serves to
//  provide common functionality for all indicator widgets
//	(dial, gauge, etc). 
//
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef NEWGAUGE_H
#define NEWGAUGE_H

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif
#define BUF_SIZE 8

#include "Canvas.h"
#include "Indicator.h"

class NewGauge : public Indicator{
	public:
		//Constructor
		NewGauge();
		NewGauge(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit);
		~NewGauge();
		
		//Methods
		void init();
		void clear();
		
		//Overriden methods
		void show();
		void update();
		
		//Attributes

	private:
		//Methods
		void drawLimits();
		void drawScale();
};

#endif // NEWGAUGE_H
