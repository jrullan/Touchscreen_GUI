///////////////////////////////////////////////////////////
//  OptionButton.cpp
//  Implementation of the Class OptionButton
//  Created on:      30-Dec-2016 
///////////////////////////////////////////////////////////
#include "OptionButton.h"

OptionButton::OptionButton(){
	this->init();
	this->setSize(40,40);
	this->setColors(BLACK,WHITE,GRAY1);
}

OptionButton::OptionButton(unsigned int diameter, int backgroundColor, int textColor, int borderColor){
	this->init();
	this->setSize(diameter,diameter);
	this->setColors(backgroundColor,textColor,borderColor);	
}

OptionButton::~OptionButton(){}

void OptionButton::init(){
	Button::init();
	this->type = 0x34;
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);
	this->labelPos = 2;
	this->isRound = true;
	lastMillis = millis();	
}

void OptionButton::update(){
	int radius = this->w/2;
	
	myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius), this->borderColor);
	myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius) - this->borderWidth, this->bgColor);

	if(this->touched){
		myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius) - this->borderWidth - 2, this->fgColor);
	}else{
		myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius) - this->borderWidth - 2, this->bgColor);
	}
}

void OptionButton::show(){
	drawLabel();
	update();
}