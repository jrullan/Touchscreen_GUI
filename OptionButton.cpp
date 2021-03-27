///////////////////////////////////////////////////////////
//  OptionButton.cpp
//  Implementation of the Class OptionButton
//  Created on:      30-Dec-2016 
///////////////////////////////////////////////////////////
#include "OptionButton.h"

OptionButton::OptionButton(){
	this->init();
}

OptionButton::OptionButton(unsigned int diameter, int backgroundColor, int textColor, int borderColor){
	this->init();
	this->setSize(diameter,diameter);
	this->setColors(backgroundColor,textColor,borderColor);	
}

OptionButton::~OptionButton(){}

void OptionButton::init(){
	//Button::init();
	/*
	type = 0x30;
	debounceTime = 500;
	isButton = true;
	isRound = false;
	touched = false;
	borderWidth = 2;
	bgColor = GRAY1;
	fgColor = BLACK;
	borderColor = WHITE;
	setSize(100,40);
	setText("Button");
	x = 0;
	y = 0;
	lastMillis = millis();	
	*/
	if( (label = (char *)malloc(DISPLAY_SIZE+1)) ) memset(label,0,DISPLAY_SIZE+1);
	type = 0x34;
	debounceTime = 500;
	isButton = true;
	isRound = true;
	touched = false;
	borderWidth = 2;
	bgColor = GRAY1;
	fgColor = BLACK;
	borderColor = WHITE;
	setSize(40,40);
	setLabel("Option");
	labelPos = 2;
	x = 0;
	y = 0;
	lastMillis = millis();	
}

void OptionButton::update(){
	int radius = this->w/2;
	
	if(this->touched){
		myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius) - this->borderWidth - OPTION_GAP, this->fgColor);
	}else{
		myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius) - this->borderWidth - OPTION_GAP, this->bgColor);
	}
}

void OptionButton::show(){
	int radius = this->w/2;

	drawLabel();
	
	myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius), this->borderColor);
	myCanvas->tft->fillCircle(this->x+radius,this->y+radius,(radius) - this->borderWidth, this->bgColor);	
	
	update();
}