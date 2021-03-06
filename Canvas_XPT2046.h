/*
	
	This class inherits from Canvas to provide a specific 
	touch chipset implementation and reduce the amount of
	included libraries in the compilation.
	
	In theory every chipset should have a correspoding 
	canvas.
	
	This however still requires that each touch chipset
	library is adapted to inherit from Touch.
	
	*/
	
#if !defined(__CANVAS_XPT2046__)
#define __CANVAS_XPT2046__

#include "Canvas.h"
#include "XPT2046_Touchscreen.h" 

#define XPT2046_MIN_X 3894
#define XPT2046_MAX_X 261
#define XPT2046_MIN_Y 3891
#define XPT2046_MAX_Y 320

class Canvas_XPT2046: public Canvas{
	public:
	Canvas_XPT2046(int mode, int color, int tft_cs, int tft_ds, int ts_cs):Canvas(mode, color, tft_cs, tft_ds, ts_cs){
		this->init();
	};
	~Canvas_XPT2046(){
		
	};
	void init();
	Point* getTouchedPoint();
};


/*
 * Init function initializes 
 * the TFT, the touch chipset
 * and other timing variables
 */
void Canvas_XPT2046::init(){
	tft->begin();
	tft->fillScreen(bgColor);
	lastMillis = millis();
	touchSampling = millis();
	scanSampling = millis();
	scanSampleDelay = SCAN_SAMPLING_TIME;

	if(_mode == TFT_PORTRAIT) portrait();
	if(_mode == TFT_LANDSCAPE) landscape();	
	
	ts = new XPT2046_Touchscreen(ts_cs);
	ts->begin(0);
}

Point* Canvas_XPT2046::getTouchedPoint(){
	if((millis() > touchSampling + TOUCH_SAMPLING_TIME)){
		Point p = ts->getPoint();
		// Invert x and y
		int oldX = p.x;		
		p.x = p.y;
		p.y = oldX;
		
		if(_mode == TFT_PORTRAIT){
			p.x = map(p.x, XPT2046_MIN_X, XPT2046_MAX_X, 0, w);
			p.y = map(p.y, XPT2046_MIN_Y, XPT2046_MAX_Y, 0, h);	
		}else{
			oldX = p.x;
			p.x = map(p.y, XPT2046_MAX_Y, XPT2046_MIN_Y, 0, w);
			p.y = map(oldX, XPT2046_MIN_X, XPT2046_MAX_X, 0, h);				
		}
		
		//Serial.print("X: ");Serial.println(p.x);
		//Serial.print("Y: ");Serial.println(p.y);
		//Serial.println();

		touchedPoint.x = p.x;
		touchedPoint.y = p.y;
		touchSampling = millis();
		return &touchedPoint;
	}
	return NULL;
}

#endif
