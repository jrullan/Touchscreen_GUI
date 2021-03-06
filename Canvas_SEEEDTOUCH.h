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
	Canvas_SEEEDTOUCH(int mode, int color, int tft_cs=5, int tft_ds=6):Canvas(mode, color, tft_cs, tft_ds){
		this->init();
	};
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
	tft->begin();
	tft->fillScreen(bgColor);		
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
		
		//Serial.println("-----------");
		//Serial.print("x: ");Serial.println(p.x);
		//Serial.print("y: ");Serial.println(p.y);
		//Serial.print("tft->width(): ");Serial.println(tft->width());
		//Serial.print("tft->height(): ");Serial.println(tft->height());
		
		if(tft->layoutMode == TFT_PORTRAIT){
			p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft->width());
			p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft->height());
		}else{ // Landscape:
			int oldX = p.x;
			p.x = map(p.y, TS_MAXY, TS_MINY, 0, tft->width());
			p.y = map(oldX, TS_MINX, TS_MAXX, 0, tft->height());
		}

		//Serial.print("TS_MINX: ");Serial.println(TS_MINX);
		//Serial.print("TS_MAXX: ");Serial.println(TS_MAXX);
		//Serial.print("TS_MINY: ");Serial.println(TS_MINY);
		//Serial.print("TS_MAXY: ");Serial.println(TS_MAXY);
		//Serial.print("X: ");Serial.println(p.x);
		//Serial.print("Y: ");Serial.println(p.y);
		//Serial.print("tft->width(): ");Serial.println(tft->width());
		//Serial.print("tft->height(): ");Serial.println(tft->height());
		
		touchedPoint.x = p.x;
		touchedPoint.y = p.y;
		touchSampling = millis();
		return &touchedPoint;
	}
	return NULL;
}

#endif
