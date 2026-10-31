#include "Display.h"
#include "Button.h"
#include "Canvas.h"
#include "allocation_hooks.h"
#include <cassert>

class EmptyCanvas: public Canvas {public: EmptyCanvas():Canvas(TFT_PORTRAIT,BLACK){} void init()override{} Point* getTouchedPoint()override{return nullptr;}};
int main(){
 host_nothrow_new_fail_after=0;
 {Display d(8); assert(d.contents==nullptr); char value[]="safe";d.setText(value,true);d.append(value);d.update();}
 host_nothrow_new_fail_after=0;
 {Button b; assert(b.contents==nullptr);b.setText("safe");b.show();}
 host_nothrow_new_fail_after=0;
 {EmptyCanvas c;assert(c.tft==nullptr);assert(!c.scan());c.redraw();}
 return 0;
}
