///////////////////////////////////////////////////////////
//  Display.h
//  Implementation of the Class Display
//  Created on:      11-Mar-2015 9:12 AM
///////////////////////////////////////////////////////////

#ifndef DISPLAY_H
#define DISPLAY_H

#include "Canvas.h"
#include "Widget.h"
#include "Text.h"

class Display : public Widget
{
	public:
		Display(unsigned char textSize = 8);
		Display(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor,unsigned char textSize = 8);
		virtual ~Display();
	
		void init();
		
		void drawFrame();
		void drawText(int color);
	
		Text contents;//= Text();
		//Text label = Text();
		
		void fitToText();
		void setNum(int num, bool now = true);
		void setText(char* _text, bool now = true);
		
		void append(char* c);
		void deleteChar();
		
		//Overriden methods
		virtual bool checkTouch(Point* p);
		virtual bool isButton();
		virtual void show();
		virtual void update();
		
		//Attributes
	
		
	private:
		//char text[DISPLAY_SIZE];
		char charPos;
		//unsigned char getTextLength(char* c);
};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
