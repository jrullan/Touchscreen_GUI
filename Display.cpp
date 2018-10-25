#include "Display.h"

Display::Display(unsigned char textLength){
	x=0;
	y=0;
	contents = new Text(textLength);
	this->init();
}

Display::Display(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor, unsigned char textLength){
	x = 0;
	y = 0;
	contents = new Text(textLength);
	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
	this->init();
}

Display::~Display(){
	free(contents->text);
}

void Display::init(){
	type = 0x10;
	borderWidth = 2;
	charPos = 0;
}

void Display::drawFrame(){
  int xPos = x;	
  int width = w;
  byte yPos = y;
  byte height = h;
  for(byte i=borderWidth; i!=0;i--){
    myCanvas->tft->drawRect(xPos++,yPos++,width--,height--,borderColor);
    width--;
    height--;
  }
}

void Display::drawText(int color){
	if(*contents->text){
		char size = contents->getTextSize();
		int stringX = getCenterTextX(x,w,size);
		int stringY = getCenterTextY(y,h);
		myCanvas->tft->drawString(contents->text,stringX,stringY,fontSize,color);
	}
}

void Display::append(char* c){
	byte cSize = getTextLength(c);
	byte txtSize = getTextLength(contents->text);//getTextSize();
	byte space = contents->_textSize - txtSize;
	
	//Check that there space available to append
	if(txtSize < contents->_textSize){
		for(int i=0; i<space; i++)
		{
			contents->text[i+txtSize] = c[i];
		}
	}
	update();
}

//New testing out...
void Display::append(char c){
	byte txtSize = getTextLength(contents->text);
	//Check that there space available to append
	if(txtSize < contents->_textSize){
		contents->text[txtSize] = c;
	}
	update();	
}

void Display::fitToText(){
  if(*contents->text){
	/*
    char* chars = contents->text;
    char size = 0;
    while(*chars){
      *chars++;
      size++;
    }
	*/
	char size = contents->getTextSize();
    w = size * FONT_SPACE * borderWidth + FONT_SPACE;
    h = FONT_Y * borderWidth + FONT_Y;
  }
}

void Display::setNum(int num, bool force){
	if(contents->getNum() == num) return;
	if(force) drawText(bgColor);
	contents->setNum(num);
	if(force) drawText(fgColor);
}

void Display::setText(char* _text, bool force){
	if(force) drawText(bgColor);
	//contents->setText(_text);
	contents->text = _text; // changed from calling setText to just passing the pointer, reduced 4 bytes in program memory
	if(force) drawText(fgColor);
}

void Display::deleteChar(){
	byte textSize = contents->getTextSize();
	if(textSize){
		for(int i = textSize-1; i >= 0; i--)
		{
			if(contents->text[i] != 0){
				contents->text[i] = 0;
				break;
			}
		}
	}
	update();
}

//Overriden virtual methods

bool Display::checkTouch(Point* p){
	return true;
}

bool Display::isButton(){
	return false;
}

void Display::show(){
	drawFrame();
	update();
}

void Display::update(){
	myCanvas->tft->fillRect(x+borderWidth, y+borderWidth, w-(2*borderWidth),h-(2*borderWidth),bgColor);
	drawText(fgColor);
}
