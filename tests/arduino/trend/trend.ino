#include <Canvas_XPT2046.h>
Canvas_XPT2046 canvas(TFT_PORTRAIT, BLACK, 16, 15, 0);
#include <Trend.h>
#include <MultiTrend.h>
Trend trend(140,80,0,50,100,16);
MultiTrend multi(140,80,0,50,100,3,16);
void setup(){canvas.add(&trend,8,8,false); canvas.add(&multi,8,100,false); trend.show(); multi.show(); trend.addValue(30); multi.addValue(0,40);}
void loop(){canvas.scan();}
