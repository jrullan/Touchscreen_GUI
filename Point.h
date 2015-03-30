#ifndef _POINT_H_
#define _POINT_H_

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