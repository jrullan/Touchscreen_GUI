///////////////////////////////////////////////////////////
//  Button.cpp
//  Implementation of the Class Button
//  Created on:      02-Mar-2015 9:17:12 PM
///////////////////////////////////////////////////////////

#include "Button.h"


Button::Button(unsigned char textLength){
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);
	contents = Text(textLength);
}

Button::Button(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor,unsigned char textLength){
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);
	x = 0;
	y = 0;
	contents = Text(textLength);
	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
	this->isRound = false;
	init();
}

Button::Button(unsigned int radius, int backgroundColor, int textColor, int borderColor){
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);

	x = radius;
	y = radius;	
	contents = Text(8);
	this->setSize(2*radius,2*radius);
	this->setColors(backgroundColor,textColor,borderColor);
	this->isRound = true;
	init();
}

Button::~Button(){
	free(contents.text);
}

/*
 * Initialization of several common parameters
 */
void Button::init(){
	type = 0x30;
	isButton = true;
	touched = false;
	borderWidth = 2;
	lastMillis = millis();	
}

int Button::getLabelSize(){
	if(*label){
		return getTextLength(label)*FONT_X*fontSize;// + 6;
	}
	return 0;
}

void Button::drawBackground(int color){
	int labelSize = getLabelSize();
	int xl;
	int wl;
	
	// Label Background
	if(labelPos == 1){
	
	}else if(labelPos == 2){
		xl = x;
		wl = w;
	}else if(labelPos == 3){
		
	}else{
		xl = x + labelSize;
		wl = w;
	}

	// Button background	
	if(!this->isRound){
		Tft.fillRect(xl+borderWidth, y+borderWidth, wl-(2*borderWidth),h-(2*borderWidth),color);
	}else{
		int radius = (w>>1)-borderWidth;
		Tft.fillCircle(xl+radius+borderWidth,y+radius+borderWidth,radius,color);//radius-borderWidth/2,color);
	}
}

void Button::drawBorder(){
	int labelSize = getLabelSize();
	int xl;
	
	if(labelPos == 1){
	
	}else if(labelPos == 2){
		xl = x;// + labelSize;
		
	}else if(labelPos == 3){
		
	}else{
		xl = x + labelSize;
		
	}  
	
	int xPos = xl;	
	int width = w;
	int yPos = y;
	uint8_t height = h;
  
	for(byte i=borderWidth; i!=0;i--){
		if(!this->isRound){
			Tft.drawRect(xPos++,yPos++,width--,height--,borderColor);
			width--;
			height--;
		}else{
			int radius = width>>1;
			Tft.drawCircle(xl+radius,y+radius,(radius)-i,borderColor);
		}
	}	
}

void Button::drawText(){
	int xl=0;
	int yl=0;
	int labelSize=getLabelSize();
	
	// Draw label
	if(labelSize > 0){
		if(labelPos == 1){ // top
		
		}else if(labelPos == 2){ // right
			xl = x + w + FONT_SPACE*fontSize;
			yl = y+(h-FONT_Y*fontSize)/2;
		}else if(labelPos == 3){ // bottom
			
		}else{ 	// default 0-left
			xl = x;
			yl = y+(h-FONT_Y*fontSize)/2;	
		}
		Tft.drawString(label,xl,yl,fontSize,~this->myCanvas->bgColor);
	}
	
	// Draw contents text
	if(*contents.text){
		char length = getTextLength(contents.text);//contents.getTextSize();
		int stringX = getCenterTextX(x+labelSize,w,length);
		int stringY = getCenterTextY(y,h);
		Tft.drawString(contents.text,stringX,stringY,fontSize,fgColor);
	}
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
	update();
}

void Button::setNum(int num){
	if(contents.getNum()==num)return;
	contents.clear();
	contents.setNum(num);
}

void Button::setText(char* _text){
  contents.text = _text;
}

void Button::setLabel(char* _label){
  label = _label;
}

char* Button::getText(){
  return contents.text;
}

long Button::getNum(){
	return contents.getNum();
}

void Button::fitToText(){
  if(*contents.text){
	char length = getTextLength(contents.text);//contents.getTextSize();
    w = length * FONT_X * fontSize + FONT_SPACE;
    h = FONT_Y * fontSize + FONT_Y;
  }
}

//Overriden methods
/**
 * Check if the touched point is within bounds of this widget.
 */
bool Button::checkTouch(Point* p){
	int labelSize = getLabelSize();
	int xl;
	
	if(labelPos == 1){
	
	}else if(labelPos == 2){
		xl = x;
	}else if(labelPos == 3){
		
	}else{
		xl = x + labelSize;
	}  
	//Serial.print("Checking button ");Serial.println(text);
	if(lastMillis + debounceTime < millis()){ 
		//Serial.print("Checking button ");Serial.print(contents.text);Serial.print(" at ");Serial.print(debounceTime); Serial.println(" ms");
		if((p->x > xl) && (p->x < xl+w) && (p->y > y) && (p->y < y+h)){
			touched = !touched;
			eventHandler(this);
			lastMillis = millis();
			//block = true;
			//Serial.print("Button ");Serial.print(text);Serial.println(" pressed");
			return false;
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
	drawText();
	drawBorder();
}