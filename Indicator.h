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

#define BUF_SIZE 8

#include "Canvas.h"
#include "Widget.h"

class Indicator : public Widget{
	public:
		// Constructor
		Indicator();
		~Indicator();
		
		// Attributes
		unsigned int hiLimit;
		unsigned int lowLimit;
		unsigned int setpoint;
		unsigned int scaleMin;
		unsigned int scaleMax;
		unsigned int currentValue;
		unsigned int previousValue;

		int hiLimitColor;
		int lowLimitColor;	
		int setpointColor;			
		
		// Methods
		void init();
		void clear();		
		void setCV(int cv);
		void setHiLimit(unsigned int h, int color);		
		void setLimits(unsigned int min, unsigned int sp, unsigned int max);
		void setLowLimit(unsigned int l, int color);
		void setSP(unsigned int sp, int color);
		void setNum(int num);
		unsigned int getCV();

		// Overriden methods
		bool checkTouch(Point* p);
		bool isButton();		
		virtual void update()=0;
		virtual void show()=0;
		
	protected:
		char* buf;
		
	private:
		//Methods
		
		//Attributes

};

#endif // INDICATOR_H
