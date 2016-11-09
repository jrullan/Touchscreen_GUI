///////////////////////////////////////////////////////////
//  Screen.h
//  Implementation of a Screen
//	This class is meant to encapsulate several widgets
//	so that the application can be divided in "screens".
// 	Each screen will then have to have a collection of
//	widgets to enable/disable when the screen is shown
//	on the canvas.
//
//  Created on:      12-Mar-2015 10:24 PM
///////////////////////////////////////////////////////////

#ifndef SCREEN_H
#define SCREEN_H

#include "Canvas.h"
#include "Widget.h"
#include "Point.h" 

class Screen{
	public:
		//Constructor
		Screen();
		Screen(Canvas* c);
		~Screen();
		
		//Methods
		void init();
		void add(Widget* w,int x, int y);
		void touchWidgets(Point* p);
		void show();
		void setCanvas(Canvas* c);
		
		//Attributes
		StackArray<Widget*> widgets;
		Canvas* canvas;
				
		//int borderWidth;		
		int bgColor;
		int fgColor;
		int borderColor;
		int x;
		int y;		
		int w;
		int h;
		
	private:
};

#endif // SCREEN_H
