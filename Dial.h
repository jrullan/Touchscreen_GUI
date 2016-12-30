///////////////////////////////////////////////////////////
//  Dial.h
//  Implementation of the Class Dial
//	The Dial class implements the Dial widget 
//	inheriting from the Indicator class instead of the Widget. 
//	The Indicator class is an abstract Widget which serves to
//  provide common functionality for all indicator widgets
//	(dial, gauge, etc). 
//	
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef DIAL_H
#define DIAL_H

#include "Canvas.h"
#include "Indicator.h"

class Dial : public Indicator{
	public:
		//Constructor
		Dial();
		Dial(unsigned int radius, unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit);
		~Dial();
		
		//Methods
		void init();
		void clear();
		void setSize(int radius);
		void show();
		void update();
		void forceUpdate(){
			forcedUpdate = true;
			update();
			forcedUpdate = false;
		}
		
		//Attributes
		unsigned char gap;
		int minDegree,maxDegree;
		unsigned char radius;
		unsigned char tickDegree;
		unsigned char tickSize;
		bool showVal;
		bool showTicks;
		bool forcedUpdate = false;
		
	private:
		//Methods
		void drawBorder();
		void drawFace();
		void drawNeedle(int cX, int cY, int degree, int radius, int color);
		int getX(int cX, int deg, int radius);
		int getY(int cY, int deg, int radius);
		
		//Attributes
};

#endif // DIAL_H
