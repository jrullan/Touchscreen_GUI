/*
	
	This class inherits from Canvas to provide a specific 
	touch chipset implementation and reduce the amount of
	included libraries in the compilation.
	
	In theory every chipset should have a correspoding 
	canvas.
	
	This however still requires that each touch chipset
	library is adapted to inherit from Touch.
	
	*/
	
#if !defined(__CANVAS_FT6206__)
#define __CANVAS_FT6206__

#include "Canvas.h"
#include "Adafruit_FT6206.h" 

class Canvas_FT6206: public Canvas{
	public:
	Canvas_FT6206(int mode, int color, int tft_cs, int tft_ds, int ts_cs):Canvas(mode, color, tft_cs, tft_ds, ts_cs){};
	~Canvas_FT6206(){};
	void init();
	Point* getTouchedPoint();
};

/*
 * Init function initializes 
 * the TFT, the touch chipset
 * and other timing variables
 */
void Canvas_FT6206::init(){
	Serial.println("FT6206 init");
	
	tft->begin();
	tft->fillScreen(bgColor);
	lastMillis = millis();
	touchSampling = millis();
	scanSampling = millis();
	scanSampleDelay = SCAN_SAMPLING_TIME;

	if(_mode == TFT_PORTRAIT) portrait();
	if(_mode == TFT_LANDSCAPE) landscape();	
	
	ts = new Adafruit_FT6206();
	ts->begin(FT62XX_DEFAULT_THRESHOLD);
}

Point* Canvas_FT6206::getTouchedPoint(){
	if((millis() > touchSampling + TOUCH_SAMPLING_TIME)){
		Point p = ts->getPoint();
		
		if(tft->layoutMode == TFT_PORTRAIT){
			p.x = 240 - p.x;
			p.y = 320 - p.y;
			//p.rotate(POINT_PORTRAIT2);
		}
		if(tft->layoutMode == TFT_LANDSCAPE){
			int oldX = p.x;
			p.x = p.y;
			p.y = 240-oldX;	
			//p.rotate(POINT_LANDSCAPE2);
		}

		touchedPoint.x = p.x;
		touchedPoint.y = p.y;
		touchSampling = millis();
		return &touchedPoint;
	}
	return NULL;
}

#endif
