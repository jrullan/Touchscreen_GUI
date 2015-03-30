///////////////////////////////////////////////////////////
//  Display.h
//  Implementation of the Class GAUGE
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef GAUGE_H
#define GAUGE_H

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif
#define BUF_SIZE 8

#include "Canvas.h"

class Gauge : public Widget{
	public:
		//Constructor
		Gauge();
		Gauge(unsigned int width, unsigned int height, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit);
		~Gauge();
		
		//Methods
		void init();
		void clear();
		unsigned int getCV();
		void setCV(int cv);
		void setLimits(unsigned int min, unsigned int sp, unsigned int max);
		void setHiLimit(unsigned int h, int color);
		void setLowLimit(unsigned int l, int color);
		
		//Overriden methods
		bool checkTouch(Point* p);
		bool isButton();
		void show();
		void update();
		
		
		//Attributes
		unsigned int currentValue;
		unsigned int setpoint;
		unsigned int hiLimit;
		unsigned int lowLimit;
		int hiLimitColor;
		int lowLimitColor;
				
	private:
		//Methods
		void drawLimits();
		void drawScale();
		void setNum(int num);
		
		//Attributes
		unsigned int scaleMin;//minLimit;
		unsigned int scaleMax;//maxLimit;
		char* buf;
};

#endif // GAUGE_H