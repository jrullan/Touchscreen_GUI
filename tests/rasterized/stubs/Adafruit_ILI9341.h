#pragma once
#include "Adafruit_GFX.h"
#include <vector>
class Adafruit_ILI9341: public Adafruit_GFX {
public:
std::vector<uint16_t> pixels=std::vector<uint16_t>(240*320);
Adafruit_ILI9341(int8_t=-1,int8_t=-1,int8_t=-1):Adafruit_GFX(240,320){}
void begin(uint32_t=0){}
void drawPixel(int16_t x,int16_t y,uint16_t c) override {if(x>=0&&y>=0&&x<width()&&y<height())pixels[y*width()+x]=c;}
};
