#pragma once
#include "Arduino.h"

// Hardware-boundary compile seam only. Pixel-exact glyph coverage lives in the
// isolated real-Adafruit-GFX raster runners, not this host stub.
class Adafruit_GFX {
public:
    Adafruit_GFX(int16_t, int16_t) {}
    virtual ~Adafruit_GFX() = default;
    virtual void drawPixel(int16_t, int16_t, uint16_t) = 0;
    virtual void writeFillRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawChar(int16_t, int16_t, unsigned char, uint16_t, uint16_t, uint8_t) {}
};
