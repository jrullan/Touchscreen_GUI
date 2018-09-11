///////////////////////////////////////////////////////////
//  IconButton.cpp
//  Implementation of the IconButton
//  Created on:     
///////////////////////////////////////////////////////////
#include "IconButton.h"

IconButton::IconButton(int _w, int _h, const uint16_t* _imageUnpressed, const uint16_t* _imagePressed){
	
	this->type = 0x35;
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);
	this->labelPos = 2;
	this->isButton = true;
	this->touched = false;
	this->borderWidth = 0;
	this->setSize(_w,_h);
	this->debounceTime = DEBOUNCE;
	this->imageUnpressed = _imageUnpressed;
	this->imagePressed = _imagePressed;
	lastMillis = millis();
}

IconButton::~IconButton(){}

void IconButton::show(){
	update();
}

void IconButton::update(){
	drawLabel();
	if(this->imagePressed != NULL){
		if(this->touched){
			Tft.drawRGBBitmap(this->x,this->y,this->imagePressed,this->w,this->h);
		}else{
			Tft.drawRGBBitmap(this->x,this->y,this->imageUnpressed,this->w,this->h);			
		}
	}else{
		Tft.drawRGBBitmap(this->x,this->y,this->imageUnpressed,this->w,this->h);
	}
}