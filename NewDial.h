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
		void show();
		void update();
		
		//Attributes
		unsigned char gap;
		int minDegree,maxDegree;
		unsigned char radius;
		unsigned char tickDegree;
		unsigned char tickSize;
		bool showVal;
		bool showTicks;
		
	private:
		//Methods
		void drawBorder();
		void drawFace();
		void drawNeedle(int cX, int cY, int degree, int radius, int color);
		int getX(int cX, int deg, int radius);
		int getY(int cY, int deg, int radius);
		
		//Attributes
};

#endif // NEWDIAL_H
