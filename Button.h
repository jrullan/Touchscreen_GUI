///////////////////////////////////////////////////////////
//  Button.h
//  Implementation of the Class Button
//  Created on:      02-Mar-2015 9:17:12 PM
///////////////////////////////////////////////////////////

#if !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
#define EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_

#include "Canvas.h"
#include "Widget.h"
#include "Text.h"

#define DEBOUNCE 200

class Button : public Widget
{

public:

	Button(unsigned char textLength = 8);
	Button(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor, unsigned char textLength = 8);
	Button(unsigned int radius, int backgroundColor, int textColor, int borderColor);
	virtual ~Button();

	void init();	
	void drawBackground(int color);
	void drawBorder();
	void drawText();
	void setEventHandler(void (*functionPointer)(Button *));
	void fitToText();
	void setNum(int n);
	void setText(char* _text);
	char* getText();
	long getNum();
	void setLabel(char* _label);
	int getLabelSize();
	void setDebounce(unsigned int d);
	
	//Overriden methods
	virtual void show();
	virtual void update();	
	bool checkTouch(Point* p);
	void (*eventHandler)(Button*);
	
	//Attributes
	Text contents;
	char* label;
	char labelPos=0; //label position: 0-left, 1-top, 2-right, 3-bottom
	bool touched;
	bool block = false;
	unsigned long lastMillis;
	
protected:	
	unsigned int debounceTime;	
	bool isRound;
};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
