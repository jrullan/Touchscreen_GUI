#include "Trend.h"
#include "MultiTrend.h"
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

struct AxisCanvas : Canvas {
    AxisCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
template<class Chart> struct Axis : Chart {
    using Chart::Chart;
    using TrendBase::drawXScale;
    int stripTop() const { return this->y+this->chartH(); }
};
static void callerState(Guitft& panel, const GFXfont* font, bool wrap, int size) {
    panel.setFont(font);
    panel.setCursor(201, 46);
    panel.setTextWrap(wrap);
    panel.setTextSize(size);
    panel.setTextColor(0x37e5, 0x2842);
}
template<class Chart> static void check(Axis<Chart>& chart, AxisCanvas& canvas,
                                      const GFXfont* font, bool wrap, int size,
                                      bool disabled) {
    auto& panel=*canvas.tft;
    callerState(panel, nullptr, false, 1);
    std::fill(panel.pixels.begin(),panel.pixels.end(),0);
    chart.showXScale=!disabled;
    chart.drawXScale();
    const auto classic=panel.pixels;
    int glyphs=0;
    // This calls only drawXScale: no Y labels, frame, samples or marker pixels.
    const int textTop=std::min(chart.stripTop()+chart.borderWidth+18,chart.y+chart.h-8);
    for(int y=0;y<panel.height();++y) for(int x=0;x<panel.width();++x) {
        if(!classic[y*panel.width()+x]) continue;
        assert(!disabled);
        assert(x>=chart.x && x<chart.x+chart.w);
        assert(y>=chart.stripTop() && y<chart.y+chart.h);
        if(y>=textTop) ++glyphs;
    }
    if(!disabled && chart.w>=6) assert(glyphs>0);
    if(disabled) assert(std::all_of(classic.begin(),classic.end(),[](uint16_t p){return p==0;}));

    callerState(panel,font,wrap,size);
    std::fill(panel.pixels.begin(),panel.pixels.end(),0);
    chart.drawXScale();
    if(panel.pixels!=classic) {
        int outside=0,rightmost=-1;
        for(int y=0;y<panel.height();++y) for(int x=0;x<panel.width();++x)
            if(panel.pixels[y*panel.width()+x]) {
                rightmost=std::max(rightmost,x);
                if(x<chart.x || x>=chart.x+chart.w || y<chart.stripTop() || y>=chart.y+chart.h) ++outside;
            }
        std::fprintf(stderr,"X-axis font leak: outside_axis_pixels=%d rightmost=%d widget_right_exclusive=%d\n",outside,rightmost,chart.x+chart.w);
        assert(false && "X-axis must rasterize classic glyphs regardless of caller font");
    }
    // Observe all caller drawing state by its next real GFX rendering, including
    // inherited font, cursor, wrap, X/Y size, and foreground/background colors.
    Guitft reference(-1,-1);
    callerState(reference,font,wrap,size);
    assert(panel.getCursorX()==reference.getCursorX());
    assert(panel.getCursorY()==reference.getCursorY());
    std::fill(panel.pixels.begin(),panel.pixels.end(),0);
    panel.print("Az09 wrap\nQ");
    reference.print("Az09 wrap\nQ");
    assert(panel.pixels==reference.pixels);
    assert(panel.getCursorX()==reference.getCursorX());
    assert(panel.getCursorY()==reference.getCursorY());
}
template<class Chart> static void matrix(Axis<Chart>& chart, AxisCanvas& canvas) {
    chart.setCanvas(&canvas); chart.x=10; chart.y=80; chart.showYScale=false;
    chart.borderColor=WHITE; chart.bgColor=BLACK;
    unsigned count=0;
    // Start with the independently confirmed 120x100 / FreeSans18 / 3600s repro.
    chart.w=120; chart.h=100; chart.setWindow(0,254); chart.setMaxX(3600);
    check(chart,canvas,&FreeSans18pt7b,false,1,false);
    for(const GFXfont* font : {static_cast<const GFXfont*>(nullptr), &FreeSans18pt7b, &FreeSans24pt7b})
    for(bool wrap : {false,true}) for(int size : {1,3})
    for(int width : {1,6,12,40,120}) for(int seconds : {0,60,3600})
    for(bool disabled : {false,true}) {
        chart.w=width; chart.setMaxX(seconds);
        chart.setWindow(0,254);
        check(chart,canvas,font,wrap,size,disabled); ++count;
    }
    std::printf("real-GFX X-axis cases=%u PASS\n",count+1);
}
int main(int argc, char** argv) {
    AxisCanvas canvas;
    assert(argc==2);
    if(std::string(argv[1])=="Trend") {
        Axis<Trend> trend(120,100,BLACK,50,100,255);
        matrix(trend,canvas);
    } else {
        Axis<MultiTrend> multi(120,100,BLACK,50,100,2,255);
        matrix(multi,canvas);
    }
    std::puts("xaxis real glyphs/state: PASS");
}
