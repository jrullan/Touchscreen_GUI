#pragma once
#include "Adafruit_GFX.h"
#include <vector>
class Adafruit_ILI9341 : public Adafruit_GFX {
public:
    std::vector<uint16_t> pixels = std::vector<uint16_t>(240*320);
    std::vector<unsigned> writes = std::vector<unsigned>(240*320);
    size_t pixelWrites = 0;
    Adafruit_ILI9341(int8_t=-1,int8_t=-1,int8_t=-1):Adafruit_GFX(240,320){}
    void begin(uint32_t=0) {}
    void drawPixel(int16_t x,int16_t y,uint16_t color) override {
        if(x>=0 && y>=0 && x<width() && y<height()) {
            pixels[y*width()+x]=color; ++writes[y*width()+x]; ++pixelWrites;
        }
    }
    void resetWrites() { std::fill(writes.begin(),writes.end(),0); pixelWrites=0; }
    uint16_t foreground() const { return textcolor; }
    uint16_t background() const { return textbgcolor; }
    uint8_t textSize() const { return textsize_x; }
};
