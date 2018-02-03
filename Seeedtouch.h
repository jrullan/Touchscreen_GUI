#ifndef SEEEDSTUDIOTOUCH_H
#define SEEEDSTUDIOTOUCH_H

#include "Touch.h"
//#include "Point.h" 

/*
  SeeedTouchScreen.h - Library for 4-line resistance touch screen.
  Modified by loovee Aug 12, 2012.
  (c) ladyada / adafruit
  Code under MIT License.
*/
#define __PRESURE 10

class SeeedstudioTouch : public Touch
{   
    public:
    SeeedstudioTouch(unsigned char xp, unsigned char yp, unsigned char xm, unsigned char ym);
	bool begin(uint8_t threshold = 0){};
    bool isTouching(void);
	uint8_t touched(void);
    Point getPoint();

	private:
    unsigned char _yp, _ym, _xm, _xp;	
};

#endif