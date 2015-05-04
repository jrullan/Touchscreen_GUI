///////////////////////////////////////////////////////////
//  Numkey.h
//  Implementation of the Class Numkey
//  Created on:      11-Mar-2015 9:12 AM
///////////////////////////////////////////////////////////

#ifndef NUMKEY_H
#define NUMKEY_H

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif
#define DISPLAY_SIZE 6

#include "Canvas.h"
#include "Button.h"

class Numkey : public Button
{

public:
	Numkey();
	Numkey(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor);
	virtual ~Numkey();

	void append(char* c);
	void appendNum(unsigned char c);
	void drawFrame();
  void fitToText();	
  long getNum();
	char* getText();
	byte getTextSize();
	void init();
	
	void setNum(int num);
	void setNumkeyEventHandler(void (*functionPointer)(Numkey*));
  //void setText(char* _text);
  void deleteChar();
  void clear();
  
	//Overriden methods
	virtual bool checkTouch(Point* p);
	virtual bool isButton();
	virtual void show();
	virtual void update();
	
	bool isPopUp = true;	//Forces autoremove from Canvas when = is pressed.
	
private:
	//String text;
	char* text; 
	//char text[DISPLAY_SIZE];
	char charPos;
	//byte maxSize = 6;
	byte getTextLength(char* c);
	void (*eventHandler)(Numkey*);

};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
