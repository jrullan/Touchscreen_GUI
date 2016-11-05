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

static char* Widget::getType(Widget* w){
	if((w->type & 0xFF) == 0x00){ 
		return "widget";
	}else if ((w->type & 0xFF) == 0x10){
		return "display";
	}else if ((w->type & 0xFF) == 0x20){
		return "indicator";
	}else if ((w->type & 0xFF) == 0x21){
		return "dial";
	}else if ((w->type & 0xFF) == 0x22){
		return "gauge";
	}else if ((w->type & 0xFF) == 0x23){
		return "trend";
	}else if ((w->type & 0xFF) == 0x30){
		return "button";
	}else if ((w->type & 0xFF) == 0x31){
		return "buttongrid";
	}else if ((w->type & 0xFF) == 0x32){
		return "numkey";
	}else if ((w->type & 0xFF) == 0x33){
		return "popup";
	}
}
