#include "IconButton.h"
#include <cassert>
#include <cstdio>

static const uint16_t unpressed[] PROGMEM = {RED, BLACK, BLUE, GREEN};
static const uint16_t pressed[] PROGMEM = {GREEN, BLUE, BLACK, RED};
static int calls = 0;
static void countTouch(Button*) { ++calls; }

static void test_default_debounce() {
    host_millis = 1000;
    IconButton button(2, 2, unpressed, pressed);
    button.setEventHandler(countTouch);
    Point inside(1, 1, 1);
    calls = 0;
    host_millis = 1001;
    button.checkTouch(&inside);
    assert(calls == 0 && !button.touched);
    host_millis = 1499;
    button.checkTouch(&inside);
    assert(calls == 0);
    host_millis = 1500;
    button.checkTouch(&inside);
    assert(calls == 0); // Existing Button contract is strictly greater than.
    host_millis = 1501;
    button.checkTouch(&inside);
    assert(calls == 1 && button.touched);
    host_millis = 1601;
    button.checkTouch(&inside);
    assert(calls == 1);
    host_millis = 2002;
    button.checkTouch(&inside);
    assert(calls == 2 && !button.touched); // Held level can repeat; no edge detector.
}

class IconCanvas : public Canvas {
public:
    IconCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};

static void test_explicit_zero_and_custom_debounce() {
    host_millis = 3000;
    IconButton button(2, 2, unpressed, pressed);
    button.setEventHandler(countTouch);
    Point inside(1, 1, 1);
    calls = 0;
    button.setDebounce(0);
    button.checkTouch(&inside);
    assert(calls == 0); // Even zero retains the inherited strict comparison.
    host_millis = 3001;
    button.checkTouch(&inside);
    assert(calls == 1 && button.touched);
    button.checkTouch(&inside);
    assert(calls == 1);
    host_millis = 3002;
    button.checkTouch(&inside);
    assert(calls == 2 && !button.touched);
    button.setDebounce(10);
    host_millis = 3012;
    button.checkTouch(&inside);
    assert(calls == 2);
    host_millis = 3013;
    button.checkTouch(&inside);
    assert(calls == 3 && button.touched);
}

static uint16_t pixel(IconCanvas& canvas, int x, int y) {
    return canvas.tft->pixels[y * canvas.tft->width() + x];
}

static void test_touch_state_and_explicit_rendering() {
    host_millis = 5000;
    IconCanvas canvas;
    IconButton button(2, 2, unpressed, pressed);
    button.setCanvas(&canvas);
    button.x = 10;
    button.y = 20;
    Point inside(11, 21, 1);
    button.show();
    assert(pixel(canvas, 10, 20) == RED);
    const auto before = canvas.tft->pixels;
    const auto last = button.lastMillis;
    host_millis = 6000;
    assert(!button.checkTouch(&inside));
    assert(!button.touched && button.lastMillis == last);
    assert(canvas.tft->pixels == before); // No callback means no state transition.

    calls = 0;
    button.setEventHandler(countTouch);
    assert(!button.checkTouch(&inside));
    assert(calls == 1 && button.touched && button.lastMillis == 6000);
    assert(canvas.tft->pixels == before); // A counting callback does not redraw.
    button.update();
    assert(pixel(canvas, 10, 20) == GREEN);
    assert(pixel(canvas, 11, 20) == BLUE);
    assert(pixel(canvas, 10, 21) == BLUE); // Transparent black retains old pixel.
    assert(pixel(canvas, 11, 21) == RED);

    // Inherited callback uses virtual update() to draw the newly toggled state.
    button.setEventHandler([](Button* b) { ++calls; b->update(); });
    host_millis = 6501;
    button.checkTouch(&inside);
    assert(calls == 2 && !button.touched);
    assert(pixel(canvas, 10, 20) == RED);
    host_millis = 7002;
    button.checkTouch(&inside);
    assert(calls == 3 && button.touched);
    assert(pixel(canvas, 10, 20) == GREEN);
    button.useTransparency = false;
    button.update();
    assert(pixel(canvas, 10, 21) == BLACK);

    IconButton fallback(2, 2, unpressed);
    fallback.setCanvas(&canvas);
    fallback.x = 30;
    fallback.y = 20;
    fallback.touched = true;
    fallback.update();
    assert(pixel(canvas, 30, 20) == RED); // No pressed image: unpressed fallback.
    fallback.hide();
    const auto hidden = canvas.tft->pixels;
    fallback.update();
    assert(canvas.tft->pixels == hidden);
}

int main() {
    static_assert(DEBOUNCE == 0, "Canvas global debounce must remain unchanged");
    test_default_debounce();
    test_explicit_zero_and_custom_debounce();
    test_touch_state_and_explicit_rendering();
    std::printf("sizeof(IconButton)=%zu sizeof(Button)=%zu (local static default adds no object state)\n",
                sizeof(IconButton), sizeof(Button));
    std::puts("icon_contract: PASS");
}
