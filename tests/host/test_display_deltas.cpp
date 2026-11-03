#include "Display.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstring>
class DeltaCanvas : public Canvas {
public:
    DeltaCanvas():Canvas(TFT_PORTRAIT,BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void setup(Display& d, DeltaCanvas& c) {
    d.setCanvas(&c); d.x=20; d.y=30; d.setSize(180,40);
    d.borderWidth=2; d.fontSize=2; d.setColors(BLACK,WHITE,GREEN);
}
int main() {
    DeltaCanvas c; Display d(8); setup(d,c);
    d.setText((char*)"12:34:56",true); c.tft->ops.clear();
    d.setText((char*)"12:34:57",true);
    assert(c.tft->ops.size()==1 && c.tft->ops[0].name=="char:7");
    const int tx=d.getCenterTextX(d.x,d.w,8), ty=d.getCenterTextY(d.y,d.h);
    assert(c.tft->ops[0].x==tx+7*12 && c.tft->ops[0].y==ty);
    assert(c.tft->ops[0].w==12 && c.tft->ops[0].h==16);
    c.tft->ops.clear(); d.setText((char*)"12:34:57",true); assert(c.tft->ops.empty());
    d.setText((char*)"12:34:59",false); assert(c.tft->ops.empty());
    d.setText((char*)"12:35:00",true); assert(c.tft->ops.size()==3);
    for(const auto& op:c.tft->ops) assert(op.name.compare(0,5,"char:")==0);
    d.contents->text[0]='9'; c.tft->ops.clear(); d.setText(d.contents->text,true);
    assert(c.tft->ops.size()==1 && c.tft->ops[0].name=="char:9");
    d.setText(d.contents->text+1,true); assert(std::strcmp(d.contents->text,"2:35:00")==0);
    c.tft->ops.clear(); d.update(); assert(c.tft->ops.front().name=="fillRect");
    c.tft->ops.clear(); d.forceUpdate(); assert(c.tft->ops.front().name=="fillRect");
    c.tft->ops.clear(); d.show(); assert(c.tft->ops.front().name=="fillRect");
    d.hide(); c.tft->ops.clear(); d.setText((char*)"12:34:56",true); assert(c.tft->ops.empty());
    d.show(); assert(c.tft->ops.front().name=="fillRect");
    d.setText(nullptr,true); c.tft->ops.clear(); d.setText(nullptr,true); assert(c.tft->ops.empty());
    // First render must repaint; failed snapshot allocation must never use deltas.
    for(int failure=0;failure<2;++failure) {
        host_allocation_fail_after=failure; Display oom(8); host_allocation_fail_after=-1; setup(oom,c);
        c.tft->ops.clear(); oom.setText((char*)"12:34:56",true);
        assert(!c.tft->ops.empty() && c.tft->ops.front().name=="fillRect");
        c.tft->ops.clear(); oom.setText((char*)"12:34:57",true);
        assert(!c.tft->ops.empty() && c.tft->ops.front().name=="fillRect");
    }
    host_nothrow_new_fail_after=0;
    { Display oom(8); setup(oom,c); oom.setText(nullptr,true); oom.update(); assert(!oom.contents); }
    d.setText((char*)"12:34:56",true); c.tft->ops.clear(); c.tft->ops.reserve(1024);
    const auto before=host_allocation_calls;
#ifdef HOST_HEAP_MEASURE
    host_heap_begin();
#endif
    for(int i=0;i<1000;++i) {
        c.tft->ops.clear(); d.setText((char*)(i%2?"12:34:56":"12:34:57"),true);
        assert(c.tft->ops.size()==1);
    }
    assert(host_allocation_calls==before);
#ifdef HOST_HEAP_MEASURE
    assert(host_heap_stats().calls==0 && host_heap_stats().retained==0); host_heap_end();
#endif
    std::printf("display_deltas host: PASS; 1000 changed updates, zero allocations; sizeof(Display)=%zu\n",sizeof(Display));
}
