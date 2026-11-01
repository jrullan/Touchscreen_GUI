#include <cassert>
#include <cstdio>
#include <type_traits>
#include "Touch.h"
// Local hardware seam: production adapter is exercised, not copied.
#define _XPT2046_Touchscreen_h_
class XPT2046_Touchscreen : public Touch {
public:
    static int live, constructed, begins, touches, reads;
    static bool result, down;
    explicit XPT2046_Touchscreen(uint8_t, uint8_t = 255) { ++live; ++constructed; }
    ~XPT2046_Touchscreen() override { --live; }
    bool begin(uint8_t threshold) override { assert(threshold == 0); ++begins; return result; }
    uint8_t touched() override { ++touches; return down; }
    Point getPoint() override { ++reads; return Point(12,34,56); }
};
int XPT2046_Touchscreen::live=0, XPT2046_Touchscreen::constructed=0;
int XPT2046_Touchscreen::begins=0, XPT2046_Touchscreen::touches=0, XPT2046_Touchscreen::reads=0;
bool XPT2046_Touchscreen::result=false, XPT2046_Touchscreen::down=true;
#include "TouchDriver_XPT2046.h"
static bool displayDeselected=false;
static void observeDigitalWrite(int pin, int level) {
    if (pin==1) displayDeselected=(level==HIGH);
    digitalWrite(pin,level);
}
#define digitalWrite observeDigitalWrite
#include "Canvas_Touch.h"
#undef digitalWrite
#include "Widget.h"
#include "Screen.h"
#include "allocation_hooks.h"
class FakeDriver : public TouchDriver {
public:
    int begins=0, touches=0, reads=0;
    bool result=false, down=true;
    bool begin() override { ++begins; return result; }
    bool touched() override { assert(displayDeselected); ++touches; return down; }
    void readRaw(int16_t& x,int16_t& y,int16_t& z) override { assert(displayDeselected); ++reads; x=20; y=30; z=50; }
    bool needsFiltering() override { return false; }
};
static void startup() {
    const TouchCalibration cal={0,239,0,319,false};
    host_millis=0;
    Canvas_Touch empty(TFT_PORTRAIT,BLACK,nullptr,1,2);
    assert(!empty.isReady() && !empty.scan() && empty.ts==nullptr);
    FakeDriver driver;
    TouchMapper mapper(&driver,cal);
    {
        Canvas_Touch canvas(TFT_PORTRAIT,BLACK,&mapper,1,2);
        assert(!canvas.isReady() && canvas.ts==nullptr && driver.begins==1);
        host_millis=101;
        assert(!canvas.scan() && canvas.getTouchedPoint()==nullptr);
        assert(driver.reads==0 && driver.touches==0);
        driver.result=true;
        canvas.init();
        assert(canvas.isReady() && canvas.ts && driver.begins==2);
        Touch* bridge=canvas.ts;
        auto draws=canvas.tft->ops.size();
        canvas.init(); canvas.init();
        assert(canvas.ts==bridge && canvas.tft->ops.size()==draws && driver.begins==2);
        assert(!std::is_copy_constructible<Canvas_Touch>::value);
    }
    // Driver and mapper are borrowed, still usable after canvas destruction.
    assert(mapper.isReady() && driver.begin());
}
class Probe : public Widget {
public:
    std::vector<int>& order; int id; bool propagate=true; Point point;
    Probe(std::vector<int>& order,int id) : order(order),id(id) { isButton=true; }
    bool checkTouch(Point* p) override { point=*p; order.push_back(id); return propagate; }
    void show() override {}
    void update() override {}
};
static void dispatch() {
    host_millis=0;
    FakeDriver driver; driver.result=true;
    const TouchCalibration cal={0,239,0,319,false};
    TouchMapper mapper(&driver,cal);
    Canvas_Touch canvas(TFT_PORTRAIT,BLACK,&mapper,1,2);
    std::vector<int> order;
    Probe a(order,1),b(order,2),c(order,3);
    canvas.add(&a,0,0,false); canvas.add(&b,0,0,false);
    Screen screen(&canvas,0,0,240,320); screen.add(&c,0,0);
    canvas.setScreen(&screen,true);
    host_millis=100;
    assert(!canvas.scan() && driver.touches==0);
    displayDeselected=false;
    host_millis=101;
    assert(canvas.scan());
    assert(driver.touches==1 && driver.reads==1);
    assert((order==std::vector<int>{2,1,3}));
    assert(a.point.x==20 && a.point.y==30 && a.point.z==50);
    assert(!canvas.scan() && driver.touches==1);
    order.clear(); b.propagate=false;
    displayDeselected=false;
    host_millis=202;
    assert(canvas.scan() && mapper.state()==TOUCH_HELD);
    assert((order==std::vector<int>{2}));
    driver.down=false; order.clear();
    host_millis=303;
    assert(!canvas.scan() && mapper.state()==TOUCH_RELEASED && order.empty());
    host_millis=404;
    assert(!canvas.scan() && mapper.state()==TOUCH_IDLE && order.empty());
    // Faster scan cadence must still honor the legacy point sampling gate.
    canvas.setScanSampling(0); driver.down=true;
    host_millis=405;
    assert(!canvas.scan() && driver.touches==4);
    host_millis=430;
    assert(canvas.scan() && driver.touches==5);
    canvas.setScreen(nullptr,false);
}
static void oom() {
    host_millis=0;
    const TouchCalibration cal={0,239,0,319,false};
    {
        FakeDriver driver; driver.result=true;
        TouchMapper mapper(&driver,cal);
        host_nothrow_new_fail_after=1; // TFT succeeds; bridge allocation fails.
        Canvas_Touch canvas(TFT_PORTRAIT,BLACK,&mapper,1,2);
        assert(canvas.tft && !canvas.ts && !canvas.isReady() && driver.begins==0);
        host_millis=101;
        assert(!canvas.scan() && driver.reads==0);
        auto draws=canvas.tft->ops.size();
        canvas.init();
        assert(canvas.isReady() && canvas.ts && driver.begins==1);
        assert(canvas.tft->ops.size()==draws);
    }
    {
        FakeDriver driver; driver.result=true;
        TouchMapper mapper(&driver,cal);
        host_nothrow_new_fail_after=0; // TFT itself unavailable, retry recovers.
        Canvas_Touch canvas(TFT_PORTRAIT,BLACK,&mapper,1,2);
        assert(!canvas.tft && !canvas.ts && !canvas.isReady() && driver.begins==0);
        canvas.init();
        assert(canvas.isReady() && canvas.tft && canvas.ts && driver.begins==1);
    }
    {
        FakeDriver driver;
        TouchMapper mapper(&driver,cal);
        Canvas_Touch canvas(TFT_PORTRAIT,BLACK,&mapper,1,2);
        auto* original=canvas.tft;
        auto draws=canvas.tft->ops.size();
        host_nothrow_new_fail_after=0;
        canvas.init(); // Retry a failed begin: no TFT/bridge reallocation.
        assert(host_nothrow_new_fail_after==0 && driver.begins==2);
        assert(canvas.tft==original && canvas.tft->ops.size()==draws && !canvas.ts);
        host_nothrow_new_fail_after=-1;
        // Destructor must delete the unpublished bridge, not the borrowed driver.
    }
}
static void rotations() {
    host_millis=0;
    FakeDriver driver; driver.result=true;
    const TouchCalibration cal={0,239,0,319,false};
    TouchMapper mapper(&driver,cal);
    Canvas_Touch canvas(TFT_LANDSCAPE,BLACK,&mapper,1,2);
    const int xs[]={20,30,219,289}, ys[]={30,219,289,20};
    // Constructor mode=1 means historical physical rotation=3, NOT rotation=1.
    host_millis+=101;
    assert(canvas.scan() && canvas.touchedPoint.x==289 && canvas.touchedPoint.y==20);
    for (uint8_t r=0;r<4;++r) {
        canvas.setRotation(r);
        host_millis+=101;
        assert(canvas.scan());
        assert(canvas.touchedPoint.x==xs[r] && canvas.touchedPoint.y==ys[r]);
        assert(canvas.w==((r&1)?320:240) && canvas.h==((r&1)?240:320));
        assert(canvas.tft->width()==canvas.w && canvas.tft->height()==canvas.h);
        Point p(canvas.w-1,canvas.h-1,1); assert(canvas.inBounds(&p));
        ++p.x; assert(!canvas.inBounds(&p));
        p.x=0; ++p.y; assert(!canvas.inBounds(&p));
        p.y=0; p.x=-1; assert(!canvas.inBounds(&p));
    }
    Canvas* base=&canvas;
    base->portrait(); host_millis+=101;
    assert(canvas.scan() && canvas.touchedPoint.x==20 && canvas.w==240 && canvas.h==320);
    base->landscape(); host_millis+=101;
    assert(canvas.scan() && canvas.touchedPoint.x==289 && canvas.w==320 && canvas.h==240);
    canvas.setRotation(5); host_millis+=101;
    assert(canvas.scan() && canvas.touchedPoint.x==30 && canvas.touchedPoint.y==219);
    assert(driver.begins==1);
}
int main() {
    startup();
    dispatch();
    oom();
    rotations();
    using HW=XPT2046_Touchscreen;
    {
        host_nothrow_new_fail_after=0;
        TouchDriver_XPT2046 driver(3);
        assert(host_nothrow_new_fail_after==0); // Value-owned hardware, no allocation.
        host_nothrow_new_fail_after=-1;
        assert(!std::is_copy_constructible<TouchDriver_XPT2046>::value);
        int16_t x=99,y=99,z=99;
        assert(!driver.touched());
        driver.readRaw(x,y,z);
        assert(x==0 && y==0 && z==0 && HW::reads==0 && HW::touches==0);
        assert(!driver.begin());
        assert(!driver.touched());
        driver.readRaw(x,y,z);
        assert(HW::reads==0 && HW::touches==0);
        HW::result=true;
        assert(driver.begin() && driver.begin());
        assert(HW::begins==2 && HW::constructed==1 && HW::live==1);
        assert(driver.touched());
        driver.readRaw(x,y,z);
        assert(x==12 && y==34 && z==56);
    }
    assert(HW::live==0);
    std::puts("touch canvas: PASS");
}
