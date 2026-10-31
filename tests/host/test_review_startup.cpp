#include <cassert>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "allocation_hooks.h"
#include "lifecycle_drivers.h"
#include "Slider.h"

class ReviewCanvas : public Canvas {
public:
    ReviewCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};

static void sliderVisibility() {
    ReviewCanvas canvas;
    Slider slider;
    canvas.add(&slider, 10, 10, false);
    slider.hide();
    assert(!slider.visible);
    canvas.tft->ops.clear();
    slider.show();
    assert(slider.visible);
    assert(slider.canDraw());
    bool border = false, fill = false;
    for (const auto& op : canvas.tft->ops) {
        border |= op.name == "rect";
        fill |= op.name == "fillRect";
    }
    assert(border && fill);
    assert(canvas.remove(&slider));
}

class TouchProbe : public Widget {
public:
    int calls = 0;
    TouchProbe() { isButton = true; }
    bool checkTouch(Point*) override { ++calls; return true; }
    void show() override {}
    void update() override {}
};

static void sliderPropagation() {
    ReviewCanvas canvas;
    TouchProbe lower;
    Slider slider;
    canvas.add(&lower, 0, 0, false);
    canvas.add(&slider, 10, 10, false); // Reverse dispatch visits Slider first.
    Point inside(20, 20, 1), outside(200, 200, 1);
    host_millis += 1000;
    const auto value = slider.getCV();
    canvas.tft->ops.clear();
    for (bool block : {false, true}) {
        slider.block = block;
        // Deferred legacy behavior: handlerless sliders stop propagation even
        // outside their bounds, independent of block. Do not redesign here.
        assert(!slider.checkTouch(&inside));
        assert(!slider.checkTouch(&outside));
        assert(!canvas.touchWidgets(&inside));
        assert(!canvas.touchWidgets(&outside));
        assert(lower.calls == 0 && slider.getCV() == value);
        assert(canvas.tft->ops.empty());
    }
    slider.hide();
    assert(canvas.touchWidgets(&inside) && lower.calls == 1);
    assert(canvas.remove(&slider) && canvas.remove(&lower));
}

template<class Adapter>
class StartupCanvas : public Adapter {
public:
    StartupCanvas() : Adapter(TFT_PORTRAIT, BLACK, 1, 2, 3) {}
    bool ready() const { return this->initialized; }
};
template<>
class StartupCanvas<Canvas_SEEEDTOUCH> : public Canvas_SEEEDTOUCH {
public:
    StartupCanvas() : Canvas_SEEEDTOUCH(TFT_PORTRAIT, BLACK, 1, 2) {}
    bool ready() const { return initialized; }
};

static void noTouchReads(Canvas& canvas) {
    const int points = LifecycleDriver::pointCalls;
    const int touches = LifecycleDriver::touchCalls;
    host_millis += 1000;
    assert(!canvas.scan()); // Base reference: an adapter-only scan guard is insufficient.
    assert(LifecycleDriver::touchCalls == touches);
    assert(canvas.getTouchedPoint() == nullptr);
    assert(LifecycleDriver::pointCalls == points);
}

template<class Adapter>
static void allocationRetry() {
    const int live = LifecycleDriver::live;
    const int constructed = LifecycleDriver::constructed;
    LifecycleDriver::beginResult = true;
    // Canvas allocates Guitft first; the next nothrow new is the touch driver.
    host_nothrow_new_fail_after = 1;
    {
        StartupCanvas<Adapter> canvas;
        assert(host_nothrow_new_fail_after == -1); // Prove the intended failure fired.
        assert(canvas.tft && !canvas.ts && !canvas.ready());
        assert(LifecycleDriver::constructed == constructed);
        noTouchReads(canvas);
        auto* display = canvas.tft;
        canvas.init();
        assert(canvas.ready() && canvas.ts && canvas.tft == display);
        assert(LifecycleDriver::constructed == constructed + 1);
        Touch* driver = canvas.ts;
        const int begins = static_cast<LifecycleDriver*>(driver)->begins;
        canvas.init(); canvas.init();
        assert(canvas.ts == driver && canvas.tft == display);
        assert(static_cast<LifecycleDriver*>(driver)->begins == begins);
        host_millis += 1000;
        const int touches = LifecycleDriver::touchCalls;
        canvas.scan();
        assert(LifecycleDriver::touchCalls == touches + 1);
    }
    assert(LifecycleDriver::live == live);
}

