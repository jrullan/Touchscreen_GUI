#include "Display.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
class RasterCanvas : public Canvas {
public:
    RasterCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
int main(int argc, char** argv) {
    const bool zero = argc > 1 && std::strcmp(argv[1], "font0") == 0;
    RasterCanvas c; Display d(8);
    d.setCanvas(&c); d.borderWidth=0; d.setColors(BLACK,WHITE,WHITE);
    d.x=zero?50:210; d.y=zero?50:30;
    d.setSize(zero?20:30,zero?10:20); d.fontSize=zero?0:2;
    d.setText((char*)"8888",true);
    const auto before=c.tft->pixels;
    const int glyphs=std::count(before.begin(),before.end(),WHITE);
    const int witness=(zero?82:7)+62*240;
    if (glyphs != (zero?68:272) || before[witness] != WHITE) {
        std::fprintf(stderr,"FAIL: actual classic-font glyph/wrap witness missing (%d pixels)\n",glyphs);
        return 1;
    }
    d.setText((char*)"",true);
    const int stale=std::count(c.tft->pixels.begin(),c.tft->pixels.end(),WHITE);
    std::printf("%s: rendered=%d stale=%d witness(%d,62)=%u\n",zero?"font0":"wrap",glyphs,stale,zero?82:7,c.tft->pixels[witness]);
    if (stale || c.tft->pixels[witness] != BLACK) return 1;
    // Shorter text must match a clean render, not just remove one witness.
    d.setText((char*)"8888",true);
    d.setText((char*)"8",true);
    RasterCanvas fresh; Display expected(8);
    expected.setCanvas(&fresh); expected.borderWidth=0;
    expected.setColors(BLACK,WHITE,WHITE);
    expected.x=d.x; expected.y=d.y; expected.setSize(d.w,d.h); expected.fontSize=d.fontSize;
    expected.setText((char*)"8",true);
    if (c.tft->pixels != fresh.tft->pixels) {
        std::fputs("FAIL: shortened value differs from clean raster\n",stderr); return 1;
    }
    // Cleanup must use committed bounds even after backend wrap/font changes.
    d.setText((char*)"8888",true);
    d.fontSize=1; c.tft->setTextWrap(false);
    d.setText((char*)"",true);
    if (std::count(c.tft->pixels.begin(),c.tft->pixels.end(),WHITE)) {
        std::fputs("FAIL: old footprint lost after font/wrap mutation\n",stderr); return 1;
    }
    // Display must not silently re-enable wrapping for the next caller.
    c.tft->drawString("88",230,100,2,WHITE);
    if (c.tft->getCursorY() != 100) {
        std::fputs("FAIL: backend wrap state changed\n",stderr); return 1;
    }
    std::puts("rasterized Display: PASS");
}
