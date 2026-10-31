#include "Gauge.h"
#include <cassert>
#include <climits>
int main(){
 Gauge g(40,120,0,50,100);
 g.setCV(map(75,0,100,0,100),false);
 assert(g.getCV()==75);
 g.setCV(LONG_MIN,false);assert(g.getCV()==0);
 g.setCV(LONG_MAX,false);assert(g.getCV()==100);
 g.setCV(-1,false);assert(g.getCV()==0);
 g.setLimits(0,0,UINT_MAX);g.setCV(UINT_MAX,false);assert(g.getCV()==UINT_MAX);
}
