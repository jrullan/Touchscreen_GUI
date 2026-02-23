///////////////////////////////////////////////////////////
//  TouchDriver.h
//  Abstract interface for touch hardware drivers
///////////////////////////////////////////////////////////

#ifndef TOUCHDRIVER_H
#define TOUCHDRIVER_H

#include <Arduino.h>

class TouchDriver {
public:
	virtual ~TouchDriver() {}

	virtual bool begin() = 0;
	virtual bool touched() = 0;
	virtual void readRaw(int16_t &x, int16_t &y, int16_t &z) = 0;

	// Number of simultaneous touch points (1 for resistive, 2+ for capacitive)
	virtual uint8_t touchCount() { return touched() ? 1 : 0; }

	// Whether the mapper should apply median filtering (false if driver already filters)
	virtual bool needsFiltering() { return true; }
};

#endif // TOUCHDRIVER_H
