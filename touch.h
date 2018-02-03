/*
 *
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