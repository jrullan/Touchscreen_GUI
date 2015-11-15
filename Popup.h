///////////////////////////////////////////////////////////
//  Popup.h
//  Implementation of the Class Popup
//	This class implements a pop-up widget ("window")
// 	that can be used to ask the user for a confirmation
//	and/or display a message with an acknowledge button. 
//	The popup can have one or two buttons, so a Cancel-OK
//  popup can be implemented or just an OK popup window.
//
//  Created on:      14-Nov-2015 7:32 PM
///////////////////////////////////////////////////////////

#ifndef POPUP_H
#define POPUP_H

#define POPUP_ONE_BUTTON 0
#define POPUP_TWO_BUTTONS 1

#include "Canvas.h"
#include "Button.h"

class Popup : public Button
{
public:
	//Methods
	Popup();
	Popup(unsigned char type,char* message);
	virtual ~Popup();
	void init();
	void drawFrame(int pX, int pY, int wW, int hH);
	void drawText(int pX, int pY, int wW, int hH, char* t);
	void draw();
	void setEventHandler(void (*functionPointer)(Popup*, unsigned char));
	void setText(char* _text);
	
	int getBtnHeight();
	int getBtnWidth();
	
		//Overriden
		virtual bool checkTouch(Point* p);
		virtual bool isButton();
		virtual void show();
		virtual void update();	
	
	//Attributes
	unsigned char type; // Type of Pop-up (One or Two buttons)
	unsigned char fontSize = 2;

private:
	//Methods
	
	//Attributes
	char* text; // Holds the message
	void (*eventHandler)(Popup*, unsigned char); // Sets address of external function to call upon checkTouch
};

#endif
