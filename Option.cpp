///////////////////////////////////////////////////////////
//  Option.cpp
//  Implementation of the Class Option
//  Created on:      30-Dec-2016 
///////////////////////////////////////////////////////////
#include "Option.h"

Option::Option(unsigned int diameter, int backgroundColor, int textColor, int borderColor){
	
	this->type = 0x34;
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);
	this->labelPos = 2;
	this->isRound = true;
	this->isButton = true;
	this->touched = false;
	this->borderWidth = 2;
	this->setSize(diameter,diameter);
	this->x = diameter/2;
	this->y = this->x;
	this->setColors(backgroundColor,textColor,borderColor);
	this->debounceTime = DEBOUNCE;
	lastMillis = millis();
	
	/*
	Button::Button(diameter/2,backgroundColor,textColor,borderColor);
	this->labelPos = 2;
	this->type = 0x34;
	*/
}

Option::~Option(){}

