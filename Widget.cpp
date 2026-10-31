///////////////////////////////////////////////////////////
//  Widget.cpp
//  Implementation of the Class Widget
//  Created on:      02-Mar-2015 9:17:00 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#include "Widget.h"
#include "Canvas.h"

Widget::Widget(): x(0), y(0), h(0), w(0), borderWidth(0), bgColor(0), fgColor(0xFFFF), borderColor(0xFFFF), myCanvas(NULL){
}

Widget::~Widget(){

}

/**
 * Hides the widget
 */
void Widget::hide(){
	if(canDraw() && w > 0 && h > 0) myCanvas->tft->fillRect(x,y,w,h,myCanvas->bgColor);
	visible = false;
}

bool Widget::canDraw() const {
	return myCanvas && myCanvas->tft && visible;
}

void Widget::setCanvas(Canvas* c){
	myCanvas = c;
}

void Widget::setColors(int bgColor, int fgColor, int borderColor){
	this->bgColor = bgColor;
	this->fgColor = fgColor;
	this->borderColor = borderColor;
}

void Widget::setSize(int width, int height){
	this->w = width;
	this->h = height;
}

