///////////////////////////////////////////////////////////
//  Button.h
//  Implementation of the Class Button
//  Created on:      02-Mar-2015 9:17:12 PM
///////////////////////////////////////////////////////////

#if !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
#define EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_

#include "Canvas.h"
#include "Widget.h"

#define DISPLAY_SIZE 6
#define DEBOUNCE 200

class Button : public Widget
{

public:
	//Button();
	/*Button(unsigned int _x, unsigned int _y, unsigned int _w, unsigned int _h,
    int bgColor, int textColor, int borderColor);*/
  Button();
	Button(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor);
	Button(unsigned int radius, int backgroundColor, int textColor, int borderColor);
	virtual ~Button();

	void setEventHandler(void (*functionPointer)(Button *));
	virtual void show();
	virtual void update();
	
	void drawBackground(int color);
	void drawBorder();
	void drawText();
	
	char* getText();
	void init();
	void clear();
	byte getTextLength(char* c);
	byte getTextSize();
	void setDebounce(unsigned int d);
	void setNum(int n);
	void setText(char* _text);
  //void setSize(char _size);
  void fitToText();
  
	//Overriden methods
	bool checkTouch(Point* p);
	bool isButton();
	bool touched;
	unsigned long lastMillis;
	
protected:	
	unsigned int debounceTime;	
	//A pointer to the event handler function.
	void (*eventHandler)(Button*);	

private:
	bool isRound;
	char* text;

};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
