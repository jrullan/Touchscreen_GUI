///////////////////////////////////////////////////////////
//  Canvas.cpp
//  Implementation of the Class Canvas
//  Created on:      02-Mar-2015 9:16:46 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#include "Canvas.h"
#include "Button.h"


Canvas::Canvas(){
}

Canvas::~Canvas(){

}

void Canvas::init(){
	Tft.TFTinit();
	//Serial.println("Tft.TFTinit() called");
	lastMillis = millis();
	debounceTime = 400;
}

void Canvas::init(int mode){
	this->init();
	if(mode = TFT_PORTRAIT) portrait();
	if(mode = TFT_LANDSCAPE) landscape();
}

/**
 * This method adds a widget to the widgets array and places it in the X,Y
 * position of the canvas. The value passed is the address of the widget object.
 */
void Canvas::add(Widget* widget, int x, int y){
	//widgets.push_back(widget);	//Casting necessary to store pointer address
	widgets.push(widget);
	widget->x = x;
	widget->y = y;
	widget->show();
}

Point Canvas::getTouchedPoint(){
  Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);
  if(Tft.layoutMode == TFT_LANDSCAPE) p.toLandscape();
  return p;
}

void Canvas::landscape(){
	Tft.layoutMode = TFT_LANDSCAPE;
}

void Canvas::portrait(){
	Tft.layoutMode = TFT_PORTRAIT;
}

/**
 * Removes a widget from the widgets array and destroys it.
 */
void Canvas::remove(Widget* widget){
	/*
	for(int i=widgets.size(); i!=0; i--){
		if(widgets[i] == widget){
			widgets.erase(widgets.begin()+i);
		}
	}
	*/
}
Widget* Canvas::pop(){
  Widget* widget = widgets.pop();
  Tft.fillRectangle(widget->x,widget->y,widget->w,widget->h,this->bgColor);
  //Tft.fillRectangle(widget->x,widget->y,widget->x+widget->w,widget->y+widget->h,widget->bgColor);
  redraw();
	return widget;
}


void Canvas::scan(){
	if(millis()>lastMillis + debounceTime){ //Debouncing of the touchscreen resistance
    Point tP = getTouchedPoint();
    touchWidgets(&tP); //Update all buttons on Canvas :)
    lastMillis=millis();
  }
}

void Canvas::setDebounce(unsigned int d){
	this->debounceTime = d;
}

/**
 * Checks for a touch event and notifies all subscribed widgets. The notification
 * involves calling the widget's checkTouch method and passing the touched Point.
 */
void Canvas::touchWidgets(Point* p){
	//for(int i=0; i<widgets.size(); i++){
	byte cnt = widgets.count();
	for(int i=1; i<=cnt; i++){
	//for(int i=widgets.count()-1; i>=0; i++){
		if(widgets[cnt-i]->isButton()){
			if(!widgets[cnt-i]->checkTouch(p)) break;  //Break if widget blocks event after handling it.
		}
	}
}


void Canvas::updateTouch(Point* p){
	this->touchedPoint = p;
}

/*
char Canvas::getWidgetsQty(){
	return widgets.size();
}
*/

void Canvas::redraw(){
	//Tft.fillScreen();
	//for(int i=0; i<widgets.size(); i++){
	for(int i=0; i<widgets.count(); i++){
		widgets[i]->show();
	}
}