///////////////////////////////////////////////////////////
//  Numkey.h
//  Implementation of the Class Numkey
//  Created on:      11-Mar-2015 9:12 AM
///////////////////////////////////////////////////////////

#ifndef NUMKEY_H
#define NUMKEY_H

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
	void setEventHandler(void (*functionPointer)(Numkey*));
	void deleteChar();
	void clear();
	void hide();
	
	/*
	 * This version of show takes a pointer to a button
	 * and uses it to trigger that button's 
	 * registered event handler.
	 * 
	 * Since this numkey is probably shown initially when that button is pressed,
	 * the decision to continue upon an entry should be done there.
	 * 
	 * Example code in the button event's handler:
	 * 
	 * void btnEventHandler(Button* btn){ 
	 * 		if(numkey.entry == NULL){
	 * 			numkey.show(btn);
	 * 		}else{ 
	 * 			...button code...
	 * 			numkey.reset(); // makes selection NULL again
	 * 		}
	 * } 
	 */	
	void show(Button* btn);
	void reset();
	
	//Overriden methods
	virtual bool checkTouch(Point* p);
	virtual void show();
	virtual void update();
	
	bool autoremove; //Forces autoremove from Canvas when = is pressed.

	Button* targetButton;
	void targetEventHandler();
	int entry = NULL;
		
private:
	//String text;
	char* text; 
	char charPos;
	byte getTextLength(char* c);
	void (*eventHandler)(Numkey*);

};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
