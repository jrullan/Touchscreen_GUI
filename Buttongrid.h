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

#define BUTTONGRID_HIGHLIGHT 	0b0001
#define BUTTONGRID_MAINTAINED 	0b0010
#define BUTTONGRID_USELABELS	0b0100
#define BUTTONGRID_NUMLABELS    0b1000

#include "Canvas.h"
#include "Button.h"

class Buttongrid : public Button
{

public:
	Buttongrid(unsigned char r, unsigned char c, unsigned char configuration);
	//Buttongrid(unsigned char gridSize);
	virtual ~Buttongrid();

	// Attributes
	byte gridSize = 2;
	byte columns;
	byte rows;
	byte hGap = 0;
	byte vGap = 0;
	unsigned char config = 0;
	int btnWidth,btnHeight;
	unsigned char lastPressed = 0; //Last pressed button ID (number)
	char** labels;
	unsigned char* states;
	int highlightColor = YELLOW;
	unsigned char maintained = 1; // used to decide if to redraw as a maintained or as a momentary
	
	// Methods	
	void init();
	void setSize(int width, int height);
	void configure(byte gridSize);
	void setLabel(unsigned char id,  char name[8]);
	unsigned char getColumn(unsigned char number);
	unsigned char getRow(unsigned char number);
	unsigned char getId(unsigned char row, unsigned char col);
	void drawGrid();
	void drawLabel(unsigned char id);
	void clear();
	// This is a modified declaration for setting the event handler function
	// See comment below in the eventHandler declaration.
	void setEventHandler(void (*functionPointer)(Buttongrid*, unsigned char));
	
	//Overriden methods
	virtual bool checkTouch(Point* p);
	virtual void show();
	virtual void update();
	
private:
	//String text;
	char* text; 
	char charPos;
	// Modified the special declaration of eventHandler to accept
	// a second argument which would be the value of the button pressed
	// This is set in the checkTouch method.
	void (*eventHandler)(Buttongrid*, unsigned char);

};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
