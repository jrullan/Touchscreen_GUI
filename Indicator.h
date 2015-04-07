///////////////////////////////////////////////////////////
//  Indicator.h
//  Implementation of the Class Indicator
//	This class is meant to be the parent class
//	of the Dial and the Gauge classes by
//	refactoring common attributes and methods.
//
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef INDICATOR_H
#define INDICATOR_H

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif
#define BUF_SIZE 8

#include "Canvas.h"

class Indicator : public Widget{
	public:
		//Constructor
		Indicator();
		~Indicator();
		
		//Methods
		void clear();		
		unsigned int getCV();
		void init();
		void setCV(int cv);
		void setHiLimit(unsigned int h, int color);		
		void setLimits(unsigned int min, unsigned int sp, unsigned int max);
		void setLowLimit(unsigned int l, int color);
		
		//Overriden methods
		bool checkTouch(Point* p);
		void show();
		void update();
		bool isButton();
		
		//Attributes
		unsigned int currentValue,previousValue;
		unsigned int hiLimit;
		unsigned int lowLimit;
		int hiLimitColor;
		int lowLimitColor;
		unsigned int scaleMin;
		unsigned int scaleMax;
		unsigned int setpoint;
		//bool isButton = false;

		
	private:
		//Methods
		
		//Attributes
		char* buf;
};

#endif // INDICATOR_H
