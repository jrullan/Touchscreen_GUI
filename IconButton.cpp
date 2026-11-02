///////////////////////////////////////////////////////////
//  IconButton.cpp
//  Implementation of the IconButton
//  Created on:     
///////////////////////////////////////////////////////////
#include "IconButton.h"

namespace {
// Prefer the transaction-free uniform-color primitive on modern GFX panels.
// Legacy STM32 AS panels (and older drivers) expose only drawFastHLine.
// Overload selection is compile-time: no buffer, runtime probe, or public API.
template<class Display>
auto writeIconRun(Display& display, int16_t x, int16_t y, int16_t w,
                  uint16_t color, int)
    -> decltype(display.writeFastHLine(x, y, w, color), void()) {
    display.writeFastHLine(x, y, w, color);
}
template<class Display>
void writeIconRun(Display& display, int16_t x, int16_t y, int16_t w,
                  uint16_t color, long) {
    display.drawFastHLine(x, y, w, color);
}
}

IconButton::IconButton(int _w, int _h, const uint16_t* _imageUnpressed, const uint16_t* _imagePressed, char* _label, TextStorage storage) : Button(storage){
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

	setLabel(_label);
	fgColor = WHITE;
	type = 0x35;
	debounceTime = DEFAULT_DEBOUNCE_MS;
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
    visible = true;
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
    if (!canDraw() || !bitmap || w <= 0 || h <= 0) return;
    // Clip in wide destination coordinates before touching flash. In particular,
    // neither x+w/y+h nor the source row product may use AVR's 16-bit int.
    const int32_t left = x < 0 ? 0 : x;
    const int32_t top = y < 0 ? 0 : y;
    const int32_t rightEdge = int32_t(x) + w;
    const int32_t bottomEdge = int32_t(y) + h;
    const int32_t right = rightEdge < myCanvas->tft->width() ? rightEdge : myCanvas->tft->width();
    const int32_t bottom = bottomEdge < myCanvas->tft->height() ? bottomEdge : myCanvas->tft->height();
    if (left >= right || top >= bottom) return;

    // One bounded scope, even for an all-transparent visible image: avoid a
    // second flash pass or retained state just to eliminate an empty transaction.
    Guitft::ScopedWrite write(*myCanvas->tft);
    for (int32_t row = top; row < bottom; ++row) {
        const int32_t offset = (row - int32_t(y)) * w + (left - int32_t(x));
        int32_t runStart = left;
        uint16_t runColor = pgm_read_word(&bitmap[offset]);
        for (int32_t column = left + 1; column <= right; ++column) {
            const uint16_t color = column < right
                ? pgm_read_word(&bitmap[offset + column - left]) : 0;
            if (column == right || color != runColor) {
                if (!useTransparency || runColor != transparentColor)
                    writeIconRun(*myCanvas->tft, int16_t(runStart), int16_t(row),
                                 int16_t(column - runStart), runColor, 0);
                runStart = column;
                runColor = color;
            }
        }
    }
}
