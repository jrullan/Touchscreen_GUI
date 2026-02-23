///////////////////////////////////////////////////////////
//  TouchMapper.cpp
//  Coordinate mapping, filtering, and touch state machine
///////////////////////////////////////////////////////////

#include "TouchMapper.h"

TouchMapper::TouchMapper(TouchDriver* driver, const TouchCalibration& cal)
	: _driver(driver), _cal(cal), _rotation(0),
	  _state(TOUCH_IDLE), _pressStartTime(0) {}

void TouchMapper::begin() {
	_driver->begin();
}

void TouchMapper::poll() {
	bool isTouched = _driver->touched();

	switch(_state) {
		case TOUCH_IDLE:
			if(isTouched) {
				readFiltered();
				_pressStart = _current;
				_pressStartTime = millis();
				_state = TOUCH_PRESSED;
			}
			break;

		case TOUCH_PRESSED:
			if(isTouched) {
				readFiltered();
				_state = TOUCH_HELD;
			} else {
				_state = TOUCH_RELEASED;
			}
			break;

		case TOUCH_HELD:
			if(isTouched) {
				readFiltered();
			} else {
				_state = TOUCH_RELEASED;
			}
			break;

		case TOUCH_RELEASED:
			_state = TOUCH_IDLE;
			break;
	}
}

unsigned long TouchMapper::pressDuration() const {
	if(_state == TOUCH_IDLE) return 0;
	return millis() - _pressStartTime;
}

void TouchMapper::setOrientation(uint8_t rotation) {
	_rotation = rotation % 4;
}

void TouchMapper::setCalibration(const TouchCalibration& cal) {
	_cal = cal;
}

void TouchMapper::readFiltered() {
	int16_t rawX, rawY, rawZ;

	if(_driver->needsFiltering()) {
		// Median-of-3 filter for noisy resistive touchscreens
		int16_t xs[3], ys[3];
		for(int i = 0; i < 3; i++) {
			_driver->readRaw(xs[i], ys[i], rawZ);
		}
		rawX = median3(xs[0], xs[1], xs[2]);
		rawY = median3(ys[0], ys[1], ys[2]);
	} else {
		_driver->readRaw(rawX, rawY, rawZ);
	}

	int screenX, screenY;
	mapRawToScreen(rawX, rawY, screenX, screenY);
	_current.x = screenX;
	_current.y = screenY;
	_current.z = rawZ;
}

void TouchMapper::mapRawToScreen(int16_t rawX, int16_t rawY, int& screenX, int& screenY) {
	// Apply axis swap if needed
	int16_t rx = _cal.swapXY ? rawY : rawX;
	int16_t ry = _cal.swapXY ? rawX : rawY;

	// Map to portrait base coordinates (240x320)
	int px = map(rx, _cal.rawMinX, _cal.rawMaxX, 0, 239);
	int py = map(ry, _cal.rawMinY, _cal.rawMaxY, 0, 319);

	// Apply display rotation
	switch(_rotation) {
		case 0:  // Portrait (setRotation 0)
			screenX = px;
			screenY = py;
			break;
		case 1:  // Landscape CW (setRotation 1)
			screenX = py;
			screenY = 239 - px;
			break;
		case 2:  // Portrait 180 (setRotation 2)
			screenX = 239 - px;
			screenY = 319 - py;
			break;
		case 3:  // Landscape CCW (setRotation 3)
			screenX = 319 - py;
			screenY = px;
			break;
		default:
			screenX = px;
			screenY = py;
			break;
	}
}

int16_t TouchMapper::median3(int16_t a, int16_t b, int16_t c) {
	if(a > b) {
		if(b > c) return b;
		if(a > c) return c;
		return a;
	} else {
		if(a > c) return a;
		if(b > c) return c;
		return b;
	}
}
