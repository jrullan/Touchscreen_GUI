#include "Point.h"

Point::Point(void) {
    x = y = 0;
}

Point::Point(int x0, int y0, int z0)
{
    x = x0;
    y = y0;
    z = z0;
}

bool Point::operator==(Point p1)
{
    return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool Point::operator!=(Point p1)
{
    return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

void Point::toLandscape(){
	int newY = x;
	x = 320 - y;
	y = newY;
}

void Point::rotate(char dir){
	int oldX,oldY;
	
	switch(dir){
		case(POINT_PORTRAIT1):
			break;
		case(POINT_LANDSCAPE1):
			oldX = x;
			x = 320 - y;
			y = oldX;
			break;
		case(POINT_STMPE610_LANDSCAPE):
			oldX = x;
			x = 320 - y;
			y = 240-oldX;
			break;
		case(POINT_STMPE610_PORTRAIT):
			x = 240-x;
			//y = 320-y;
			break;	
		case(POINT_PORTRAIT2):
			x = 240-x;
			y = 320-y;
			break;			
		case(POINT_LANDSCAPE2):
			oldX = x;
			x = y;
			y = 240-oldX;	
			break;
	}

}
