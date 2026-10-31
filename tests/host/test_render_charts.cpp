#include "Trend.h"
#include "MultiTrend.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

struct HostCanvas : Canvas {
    HostCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void setup(TrendBase& t, HostCanvas& c) {
    t.setCanvas(&c); t.x=10; t.y=15;
    t.showXScale=t.showYScale=false;
    t.showGridlines=true; t.showThresholds=false;
}
template<class T> static void expectFresh(T& t, HostCanvas& c) {
    const auto pixels=c.tft->pixels;
    c.tft->fillScreen(BLACK); t.show();
    assert(pixels==c.tft->pixels);
    c.tft->ops.clear(); t.update(); assert(c.tft->ops.empty());
}
static void benchmark() {
    HostCanvas c; MultiTrend t(180,100,0,50,100,3,32);
    setup(t,c); t.setMarkerRadius(2); t.show(); c.tft->ops.clear();
    for(int frame=0;frame<20;++frame) {
        for(int s=0;s<3;++s) for(int k=0;k<4;++k)
            t.addValue(s,(frame*7+s*31+k*11)%101,false);
        t.update();
    }
    const auto changed=c.tft->ops.size(); c.tft->ops.clear();
    for(int i=0;i<20;++i) t.update();
    std::printf("BENCH charts changed20=%zu unchanged20=%zu sizeofBase=%zu sizeofTrend=%zu sizeofMulti=%zu\n",
        changed,c.tft->ops.size(),sizeof(TrendBase),sizeof(Trend),sizeof(MultiTrend));
}
int main() {
    benchmark(); if(std::getenv("CHART_BENCH_ONLY")) return 0;
    HostCanvas c;
    Trend t(180,100,0,50,100,8); setup(t,c); t.show();
    c.tft->ops.clear(); t.addValue(30,false); t.addValue(70,false); t.clear();
    assert(c.tft->ops.empty());
    MultiTrend m(180,100,0,50,100,3,8); setup(m,c); m.setMarkerRadius(2); m.show();
    c.tft->ops.clear(); m.addValue(0,30,false); m.addValue(1,70,false); m.clear();
    assert(c.tft->ops.empty());
    // A commit is pixel-identical to a clean show, including intersecting series.
    for(int frame=0;frame<12;++frame) {
        for(int k=0;k<3;++k) {
            t.addValue((frame*17+k*33)%101,false);
            for(int s=0;s<3;++s) m.addValue(s,(frame*17+k*33+s*50)%101,false);
        }
        c.tft->fillScreen(BLACK); t.show();
        t.addValue(80,false); t.addValue(20,false); t.update();
        auto pixels=c.tft->pixels;
        c.tft->fillScreen(BLACK); t.show(); assert(pixels==c.tft->pixels);
        c.tft->ops.clear(); t.update(); assert(c.tft->ops.empty());
        c.tft->fillScreen(BLACK); m.show();
        m.addValue(0,80,false); m.addValue(1,20,false); m.addValue(2,80,false); m.update();
        pixels=c.tft->pixels;
        c.tft->fillScreen(BLACK); m.show(); assert(pixels==c.tft->pixels);
        c.tft->ops.clear(); m.update(); assert(c.tft->ops.empty());
    }
    {
        HostCanvas canvas;
        MultiTrend chart(180,100,0,50,100,3,8); setup(chart,canvas);
        chart.setMarkerRadius(2); chart.show();
        chart.enableAutoFit=true; chart.addValue(0,100); expectFresh(chart,canvas);
        chart.autoFit(true); expectFresh(chart,canvas);
        chart.enableAutoFit=false;
        for(int change=0;change<21;++change) {
            switch(change) {
                case 0: chart.x+=12; chart.y+=20; break;
                case 1: chart.w-=20; chart.h+=5; break;
                case 2: chart.setLimits(10,40,120); break;
                case 3: chart.setWindow(2,5); break;
                case 4: chart.numSeries=1; break;
                case 5: chart.numSeries=3; break;
                case 6: chart.setMarkerRadius(4); break;
                case 7: chart.setMarkerRadius(0); break;
                case 8: chart.setMarkerRadius(2); chart.markerWarn=30; chart.markerCrit=70; break;
                case 9: chart.showThresholds=true; break;
                case 10: chart.showGridlines=false; break;
                case 11: chart.showXScale=chart.showYScale=true; break;
                case 12: chart.setMaxX(1200); break;
                case 13: chart.borderWidth=3; chart.borderColor=YELLOW; break;
                case 14: chart.forceSquareWaveform=true; break;
                case 15: chart.setSeriesColor(0,CYAN); break;
                case 16: chart.bgColor=GRAY2; break;
                case 17: chart.showBorder=false; break;
                case 18: chart.rightInset=10; chart.topInset=7; chart.bottomInset=9; break;
                case 19: chart.setSP(55,CYAN); chart.setHiLimit(90,RED); chart.setLowLimit(15,GREEN); break;
                case 20: chart.markerRadius=5; chart.markerCritColor=YELLOW; break;
            }
            chart.update(); expectFresh(chart,canvas);
        }
    }
    {
        HostCanvas canvas;
        MultiTrend chart(180,100,0,50,100,2,8); setup(chart,canvas);
        chart.setMarkerRadius(3);
        chart.values[0]=90; chart.values[2]=10; chart.setWindow(2,5); chart.show();
        bool newest=false;
        for(const auto& op:canvas.tft->ops)
            if(op.name=="fillCircle" && op.color==BLUE && op.y==chart.latestY(0)) newest=true;
        assert(newest); // window starts at 2, but marker remains the newest sample.
        chart.values[0]=20; chart.update(); expectFresh(chart,canvas);
    }
    {
        HostCanvas canvas;
        MultiTrend chart(180,100,0,50,100,3,8); setup(chart,canvas);
        chart.setMarkerRadius(2); chart.values[0]=90;
        host_allocation_fail_after=0; chart.show(); // initial cache allocation fails
        assert(chart.latest(0)==90);
        chart.x+=15; chart.y+=20; chart.update(); expectFresh(chart,canvas);
        chart.setWindow(3,4); chart.update(); chart.init(); chart.setWindow(3,4); chart.show();
        // init released the large cache; expanding the visible window must grow it.
        chart.setWindow(0,7); chart.addValue(1,70,false);
        host_allocation_fail_after=0; chart.update();
        chart.x-=10; chart.update(); expectFresh(chart,canvas);
        Trend single(180,100,0,50,100,8); setup(single,canvas);
        canvas.tft->fillScreen(BLACK);
        host_allocation_fail_after=0; single.addValue(90);
        single.y+=10; single.update(); expectFresh(single,canvas);
        host_allocation_fail_after=-1;
    }
    {
        HostCanvas canvas;
        MultiTrend chart(180,100,0,50,100,2,8); setup(chart,canvas);
        chart.showGridlines=false; chart.setMarkerRadius(2);
        for(int i=0;i<8;++i) { chart.values[i]=(i%2)*100; chart.values[8+i]=(1-i%2)*100; }
        chart.show(); chart.clear(); chart.eraseTrend();
        for(auto pixel:canvas.tft->pixels) assert(pixel!=BLUE && pixel!=GREEN);
        chart.update(); expectFresh(chart,canvas);
        Trend single(180,100,0,50,100,8); setup(single,canvas);
        single.showGridlines=false; canvas.tft->fillScreen(BLACK);
        for(int i=0;i<8;++i) single.values[i]=(i%2)*100;
        single.show(); single.clear(); single.eraseTrend();
        for(auto pixel:canvas.tft->pixels) assert(pixel!=BLUE);
        single.update(); expectFresh(single,canvas);
    }
    {
        HostCanvas canvas;
        MultiTrend chart(180,100,0,50,100,3,8); setup(chart,canvas);
        chart.setMarkerRadius(2); chart.markerWarn=30; chart.markerCrit=70; chart.show();
        canvas.tft->ops.clear();
        const auto allocations=host_allocation_calls;
        for(int i=0;i<5;++i) for(int s=0;s<3;++s) chart.addValue(s,100-i*20,false);
        assert(canvas.tft->ops.empty());
        chart.update(); assert(host_allocation_calls==allocations);
        int erased=0,drawn=0; bool foreground=false;
        for(const auto& op:canvas.tft->ops) {
            assert(op.name!="fillRect"); // no unconditional frame wipe
            if(op.name=="fillCircle") {
                if(op.color==BLACK) { assert(!foreground); ++erased; }
                else { foreground=true; ++drawn; }
            } else if(op.color!=BLACK) foreground=true;
            else assert(!foreground); // old geometry entirely erased first
        }
        assert(erased==3 && drawn==3); expectFresh(chart,canvas);
        chart.addValue(0,70); expectFresh(chart,canvas); // immediate default commits
        chart.hide(); canvas.tft->ops.clear();
        chart.clear(); chart.addValue(0,90); chart.forceUpdate(); chart.autoFit(true);
        assert(canvas.tft->ops.empty()); chart.show(); expectFresh(chart,canvas);
        for(int mode=0;mode<3;++mode) {
            canvas.tft->fillRect(chart.x+8,chart.y+8,20,20,0xF81F);
            if(mode==0) chart.forceUpdate();
            else if(mode==1) { chart.invalidate(); chart.update(); }
            else chart.show();
            expectFresh(chart,canvas);
        }
        Trend single(180,100,0,50,100,8); setup(single,canvas);
        canvas.tft->fillScreen(BLACK); single.show(); single.enableAutoFit=true;
        single.addValue(100); expectFresh(single,canvas);
        single.clear(); single.update(); expectFresh(single,canvas);
        single.hide(); canvas.tft->ops.clear(); single.addValue(20); single.forceUpdate();
        assert(canvas.tft->ops.empty()); single.show(); expectFresh(single,canvas);
        canvas.tft->fillRect(single.x+8,single.y+8,20,20,0xF81F);
        single.forceUpdate(); expectFresh(single,canvas);
        single.forceSquareWaveform=true; single.update(); expectFresh(single,canvas);
        single.setWindow(3,3); single.update(); expectFresh(single,canvas);
        chart.numSeries=0; canvas.tft->fillScreen(BLACK); chart.show();
        chart.update(); expectFresh(chart,canvas);
    }
    {
        HostCanvas canvas;
        MultiTrend chart(180,100,0,50,100,2,8); setup(chart,canvas);
        chart.setMarkerRadius(2); chart.values[0]=80;
        host_allocation_fail_after=0; chart.show();
        chart.clear(); chart.eraseTrend();
        for(auto pixel:canvas.tft->pixels) assert(pixel!=BLUE && pixel!=GREEN);
        host_allocation_fail_after=-1;
    }
    {
        HostCanvas canvas;
        Trend chart(180,100,0,50,100,8); setup(chart,canvas); chart.show();
        chart.addValue(95,false); chart.drawTrend();
        chart.addValue(10,false); chart.update(); expectFresh(chart,canvas);
        MultiTrend multi(180,100,0,50,100,2,8); setup(multi,canvas);
        canvas.tft->fillScreen(BLACK); multi.show();
        multi.addValue(0,95,false); multi.drawTrend();
        multi.addValue(0,10,false); multi.update(); expectFresh(multi,canvas);
    }
    {
        HostCanvas canvas;
        MultiTrend chart(100,80,0,50,100,1,8); setup(chart,canvas);
        chart.x=30; chart.y=40; chart.showBorder=chart.showGridlines=false;
        chart.markerRadius=12; // Public assignment intentionally does not add insets.
        chart.show(); chart.init(); chart.show(); expectFresh(chart,canvas);
    }
    {
        HostCanvas canvas;
        Trend chart(80,60,0,50,100,8); setup(chart,canvas);
        chart.x=30; chart.y=40; chart.showBorder=chart.showGridlines=false;
        chart.show(); chart.x=140; chart.drawTrend();
        chart.x=50; chart.update(); expectFresh(chart,canvas);
        MultiTrend multi(80,60,0,50,100,1,8); setup(multi,canvas);
        canvas.tft->fillScreen(BLACK); multi.markerRadius=12;
        multi.showBorder=multi.showGridlines=false; multi.show();
        multi.x=140; multi.drawTrend(); multi.x=200; multi.drawTrend();
        multi.x=50; multi.update(); expectFresh(multi,canvas);
    }
    {
        for(int dimension=0;dimension<2;++dimension) for(int size: {0,-1}) {
            HostCanvas canvas;
            MultiTrend chart(100,80,0,50,100,1,8); setup(chart,canvas);
            chart.x=30; chart.y=40; chart.showBorder=chart.showGridlines=false;
            chart.markerRadius=12; chart.show();
            if(dimension==0) chart.w=size; else chart.h=size;
            chart.update();
            for(auto pixel:canvas.tft->pixels) assert(pixel==BLACK);
            canvas.tft->ops.clear(); chart.update(); assert(canvas.tft->ops.empty());
            chart.drawTrend(); assert(canvas.tft->ops.empty());
        }
        HostCanvas canvas;
        Trend chart(80,60,0,50,100,8); setup(chart,canvas); chart.show();
        chart.w=0; chart.update();
        for(auto pixel:canvas.tft->pixels) assert(pixel==BLACK);
        canvas.tft->ops.clear(); chart.update(); assert(canvas.tft->ops.empty());
    }
    {
        HostCanvas canvas;
        MultiTrend chart(100,80,0,50,100,1,8); setup(chart,canvas);
        chart.x=30; chart.y=40; chart.showBorder=chart.showGridlines=false;
        chart.bgColor=GRAY2; chart.markerRadius=12;
        chart.show(); chart.addValue(0,100); expectFresh(chart,canvas);
        chart.w=0; chart.update();
        for(auto pixel:canvas.tft->pixels) assert(pixel==BLACK);
    }
    {
        HostCanvas canvas;
        MultiTrend chart(100,80,0,50,100,1,8); setup(chart,canvas);
        chart.x=30; chart.y=40; chart.showBorder=chart.showGridlines=false;
        chart.bgColor=GRAY2; chart.markerRadius=12;
        host_allocation_fail_after=0; chart.show();
        chart.eraseTrend();
        // OOM cleanup includes outlying markers and restores the canvas outside.
        assert(canvas.tft->pixels[chart.latestY(0)*canvas.tft->width()+135]==BLACK);
        chart.x=60; chart.update(); expectFresh(chart,canvas);
        host_allocation_fail_after=-1;
    }
    {
        HostCanvas first, second, replacement;
        MultiTrend chart(80,60,0,50,100,1,8); setup(chart,first);
        chart.markerRadius=12; chart.show(); chart.x=140; chart.drawTrend();
        const auto oldPixels=first.tft->pixels;
        first.tft->ops.clear();
        chart.setCanvas(&second); chart.x=30; chart.update();
        assert(first.tft->ops.empty() && first.tft->pixels==oldPixels);
        expectFresh(chart,second);
        chart.x=140; chart.drawTrend();
        const auto priorDisplay=second.tft;
        second.tft=replacement.tft; replacement.tft=priorDisplay; // Owned driver swap.
        const auto priorPixels=priorDisplay->pixels; priorDisplay->ops.clear();
        chart.x=30; chart.update();
        assert(priorDisplay->ops.empty() && priorDisplay->pixels==priorPixels);
        expectFresh(chart,second);
    }
    std::puts("render_charts: PASS");
}
