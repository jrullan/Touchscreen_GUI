///////////////////////////////////////////////////////////
//  Canvas.cpp
//  Implementation of the Class Canvas
//  Created on:      02-Mar-2015 9:16:46 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#include "Canvas.h"
//#include "Button.h"

// Forward declaration of class Widget to solve issue with mutual 
// includes
#include "Widget.h"


Canvas::Canvas(){

}

Canvas::~Canvas(){

}

// Initializes the LCD screen (Tft) and sets debounce and lastmillis variables.
// Last millis is used for the Canvas-wide debounce of touch events.
void Canvas::init(){
	Tft.TFTinit();
	lastMillis = millis();
	debounceTime = DEBOUNCE;
}

// Alternative initialization routine that sets the orientation
// of the screen. By default the orientation is Portrait.
void Canvas::init(int mode){
	this->init();
	if(mode = TFT_PORTRAIT) portrait();
	if(mode = TFT_LANDSCAPE) landscape();
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

// This method calculates the x,y coordinates of the touched point
// according to the currently set orientation mode.
Point Canvas::getTouchedPoint(){
  Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);
  if(Tft.layoutMode == TFT_LANDSCAPE) p.toLandscape();
  return p;
}

// This method sets the TFT orientation mode to landscape.
void Canvas::landscape(){
	Tft.layoutMode = TFT_LANDSCAPE;
}

// This method sets the TFT orientation mode to portrait.
void Canvas::portrait(){
	Tft.layoutMode = TFT_PORTRAIT;
}

// This method is deprecated
/**
 * Removes a widget from the widgets array and destroys it.

void Canvas::remove(Widget* widget){
	
	for(int i=widgets.count(); i!=0; i--){
		if(widgets[i] == widget){
			//widgets.erase(widgets.begin()+i);
		}
	}
	
}
 */
 
// This method can be invoked to remove the last widget added to the canvas.
Widget* Canvas::pop(){
  Widget* widget = widgets.pop();
  Tft.fillRectangle(widget->x,widget->y,widget->w,widget->h,this->bgColor);
  redraw();
  //delete widget;
	return widget;
}

// This method scans the touchscreen for touch events.
// If an event is detected, all registered widgets are notified of the event by
// calling its event handler function.
bool Canvas::scan(){
	if(millis()>lastMillis + debounceTime){ //Debouncing of the touchscreen resistance
    Point tP = getTouchedPoint();
    //Serial.print("Touched x = ");
    //Serial.print(tP.x);
    //Serial.print(" Touched y = ");
    //Serial.println(tP.y);
    if(Tft.layoutMode == TFT_LANDSCAPE){
    	if(tP.x > 319 || tP.y > 239) return false;
    }else{
    	if(tP.x > 239 || tP.y > 319) return false;
    }
    touchWidgets(&tP); //Update all buttons on Canvas :)
    lastMillis=millis();
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
void Canvas::touchWidgets(Point* p){
	byte cnt = widgets.count();
	for(int i=1; i<=cnt; i++){
		//Serial.print("Widget ");Serial.print(cnt-i);Serial.print(" isButton"); Serial.println(widgets[cnt-i]->isButton);
		//widgets[cnt-i]->isButton ? Serial.println(" true") : Serial.println(" false");
		// Only send touch event to visible Buttons
		if(widgets[cnt-i]->isButton && widgets[cnt-i]->visible){
			if(!widgets[cnt-i]->checkTouch(p)) break;  //Break if widget blocks event after handling it.
		}
	}
}

// This method updates the touchedPoint attribute of the canvas.
void Canvas::updateTouch(Point* p){
	this->touchedPoint = p;
}

// This method calls every widget's show method to force them to redraw.
void Canvas::redraw(){
	for(int i=0; i<widgets.count(); i++){
		if(widgets[i]->visible) widgets[i]->show();
	}
}
