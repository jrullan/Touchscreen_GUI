///////////////////////////////////////////////////////////
//  Button.cpp
//  Implementation of the Class Button
//  Created on:      02-Mar-2015 9:17:12 PM
///////////////////////////////////////////////////////////

#include "Button.h"


Button::Button(){}

Button::Button(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor){
	x = 0;
	y = 0;
	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
	this->isRound = false;
	init();
}

Button::Button(unsigned int radius, int backgroundColor, int textColor, int borderColor){
	x = radius;
	y = radius;
	this->setSize(2*radius,2*radius);
	this->setColors(backgroundColor,textColor,borderColor);
	this->isRound = true;
	init();
}

Button::~Button(){

}

/*
 * Initialization of several common parameters
 */
void Button::init(){
	isButton = true;
	touched = false;
	borderWidth = 2;
	debounceTime = DEBOUNCE;
	lastMillis = millis();	
}

void Button::drawBackground(int color){
	//Fill background
	if(!isRound){
		Tft.fillRectangle(x+borderWidth, y+borderWidth, w-(2*borderWidth),h-(2*borderWidth),color);
  }else{
  	int radius = (w>>1)-borderWidth;
  	Tft.fillCircle(x+radius+borderWidth,y+radius+borderWidth,radius,color);
  }
}

void Button::drawBorder(){
  int xPos = x;	
  int width = w;
  byte yPos = y;
  byte height = h;
	for(byte i=borderWidth; i!=0;i--){
		if(!isRound){
			Tft.drawRectangle(xPos++,yPos++,width--,height--,borderColor);
			width--;
			height--;
		}else{
			int radius = width>>1;
			Tft.drawCircle(x+radius,y+radius,(radius)-i,borderColor);
		}
	}	
}

void Button::drawText(){
	//Count characters to center on the button - Nice trick from the Tft2 library
	if(*text){
		char* chars = text;
		char size = 0;
		while(*chars){
			*chars++;
			size++;
		}
		//Calculate centered position of the text
		int stringX = x+(w-size*6*borderWidth)/2;
		int stringY = y+(h-8*borderWidth)/2;
		Tft.drawString(text,stringX,stringY,borderWidth,fgColor);
	}
}


void Button::clear(){
	byte textSize = getTextSize();
	if(textSize){
		for(int i = textSize-1; i >= 0; i--)
		{
				text[i] = 0;
		}
	}
}

byte Button::getTextLength(char* c){
	char size = 0;
	if(*c){
    char* chars = c;
    while(*chars){
      *chars++;
      size++;
    }
  }
  return size;
}

byte Button::getTextSize(){
  return getTextLength(text);
}

void Button::setDebounce(unsigned int d){
	debounceTime = d;
}

/**
 * Sets the event handler: The function that should be called whenever the touched
 * event is detected.
 */
void Button::setEventHandler(void (*functionPointer)(Button *)){
	eventHandler = functionPointer;
}

/**
 * Shows the widget on the screen. This method must be overriden by the widget to
 * show the particular widget. It should be a pure virtual function: (i.e. virtual
 * void function() = 0;
 */
void Button::show(){
	drawBackground(bgColor);
	drawText();
  update();
}

void Button::setNum(int num){
	clear();
	char numChar[DISPLAY_SIZE];
	char chars = 0;
	while(num >= 10)	// Extract characters representing the powers of ten
	{
		numChar[chars++] = num%10;
		num /= 10;
	}
	numChar[chars++] = num;
	for(int j = 0; j < chars; j++)//DISPLAY_SIZE; j++)
	{
		text[chars-1-j] = '0'+numChar[j];
	}
	text[chars]=0;
	update();
}

void Button::setText(char* _text){
  text = _text;
}

char* Button::getText(){
  return text;
}

long Button::getNum(){
	char size = getTextSize();
	long result = 0;
	for(int i = 0; i<size; i++){
		if(text[i] == '.') break;  // Only process integer side
		result = result * 10 + text[i]-'0';
	}
	return result;
}

void Button::fitToText(){
  if(*text){
    char* chars = text;
    char size = 0;
    while(*chars){
      *chars++;
      size++;
    }
    w = size * 6 * borderWidth + 6;
    h = 8 * borderWidth + 8;
    //drawString(text,x+5,y+5,2,textColor);
  }
}

//Overriden methods
/**
 * Check if the touched point is within bounds of this widget.
 */
bool Button::checkTouch(Point* p){
	//Serial.println("Button checkTouch called");
	if(lastMillis + debounceTime < millis()){ 
		if((p->x > x) && (p->x < x + w) && (p->y > y) && (p->y < y+h)){
			touched = !touched;
			eventHandler(this);
			lastMillis = millis();
			//Serial.print("Button ");Serial.print(text);Serial.println(" pressed");
		}
	}
	// if block is true return false, so canvas will not continue to 
	// process the event.
	return !block; 
}

/*
 * This is the update() method.
 */
void Button::update(){
	//drawBackground(bgColor);
	drawText();
	drawBorder();
}

/*
 * This method is called by the canvas and passes a pointer to it. 
 * The button class uses this pointer to get a pointer to the 
 * touched point, if any, and calls the checkTouch routine to detect
 * if the button was touched.
 */
/*void Button::update(Canvas* c){
	checkTouch(c->touchedPoint);
}*/
