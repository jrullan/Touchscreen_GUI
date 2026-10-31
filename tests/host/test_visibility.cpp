#include "Canvas.h"
#include "Trend.h"
#include "MultiTrend.h"
#include "Gauge.h"
#include "Dial.h"
#include <cassert>
class VisibilityCanvas: public Canvas {public:VisibilityCanvas():Canvas(TFT_PORTRAIT,BLACK){}void init()override{}Point* getTouchedPoint()override{return nullptr;}};
int main(){VisibilityCanvas c;Trend t(140,80,0,50,100,16);MultiTrend m(140,80,0,50,100,3,16);Gauge g(80,120,0,50,100);Dial d(40,0,50,100);
 Widget* ws[]={&t,&m,&g,&d};
 for(auto* w:ws){c.add(w,5,5,false);w->hide();c.tft->ops.clear();w->show();assert(w->visible);assert(!c.tft->ops.empty());c.remove(w);}
}
