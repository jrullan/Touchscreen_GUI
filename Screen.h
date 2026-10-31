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
		Screen(Canvas* c, int _x, int _y, int _w, int _h);
		~Screen();
		Screen(const Screen&) = delete;
		Screen& operator=(const Screen&) = delete;
		
		//Methods
		void init();
		void add(Widget* w,int x, int y);
		bool remove(Widget* widget);
		Widget* pop();
		// Call before this Screen or its Canvas dies if the other remains usable.
		// Registrations are retained; widget drawing contexts become null.
		void detach();
		void touchWidgets(Point* p);
		void hide();
		void show();
		void setCanvas(Canvas* c);
		
		//Attributes
		// Non-owning; remove before destroying a widget while this Screen is usable.
		StackArray<Widget*> widgets;
		Canvas* canvas = NULL;
				
		//int borderWidth;		
		int bgColor = 0;
		int fgColor = 0;
		int borderColor = 0;
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;
		
	private:
};

#endif // SCREEN_H
