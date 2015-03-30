///////////////////////////////////////////////////////////
//  Widget.cpp
//  Implementation of the Class Widget
//  Created on:      02-Mar-2015 9:17:00 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#include "Widget.h"


Widget::Widget(){

}



Widget::~Widget(){

}



/**
 * Hides the widget
 */
void Widget::hide(){

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



//
/*
void Widget::drawString(char *string,INT16U poX, INT16U poY, INT16U size,INT16U fgcolor)
{
    while(*string)
    {
        createChar(*string, poX, poY, size, fgcolor);
        *string++;

        if(poX < MAX_Y)
        {
            poX += 6*size;                                     
        }
    }
}

// Characters are 4 pixels width x Size x 2
void Widget::createChar( INT8U ascii, INT16U poX, INT16U poY,INT16U size, INT16U fgcolor)
{
    if((ascii>=32)&&(ascii<=127))
    {
        ;
    }
    else
    {
        ascii = '?';
    }
    for (int i =0; i<8; i++ ) {
        INT8U temp = pgm_read_byte(&simpleFont[ascii-0x20][i]);
        for(INT8U f=0;f<8;f++)
        {
            if((temp>>f)&0x01)
            {
                fillRect(poX+i*size, poY+f*size, size, size, fgcolor);
            }

        }

    }
}

// Create a "landscape" rectangle
void Widget::fillRect(int x, int y, int w, int h, INT16U color){
	int x1 = y;
	int y1 = map(x, 0, 320, 320, 0); 
	Tft.fillRectangle(x1,y1-w,h,w,color);
}

// Create a "landscape" rectangle
void Widget::drawRect(int x, int y, int w, int h, INT16U color){
	int x1 = y;
	int y1 = map(x, 0, 320, 320, 0); 
	Tft.drawRectangle(x1,y1-w,h,w,color);
}
*/
/*
template <typename T> void Widget::update(T* c){
	checkTouch(c->touchedPoint);
}*/