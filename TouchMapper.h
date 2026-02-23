///////////////////////////////////////////////////////////
//  TouchMapper.h
//  Coordinate mapping, filtering, and touch state machine
///////////////////////////////////////////////////////////

#ifndef TOUCHMAPPER_H
#define TOUCHMAPPER_H

#include <Arduino.h>
#include "Point.h"
#include "TouchDriver.h"
#include "TouchCalibration.h"

enum TouchState : uint8_t {
	TOUCH_IDLE,
	TOUCH_PRESSED,
	TOUCH_HELD,
	TOUCH_RELEASED
};

class TouchMapper {
public:
	TouchMapper(TouchDriver* driver, const TouchCalibration& cal);

	void begin();
	void poll();

	TouchState state() const { return _state; }
	Point currentPoint() const { return _current; }
	Point pressPoint() const { return _pressStart; }
	unsigned long pressDuration() const;

	void setOrientation(uint8_t rotation);
	void setCalibration(const TouchCalibration& cal);

private:
	TouchDriver* _driver;
	TouchCalibration _cal;
	uint8_t _rotation;

	TouchState _state;
	Point _current;
	Point _pressStart;
	unsigned long _pressStartTime;

	void readFiltered();
	void mapRawToScreen(int16_t rawX, int16_t rawY, int& screenX, int& screenY);
	static int16_t median3(int16_t a, int16_t b, int16_t c);
};

#endif // TOUCHMAPPER_H
