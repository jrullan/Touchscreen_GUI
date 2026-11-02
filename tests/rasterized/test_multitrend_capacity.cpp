// Compare every physical RGB565 pixel through actual Guitft + installed GFX.
#include "MultiTrend.h"
#include <algorithm>
#include <cassert>
#include <cstdio>

struct CapacityCanvas : Canvas {
    CapacityCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static unsigned frames=0;
static void equal(CapacityCanvas& a, CapacityCanvas& b, int s,int n,int r,int stage) {
    if(a.tft->pixels!=b.tft->pixels) {
        std::fprintf(stderr,"capacity raster mismatch S=%d N=%d rotation=%d stage=%d\n",s,n,r,stage);
        assert(false);
    }
    assert(a.tft->starts==a.tft->ends && a.tft->depth==0);
    assert(b.tft->starts==b.tft->ends && b.tft->depth==0);
    ++frames;
}
static void matrix(int s,int n,int rotation) {
    CapacityCanvas a,b;
    a.tft->setRotation(rotation); b.tft->setRotation(rotation);
    MultiTrend baseline(180,140,0,50,100,s,n), exact(180,140,0,50,100,s,n,s);
    baseline.setCanvas(&a); exact.setCanvas(&b);
    for(auto* m : {&baseline,&exact}) {
        m->x=12; m->y=16;
        for(int j=0;j<s;++j) {
            m->setSeriesColor(j,j==3?YELLOW:j==2?RED:j==1?GREEN:BLUE);
            for(int k=0;k<n+7;++k) m->addValue(j,(k*17+j*29)%101,false);
        }
        m->show();
    }
    equal(a,b,s,n,rotation,0);
    assert(std::count_if(a.tft->pixels.begin(),a.tft->pixels.end(),[](uint16_t c){return c!=BLACK;})>100);
    const auto initial=a.tft->pixels;
    for(int stage=1;stage<=16;++stage) {
        for(auto* m : {&baseline,&exact}) {
            switch(stage) {
            case 1: m->setMarkerRadius(3); m->markerWarn=40; m->markerCrit=80; break;
            case 2: m->showXScale=false; m->showYScale=false; break;
            case 3: m->showBorder=false; m->showThresholds=false; m->showGridlines=true; break;
            case 4: m->forceSquareWaveform=true; break;
            case 5: m->setWindow(2,n-3); m->setMaxX(3600); break;
            case 6: m->showXScale=true; m->showYScale=true; break;
            case 7: m->autoFit(true); break;
            case 8: m->enableAutoFit=true; m->addValue(0,200,false); break;
            case 9: m->setMarkerRadius(0); break;
            case 10: m->clear(); break;
            case 11: m->init(); break;
            case 12: m->numSeries=1; break;
            case 13: m->numSeries=s; m->setWindow(0,n-1); break;
            case 14: m->visible=false; m->addValue(0,42); break;
            case 15: m->show(); break;
            case 16: m->forceUpdate(); break;
            }
            if(stage!=14) {
                for(int j=0;j<s;++j) m->addValue(j,(stage*19+j*31)%101,false);
                m->update();
            }
        }
        equal(a,b,s,n,rotation,stage);
    }
    assert(a.tft->pixels!=initial); // Non-vacuous changing, visible raster.
}
int main() {
    for(int rotation=0;rotation<4;++rotation)
        for(int n : {16,32,255}) for(int s : {1,2,3,4}) matrix(s,n,rotation);
    std::printf("multitrend capacity real-GFX: 48 configurations, %u exact full-panel frame comparisons PASS\n",frames);
}
