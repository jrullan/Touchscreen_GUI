///////////////////////////////////////////////////////////
//  TouchDriver_XPT2046.h
//  TouchDriver adapter for XPT2046_Touchscreen
///////////////////////////////////////////////////////////

#ifndef TOUCHDRIVER_XPT2046_H
#define TOUCHDRIVER_XPT2046_H

#include "TouchDriver.h"
#include "XPT2046_Touchscreen.h"

class TouchDriver_XPT2046 : public TouchDriver {
public:
	TouchDriver_XPT2046(uint8_t csPin) : _csPin(csPin), _ts(NULL) {}

	~TouchDriver_XPT2046() {
		if(_ts) delete _ts;
	}

	bool begin() override {
		_ts = new XPT2046_Touchscreen(_csPin);
		return _ts->begin(0);
	}

	bool touched() override {
		return _ts->touched();
	}

	void readRaw(int16_t &x, int16_t &y, int16_t &z) override {
		Point p = _ts->getPoint();
		x = p.x;
		y = p.y;
		z = p.z;
	}

	// XPT2046_Touchscreen already applies besttwoavg() filtering internally
	bool needsFiltering() override { return false; }

private:
	uint8_t _csPin;
	XPT2046_Touchscreen* _ts;
};

#endif // TOUCHDRIVER_XPT2046_H
