///////////////////////////////////////////////////////////
//  Canvas_Touch.h
//  Unified Canvas subclass using TouchMapper
//
//  Works with any touch controller via the TouchDriver
//  interface. Supports all 4 ILI9341 display orientations.
//
//  Usage:
//    #include <Canvas_Touch.h>
//    #include <TouchDriver_XPT2046.h>
//    #include <TouchCalibration.h>
//
//    TouchDriver_XPT2046 touchDriver(TS_CS);
//    TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
//    Canvas_Touch canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DC, &mapper);
///////////////////////////////////////////////////////////

#ifndef CANVAS_TOUCH_H
#define CANVAS_TOUCH_H

#include "Canvas.h"
#include "TouchMapper.h"

class Canvas_Touch : public Canvas {
public:
	Canvas_Touch(int mode, int color, int tft_cs, int tft_ds, TouchMapper* mapper)
		: Canvas(mode, color, tft_cs, tft_ds) {
		_mapper = mapper;
		this->init();
	}

	~Canvas_Touch() {}

	void init() override {
		tft->begin();
		tft->fillScreen(bgColor);
		lastMillis = millis();
		touchSampling = millis();
		scanSampling = millis();
		scanSampleDelay = SCAN_SAMPLING_TIME;

		if(_mode == TFT_PORTRAIT) portrait();
		else if(_mode == TFT_LANDSCAPE) landscape();
		else if(_mode == TFT_PORTRAIT2) portrait2();
		else if(_mode == TFT_LANDSCAPE2) landscape2();

		if(_mapper) {
			_mapper->begin();
			// Map layout mode to ILI9341 rotation for the touch mapper
			uint8_t rotation = 0;
			if(_mode == TFT_PORTRAIT) rotation = 0;
			else if(_mode == TFT_LANDSCAPE) rotation = 3;
			else if(_mode == TFT_PORTRAIT2) rotation = 2;
			else if(_mode == TFT_LANDSCAPE2) rotation = 1;
			_mapper->setOrientation(rotation);
		}
	}

	// Not used when _mapper is set; scan() uses the mapper path directly
	Point* getTouchedPoint() override {
		return NULL;
	}
};

#endif // CANVAS_TOUCH_H
