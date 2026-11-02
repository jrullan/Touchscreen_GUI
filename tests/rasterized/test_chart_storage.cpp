#include "Trend.h"
#include "MultiTrend.h"
#include "tests/host/allocation_hooks.h"
#include <cassert>
#include <cstdio>
struct C:Canvas { C():Canvas(TFT_PORTRAIT,BLACK){} void init() override{} Point* getTouchedPoint() override{return nullptr;} };
template<class T> static void fresh(T& t,C& c){ const auto pixels=c.tft->pixels; c.tft->fillScreen(BLACK);t.show();assert(pixels==c.tft->pixels);}
static void setup(TrendBase& t,C& c){t.setCanvas(&c);t.x=7;t.y=9;}
int main(){
 for(int rotation=0;rotation<4;++rotation) for(int n: {1,2,16,32,255}){
  C a,b; a.tft->setRotation(rotation);b.tft->setRotation(rotation);
  Trend t(180,140,0,50,100,n),u(180,140,0,50,100,n,HistoryStorage::Circular);
  setup(t,a);setup(u,b);t.show();u.show();assert(a.tft->pixels==b.tft->pixels);
  for(int k=0;k<2*n+7;++k){
   t.addValue((k*73)%256,false);u.addValue((k*73)%256,false);
   t.update();u.update();assert(a.tft->pixels==b.tft->pixels);
  }
  a.tft->fillScreen(BLACK);b.tft->fillScreen(BLACK);
  MultiTrend m(180,140,0,50,100,4,n,4),r(180,140,0,50,100,4,n,4,HistoryStorage::Circular);
  setup(m,a);setup(r,b);m.setMarkerRadius(3);r.setMarkerRadius(3);
  m.show();r.show();assert(a.tft->pixels==b.tft->pixels);
  unsigned seed=741;
  for(int k=0;k<2*n+17;++k){
   seed=seed*1664525u+1013904223u;int s=(seed>>16)%4;uint8_t v=seed>>24;
   m.addValue(s,v,false);r.addValue(s,v,false);
   
   if(k%23==0){
    m.forceSquareWaveform=r.forceSquareWaveform=(k/23)%2;
    m.showGridlines=r.showGridlines=(k/23)%2;
    m.showYScale=r.showYScale=(k/23)%2;
    m.showXScale=r.showXScale=(k/46)%2;
    m.setMaxX(k*57);r.setMaxX(k*57);
    m.setWindow(k%n,(k*7)%n);r.setWindow(k%n,(k*7)%n);
    m.markerWarn=r.markerWarn=80;m.markerCrit=r.markerCrit=150;
    m.setMarkerRadius((k/23)%5);r.setMarkerRadius((k/23)%5);
    m.enableAutoFit=r.enableAutoFit=true;
   }
   m.update();r.update();assert(a.tft->pixels==b.tft->pixels);
   if(k%31==0){fresh(m,a);fresh(r,b);}

  }
  m.clear();r.clear();m.update();r.update();assert(a.tft->pixels==b.tft->pixels);
  m.drawTrend();r.drawTrend();m.eraseTrend();r.eraseTrend();m.update();r.update();assert(a.tft->pixels==b.tft->pixels);
  m.init();r.init();m.show();r.show();assert(a.tft->pixels==b.tft->pixels);
  m.trendWindow={-999,999};r.trendWindow={-999,999};m.update();r.update();assert(a.tft->pixels==b.tft->pixels);
  m.w=r.w=-1;m.update();r.update();assert(a.tft->pixels==b.tft->pixels);
  m.w=r.w=180;m.show();r.show();assert(a.tft->pixels==b.tft->pixels);
 }
 // Cache allocation failure: full direct rendering uses logical samples too.
 for(auto mode: {HistoryStorage::NewestFirst,HistoryStorage::Circular}){
  C c;MultiTrend m(180,140,0,50,100,3,16,3,mode);setup(m,c);m.setMarkerRadius(6);
  for(int k=0;k<49;++k)m.addValue(k%3,k*5,false);
  host_allocation_fail_after=0;m.show();assert(host_allocation_fail_after==-1);
  const auto pixels=c.tft->pixels;m.update();assert(pixels==c.tft->pixels);
  fresh(m,c);
  m.x=40000;m.y=-40000;m.update();m.x=7;m.y=9;m.update();fresh(m,c);
  m.setSeriesColor(0,-1);m.update();fresh(m,c);
  m.setSeriesColor(0,0x1ffff);m.update();fresh(m,c);
  m.init();m.show();m.setWindow(4,6);m.update();
  host_allocation_fail_after=0;m.setWindow(0,15);m.x=40000;m.update();assert(host_allocation_fail_after==-1);
  m.x=7;m.update();fresh(m,c);
 }
 std::puts("chart storage real GFX pixels PASS");
}
