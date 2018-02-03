/*
 * This is the abstract class that Canvas
 * needs to interact with the touch events
 * on the screen. The need for an abstract
 * class came from the need to support different
 * drivers for the touchscreen. 
 * (Seeedstudio's resistive and Adafruit's capacitive)
 *
 * As of this version, the two classes that
 * implement this class are SeeedTouch and
 * Adafruit_FT6206
 *
 *  Author: Jose Rullan
 *	Release Date: 02/03/2018
 *	Release Version: 0.1 beta
 *
 *	License: Whatever only requires to give credit to original 
 *	author (i.e. Jose Rullan) 
 */
 #if !defined(TOUCH_H)
 #define TOUCH_H
 
 #include "Arduino.h"
 #include "Point.h"
 
 #define TOUCHTYPE_SEEEDSTUDIO_RESISTIVE 	0
 #define TOUCHTYPE_ADAFRUIT_CAPACITIVE 		1
 
 class Touch
 {
	public:
		Touch(void);
		~Touch(void);
		
		virtual bool begin(uint8_t thresh) = 0;
		virtual Point getPoint(void) = 0;
		virtual uint8_t touched(void) = 0;
 };
 
 #endif