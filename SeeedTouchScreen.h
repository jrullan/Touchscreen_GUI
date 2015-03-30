#ifndef _SEEEDTOUCHSCREEN_H_
#define _SEEEDTOUCHSCREEN_H_

#include "Point.h"
/*
  SeeedTouchScreen.h - Library for 4-line resistance touch screen.
  Modified by loovee Aug 12, 2012.
  (c) ladyada / adafruit
  Code under MIT License.
*/
#define __PRESURE 10

class TouchScreen {
    private:
    unsigned char _yp, _ym, _xm, _xp;

    public:
    TouchScreen(unsigned char xp, unsigned char yp, unsigned char xm, unsigned char ym);
    bool isTouching(void);
    Point getPoint();

};

#endif