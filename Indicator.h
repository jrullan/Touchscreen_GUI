///////////////////////////////////////////////////////////
//  Indicator.h
//  Implementation of the Class Indicator
//	This class is meant to be the parent class
//	of the Dial and the Gauge classes by
//	refactoring common attributes and methods.
//
//  Created on:      05-Apr-2015 11:15 PM
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
#include "Widget.h"

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
		void setNum(int num);
		
		//Overriden methods
		bool checkTouch(Point* p);
		bool isButton();		
		virtual void update()=0;
		virtual void show()=0;
		
		//Attributes
		unsigned int currentValue,previousValue;
		unsigned int hiLimit;
		int hiLimitColor;		
		unsigned int lowLimit;
		int lowLimitColor;
		unsigned int scaleMax;
		unsigned int scaleMin;
		unsigned int setpoint;
		//bool isButton = false;

	protected:
		char* buf;
		
	private:
		//Methods
		
		//Attributes

};

#endif // INDICATOR_H
