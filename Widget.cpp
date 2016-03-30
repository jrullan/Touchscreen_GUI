///////////////////////////////////////////////////////////
//  Widget.cpp
//  Implementation of the Class Widget
//  Created on:      02-Mar-2015 9:17:00 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#include "Widget.h"
#include "Canvas.h"

Widget::Widget(){

}



Widget::~Widget(){

}


/**
 * Hides the widget
 */
void Widget::hide(){
	Tft.fillRectangle(x,y,w,h,myCanvas->bgColor);
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
