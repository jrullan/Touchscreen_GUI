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
	Canvas_FT6206(int mode, int color, int tft_cs, int tft_ds, int ts_cs):Canvas(mode, color, tft_cs, tft_ds, ts_cs){
		this->init();
	};
	~Canvas_FT6206(){ delete pendingTouch; };
	void init();
	Point* getTouchedPoint();
private:
	// Keep failed startup owned but unavailable to Canvas::scan().
	Touch* pendingTouch = nullptr;
};

/*
 * Init function initializes 
 * the TFT, the touch chipset
 * and other timing variables
 */
inline void Canvas_FT6206::init(){
	if (initialized || !tft) return;
	tft->begin();
	tft->fillScreen(bgColor);
	lastMillis = millis();
	touchSampling = millis();
	scanSampling = millis();
	scanSampleDelay = SCAN_SAMPLING_TIME;

	if(_mode == TFT_PORTRAIT) portrait();
	if(_mode == TFT_LANDSCAPE) landscape();	
	
	if (!pendingTouch) pendingTouch = new (std::nothrow) Adafruit_FT6206();
	if (!pendingTouch || !pendingTouch->begin(FT62XX_DEFAULT_THRESHOLD)) return;
	ts = pendingTouch;
	pendingTouch = nullptr;
	initialized = true;
}

inline Point* Canvas_FT6206::getTouchedPoint(){
	if (!initialized || !ts || !tft) return NULL;
	if (static_cast<unsigned long>(millis() - touchSampling) > TOUCH_SAMPLING_TIME) {
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
