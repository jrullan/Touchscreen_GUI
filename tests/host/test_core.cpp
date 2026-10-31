#include "Widget.h"
#include "Canvas.h"
#include "Screen.h"
#include <cassert>
#include <climits>
#include <cstring>
class TestWidget: public Widget {public: bool checkTouch(Point*) override{return true;} void show()override{} void update()override{} Canvas* context(){return myCanvas;}};
int main(){
 TestWidget w;
 assert(w.context()==nullptr);
 assert(w.x==0 && w.y==0 && w.w==0 && w.h==0);
 w.hide(); assert(!w.visible);
 assert(std::strcmp(Widget::getType(&w),"widget")==0);
 w.type=0x7f; assert(std::strcmp(Widget::getType(&w),"unknown")==0);
 char buf[32]; Widget::convert_str(INT_MIN,buf); assert(std::strcmp(buf,"-2147483648")==0);
 assert(Widget::getTextLength(nullptr)==0);
 assert(std::strcmp(Widget::getType(nullptr),"unknown")==0);
 assert(Widget::getIntLength(INT_MIN)==11);
 return 0;
}
