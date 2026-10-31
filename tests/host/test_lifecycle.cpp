#include <cassert>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <unistd.h>
#include "allocation_hooks.h"
#include "Canvas.h"
#include "Screen.h"
#include "Widget.h"
#include "lifecycle_drivers.h"
#include "Slider.h"

class TestCanvas : public Canvas {
public:
    TestCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
    void setScanTime(unsigned long value) { scanSampling = value; }
};
class TestTouch : public Touch {
public:
    static int destroyed;
    ~TestTouch() { ++destroyed; }
    bool begin(uint8_t) override { return true; }
    Point getPoint() override { return Point(); }
    int calls = 0;
    uint8_t touched() override { ++calls; return 0; }
};
int TestTouch::destroyed = 0;

class Probe : public Widget {
public:
    int touches = 0, shows = 0;
    Probe() { isButton = true; setSize(10,10); }
    bool checkTouch(Point*) override { ++touches; return true; }
    void show() override { ++shows; }
    void update() override {}
};

template<class DriverCanvas>
class TimedCanvas : public DriverCanvas {
public:
    using DriverCanvas::DriverCanvas;
    void setTouchTime(unsigned long value) { this->touchSampling = value; }
};

template<class C> void checkTouchRollover(C& canvas) {
    canvas.setTouchTime(ULONG_MAX - 10UL);
    host_millis = ULONG_MAX - 5UL;
    assert(canvas.getTouchedPoint() == nullptr);
    host_millis = 10;
    assert(canvas.getTouchedPoint() == nullptr);
    host_millis = 20;
    assert(canvas.getTouchedPoint() != nullptr);
    host_millis = 0;
}

