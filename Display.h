///////////////////////////////////////////////////////////
//  Display.h
//  Implementation of the Class Display
//  Created on:      11-Mar-2015 9:12 AM
///////////////////////////////////////////////////////////

#ifndef DISPLAY_H
#define DISPLAY_H


#define DISPLAY_SIZE 8

#include "Canvas.h"
#include "Widget.h"

class Display : public Widget
{

public:
	Display();
	Display(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor);
	virtual ~Display();
	
	void append(char* c);
	void drawFrame();
	void fitToText();	
	long getNum();
	char* getText();
	unsigned char getTextSize();
	void init();
	//void setText(String _text);
	void setNum(int num);
	void setText(char* _text);
	void deleteChar();
	void clear();
  
	//Overriden methods
	virtual bool checkTouch(Point* p);
	virtual bool isButton();
	virtual void show();
	virtual void update();
	
private:
	//String text;
	char* text; 
	//char text[DISPLAY_SIZE];
	char charPos;
	
	unsigned char getTextLength(char* c);

};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
