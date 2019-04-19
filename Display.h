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
	Display(unsigned char textLength = 8);
	Display(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor,unsigned char textLength = 8);
	virtual ~Display();

	// Attributes	
	Text* contents;		

	// Methods
	void init();
	void drawFrame();
	void drawText(int color);
	void fitToText();
	//void setNum(int num, bool force = false); -- See Text.cpp --
	void setText(char* _text, bool force = false);
	void append(char* c);
	
	//new testing out...
	void append(char c);
	
	void deleteChar();
	
	//Overriden methods
	virtual bool checkTouch(Point* p);
	virtual void show();
	virtual void update();

private:
	char charPos;
};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
