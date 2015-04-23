///////////////////////////////////////////////////////////
//  NewDial.h
//  Implementation of the Class NewDial
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef NEWDIAL_H
#define NEWDIAL_H

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif
#define BUF_SIZE 8

#include "Canvas.h"
#include "Indicator.h"

class NewDial : public Indicator{
	public:
		//Constructor
		NewDial();
		NewDial(unsigned int radius, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit);
		~NewDial();
		
		//Methods
		void init();
		void clear();
		void setSize(int radius);

		/*
		unsigned int getCV();
		void setCV(int cv);
		void setLimits(unsigned int min, unsigned int sp, unsigned int max);
		void setHiLimit(unsigned int h, int color);
		void setLowLimit(unsigned int l, int color);
		*/

		//Overriden methods
		/*
		bool checkTouch(Point* p);
		bool isButton();
		*/
		void show();
		void update();
		
		//Attributes
		
		//unsigned int currentValue,previousValue;
		unsigned char gap;
		//unsigned int hiLimit;
		//unsigned int lowLimit;
		//int hiLimitColor;
		//int lowLimitColor;
		int minDegree,maxDegree;
		unsigned char radius;
		//unsigned int scaleMin;//minLimit;
		//unsigned int scaleMax;//maxLimit;
		//unsigned int setpoint;
		unsigned char tickDegree;
		unsigned char tickSize;
		bool showVal;
		bool showTicks;
		
	private:
		//Methods
		//void setNum(int num);
		void drawBorder();
		void drawFace();
		void drawNeedle(int cX, int cY, int degree, int radius, int color);
		int getX(int cX, int deg, int radius);
		int getY(int cY, int deg, int radius);
		
		//Attributes
		//char* buf;
};

#endif // NEWDIAL_H
