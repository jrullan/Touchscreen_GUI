#pragma once
#include "Arduino.h"
#include <vector>

struct HostDraw {std::string name; int x,y,w,h; uint16_t color;};
// A glyph's virtual drawPixel forwarding identifies the destination panel.
// Keep text-cell traces at this hardware seam, not in chart production code.
inline std::vector<HostDraw>* hostGlyphDestination = nullptr;

// Hardware-boundary compile seam only. Pixel-exact glyph coverage lives in the
// isolated real-Adafruit-GFX raster runners, not this host stub.
class Adafruit_GFX {
public:
    Adafruit_GFX(int16_t, int16_t) {}
    virtual ~Adafruit_GFX() = default;
    virtual void drawPixel(int16_t, int16_t, uint16_t) = 0;
    virtual void writeFillRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t fg, uint16_t, uint8_t size) {
        if(!size) size=1;
        hostGlyphDestination=nullptr;
        drawPixel(x,y,fg); // destination routing witness, NOT a glyph raster
        if(!hostGlyphDestination) return;
        auto& ops=*hostGlyphDestination;
        // Consecutive same-style cells form the same text rectangle print()
        // records. Tick/erase operations naturally separate successive labels.
        if(!ops.empty() && ops.back().name.compare(0,5,"text:")==0 &&
           ops.back().x+ops.back().w==x && ops.back().y==y &&
           ops.back().h==8*size && ops.back().color==fg) {
            ops.back().name+=char(c); ops.back().w+=6*size;
        } else ops.push_back({std::string("text:")+char(c),x,y,6*size,8*size,fg});
    }
};