template<class Adapter>
static void beginRetry() {
    const int live = LifecycleDriver::live;
    const int constructed = LifecycleDriver::constructed;
    LifecycleDriver::beginResult = false;
    {
        alarm(3); // RED must terminate even with the legacy STMPE610 retry loop.
        StartupCanvas<Adapter> canvas;
        alarm(0);
        assert(!canvas.ready());
        auto* driver = LifecycleDriver::latest;
        auto* display = canvas.tft;
        assert(driver && driver->begins == 1 && canvas.ts == nullptr);
        assert(LifecycleDriver::live == live + 1);
        noTouchReads(canvas);
        // Retrying begin must not require another touch-driver allocation.
        host_nothrow_new_fail_after = 0;
        canvas.init();
        assert(host_nothrow_new_fail_after == 0);
        host_nothrow_new_fail_after = -1;
        assert(!canvas.ready() && driver->begins == 2 && canvas.ts == nullptr);
        assert(LifecycleDriver::latest == driver && canvas.tft == display);
        assert(LifecycleDriver::constructed == constructed + 1);
        noTouchReads(canvas);
        LifecycleDriver::beginResult = true;
        canvas.init();
        assert(canvas.ready() && canvas.ts == driver && driver->begins == 3);
        assert(LifecycleDriver::constructed == constructed + 1);
        canvas.init(); canvas.init();
        assert(canvas.ts == driver && canvas.tft == display && driver->begins == 3);
        host_millis += 1000;
        const int points = LifecycleDriver::pointCalls;
        assert(canvas.getTouchedPoint());
        assert(LifecycleDriver::pointCalls == points + 1);
        const int touches = LifecycleDriver::touchCalls;
        canvas.scan();
        assert(LifecycleDriver::touchCalls == touches + 1);
    }
    assert(LifecycleDriver::live == live);
    LifecycleDriver::beginResult = true;
}

template<class Adapter>
static void failedBeginDestruction() {
    const int live = LifecycleDriver::live;
    LifecycleDriver::beginResult = false;
    // Exercise virtual destruction with a still-private, unsuccessful driver.
    alarm(3);
    Canvas* canvas = new StartupCanvas<Adapter>;
    alarm(0);
    assert(canvas->ts == nullptr && LifecycleDriver::live == live + 1);
    noTouchReads(*canvas);
    delete canvas;
    assert(LifecycleDriver::live == live);
    LifecycleDriver::beginResult = true;
}

int main(int argc, char** argv) {
    const char* which = argc > 1 ? argv[1] : "all";
    const bool all = !std::strcmp(which, "all");
    if (all || !std::strcmp(which, "slider")) sliderVisibility();
    if (all || !std::strcmp(which, "propagation")) sliderPropagation();
    if (all || !std::strcmp(which, "allocation")) {
        allocationRetry<Canvas_XPT2046>();
        allocationRetry<Canvas_FT6206>();
        allocationRetry<Canvas_STMPE610>();
        allocationRetry<Canvas_SEEEDTOUCH>();
    }
    if (all || !std::strcmp(which, "xpt_begin")) beginRetry<Canvas_XPT2046>();
    if (all || !std::strcmp(which, "ft_begin")) beginRetry<Canvas_FT6206>();
    if (all || !std::strcmp(which, "stm_begin")) beginRetry<Canvas_STMPE610>();
    if (all || !std::strcmp(which, "failed_destruction")) {
        failedBeginDestruction<Canvas_XPT2046>();
        failedBeginDestruction<Canvas_FT6206>();
        failedBeginDestruction<Canvas_STMPE610>();
    }
    std::puts("review_startup: PASS");
}
