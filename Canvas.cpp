///////////////////////////////////////////////////////////
//  Canvas.cpp
//  Implementation of the Class Canvas
//  Created on:      02-Mar-2015 9:16:46 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#include "Canvas.h"
#include "Widget.h"
#include "Screen.h" 

Canvas::Canvas(){

}


Canvas::Canvas(int mode, int color){
	_mode = mode;
	bgColor = color;
}

Canvas::~Canvas(){

}

// Initializes the LCD screen (Tft) and sets debounce and lastmillis variables.
// Last millis is used for the Canvas-wide debounce of touch events.
void Canvas::init(){
	Tft.begin();
	Tft.fillScreen(BLACK);
	lastMillis = millis();
	debounceTime = DEBOUNCE;
	//portrait();
	//return;
	if(_mode == TFT_PORTRAIT) portrait();
	if(_mode == TFT_LANDSCAPE) landscape();	
}

// This method sets the TFT orientation mode to landscape.
void Canvas::landscape(){
	Tft.layoutMode = TFT_LANDSCAPE;
	Tft.setRotation(3);
	w=320;
	h=240;
}

// This method sets the TFT orientation mode to portrait.
void Canvas::portrait(){
	Tft.layoutMode = TFT_PORTRAIT;
	Tft.setRotation(0);
	w=240;
	h=320;
}

// This method adds a widget to the widgets array and places it in the X,Y
// position of the canvas. The value passed is the address of the widget object.
void Canvas::add(Widget* widget, int x, int y){
	widgets.push(widget);
	widget->setCanvas(this);
	widget->x = x;
	widget->y = y;
	if(widget->visible) widget->show();
}

void Canvas::setScreen(Screen* screen){
	currentScreen = screen;
	currentScreen->show();
	showWidgets();
}

// This method calculates the x,y coordinates of the touched point
// according to the currently set orientation mode.
Point* Canvas::getTouchedPoint(){
	Point p = ts.getPoint();
	if(millis() > touchSampling + TOUCH_SAMPLING_TIME){ //Debouncing of the touchscreen resistance
		p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
		p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);			
		if(Tft.layoutMode == TFT_LANDSCAPE){
			p.toLandscape();
		}
		touchSampling = millis();
		touchedPoint.x = p.x;
		touchedPoint.y = p.y;
		//Tft.drawCircle(p.x,p.y,10,RED);
		return &touchedPoint;
	}
	return NULL;
}
 
// This method can be invoked to remove the last widget added to the canvas.
Widget* Canvas::pop(){
	Widget* widget = widgets.pop();
	Tft.fillRect(widget->x,widget->y,widget->w,widget->h,this->bgColor);
	redraw();
	return widget;
}

// This method scans the touchscreen for touch events.
// If an event is detected, all registered widgets are notified of the event by
// calling its event handler function.
bool Canvas::scan(){

	if(millis()>lastMillis + debounceTime){ //Debouncing of the touchscreen resistance
		Point* tP = getTouchedPoint();

		//Early exits
		if(tP == NULL) return false;
		if(!inBounds(tP)){
			return false;
		}
		
		// Send event to canvas widgets, then to screen widgets
		// if no canvas widget blocks the event.
		if(touchWidgets(tP)){
			if(currentScreen != NULL){
				currentScreen->touchWidgets(tP);
			}
		}
		
		lastMillis=millis();
	}else{
		return false;
	}
  return true;
}


bool Canvas::inBounds(Point* tP){
	if(Tft.layoutMode == TFT_LANDSCAPE){
		if(tP->x > 319 || tP->y > 239) return false;
	}else{
		if(tP->x > 239 || tP->y > 319) return false;
	}
	return true;
}

// This method sets the canvas debouncing time to a value. By default it is 0.
void Canvas::setDebounce(unsigned int d){
	this->debounceTime = d;
}

/**
 * Checks for a touch event and notifies all subscribed widgets. The notification
 * involves calling the widget's checkTouch method and passing the touched Point.
 */
bool Canvas::touchWidgets(Point* p){
	byte cnt = widgets.count();
	for(int i=1; i<=cnt; i++){
		// Only send touch event to visible Buttons
		if(widgets[cnt-i]->isButton && widgets[cnt-i]->visible){
			if(!widgets[cnt-i]->checkTouch(p)) return false;  //Break if widget blocks event after handling it.
		}
	}
	return true;
}

// This method calls every widget's show method to force them to redraw.
void Canvas::redraw(){
	if(currentScreen != NULL){
		currentScreen->show();
	}
	showWidgets();
}

void Canvas::showWidgets(){
	for(int i=0; i<widgets.count(); i++){
		if(widgets[i]->visible) widgets[i]->show();
	}
}
