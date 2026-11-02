#pragma once
#include <Adafruit_GFX.h>
#include <vector>
#include <cassert>
class Adafruit_ILI9341 : public Adafruit_GFX {
public:
    std::vector<uint16_t> pixels = std::vector<uint16_t>(240*320);
    unsigned starts=0, ends=0, depth=0, pixelWrites=0, lineWrites=0, drawLineWrappers=0;
    Adafruit_ILI9341(int8_t=-1,int8_t=-1,int8_t=-1) : Adafruit_GFX(240,320) {}
    void begin(uint32_t=0) {}
    void resetCounts() { starts=ends=pixelWrites=lineWrites=drawLineWrappers=0; assert(depth==0); }
    void startWrite() override { ++starts; ++depth; }
    void endWrite() override { ++ends; assert(depth==1); --depth; }
    void drawPixel(int16_t x,int16_t y,uint16_t c) override {
        if(x<0 || y<0 || x>=width() || y>=height()) return;
        int px=x, py=y;
        switch(getRotation()) {
            case 1: px=239-y; py=x; break;
            case 2: px=239-x; py=319-y; break;
            case 3: px=y; py=319-x; break;
        }
        pixels[py*240+px]=c;
    }
    void writePixel(int16_t x,int16_t y,uint16_t c) override {
        ++pixelWrites; assert(depth==1); drawPixel(x,y,c);
    }
    void drawFastHLine(int16_t x,int16_t y,int16_t w,uint16_t c) override {
        // Model the optimized SPITFT panel boundary, not GFX's Bresenham fallback.
        ++drawLineWrappers;
        startWrite(); writeFastHLine(x,y,w,c); endWrite();
    }
    void writeFastHLine(int16_t x,int16_t y,int16_t w,uint16_t c) override {
        ++lineWrites; assert(depth==1);
        // Backend hardware seam: one uniform-color transfer, not GFX's
        // per-pixel fallback. Rotation/raster still uses real GFX dispatch.
        assert(x>=0 && y>=0 && x+w<=width() && y<height() && w>0);
        for(int i=0;i<w;++i) drawPixel(x+i,y,c);
    }
};
