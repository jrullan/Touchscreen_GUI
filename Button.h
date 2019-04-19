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
#define LABEL_LEFT		0
#define LABEL_TOP			1
#define LABEL_RIGHT		2
#define LABEL_BOTTOM	3


class Button : public Widget
{

public:

	//Button(unsigned char textLength = 8);
	Button();
	Button(char* text, char* label = 0);
	Button(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor, unsigned char textLength = 8);
	Button(unsigned int radius, int backgroundColor, int textColor, int borderColor);
	virtual ~Button();

	void init();	
	void drawBackground(int color);
	void drawBorder();
	void drawLabel();
	void drawText();
	void setEventHandler(void (*functionPointer)(Button *));
	void fitToText();
	//void setNum(int n); -- See notes in Text.cpp --
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
	Text* contents;		// changed from a Text object to a pointer: 6 bytes less in Program Memory and 4 less bytes in RAM
	char* label = 0;	// had to initilize to zero to avoid trying to get it in getLabelSize if not initialized
	char labelPos = LABEL_LEFT; //label position: 0-left, 1-top, 2-right, 3-bottom
	bool touched;
	bool block = false;
	uint8_t cornerRadius = 0;
	unsigned long lastMillis;
	
protected:	
	unsigned int debounceTime = DEBOUNCE;	
	bool isRound;
};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
