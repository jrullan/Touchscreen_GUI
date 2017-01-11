#ifndef _POINT_H_
#define _POINT_H_

//#include <Arduino.h>
//#include <avr/pgmspace.h>

class Point {
    public:
    int x, y, z;

    public:
    Point(void);
    Point(int x, int y, int z);
    bool operator==(Point);
    bool operator!=(Point);
    
    //New method
    void toLandscape();

};

#endif
