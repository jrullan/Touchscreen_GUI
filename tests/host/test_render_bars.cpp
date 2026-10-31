#include "Gauge.h"
#include "Slider.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <climits>

struct HostCanvas : Canvas {
    HostCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
template<class T> static void setup(T& bar, HostCanvas& c) {
    bar.setCanvas(&c); bar.x=20; bar.y=30; bar.setSize(40,120);
    bar.setColors(GRAY2,BLUE,WHITE); bar.setCV(40,false);
}
#if defined(BAR_REAL_GFX)
template<class T> static void freshPixels(T& bar, HostCanvas& c) {
    const auto pixels=c.tft->pixels;
    c.tft->fillScreen(BLACK); bar.show();
    if(pixels!=c.tft->pixels) {
        std::fprintf(stderr,"real GFX mismatch x=%d y=%d w=%d h=%d bw=%d cv=%u\n",bar.x,bar.y,bar.w,bar.h,bar.borderWidth,bar.currentValue);
        assert(false);
    }
}
template<class T> static void rasterBars() {
    HostCanvas c; T bar; setup(bar,c); bar.show();
    for(int frame=0;frame<20;++frame) {
        bar.setCV(95,false); bar.setCV(5,false); bar.setCV(frame%2?40:60,false);
        bar.update(); freshPixels(bar,c);
    }
    for(int change=0;change<12;++change) {
        switch(change) {
            case 0: bar.x=205; break; // label wrapping at the display edge
            case 1: bar.setLimits(0,UINT_MAX/2,UINT_MAX); bar.setCV(UINT_MAX); break;
            case 2: bar.x=30; bar.y=50; break;
            case 3: bar.setLimits(0,50,100); bar.setCV(40); break;
            case 4: bar.w=15; bar.h=5; break; // labels outside the nominal rectangle
            case 5: bar.w=50; bar.h=120; break;
            case 6: bar.bgColor=CYAN; break;
            case 7: bar.borderWidth=0; break;
            case 8: bar.setHiLimit(30,RED); break;
            case 9: bar.setLowLimit(60,YELLOW); break;
            case 10: bar.w=0; break;
            case 11: bar.w=40; bar.h=0; break;
        }
        bar.update(); freshPixels(bar,c);
    }
}
#if defined(BAR_TRANSACTIONS)
template<class T> static void transactionBars() {
    HostCanvas c; T bar; setup(bar,c); bar.show();
    assert(c.tft->begins==1 && c.tft->ends==1 && !c.tft->selected);
    auto begins=c.tft->begins;
    bar.update(); assert(c.tft->begins==begins);
    bar.setCV(60); assert(c.tft->begins==begins+1 && c.tft->ends==begins+1);
    begins=c.tft->begins; bar.x+=50; bar.update();
    assert(c.tft->begins==begins+1 && c.tft->ends==begins+1);
    begins=c.tft->begins; bar.hide();
    assert(c.tft->begins==begins+1 && !c.tft->selected);
    begins=c.tft->begins; bar.forceUpdate(); assert(c.tft->begins==begins);
    bar.show(); assert(c.tft->begins==begins+1 && c.tft->begins==c.tft->ends);
    assert(c.tft->writesOutside==0);
    bar.borderWidth=80; bar.show(); begins=c.tft->begins;
    bar.setCV(80); assert(c.tft->begins==begins); // no drawable interior
    std::printf("bar scoped transactions: begins=%u ends=%u outside=%u\n",c.tft->begins,c.tft->ends,c.tft->writesOutside);
}
static void callbackTransactionBoundary() {
    HostCanvas c; Slider bar; setup(bar,c); bar.show();
    static Guitft* panel; static bool called;
    panel=c.tft; called=false;
    bar.setEventHandler([](Slider*) { called=true; assert(!panel->selected && panel->begins==panel->ends); });
    host_millis=1000; Point p; p.x=bar.x+10; p.y=bar.y+40;
    bar.checkTouch(&p); assert(called && !panel->selected && panel->writesOutside==0);
}
#endif
int main() {
#if defined(BAR_TRANSACTIONS)
    transactionBars<Gauge>(); transactionBars<Slider>(); callbackTransactionBoundary();
#endif
    HostCanvas c; Gauge gauge; setup(gauge,c); gauge.show();
    size_t glyphPixels=0;
    for(int y=30;y<150;++y) for(int x=20;x<37;++x)
        glyphPixels+=c.tft->pixels[y*c.tft->width()+x]==WHITE;
    assert(glyphPixels>0); // real glcdfont glyphs, not a text no-op stub
    rasterBars<Gauge>(); rasterBars<Slider>();
    std::printf("render_bars real GFX: PASS glyphPixels=%zu\n",glyphPixels);
}
#else
static size_t pixelTraffic(const Guitft& tft) {
    size_t n=0;
    for(const auto& op:tft.ops) {
        if(op.name=="fillRect" || op.name=="hline" || op.name=="vline")
            n+=size_t(op.w>0?op.w:0)*size_t(op.h>0?op.h:0);
    }
    return n;
}
template<class T> static void benchmark(const char* name) {
    HostCanvas c; T bar; setup(bar,c); bar.show(); c.tft->ops.clear();
    // Stable-color, genuinely changing frames; 3 deferred writes per commit.
    for(int frame=0;frame<20;++frame) {
        bar.setCV(20,false); bar.setCV(80,false);
        bar.setCV(frame%2?40:60,false); bar.update();
    }
    const auto changed=c.tft->ops.size(), pixels=pixelTraffic(*c.tft);
    c.tft->ops.clear(); for(int i=0;i<20;++i) bar.update();
    std::printf("BENCH %s changed20 ops=%zu pixels=%zu unchanged20 ops=%zu pixels=%zu sizeof=%zu\n",
        name,changed,pixels,c.tft->ops.size(),pixelTraffic(*c.tft),sizeof(T));
}
template<class T> static void mixedBenchmark(const char* name) {
    HostCanvas c; T bar; setup(bar,c); bar.show(); c.tft->ops.clear();
    for(int frame=0;frame<20;++frame) {
        bar.setCV(20,false); bar.setCV(80,false);
        bar.setCV((frame*37)%101,false);
        if(frame==4) bar.x+=50;
        if(frame==8) bar.setSize(35,100);
        if(frame==12) bar.setColors(CYAN,GREEN,YELLOW);
        if(frame==16) bar.setLimits(0,30,120);
        if(frame==18) bar.show(); else bar.update();
    }
    const auto operations=c.tft->ops.size(), pixels=pixelTraffic(*c.tft);
    c.tft->ops.clear(); for(int i=0;i<20;++i) bar.update();
    std::printf("BENCH %s mixed20 ops=%zu pixels=%zu unchanged20 ops=%zu pixels=%zu\n",
        name,operations,pixels,c.tft->ops.size(),pixelTraffic(*c.tft));
}
template<class T> static void noOp() {
    HostCanvas c; T bar; setup(bar,c); bar.show(); c.tft->ops.clear();
    bar.update(); assert(c.tft->ops.empty());
    bar.setLimits(0,500,1000); bar.setHiLimit(1000,RED); bar.setLowLimit(0,GREEN);
    bar.setCV(400); c.tft->ops.clear(); bar.setCV(401);
    assert(c.tft->ops.empty()); // distinct values, identical rasterized position/color
}
template<class T> static void expectFresh(T& bar, HostCanvas& c) {
    const auto pixels=c.tft->pixels;
    c.tft->fillScreen(BLACK); bar.show();
    if(pixels!=c.tft->pixels) {
        std::fprintf(stderr,"pixel mismatch x=%d y=%d w=%d h=%d bw=%d cv=%u\n",bar.x,bar.y,bar.w,bar.h,bar.borderWidth,bar.currentValue);
        assert(false);
    }
    c.tft->ops.clear(); bar.update(); assert(c.tft->ops.empty());
}
template<class T> static void deltas() {
    HostCanvas c; T bar; setup(bar,c); bar.show();
    for(int value: {60,25,75,35}) {
        c.tft->ops.clear();
        bar.setCV(95,false); bar.setCV(5,false); bar.setCV(value,false);
        assert(c.tft->ops.empty()); bar.update();
        assert(c.tft->ops.size()==1 && c.tft->ops[0].name=="fillRect");
        assert(c.tft->ops[0].w==bar.w-2*bar.borderWidth);
        assert(c.tft->ops[0].h < bar.h/2);
        expectFresh(bar,c);
    }
}
template<class T> static void styles() {
    HostCanvas c; T bar; setup(bar,c); bar.show();
    for(int change=0;change<12;++change) {
        switch(change) {
            case 0: bar.x+=80; bar.y+=10; break;
            case 1: bar.w-=9; bar.h-=20; break;
            case 2: bar.bgColor=GREEN; break;
            case 3: bar.fgColor=CYAN; break;
            case 4: bar.borderColor=YELLOW; break;
            case 5: bar.borderWidth=5; break;
            case 6: bar.setLimits(10,50,200); break;
            case 7: bar.setSP(80,BLUE); break;
            case 8: bar.setHiLimit(70,CYAN); break;
            case 9: bar.setLowLimit(30,WHITE); break;
            case 10: bar.w=0; break;
            case 11: bar.w=32; bar.h=0; break;
        }
        bar.update(); expectFresh(bar,c);
    }
}
template<class T> static void lifecycle() {
    HostCanvas c; T bar; setup(bar,c); bar.show();
    // Hide must erase the committed footprint even after deferred geometry edits.
    bar.x+=60; bar.y+=10; bar.hide();
    for(auto pixel:c.tft->pixels) assert(pixel==BLACK);
    c.tft->ops.clear(); bar.setCV(90); bar.bgColor=CYAN; bar.update();
    assert(c.tft->ops.empty()); bar.show(); expectFresh(bar,c);
    c.tft->fillRect(bar.x+8,bar.y+8,10,10,RED); bar.show(); expectFresh(bar,c);
}
template<class T> static auto repair(T& bar, HostCanvas& c, int)
    -> decltype(bar.forceUpdate(),bar.invalidate(),void()) {
    for(int mode=0;mode<2;++mode) {
        c.tft->fillRect(bar.x+8,bar.y+8,10,10,RED);
        if(mode==0) bar.forceUpdate();
        else { c.tft->ops.clear(); bar.invalidate(); assert(c.tft->ops.empty()); bar.update(); }
        expectFresh(bar,c);
    }
    bar.hide(); c.tft->ops.clear(); bar.invalidate(); bar.forceUpdate();
    assert(c.tft->ops.empty()); bar.show(); expectFresh(bar,c);
}
template<class T> static void repair(T&, HostCanvas&, long) {
    assert(false && "explicit forceUpdate/invalidate repair APIs missing");
}
template<class T> static void repairs() { HostCanvas c; T bar; setup(bar,c); bar.show(); repair(bar,c,0); }
static void thresholds() {
    HostCanvas c; Gauge bar; setup(bar,c); bar.show();
    bar.setLimits(0,500,1000); bar.setHiLimit(401,BLUE+0x10000); bar.setLowLimit(0,GREEN);
    bar.setCV(400); c.tft->ops.clear(); bar.setCV(401); assert(c.tft->ops.empty());
    bar.setHiLimit(70,RED); bar.setLowLimit(30,GREEN); bar.setLimits(0,50,100);
    bar.setCV(40);
    for(unsigned value: {80u,20u,95u,5u,50u,50u}) { bar.setCV(value); expectFresh(bar,c); }
    bar.yScaleWidth=45; bar.update(); expectFresh(bar,c);
    bar.setLimits(0,UINT_MAX/2,UINT_MAX); bar.setHiLimit(UINT_MAX,RED);
    for(unsigned value: {0u,UINT_MAX/2,UINT_MAX-1,UINT_MAX}) { bar.setCV(value); expectFresh(bar,c); }
}
template<class T> static void emptyAndBorders() {
    for(int bw: {-2,0,2,80}) for(int width: {-1,0,40}) for(int height: {-1,0,120}) {
        HostCanvas c; T bar; setup(bar,c); bar.w=width; bar.h=height; bar.borderWidth=bw;
        bar.show(); c.tft->ops.clear(); bar.setCV(75);
        for(const auto& op:c.tft->ops) assert(op.w>0 && op.h>0);
        if(width<=0 || height<=0) assert(c.tft->ops.empty());
        expectFresh(bar,c);
    }
}
template<class T> static void targetsAndTransientBorders() {
    HostCanvas first,second,replacement; T bar; setup(bar,first); bar.show();
    bar.x=100; bar.drawBorder(); bar.x=150; bar.drawBorder();
    bar.x=30; bar.update(); expectFresh(bar,first);
    const auto priorPixels=first.tft->pixels; first.tft->ops.clear();
    bar.setCanvas(&second); bar.update();
    assert(first.tft->ops.empty() && first.tft->pixels==priorPixels); expectFresh(bar,second);
    auto* oldDriver=second.tft;
    second.tft=replacement.tft; replacement.tft=oldDriver;
    const auto oldPixels=oldDriver->pixels; oldDriver->ops.clear(); bar.update();
    assert(oldDriver->ops.empty() && oldDriver->pixels==oldPixels); expectFresh(bar,second);
}
static void directPaint() {
    HostCanvas c; Gauge g; setup(g,c); g.show();
    g.setCV(80,false); g.drawFill(); g.setCV(40,false); g.update(); expectFresh(g,c);
    c.tft->fillScreen(BLACK); Slider s; setup(s,c); s.show();
    s.drawCV(80); s.update(); expectFresh(s,c);
    c.tft->ops.clear(); s.setEventHandler([](Slider*) {});
    host_millis=1000;
    Point point; point.x=s.x+s.w/2; point.y=s.y+s.h-s.borderWidth-46;
    s.checkTouch(&point); expectFresh(s,c);
    c.tft->ops.clear(); host_millis+=1000; s.checkTouch(&point);
    assert(c.tft->ops.empty()); // same accepted touch must not repaint twice
}
int main() {
    benchmark<Gauge>("Gauge"); benchmark<Slider>("Slider");
    mixedBenchmark<Gauge>("Gauge"); mixedBenchmark<Slider>("Slider"); std::fflush(stdout);
    if(std::getenv("BAR_BENCH_ONLY")) return 0;
    noOp<Gauge>(); noOp<Slider>(); deltas<Gauge>(); deltas<Slider>();
    styles<Gauge>(); styles<Slider>(); lifecycle<Gauge>(); lifecycle<Slider>();
    repairs<Gauge>(); repairs<Slider>(); thresholds();
    emptyAndBorders<Gauge>(); emptyAndBorders<Slider>(); directPaint();
    targetsAndTransientBorders<Gauge>(); targetsAndTransientBorders<Slider>();
    std::puts("render_bars: PASS");
}

#endif // BAR_REAL_GFX
