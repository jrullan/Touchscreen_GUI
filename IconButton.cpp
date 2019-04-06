///////////////////////////////////////////////////////////
//  IconButton.cpp
//  Implementation of the IconButton
//  Created on:     
///////////////////////////////////////////////////////////
#include "IconButton.h"

IconButton::IconButton(int _w, int _h, const uint16_t* _imageUnpressed, const uint16_t* _imagePressed){	
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
	type = 0x35;
	debounceTime = DEBOUNCE;
	isButton = true;
	isRound = false;
	touched = false;
	borderWidth = 0;
	setSize(_w,_h);
	x = 0;
	y = 0;
	lastMillis = millis();	
	imageUnpressed = _imageUnpressed;
	imagePressed = _imagePressed;
}

IconButton::~IconButton(){}

void IconButton::show(){
	drawLabel();
	update();
}

/* 
 * Draws the corresponding image based on the status of the touched attribute 
 * using a customized version of the drawRGBBitmap to support drawing a transparent
 * color
 */
void IconButton::update(){
	if(this->imagePressed != NULL && this->touched){
		drawRGBABitmap(this->x,this->y,this->imagePressed,this->w,this->h, (uint16_t) transparentColor);
	}else{
		drawRGBABitmap(this->x,this->y,this->imageUnpressed,this->w,this->h, (uint16_t) transparentColor);
	}
}

/**************************************************************************/
/*!
   @brief   Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.  
   For 16-bit display devices; no color reduction performed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with 16-bit color bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
/**************************************************************************/
// Modified version of Adafruit_GFX drawRGBBitmap to support defining one color as transparent
// void Adafruit_GFX::drawRGBBitmap(int16_t x, int16_t y,
void IconButton::drawRGBABitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h, uint16_t transparentColor) {
		uint16_t pixel;
    myCanvas->tft->startWrite();

    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
						pixel = bitmap[j*w+i];
						if(pixel != transparentColor){
							myCanvas->tft->writePixel(x+i, y, pgm_read_word(&bitmap[j * w + i]));
						}
        }
    }
 	
    myCanvas->tft->endWrite();
}
