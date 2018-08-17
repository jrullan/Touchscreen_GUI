/*
	
	This class inherits from Canvas to provide a specific 
	touch chipset implementation and reduce the amount of
	included libraries in the compilation.
	
	In theory every chipset should have a correspoding 
	canvas.
	
	This however still requires that each touch chipset
	library is adapted to inherit from Touch.
	
	*/
	
#if !defined(__CANVAS_SEEEDTOUCH__)
#define __CANVAS_SEEEDTOUCH__

#include "Canvas.h"
#include "Seeedtouch.h" 

class Canvas_SEEEDTOUCH: public Canvas{
	public:
	Canvas_SEEEDTOUCH(int mode, int color):Canvas(mode, color){};
	~Canvas_SEEEDTOUCH(){};
	void init();
	Point* getTouchedPoint();
};

/*
 * Init function initializes 
 * the TFT, the touch chipset
 * and other timing variables
 */
void Canvas_SEEEDTOUCH::init(){
	Serial.println("Seeedtouch init");
	
	Tft.begin();
	Tft.fillScreen(BLACK);
	lastMillis = millis();
	touchSampling = millis();
	scanSampling = millis();
	scanSampleDelay = SCAN_SAMPLING_TIME;

	if(_mode == TFT_PORTRAIT) portrait();
	if(_mode == TFT_LANDSCAPE) landscape();	
	
	ts = new SeeedstudioTouch(XP,YP,XM,YM);
}

Point* Canvas_SEEEDTOUCH::getTouchedPoint(){
	if((millis() > touchSampling + TOUCH_SAMPLING_TIME)){
		Point p = ts->getPoint();
		/*
		Serial.println("-----------");
		Serial.print("x: ");Serial.println(p.x);
		Serial.print("y: ");Serial.println(p.y);
		Serial.print("Tft.width(): ");Serial.println(Tft.width());
		Serial.print("Tft.height(): ");Serial.println(Tft.height());
		*/
		p.x = map(p.x, TS_MINX, TS_MAXX, 0, Tft.height());
		p.y = map(p.y, TS_MINY, TS_MAXY, 0, Tft.width());			
		if(Tft.layoutMode == TFT_LANDSCAPE){
			int oldX = p.x;
			p.x = 320 - p.y;
			p.y = oldX;			
			//p.toLandscape();
		}
		/*
		Serial.print("X: ");Serial.println(p.x);
		Serial.print("Y: ");Serial.println(p.y);
		*/
		touchedPoint.x = p.x;
		touchedPoint.y = p.y;
		touchSampling = millis();
		return &touchedPoint;
	}
	return NULL;
}

#endif
