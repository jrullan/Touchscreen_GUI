/*
	
	This class inherits from Canvas to provide a specific 
	touch chipset implementation and reduce the amount of
	included libraries in the compilation.
	
	In theory every chipset should have a correspoding 
	canvas.
	
	This however still requires that each touch chipset
	library is adapted to inherit from Touch.
	
	*/
	
#if !defined(__CANVAS_STMPE610__)
#define __CANVAS_STMPE610__

#include "Canvas.h"
#include "Adafruit_STMPE610.h" 

class Canvas_STMPE610: public Canvas{
	public:
	Canvas_STMPE610(int mode, int color):Canvas(mode, color){};
	~Canvas_STMPE610(){};
	void init();
	Point* getTouchedPoint();
};

/*
 * Init function initializes 
 * the TFT, the touch chipset
 * and other timing variables
 */
void Canvas_STMPE610::init(){
	Serial.println("STMPE init");
	
	Tft.begin();
	Tft.fillScreen(BLACK);
	lastMillis = millis();
	touchSampling = millis();
	scanSampling = millis();
	scanSampleDelay = SCAN_SAMPLING_TIME;

	if(_mode == TFT_PORTRAIT) portrait();
	if(_mode == TFT_LANDSCAPE) landscape();	
	
	ts = new Adafruit_STMPE610(STMPE_CS);
	// Wait until the touchscreen is started.
	// Original driver has an apparent bug,
	// sometimes it did start other times it didn't.
	while(!ts->begin(STMPE_ADDR)){
		Serial.print(".");
	}
}

Point* Canvas_STMPE610::getTouchedPoint(){
	if((millis() > touchSampling + TOUCH_SAMPLING_TIME)){
		Point p = ts->getPoint();		
		p.x = map(p.x, STMPE_MINX, STMPE_MAXX, 0, 240);
		p.y = map(p.y, STMPE_MINY, STMPE_MAXY, 0, 320);	
				
		if(_mode == TFT_LANDSCAPE){			
			int oldX = p.x;
			p.x = 320 - p.y;
			p.y = 240 - oldX;
			//p.rotate(POINT_STMPE610_LANDSCAPE);
		}
		if(_mode == TFT_PORTRAIT){
			p.x = 240 - p.x;
			//p.rotate(POINT_STMPE610_PORTRAIT);
		}
		
		//Serial.print("X: ");Serial.println(p.x);
		//Serial.print("Y: ");Serial.println(p.y);

		touchedPoint.x = p.x;
		touchedPoint.y = p.y;
		touchSampling = millis();
		return &touchedPoint;
	}
	return NULL;
}

#endif
