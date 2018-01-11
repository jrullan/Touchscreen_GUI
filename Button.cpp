///////////////////////////////////////////////////////////
//  Button.cpp
//  Implementation of the Class Button
//  Created on:      02-Mar-2015 9:17:12 PM
///////////////////////////////////////////////////////////

#include "Button.h"


Button::Button(unsigned char textSize){
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);
	contents = Text(textSize);
}

Button::Button(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor,unsigned char textSize){
	if(label = (char *)malloc(DISPLAY_SIZE+1)) memset(label,0,DISPLAY_SIZE+1);
	
	x = 0;
	y = 0;
	contents = Text(textSize);
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
	debounceTime = DEBOUNCE;
	lastMillis = millis();	
}

int Button::getLabelSize(){
	if(*label){
		return contents.getTextLength(label)*6*borderWidth + 6;
	}
	return 0;
}

void Button::drawBackground(int color){
	int labelSize = getLabelSize();
	int xl;
	int wl;
	if(labelPos == 1){
	
	}else if(labelPos == 2){
		xl = x;// + labelSize;
		wl = w + labelSize;
	}else if(labelPos == 3){
		
	}else{
		xl = x + labelSize;
		wl = w + labelSize;
	}
	//Fill background
	
	if(!this->isRound){
		Tft.fillRect(x+borderWidth, y+borderWidth, wl-(2*borderWidth)+1,h-(2*borderWidth)+1,color);
	}else{
		int radius = (w>>1)-borderWidth;
		Tft.fillCircle(xl+radius+borderWidth,y+radius+borderWidth,radius-borderWidth/2,color);
		// label background
		//Tft.fillRect(xl+2*radius+2*borderWidth+FONT_SPACE, y+borderWidth, labelSize-(2*borderWidth),h-(2*borderWidth),this->myCanvas->bgColor);
		//Tft.fillRect(x, y+borderWidth, labelSize-(2*borderWidth),h-(2*borderWidth),BLACK);
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
			Tft.drawRectangle(xPos++,yPos++,width--,height--,borderColor);
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
	
	if(labelSize > 0){
		if(labelPos == 1){ // top
		
		}else if(labelPos == 2){ // right
			xl = x + w + FONT_SPACE;
			yl = y+(h-8*fontSize)/2;
		}else if(labelPos == 3){ // bottom
			
		}else{ 	// default 0-left
			xl = x;
			yl = y+(h-8*fontSize)/2;	
		}
		
		Tft.drawString(label,xl,yl,fontSize,fgColor);
	}
	
	if(*contents.text){
		char size = contents.getTextSize();
		int stringX = x+labelSize+(w-size*6*fontSize)/2;
		int stringY = y+(h-8*fontSize)/2;
		Tft.drawString(contents.text,stringX,stringY,fontSize,fgColor);
	}
}

/*
void Button::clear(){
	byte textSize = getTextSize();
	if(textSize){
		for(int i = textSize-1; i >= 0; i--)
		{
				text[i] = 0;
		}
	}
}
*/


unsigned char Button::getTextLength(char* c){
	return contents.getTextLength(c);
	/*
	char size = 0;
	if(*c){
    char* chars = c;
    while(*chars){
      *chars++;
      size++;
    }
  }
  return size;
	*/
}

unsigned char Button::getTextSize(){
  //return getTextLength(text);
	return contents.getTextSize();
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
	/*
	clear();
	//Serial.println("Text cleared in setNum");Serial.print("Num to be processed: ");Serial.println(num);
	char numChar[DISPLAY_SIZE];
	char chars = 0;
	
	
	while(num > 0)	// Extract characters representing the powers of ten
	{
		numChar[chars++] = '0'+num%10;
		num /= 10;
		//Serial.print("Num after loop: ");Serial.println(numChar[chars-1]);
	}

	// Reverse the order of the characters
	for(int j = chars-1; j >= 0; j--)//DISPLAY_SIZE; j++)
	{
		text[j] = numChar[chars-1-j];
	}
	text[chars]=0;
	
	//Serial.print("Num entered: ");Serial.println(text);
	//drawBackground(bgColor);
	//update();
	*/
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
	/*
	char size = getTextSize();
	long result = 0;
	for(int i = 0; i<size; i++){
		if(text[i] == '.') break;  // Only process integer side
		result = result * 10 + text[i]-'0';
	}
	return result;
	*/
	return contents.getNum();
}

void Button::fitToText(){
  if(*contents.text){
	char size = contents.getTextSize();
	/*
    char* chars = text;
    char size = 0;
    while(*chars){
      *chars++;
      size++;
    }
	*/
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
