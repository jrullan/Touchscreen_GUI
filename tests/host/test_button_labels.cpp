#include "Button.h"
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

class LabelCanvas : public Canvas {
public:
    LabelCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};

static void expectText(LabelCanvas& canvas, Button& button, int x, int y) {
    canvas.tft->ops.clear();
    button.drawLabel();
    const auto& ops = canvas.tft->ops;
    assert(ops.size() == 1);
    assert(ops[0].name == std::string("text:") + button.label);
    assert(ops[0].x == x && ops[0].y == y);
    assert(ops[0].w == int(std::strlen(button.label)) * FONT_X * button.fontSize);
    assert(ops[0].h == FONT_Y * button.fontSize);
    assert(ops[0].color == button.fgColor);
}

static void expectWarning(LabelCanvas& canvas, Button& button) {
    canvas.tft->ops.clear();
    button.drawLabel();
    const auto& ops = canvas.tft->ops;
    assert(ops.size() == 1 && ops[0].name == "fillRect" && ops[0].color == RED);
    // The hardware boundary takes int16_t, including for legacy warning rectangles.
    assert(ops[0].x == static_cast<int16_t>(int64_t(button.x) - 4));
    assert(ops[0].y == static_cast<int16_t>(int64_t(button.y) - 4));
    assert(ops[0].w == static_cast<int16_t>(int64_t(button.w) + 8));
    assert(ops[0].h == static_cast<int16_t>(int64_t(button.h) + 8));
}

static void placementAndEdges(LabelCanvas& canvas, Button& button, char position) {
    button.labelPos = position;
    button.setLabel((char*)"Label");
    for (int font = 1; font <= 3; ++font) {
        button.fontSize = font;
        const int width = 30 * font, height = 8 * font, gap = 3 * font;
        button.x = 80; button.y = 100; button.w = 81; button.h = 40;
        expectText(canvas, button, 80 + (81 - width) / 2,
                   position == LABEL_TOP ? 100 - height - gap : 140 + gap);

        // Exact left/top fit at 1; zero remains rejected (legacy strict bounds).
        button.w = width; button.x = 1;
        button.y = position == LABEL_TOP ? height + gap + 1 : 1 - button.h - gap;
        expectText(canvas, button, 1, 1);
        button.x = 0; expectWarning(canvas, button);
        button.x = -1; expectWarning(canvas, button);
        button.x = 1; --button.y; expectWarning(canvas, button);
        --button.y; expectWarning(canvas, button);

        // Exact right/bottom fit is accepted; a pixel beyond either is rejected.
        button.x = canvas.w - width;
        button.y = position == LABEL_TOP ? canvas.h + gap : canvas.h - height - button.h - gap;
        expectText(canvas, button, canvas.w - width, canvas.h - height);
        ++button.x; expectWarning(canvas, button);
        --button.x; ++button.y; expectWarning(canvas, button);

        // Wider than the button: center from getLabelSize, no helper +1 offset.
        button.setLabel((char*)"Long label");
        button.x = 105; button.y = 100; button.w = 31;
        expectText(canvas, button, 105 + (31 - 60 * font) / 2,
                   position == LABEL_TOP ? 100 - height - gap : 140 + gap);
        button.setLabel((char*)"Label");
    }
    // A screen-wide label cannot satisfy strict x > 0.
    button.fontSize = 1; button.x = 1; button.y = 100; button.w = 240;
    button.setLabel((char*)"1234567890123456789012345678901234567890");
    expectWarning(canvas, button);
}

static void unchangedAndInert(LabelCanvas& canvas, Button& button) {
    button.setLabel((char*)"Label");
    button.x = 100; button.y = 100; button.w = 31; button.h = 41;
    for (int font = 1; font <= 3; ++font) {
        button.fontSize = font;
        button.labelPos = LABEL_LEFT;
        expectText(canvas, button, 100 - 33 * font, 100 + (41 - 8 * font) / 2);
        button.labelPos = LABEL_RIGHT;
        expectText(canvas, button, 131 + 3 * font, 100 + (41 - 8 * font) / 2);
    }
    button.fontSize = 1; button.labelPos = LABEL_LEFT; button.x = 33;
    expectWarning(canvas, button);
    button.x = 34; expectText(canvas, button, 1, 116);
    button.labelPos = LABEL_RIGHT; button.x = canvas.w - 30 - 31 - 3;
    expectText(canvas, button, canvas.w - 30, 116);
    ++button.x; expectWarning(canvas, button);
    for (char pos : {LABEL_TOP, LABEL_BOTTOM, LABEL_LEFT, LABEL_RIGHT}) {
        button.labelPos = pos;
        for (char* label : {static_cast<char*>(nullptr), (char*)""}) {
            button.setLabel(label); canvas.tft->ops.clear(); button.drawLabel();
            assert(canvas.tft->ops.empty());
        }
        button.setLabel((char*)"Label"); button.setCanvas(nullptr);
        canvas.tft->ops.clear(); button.drawLabel(); assert(canvas.tft->ops.empty());
        button.setCanvas(&canvas);
    }
}

static void extremeCoordinates(LabelCanvas& canvas, Button& button) {
    button.fontSize = 3; button.setLabel((char*)"Label");
    for (char pos : {LABEL_TOP, LABEL_BOTTOM}) {
        button.labelPos = pos;
        for (int extreme : {INT_MIN, INT_MAX}) {
            button.x = extreme; button.y = 100; button.w = 80; button.h = 40;
            expectWarning(canvas, button);
            button.x = 80; button.y = extreme;
            expectWarning(canvas, button);
            button.y = 100; button.w = extreme;
            expectWarning(canvas, button);
        }
        button.x = 80; button.y = 100; button.w = 80; button.h = INT_MAX;
        if (pos == LABEL_TOP) expectText(canvas, button, 75, 67);
        else expectWarning(canvas, button);
    }
}

int main() {
    LabelCanvas canvas;
    Button button;
    button.setCanvas(&canvas); button.fgColor = GREEN;
    placementAndEdges(canvas, button, LABEL_TOP);
    placementAndEdges(canvas, button, LABEL_BOTTOM);
    unchangedAndInert(canvas, button);
    extremeCoordinates(canvas, button);
    std::puts("PASS button labels: placement/fonts/edges/long labels/inert/LEFT/RIGHT/extremes");
}
