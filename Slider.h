#ifndef INDICATOR_H
#define INDICATOR_H

include "Indicator.h"

class Slider:public Indicator{
	public: 
		Slider();
		~Slider();
		
		// Overriden methods
		bool checkTouch(Point* p);
		virtual void update()=0;
		virtual void show()=0;	
		void (*eventHandler)(Button*);
		
		void init();
					
	bool block = false;
};


Slider::Slider(){
	init();
}

Slider::~Slider(){}

void Slider::init(){
	type=0x24;
	setHiLimit(100,RED);
	setLowLimit(0,BLUE);
	setSP(50,YELLOW);
}

bool Slider::checkTouch(Point* p){
	// Early exit if no event handler was defined
	if(!*eventHandler) return false;
	
	if(lastMillis + debounceTime < millis()){ 
		
		// 1. Check if touch event falls within the bounds of this widget
		if((p->x > x) && (p->x < x+w) && (p->y > y) && (p->y < y+h)){
			// 2. Draw the slider knob or marker at the point->y position
			//		Draw the background
			//		Draw the scale
			//		Draw the knob/marker
			
			// 3. Map the point->y position to the current Value (CV)			
			int range = h - borderWidth*2;
			int minY = x + borderWidth;
			int maxY = minValue + range;
			int sValue = map(p->y,maxY,minY,scaleMin,scaleMax);
			setCV(sValue,true);
			
			// 4. Call this widget's event handler routine
			eventHandler(this);
			
			// 5. Update debouncing timestamp
			lastMillis = millis();
			return false;
		}
	}
	// if block is true return false, so canvas will not continue to 
	// process the event.
	return !block;
}

#endif
