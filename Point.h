#ifndef _POINT_H_
#define _POINT_H_

#define POINT_PORTRAIT1 0
#define POINT_LANDSCAPE1 1
#define POINT_PORTRAIT2 2
#define POINT_LANDSCAPE2 3

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
	void rotate(char dir = 0);
};

#endif
