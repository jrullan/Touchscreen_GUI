/*
* Canvas.cpp
* Implementation of the Class Canvas
* Created on:      02-Mar-2015 9:16:46 PM
* Original author: Invision
* 
*	2018-08-08: Added initialization for Adafruit touch controller STMPE610
*
*/

#include "Canvas.h"
#include "Widget.h"
#include "Screen.h"

// Touch drivers includes
//#include "Seeedtouch.h"
//#include "Adafruit_FT6206.h"
//#include "Adafruit_STMPE610.h"


Canvas::Canvas(int mode, int color, int tft_cs, int tft_ds, int ts){
	_mode = mode;
	bgColor = color;
	ts_cs = ts;
	tft = new Guitft(tft_cs,tft_ds);
	
	/*
	#if defined(ESP32)
		tft = new Guitft(15, 33); // Use hardware SPI
	#elif defined(ESP8266)
		tft = new Guitft(16,15);
	#else
		tft = new Guitft(5, 6); // Use hardware SPI
	#endif
	*/
}

Canvas::~Canvas(){

}

// Initializes the LCD screen (tft) and sets debounce and lastmillis variables.
// Last millis is used for the Canvas-wide debounce of touch events.
/*
void Canvas::init(){
	tft->begin();
	tft->fillScreen(BLACK);
	lastMillis = millis();
	touchSampling = millis();
	scanSampling = millis();
	scanSampleDelay = SCAN_SAMPLING_TIME;

	if(_mode == TFT_PORTRAIT) portrait();
	if(_mode == TFT_LANDSCAPE) landscape();	

	if(touchType == TOUCHTYPE_SEEEDSTUDIO_RESISTIVE){
		ts = new SeeedstudioTouch(XP,YP,XM,YM);
	}
	
	if(touchType == TOUCHTYPE_ADAFRUIT_FT6206){
		ts = new Adafruit_FT6206();
		ts->begin(FT62XX_DEFAULT_THRESHOLD);
	}
	if(touchType == TOUCHTYPE_ADAFRUIT_STMPE610){
		ts = new Adafruit_STMPE610(STMPE_CS);

		//Wait until the touchscreen is started.
		//Original driver has an apparent bug,
		//sometimes it did start other times it didn't.
		while(!ts->begin(STMPE_ADDR)){
			;
		}
		//Serial.println("Touchscreen successfully started");
	}
}
*/
	
// This method sets the TFT orientation mode to landscape.
void Canvas::landscape(){
	tft->layoutMode = TFT_LANDSCAPE;
	tft->setRotation(3);
	w=320;
	h=240;
}

// This method sets the TFT orientation mode to portrait.
void Canvas::portrait(){
	tft->layoutMode = TFT_PORTRAIT;
	tft->setRotation(0);
	w=240;
	h=320;
}

// This method adds a widget to the widgets array and places it in the X,Y
// position of the canvas. The value passed is the address of the widget object.
void Canvas::add(Widget* widget, int x, int y, uint8_t show){
	widgets.push(widget);
	widget->setCanvas(this);
	widget->x = x;
	widget->y = y;
	if(widget->visible && show) widget->show();
}

void Canvas::setScreen(Screen* screen, uint8_t show){
	if(currentScreen != NULL){
		currentScreen->hide();
	}
	currentScreen = screen;
	if(show) currentScreen->show();
}

// This method can be invoked to remove the last widget added to the canvas.
Widget* Canvas::pop(){
	Widget* widget = widgets.pop();
	tft->fillRect(widget->x,widget->y,widget->w,widget->h,this->bgColor);
	redraw();
	return widget;
}

// This method scans the touchscreen for touch events.
// If an event is detected, all registered widgets are notified of the event by
// calling its event handler function.
bool Canvas::scan(){		

	if(millis() > (scanSampling + scanSampleDelay)){
			scanSampling = millis();
			//Early exits
			if(!ts->touched()){
				//Serial.println("!touched()");
				return false;
			}
			
			Point* tP = getTouchedPoint();
			if(tP == NULL){
				//Serial.println("tp == NULL");
				return false;
			}
			
			if(!inBounds(tP)){
				//Serial.println("!inBounds(tP)");
				//Serial.print("tP.x = ");Serial.println(tP->x);
				//Serial.print("tP.y = ");Serial.println(tP->y);
				return false;
			}
			
			//Serial.print("tP.x = ");Serial.println(tP->x);
			
			// Send event to canvas widgets, then to screen widgets
			// if no canvas widget blocks the event.
			if(touchWidgets(tP)){
				//Serial.println("Sent touch event to all widgets");
				if(currentScreen != NULL){
					//Serial.println("Sending touch to current screen...");
					currentScreen->touchWidgets(tP);
				}
			}
			return true;
	}
	
  return false;
}

// This method calculates the x,y coordinates of the touched point
// according to the currently set orientation mode.
/*
Point* Canvas::getTouchedPoint(){
	if((millis() > touchSampling + TOUCH_SAMPLING_TIME)){
		Point p = ts->getPoint();
		
		if(touchType == TOUCHTYPE_ADAFRUIT_FT6206){
			if(tft->layoutMode == TFT_PORTRAIT){
				p.rotate(POINT_PORTRAIT2);
			}
			if(tft->layoutMode == TFT_LANDSCAPE){
				p.rotate(POINT_LANDSCAPE2);
			}
		}
		
		if(touchType == TOUCHTYPE_ADAFRUIT_STMPE610){				
			p.x = map(p.x, STMPE_MINX, STMPE_MAXX, 0, 240);
			p.y = map(p.y, STMPE_MINY, STMPE_MAXY, 0, 320);			
			if(_mode == TFT_LANDSCAPE){
				p.rotate(POINT_STMPE610_LANDSCAPE);
			}
			if(_mode == TFT_PORTRAIT){
				p.rotate(POINT_STMPE610_PORTRAIT);
			}
		}		

		if(touchType == TOUCHTYPE_SEEEDSTUDIO_RESISTIVE){
			p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft->width());
			p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft->height());			
			if(tft->layoutMode == TFT_LANDSCAPE){
				p.toLandscape();
			}
		}
		
		touchedPoint.x = p.x;
		touchedPoint.y = p.y;
		touchSampling = millis();
		return &touchedPoint;
	}
	return NULL;
}
*/
 
// Determines if the given point falls
// within the Bounds of the layout chosen
bool Canvas::inBounds(Point* tP){
	if(tft->layoutMode == TFT_LANDSCAPE){
		if(tP->x > 319 || tP->y > 239) return false;
	}else{
		if(tP->x > 239 || tP->y > 319) return false;
	}
	
	if(tP->x < 0 || tP->y < 0) return false;
	
	return true;
}

// This method sets the canvas debouncing time to a value. By default it is 0.
/*
void Canvas::setDebounce(unsigned int d){
	this->debounceTime = d;
}
*/

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

void Canvas::setScanSampling(unsigned int d){
	scanSampleDelay = d;
}
