#ifndef SLIDER_H
#define SLIDER_H

#if !defined(DEBOUNCE)
#define DEBOUNCE 200
#endif

#include "Canvas.h"
#include "Indicator.h"

class Slider : public Indicator
{
	public: 
		Slider();
		~Slider();
		
		void setEventHandler(void (*functionPointer)(Slider *));
		void init();
		void drawBorder();
		void drawCV(int cv);
		void setDebounce(int debounce);
				
		// Overriden methods
		bool checkTouch(Point* p);
		void show();	
		void update();

	/*
	virtual bool checkTouch(Point* p)=0;
	virtual void show() =0;
	virtual void update() =0;
	*/
					
	void (*eventHandler)(Slider*);
	bool block = false;
	unsigned long lastMillis;
	
protected:	
	unsigned int debounceTime = DEBOUNCE;	
};


Slider::Slider(){
	init();
};

Slider::~Slider(){}

void Slider::init(){
	type=0x24;
	borderWidth = 2;
	scaleMax = 100;
	scaleMin = 0;
	setHiLimit(scaleMax - 10,RED);
	setLowLimit(scaleMin + 10,BLUE);
	setSP(50,YELLOW);
	setSize(40,120);
	setColors(BLACK,BLUE,WHITE);
	this->isButton = true;
}

void Slider::drawBorder(){
	for(byte i=0; i<borderWidth;i++){
		myCanvas->tft->drawRect(x+i,y+i,w-i*2,h-i*2,borderColor);
	}	
}

void Slider::drawCV(int cv){
	int cvY = map(cv,scaleMin,scaleMax,y+h-borderWidth,y+borderWidth);
	
	//		Draw the background
	int startFill = y+borderWidth;
	int fillHeight = cvY - startFill;
	myCanvas->tft->fillRect(x+borderWidth, startFill, w-2*borderWidth, fillHeight, this->bgColor);

	//		Draw the knob/marker
	startFill = cvY;
	fillHeight = y + h - borderWidth - startFill;
	myCanvas->tft->fillRect(x+borderWidth, startFill, w-2*borderWidth, fillHeight, this->fgColor);
}

void Slider::setDebounce(int debounce){
	this->debounceTime = debounce;
}

void Slider::setEventHandler(void (*functionPointer)(Slider *)){
	eventHandler = functionPointer;
}

bool Slider::checkTouch(Point* p){
	// Early exit if no event handler was defined
	if(!*eventHandler) return false;
	
	if(lastMillis + debounceTime < millis()){ 
		// 1. Check if touch event falls within the bounds of this widget
		if((p->x > x) && (p->x < x+w) && (p->y > y) && (p->y < y+h)){
			// 1. Map the point->y position to the current Value (CV)			
			int range = h - borderWidth*2;
			int minY = y + borderWidth;
			int maxY = minY + range;
			int sValue = map(p->y,maxY,minY,scaleMin,scaleMax);
			setCV(sValue,true);
			
			// 2. Update the widget
			drawCV(sValue);
			
			// 3. Call this widget's event handler routine
			eventHandler(this);
			
			// 4. Update debouncing timestamp
			lastMillis = millis();
			return true;
		}
	}
	// if block is true return false, so canvas will not continue to 
	// process the event.
	return !block;
}

void Slider::update(){
	drawBorder();
	drawCV(currentValue);
	//myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,w-2*borderWidth,h-2*borderWidth,this->fgColor);
}

void Slider::show(){
	drawBorder();
	drawCV(currentValue);
	//myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,w-2*borderWidth,h-2*borderWidth,this->fgColor);
}

#endif
