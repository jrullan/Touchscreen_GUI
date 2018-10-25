///////////////////////////////////////////////////////////
//  Button.cpp
//  Implementation of the Class Button
//  Created on:      02-Mar-2015 9:17:12 PM
///////////////////////////////////////////////////////////

/*
 * Sensible defaults mods:
 * 1. Provide simple constructor, no parameters
 * 2. Allow to have no event handler for touch events (early return in the checkTouch method)
 * 3. Set initial x, y, fgColor, bgColor, borderColor
 * 4. Set initial w and h
 * 5. Set initial fontSize
 * 6. Set initial debounce delay
 */

#include "Button.h"


/*
 * Basic Constructor, no parameters
 */
Button::Button(){
	contents = new Text(8);
	this->isRound = false;
	init();
}

/*
 * Constructor with initial "Text" and optional "Label"
 */
Button::Button(char* text, char* _label){
	contents = new Text(Widget::getTextLength(text));
	this->isRound = false;
	init();
	
	if(_label){
		setLabel(_label);
	}
	setText(text);
}

/**
 * Constructor for a typical rectangle button
 */
Button::Button(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor,unsigned char textLength){
	contents = new Text(textLength);
	this->isRound = false;
	init();
	
	this->setSize(width,height);
	this->setColors(backgroundColor,textColor,borderColor);
}

/**
 * Constructor for a round button
 */
Button::Button(unsigned int radius, int backgroundColor, int textColor, int borderColor){
	contents = new Text(8);
	this->isRound = true;
	init();
	
	x = radius;
	y = radius;	
	this->setSize(2*radius,2*radius);
	this->setColors(backgroundColor,textColor,borderColor);
}

Button::~Button(){
	free(contents->text);
	free(label);
}

/**
 * Initialization of several common parameters
 */
void Button::init(){
	type = 0x30;
	debounceTime = 1000;
	isButton = true;
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
}

/**
 * Draws the background of the button
 */
void Button::drawBackground(int color){
	int labelWidth = getLabelSize();
	int xl;
	int wl;
	
	// Label Background
	if(labelPos == LABEL_TOP){
	
	}else if(labelPos == LABEL_RIGHT){
		xl = x;
		wl = w;
	}else if(labelPos == LABEL_BOTTOM){
		
	}else{ // labelPos == LABEL_LEFT
		xl = x + labelWidth;
		wl = w;
	}

	// Button background	
	if(!this->isRound){
		if(cornerRadius > 0){
			//myCanvas->tft->fillRoundRect(xl+borderWidth, y+borderWidth, wl-(2*borderWidth),h-(2*borderWidth),cornerRadius,color);
			myCanvas->tft->fillRoundRect(x+borderWidth, y+borderWidth, wl-(2*borderWidth),h-(2*borderWidth),cornerRadius,color);
		}else{
			//myCanvas->tft->fillRect(xl+borderWidth, y+borderWidth, wl-(2*borderWidth),h-(2*borderWidth),color);
			myCanvas->tft->fillRect(x+borderWidth, y+borderWidth, wl-(2*borderWidth),h-(2*borderWidth),color);
		}
	}else{
		int radius = (w>>1)-borderWidth;
		//myCanvas->tft->fillCircle(xl+radius+borderWidth,y+radius+borderWidth,radius,color);//radius-borderWidth/2,color);
		myCanvas->tft->fillCircle(x+radius+borderWidth,y+radius+borderWidth,radius,color);//radius-borderWidth/2,color);
	}
}

/**
 * Draws the text inside the button
 */
void Button::drawText(){

	int labelWidth=getLabelSize();
	// Draw contents text
	if(*contents->text){
		char length = getTextLength(contents->text);//contents.getTextSize();
		//int stringX = getCenterTextX(x+labelWidth,w,length);
		int stringX = getCenterTextX(x,w,length);
		int stringY = getCenterTextY(y,h);
		myCanvas->tft->drawString(contents->text,stringX,stringY,fontSize,fgColor);
	}
}

/**
 * Draws the border of the button
 */
