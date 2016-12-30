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