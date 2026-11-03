#include "Display.h"
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>

class OverflowCanvas : public Canvas {
public:
    OverflowCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};

static void expectFullRender(OverflowCanvas& canvas) {
    bool text = false;
    assert(!canvas.tft->ops.empty());
    for (const auto& op : canvas.tft->ops) {
        assert(op.name.compare(0, 5, "char:") != 0);
        if (op.name.compare(0, 5, "text:") == 0) text = true;
    }
    assert(text);
}

static void offscreenExtent(int offset, int width) {
    OverflowCanvas canvas;
    Display display(8);
    display.setCanvas(&canvas);
    display.x = INT_MAX - offset;
    display.y = 30;
    display.setSize(width, 40);
    display.borderWidth = 0;
    display.fontSize = 2;
    display.setColors(BLACK, WHITE, GREEN);
    display.setText((char*)"12:34:56", true);
    canvas.tft->ops.clear();
    // Both initial legacy renders are representable. Only delta eligibility
    // overflows: the cell's right edge, or the widget's right edge, respectively.
    display.setText((char*)"12:34:57", true);
    expectFullRender(canvas);
}

static void oversizedFont(unsigned char length) {
    OverflowCanvas canvas;
    Display display(length);
    display.setCanvas(&canvas);
    display.x = 20;
    display.y = 30;
    display.setSize(180, 40);
    display.borderWidth = 0;
    display.fontSize = 255;
    display.setColors(BLACK, WHITE, GREEN);
    char text[256];
    std::memset(text, 'A', length);
    text[length] = 0;
    display.setText(text, true);
    canvas.tft->ops.clear();
    text[length - 1] = 'B';
    display.setText(text, true);
    expectFullRender(canvas);
    // FONT_X * 255 * {22,255} exceeds signed 16-bit int. This is a
    // host regression for rejecting those inputs, not an AVR execution claim:
    // the legacy full-render path still uses the target's native int helpers.
}

int main(int argc, char**) {
    // Permit each UBSan reproduction to be run independently before the fix.
    if (argc > 1) offscreenExtent(200, 220);
    else {
        offscreenExtent(100, 200);
        offscreenExtent(200, 220);
        oversizedFont(22);
        oversizedFont(255);
    }
    std::puts("display_delta_overflow host: PASS; offscreen extents and font255 lengths22/255 fall back");
}
