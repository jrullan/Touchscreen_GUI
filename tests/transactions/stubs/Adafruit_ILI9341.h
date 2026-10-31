#pragma once
#include <Adafruit_GFX.h>
#include <vector>
// Replace the panel/bus boundary only. The GFX virtual methods and rasterizer
// are real. Like SPITFT, drawPixel starts/ends its own transaction.
class Adafruit_ILI9341 : public Adafruit_GFX {
public:
  unsigned begins=0, ends=0, writes=0, writesOutside=0;
  bool selected=false;
  std::vector<uint16_t> pixels=std::vector<uint16_t>(240*320);
  Adafruit_ILI9341(int8_t=-1,int8_t=-1,int8_t=-1):Adafruit_GFX(240,320){}
  void startWrite() override { ++begins; selected=true; }
  void endWrite() override { ++ends; selected=false; }
  void drawPixel(int16_t x,int16_t y,uint16_t c) override {
    if(x<0 || y<0 || x>=width() || y>=height()) return;
    startWrite();
    ++writes;
    if(!selected) ++writesOutside;
    pixels[y*width()+x]=c;
    endWrite();
  }
};
