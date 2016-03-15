///////////////////////////////////////////////////////////
//  Buttongrid.h
//  Implementation of the Class Buttongrid
//  This class is similar to Numkey. The only difference
//  is that this class is more abstract and flexible
//  and provides a grid of A x B buttons. This class could
//	become Numkey parent class, as numkey is a special case
//	of this concept of buttongrid.
//
//  Created on:      11-Mar-2015 9:12 AM
///////////////////////////////////////////////////////////

#ifndef BUTTONGRID_H
#define BUTTONGRID_H

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_X
#define FONT_X 8
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif
#define DISPLAY_SIZE 6

#include "Canvas.h"
#include "Button.h"

class Buttongrid : public Button
{

public:
	Buttongrid();
	Buttongrid(unsigned char rows, unsigned char columns);
	Buttongrid(unsigned char gridSize);
	Buttongrid(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor);
	virtual ~Buttongrid();

	void drawGrid();
	void init();
	// This is a modified declaration for setting the event handler function
	// See comment below in the eventHandler declaration.
	void setEventHandler(void (*functionPointer)(Buttongrid*, unsigned char));
	void configure(byte gridSize, byte fontSize);
	void setNum(unsigned char num);
	void setLabel(unsigned char num, unsigned char label);
	void clear();
	unsigned char getColumn(unsigned char number);
	unsigned char getRow(unsigned char number);
	unsigned char getNumber(unsigned char row, unsigned char col);
    
	//Overriden methods
	virtual bool checkTouch(Point* p);
	virtual void show();
	virtual void update();
	
	// Attributes
	//bool autoremove; //Forces autoremove from Canvas when = is pressed.
	byte gridSize = 2;
	byte columns;
	byte rows;
	byte font_size = 2;
	unsigned char lastPressed = 0; //Last pressed button ID (number)
	unsigned char** labels = 0;
	
private:
	//String text;
	char* text; 
	char charPos;
	byte getTextLength(char* c);
	// Modified the special declaration of eventHandler to accept
	// a second argument which would be the value of the button pressed
	// This is set in the checkTouch method.
	void (*eventHandler)(Buttongrid*, unsigned char);

};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