void Button::drawBorder(){
	int labelWidth = getLabelSize();
	int xl;
	
	if(labelPos == LABEL_TOP){
	
	}else if(labelPos == LABEL_RIGHT){
		xl = x;// + labelWidth;
		
	}else if(labelPos == LABEL_BOTTOM){
		
	}else{ // LABEL_LEFT
		xl = x + labelWidth;
	}  
	
	//int xPos = xl;	
	int xPos = x;	
	int width = w;
	int yPos = y;
	uint8_t height = h;
  
	for(byte i=borderWidth; i!=0;i--){
		if(!this->isRound){
			if(cornerRadius > 0){
				myCanvas->tft->drawRoundRect(xPos++,yPos++,width--,height--,cornerRadius,borderColor);
			}else{
				myCanvas->tft->drawRect(xPos++,yPos++,width--,height--,borderColor);
			}
			width--;
			height--;
		}else{
			int radius = width>>1;
			myCanvas->tft->drawCircle(xl+radius,y+radius,(radius)-i,borderColor);
		}
	}	
}

/**
 * Draws a label for the button if one is set
 * Maximum length is DISPLAY_SIZE
 */
void Button::drawLabel(){
	int xl=0;
	int yl=0;
	int labelWidth=getLabelSize();
	int labelHeight = fontSize * FONT_Y;
	
	// Draw label
	if(labelWidth > 0){
		if(labelPos == LABEL_TOP){ // top
		
		}else if(labelPos == LABEL_RIGHT){ // right
			xl = x + w + FONT_SPACE*fontSize;
			yl = y+(h-labelHeight)/2;
		}else if(labelPos == LABEL_BOTTOM){ // bottom
			
		}else{ 	// default LABEL_LEFT 
			xl = x-(labelWidth + FONT_SPACE*fontSize);
			yl = y+(h-labelHeight)/2;	
		}
		
		//Check label is within screen area
		if(xl > 0 && xl <= (myCanvas->w - labelWidth) && yl>0 && yl <= (myCanvas->h - labelHeight)){
			myCanvas->tft->drawString(label,xl,yl,fontSize,fgColor);
		}else{	// if not within screen area draw a red rectangle around the button as a warning
			myCanvas->tft->fillRect(x-4,y-4,w+8,h+8,RED);
		}
	}	
}

/**
 * Shows the widget on the screen. This method must be overriden by the widget to
 * show the particular widget. It should be a pure virtual function: (i.e. virtual
 * void function() = 0;
 */
void Button::show(){
	drawLabel();	
	drawBackground(bgColor);
	update();
}

/**
 * OVERRIDEN - This is the update() method.
 */
void Button::update(){
	drawText();
	drawBorder();
}

/**
 * OVERRIDEN - Check if the touched point is within bounds of this widget.
 */
bool Button::checkTouch(Point* p){
	// Early exit if no event handler was defined
	if(!*eventHandler) return false;
	
	int labelWidth = getLabelSize();
	int xl = x;
	
	/*
	if(labelPos == LABEL_TOP){
	
	}else if(labelPos == LABEL_RIGHT){
		xl = x;
	}else if(labelPos == LABEL_BOTTOM){
		
	}else{	// LABEL_LEFT
		xl = x + labelWidth;
	} 
	*/

	if(lastMillis + debounceTime < millis()){ 
		if((p->x > xl) && (p->x < xl+w) && (p->y > y) && (p->y < y+h)){
			touched = !touched;
			eventHandler(this);
			lastMillis = millis();
			return false;
		}
	}
	// if block is true return false, so canvas will not continue to 
	// process the event.
	return !block; 
}

/**
 * Gets the label size, used for redrawing
 */
int Button::getLabelSize(){
	if(label){	// if initialized...
		return getTextLength(label)*FONT_X*fontSize;// + 6;
	}
	return 0;
}

/**
 * Sets the button debouncing delay
 */
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

void Button::setNum(int num){
	if(contents->getNum()==num)return;
	contents->clear();
	contents->setNum(num);
}

void Button::setText(char* _text){
  contents->text = _text;
	//contents->setText(_text);
}

void Button::setLabel(char* _label){
	if(!label){ // if not already set (initialized...)
		int labelSize = Widget::getTextLength(_label);
		if(label = (char *)malloc(labelSize+1)) memset(label,0,labelSize+1);
		label = _label;
	}
}

char* Button::getText(){
  return contents->text;
}

long Button::getNum(){
	return contents->getNum();
}

void Button::fitToText(){
  if(*contents->text){
	char length = getTextLength(contents->text);//contents.getTextSize();
    w = length * FONT_X * fontSize + FONT_SPACE;
    h = FONT_Y * fontSize + FONT_Y;
  }
}


