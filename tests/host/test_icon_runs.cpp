#include "IconButton.h"
#include <algorithm>
#include <cassert>
#include <cstdio>

struct RunCanvas : Canvas {
    RunCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
int main() {
    RunCanvas canvas;
    const uint16_t bitmap[] PROGMEM = {
        RED, RED, BLUE, BLUE, BLUE, BLACK, GREEN,
        RED, BLACK, BLACK, GREEN, GREEN, GREEN, GREEN
    };
    IconButton icon(7, 2, bitmap);
    icon.setCanvas(&canvas);
    canvas.tft->ops.clear();
    icon.drawRGBABitmap(3, 4, bitmap, 7, 2, BLACK);
    unsigned runs=0;
    for (const auto& op : canvas.tft->ops) if(op.name=="hline") ++runs;
    std::printf("same-color horizontal runs=%u expected=5\n", runs);
    std::fflush(stdout);
    assert(runs==5 && "batch each maximal same-color run, not varying-color opaque spans");
    assert(canvas.tft->pixels[4*canvas.tft->width()+3]==RED);
    assert(canvas.tft->pixels[4*canvas.tft->width()+5]==BLUE);
    assert(canvas.tft->pixels[4*canvas.tft->width()+8]==BLACK);
    // Explicit update is an unconditional repaint, including mutable image data.
    uint16_t mutableImage[2]={RED, RED};
    icon.imageUnpressed=mutableImage; icon.w=2; icon.h=1; icon.x=3; icon.y=4;
    icon.update();
    mutableImage[0]=BLUE;
    icon.update();
    assert(canvas.tft->pixels[4*canvas.tft->width()+3]==BLUE);
    icon.useTransparency=false;
    icon.drawRGBABitmap(3,4,bitmap,7,2,BLACK);
    assert(canvas.tft->pixels[4*canvas.tft->width()+8]==BLACK);
    std::puts("icon_runs host: PASS");
}
