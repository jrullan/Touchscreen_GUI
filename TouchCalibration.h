///////////////////////////////////////////////////////////
//  TouchCalibration.h
//  Runtime touch calibration data
///////////////////////////////////////////////////////////

#ifndef TOUCHCALIBRATION_H
#define TOUCHCALIBRATION_H

#include <Arduino.h>

struct TouchCalibration {
	int16_t rawMinX;  // Raw reading that maps to screen X=0 (portrait)
	int16_t rawMaxX;  // Raw reading that maps to screen X=screenWidth (portrait)
	int16_t rawMinY;  // Raw reading that maps to screen Y=0 (portrait)
	int16_t rawMaxY;  // Raw reading that maps to screen Y=screenHeight (portrait)
	bool swapXY;      // Swap raw X/Y axes before mapping
};

namespace TouchCal {
	// Default calibration for XPT2046 (derived from Canvas_XPT2046 constants)
	static const TouchCalibration XPT2046_DEFAULT = {
		3894, 261,   // rawMinX, rawMaxX (maps to 0..239 in portrait)
		3891, 320,   // rawMinY, rawMaxY (maps to 0..319 in portrait)
		true         // swapXY — XPT2046 axes are transposed relative to ILI9341
	};
}

#endif // TOUCHCALIBRATION_H
