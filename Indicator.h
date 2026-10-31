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

// Decimal digits, optional sign, and terminator for every supported int width.
#define BUF_SIZE (3 * sizeof(int) + 2)

#include <Arduino.h>
#include "Canvas.h"
#include "Widget.h"

class Indicator : public Widget{
	public:
		// Constructor
		Indicator();
		~Indicator();
		
		// Attributes
		unsigned int hiLimit = 100;
		unsigned int lowLimit = 0;
		unsigned int setpoint = 0;
		unsigned int scaleMin = 0;
		unsigned int scaleMax = 100;
		unsigned int currentValue = 0;
		unsigned int previousValue = 0;

		int hiLimitColor = 0xf800;
		int lowLimitColor = 0x07e0;
		int setpointColor = 0xffff;
		
		// Methods
		void init();
		void clear();		
		void setCV(int cv, boolean show = true);
		void setCV(unsigned int cv, boolean show = true);
		void setHiLimit(unsigned int h, int color);		
		void setLimits(unsigned int min, unsigned int sp, unsigned int max);
		void setLowLimit(unsigned int l, int color);
		void setSP(unsigned int sp, int color);
		void setNum(int num);
		void setNum(unsigned int num);
		unsigned int getCV();

		// Overriden methods
		bool checkTouch(Point* p);	
		virtual void update()=0;
		virtual void show()=0;
		
	protected:
		char* buf;
		int mapScale(int64_t value, int bottom, int top) const;
		void setCVValue(int64_t cv, boolean show);
		
	private:
		//Methods
		
		//Attributes

};

#endif // INDICATOR_H
