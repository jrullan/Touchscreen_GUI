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

#include "Canvas.h"
#include "Button.h"

#define POPUP_OK 0
#define POPUP_YES_NO 1
#define POPUP_OK_CANCEL 2

class Popup : public Button
{
	
public:
	//Methods
	Popup();
	//Popup(unsigned char type,char* message);
	Popup(char* message);
	virtual ~Popup();
	
	void init();
	void drawFrame(int pX, int pY, int wW, int hH);
	void drawText(int pX, int pY, int wW, int hH, char* t);
	void draw();
	void processEvent(unsigned char btnNo);
	void setEventHandler(void (*functionPointer)(Popup*, unsigned char));
	//void setText(char* _text, char* _btn1, char* _btn2 = NULL);
	void setText(char* _text);
	
	//void show(void (*eH)(Button*),Button*);
	/*
	 * This version of show takes a pointer to a button
	 * and uses it to trigger that button's 
	 * registered event handler if the selection of the popup is 1
	 * otherwise it will not forward the event. Since this popup is 
	 * probably shown initially when that button is pressed,
	 * the decision to continue upon a 1 selection should be done there.
	 * 
	 * Example code in the button event's handler:
	 * 
	 * void btnEventHandler(Button* btn){ 
	 * 		if(popup.selection == NULL){
	 * 			popup.show(btn);
	 * 		}else{ 
	 * 			...button code...
	 * 			popup.reset(); // makes selection NULL again
	 * 		}
	 * } 
	 */
	void show(Button*);
	void hide();
	void reset();
	
	int getBtnHeight();
	int getBtnWidth();
	
	//Overriden
	virtual bool checkTouch(Point* p);
	virtual void show();
	virtual void update();	
	
	//Attributes
	unsigned char popup_type; // Type of Pop-up (One or Two buttons)
	unsigned char fontSize = 2;
	void (*targetEventHandler)(Button*); //store a reference to another button's event handler
	Button* targetButton;
	char selection = NULL;
	
private:
	//Methods
	
	//Attributes
	char* text; // Holds the message
	char* btn1Text;
	char* btn2Text;
	void (*eventHandler)(Popup*, unsigned char); // Sets address of external function to call upon checkTouch
};

#endif