int main() {
    assert(std::has_virtual_destructor<Touch>::value);
    {
        TestCanvas canvas;
        canvas.ts = new TestTouch;
    }
    assert(TestTouch::destroyed == 1);
    {
        TestCanvas canvas;
        assert(canvas.ts == nullptr);
        assert(canvas.x == 0 && canvas.y == 0);
        assert(canvas.w == 240 && canvas.h == 320);
        assert(canvas.touchBufferIndex == 0);
        for (int i=0; i<TOUCH_BUFFER_SIZE; ++i) {
            assert(canvas.xTouchBuffer[i] == 0 && canvas.yTouchBuffer[i] == 0);
        }
        assert(!canvas.scan());
        assert(!canvas.inBounds(nullptr));
        assert(canvas.pop() == nullptr);
        canvas.add(nullptr, 0, 0);
        canvas.setScreen(nullptr);
        assert(canvas.widgets.count() == 0);
        Screen screen;
        assert(screen.canvas == nullptr && screen.x == 0 && screen.w == 0);
        screen.add(nullptr,0,0);
        screen.show();
        screen.hide();
        screen.touchWidgets(nullptr);
    }
    {
        TestCanvas canvas;
        Probe probes[300];
        Point point;
        for (auto& probe : probes) canvas.add(&probe,0,0,false);
        assert(canvas.touchWidgets(&point));
        for (auto& probe : probes) assert(probe.touches == 1);
        while (canvas.pop()) {}
        Screen screen(&canvas,0,0,240,320);
        for (auto& probe : probes) screen.add(&probe,0,0);
        canvas.setScreen(&screen);
        screen.touchWidgets(&point);
        for (auto& probe : probes) assert(probe.touches == 2 && probe.shows > 0);
        screen.hide();
        for (auto& probe : probes) assert(!probe.visible);
        canvas.setScreen(nullptr);
    }
    assert(!std::is_copy_constructible<TestCanvas>::value);
    assert(!std::is_copy_constructible<Screen>::value);
    assert(!std::is_copy_constructible<StackArray<int>>::value);
    {
        TestCanvas canvas;
        Probe probe;
        canvas.add(&probe,0,0,false);
        canvas.add(&probe,1,1,false);
        assert(canvas.widgets.count() == 1);
        assert(canvas.remove(&probe));
        assert(!canvas.remove(&probe));
        assert(!probe.canDraw());
        canvas.add(&probe,0,0,false);
        assert(canvas.pop() == &probe);
        assert(!probe.canDraw());
        Screen screen;
        screen.add(&probe,1,2);
        screen.setCanvas(&canvas);
        canvas.setScreen(&screen);
        assert(probe.canDraw());
        assert(screen.remove(&probe));
        assert(!probe.canDraw());
        screen.add(&probe,1,2);
        screen.detach();
        assert(canvas.currentScreen == nullptr && screen.canvas == nullptr);
        assert(!probe.canDraw());
        assert(screen.pop() == &probe && screen.pop() == nullptr);
    }
    {
        StackArray<int> registry(1);
        assert(registry.push(17));
        host_allocation_fail_after = 0;
        alarm(3); // legacy allocation-error LED loop must not hang the host suite
        assert(!registry.push(23));
        alarm(0);
        assert(registry.count() == 1 && registry[0] == 17);
        assert(registry.push(23));
        assert(registry.pop() == 23 && registry.pop() == 17);
        assert(registry.pop() == 0 && registry.peek() == 0);
        assert(registry[-1] == 0 && registry[0] == 0);
    }
    {
        TimedCanvas<Canvas_XPT2046> xpt(TFT_PORTRAIT,BLACK,1,2,3);
        TimedCanvas<Canvas_FT6206> ft(TFT_PORTRAIT,BLACK,1,2,3);
        TimedCanvas<Canvas_STMPE610> stm(TFT_PORTRAIT,BLACK,1,2,3);
        TimedCanvas<Canvas_SEEEDTOUCH> seeed(TFT_PORTRAIT,BLACK,1,2);
        checkTouchRollover(xpt); checkTouchRollover(ft);
        checkTouchRollover(stm); checkTouchRollover(seeed);
        Canvas* canvases[] = {&xpt,&ft,&stm,&seeed};
        for (Canvas* c : canvases) {
            Touch* original = c->ts;
            c->init(); c->init();
            assert(c->ts == original);
            assert(LifecycleDriver::live == 4);
            assert(static_cast<LifecycleDriver*>(c->ts)->begins <= 1);
        }
        for (Canvas* c : canvases) {
            delete c->ts; c->ts = nullptr;
            assert(c->getTouchedPoint() == nullptr);
        }
    }
    assert(LifecycleDriver::live == 0);
    {
        TestCanvas canvas;
        auto* touch = new TestTouch;
        canvas.ts = touch;
        canvas.setScanSampling(100);
        canvas.setScanTime(70000UL);
        host_millis = 70020UL;
        canvas.scan(); assert(touch->calls == 0);
        host_millis = 70101UL;
        canvas.scan(); assert(touch->calls == 1);
        canvas.setScanTime(ULONG_MAX - 50UL);
        host_millis = ULONG_MAX - 10UL;
        canvas.scan(); assert(touch->calls == 1);
        host_millis = 20;
        canvas.scan(); assert(touch->calls == 1);
        host_millis = 60;
        canvas.scan(); assert(touch->calls == 2);
        host_millis = 0;
    }
    {
        alignas(Slider) unsigned char storage[sizeof(Slider)];
        std::memset(storage, 0xa5, sizeof(storage));
        Slider* slider = new (storage) Slider;
        assert(slider->eventHandler == nullptr && slider->lastMillis == 0);
        assert(!slider->checkTouch(nullptr));
        slider->show(); slider->update(); slider->drawBorder(); slider->drawCV(5);
        slider->~Slider();
    }
    {
        host_allocation_fail_after = 0;
        StackArray<int> initiallyEmpty;
        assert(initiallyEmpty.count() == 0 && initiallyEmpty.push(7));
        StackArray<int> invalid(-1);
        assert(invalid.count() == 0 && invalid.push(9));
        TestCanvas canvas;
        Probe probes[3];
        canvas.add(&probes[0],0,0,false);
        canvas.add(&probes[1],0,0,false);
        host_allocation_fail_after = 0;
        canvas.add(&probes[2],99,99,false);
        assert(canvas.widgets.count() == 2 && !probes[2].canDraw());
        assert(probes[2].x == 0 && probes[2].y == 0);
        Screen screen(&canvas,0,0,240,320);
        Probe screenProbes[3];
        screen.add(&screenProbes[0],0,0); screen.add(&screenProbes[1],0,0);
        host_allocation_fail_after = 0;
        screen.add(&screenProbes[2],99,99);
        assert(screen.widgets.count() == 2 && screenProbes[2].x == 0);
        canvas.widgets.push(nullptr);
        screen.widgets.push(nullptr);
        Point point;
        canvas.showWidgets(); canvas.touchWidgets(&point);
        canvas.setScreen(&screen);
        screen.touchWidgets(&point); screen.hide();
        canvas.setScreen(nullptr);
        int calls = probes[0].touches;
        canvas.touchWidgets(nullptr);
        screen.touchWidgets(nullptr);
        assert(probes[0].touches == calls);
    }
    {
        TestCanvas canvas;
        delete canvas.tft; canvas.tft = nullptr;
        Probe probe;
        canvas.add(&probe,0,0);
        canvas.showWidgets(); canvas.redraw();
        assert(probe.shows == 0);
        assert(!canvas.scan() && !canvas.inBounds(nullptr));
        canvas.portrait(); canvas.landscape();
        assert(canvas.pop() == &probe);
    }
    std::puts("lifecycle: PASS");
}
