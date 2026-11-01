#pragma once
#include "Arduino.h"
#include "Adafruit_GFX.h"
#include <vector>
class Adafruit_ILI9341 {
 int width_=240,height_=320,cx=0,cy=0,sz=1;uint16_t fg=0xffff;
 public:
 std::vector<HostDraw> ops;
 std::vector<uint16_t> pixels=std::vector<uint16_t>(240*320);
 Adafruit_ILI9341(int8_t=-1,int8_t=-1,int8_t=-1){}
 virtual ~Adafruit_ILI9341()=default;
 void begin(uint32_t=0){} int16_t width()const{return width_;} int16_t height()const{return height_;}
 void setRotation(uint8_t r){width_=(r&1)?320:240;height_=(r&1)?240:320;}
 void drawPixel(int16_t x,int16_t y,uint16_t c){hostGlyphDestination=&ops;if(x>=0&&y>=0&&x<width_&&y<height_)pixels[y*width_+x]=c;}
 void writePixel(int16_t x,int16_t y,uint16_t c){drawPixel(x,y,c);}
 void fillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c){ops.push_back({"fillRect",x,y,w,h,c});for(int yy=max(0,(int)y);yy<min((int)height_,y+(int)h);++yy)for(int xx=max(0,(int)x);xx<min((int)width_,x+(int)w);++xx)drawPixel(xx,yy,c);}
 void fillScreen(uint16_t c){fillRect(0,0,width_,height_,c);}
 void drawFastHLine(int16_t x,int16_t y,int16_t w,uint16_t c){ops.push_back({"hline",x,y,w,1,c});for(int i=0;i<w;++i)drawPixel(x+i,y,c);}
 void drawFastVLine(int16_t x,int16_t y,int16_t h,uint16_t c){ops.push_back({"vline",x,y,1,h,c});for(int i=0;i<h;++i)drawPixel(x,y+i,c);}
 void drawRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t c){ops.push_back({"rect",x,y,w,h,c});if(w>0&&h>0){drawFastHLine(x,y,w,c);drawFastHLine(x,y+h-1,w,c);drawFastVLine(x,y,h,c);drawFastVLine(x+w-1,y,h,c);}}
 void drawLine(int16_t x,int16_t y,int16_t ex,int16_t ey,uint16_t c){int dx=abs(ex-x),sx=x<ex?1:-1,dy=-abs(ey-y),sy=y<ey?1:-1,err=dx+dy;for(;;){drawPixel(x,y,c);if(x==ex&&y==ey)break;int e=2*err;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}}
 void drawCircle(int16_t x,int16_t y,int16_t r,uint16_t c){ops.push_back({"circle",x,y,r,r,c});}
 void fillCircle(int16_t x,int16_t y,int16_t r,uint16_t c){ops.push_back({"fillCircle",x,y,r,r,c});for(int yy=-r;yy<=r;++yy)for(int xx=-r;xx<=r;++xx)if(xx*xx+yy*yy<=r*r)drawPixel(x+xx,y+yy,c);}
 void fillRoundRect(int16_t x,int16_t y,int16_t w,int16_t h,int16_t,uint16_t c){fillRect(x,y,w,h,c);}
 void drawRoundRect(int16_t x,int16_t y,int16_t w,int16_t h,int16_t,uint16_t c){drawRect(x,y,w,h,c);}
 void fillTriangle(int16_t,int16_t,int16_t,int16_t,int16_t,int16_t,uint16_t){}
 void drawTriangle(int16_t,int16_t,int16_t,int16_t,int16_t,int16_t,uint16_t){}
 void startWrite(){} void endWrite(){}
 void setCursor(int16_t x,int16_t y){cx=x;cy=y;}
 void setTextSize(uint8_t s){sz=s;} void setTextColor(uint16_t c){fg=c;} void setTextColor(uint16_t c,uint16_t){fg=c;}
 size_t print(const char* s){if(!s)return 0;ops.push_back({std::string("text:")+s,cx,cy,(int)strlen(s)*6*sz,8*sz,fg});return strlen(s);}
 size_t print(long n){std::string s=std::to_string(n);return print(s.c_str());}
 size_t print(int n){return print((long)n);} size_t print(unsigned int n){return print((long)n);}
 void println(const char* s){print(s);}
};
