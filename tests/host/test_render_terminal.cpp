#include "Terminal.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

#ifdef TERMINAL_REAL_GFX
#include <algorithm>
class CountingTerminalTft : public Guitft {
public:
    size_t pixelWrites=0;
    CountingTerminalTft() : Guitft(-1,-1) {}
    void drawPixel(int16_t x,int16_t y,uint16_t color) override {
        if(x>=0 && y>=0 && x<width() && y<height()) ++pixelWrites;
        Adafruit_ILI9341::drawPixel(x,y,color);
    }
};
class RasterTerminalCanvas : public Canvas {
public:
    RasterTerminalCanvas() : Canvas(TFT_PORTRAIT, BLUE) {
        delete tft; tft=new CountingTerminalTft; tft->fillScreen(BLUE);
    }
    size_t writes() const { return static_cast<CountingTerminalTft*>(tft)->pixelWrites; }
    void reset() { static_cast<CountingTerminalTft*>(tft)->pixelWrites=0; }
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void cleanMatches(Terminal& t, RasterTerminalCanvas& c) {
    RasterTerminalCanvas fresh; Terminal expected(120,70,t.direction);
    expected.setCanvas(&fresh); expected.x=t.x; expected.y=t.y; expected.w=t.w; expected.h=t.h;
    expected.fontSize=t.fontSize; expected.lineSpace=t.lineSpace;
    expected.borderWidth=t.borderWidth; expected.borderColor=t.borderColor;
    expected.bgColor=t.bgColor; expected.fgColor=t.fgColor; expected.keepColors=t.keepColors;
    expected.horizontalBleed=t.horizontalBleed; expected.verticalBleed=t.verticalBleed;
    for(int i=0;i<t.lines;++i) { std::strcpy(expected.linesBuffer[i],t.linesBuffer[i]); expected.linesColors[i]=t.linesColors[i]; }
    expected.show();
    if(c.tft->pixels!=fresh.tft->pixels) {
        size_t diff=0, first=0;
        for(size_t i=0;i<c.tft->pixels.size();++i) if(c.tft->pixels[i]!=fresh.tft->pixels[i]) { if(!diff) first=i; ++diff; }
        std::fprintf(stderr,"raster mismatch dir=%u font=%u text0=%s diff=%zu first=(%zu,%zu)\n",t.direction,t.fontSize,t.linesBuffer[0],diff,first%240,first/240);
    }
    assert(c.tft->pixels==fresh.tft->pixels);
}
static void rasterBenchmark() {
    RasterTerminalCanvas c; Terminal t(120,70,TERMINAL_SCROLL_UP);
    t.setCanvas(&c); t.x=20; t.y=30; t.show(); c.reset();
    for(int i=0;i<100;++i) t.update();
    std::printf("raster unchanged100: pixel_writes=%zu\n",c.writes());
    c.reset(); t.print("alpha",RED); std::printf("raster first_up: pixel_writes=%zu\n",c.writes());
    c.reset(); t.print("beta",GREEN); std::printf("raster second_up: pixel_writes=%zu\n",c.writes());
    c.reset(); for(int i=0;i<100;++i) t.update();
    std::printf("raster unchanged_loaded100: pixel_writes=%zu\n",c.writes());
    for(int i=0;i<t.lines;++i) t.print(std::to_string(i).c_str(),RED+i);
    c.reset(); t.print("shift",GREEN); std::printf("raster full_shift_up: pixel_writes=%zu\n",c.writes());
    c.reset(); t.clear(); std::printf("raster clear: pixel_writes=%zu\n",c.writes());
    Terminal down(120,70,TERMINAL_SCROLL_DOWN); down.setCanvas(&c); down.x=20; down.y=30; down.show();
    c.reset(); down.print("alpha",RED); std::printf("raster first_down: pixel_writes=%zu\n",c.writes());
    std::fflush(stdout);
}
int main() {
#ifdef TERMINAL_TRANSACTIONS
    {
        RasterTerminalCanvas c; Terminal t(120,70,TERMINAL_SCROLL_UP); t.setCanvas(&c); t.show();
        c.tft->begins=c.tft->ends=0; t.print("alpha",RED);
        std::printf("terminal row transactions: %u/%u\n",c.tft->begins,c.tft->ends); std::fflush(stdout);
        assert(c.tft->begins==1 && c.tft->ends==1 && !c.tft->selected && !c.tft->writesOutside);
        c.tft->begins=c.tft->ends=0; t.update();
        assert(!c.tft->begins && !c.tft->ends);
        t.hide(); c.tft->begins=c.tft->ends=0; t.print("hidden",GREEN); t.update();
        assert(!c.tft->begins && !c.tft->ends && !c.tft->selected);
    }
#endif
    rasterBenchmark();
#ifdef TERMINAL_BENCH_ONLY
    return 0;
#endif
    for(int dir=0;dir<2;++dir) {
        RasterTerminalCanvas c; Terminal t(120,70,dir); t.setCanvas(&c); t.x=20; t.y=30; t.show();
        for(int i=0;i<12;++i) { t.print(std::to_string(i).c_str(),i%2?RED:GREEN); cleanMatches(t,c); }
        std::strcpy(t.linesBuffer[0],"x"); t.update(); cleanMatches(t,c);
        t.linesBuffer[0][0]=0; t.update(); cleanMatches(t,c);
        t.keepColors=false; t.update(); cleanMatches(t,c);
        t.fontSize=3; t.update(); cleanMatches(t,c);
        t.clear(); cleanMatches(t,c);
    }
    // Real classic-font overflow and wrapping, not a no-op print stub.
    RasterTerminalCanvas c; Terminal t(120,70,TERMINAL_SCROLL_UP); t.setCanvas(&c);
    t.x=210; t.y=30; t.fontSize=2; t.borderWidth=0; t.borderColor=BLACK; t.show();
    t.print("8888",RED);
    const auto glyphs=std::count(c.tft->pixels.begin(),c.tft->pixels.end(),RED);
    assert(glyphs>0);
    t.fontSize=1; c.tft->setTextWrap(false); t.clear();
    const auto stale=std::count(c.tft->pixels.begin(),c.tft->pixels.end(),RED);
    std::printf("terminal real GFX: glyph_pixels=%zu stale_after_clear=%zu\n",(size_t)glyphs,(size_t)stale);
    assert(stale==0);
    // Multi-line input overlaps later logical rows: redraw the complete damaged set.
    t.x=20; t.y=30; t.w=120; t.h=70; t.update(); t.print("A\nB",RED); t.print("C",GREEN);
    cleanMatches(t,c); t.linesBuffer[0][0]=0; t.update(); cleanMatches(t,c);
    // Movement removes both old widget and wrapped/outlying glyphs.
    t.clear(); t.fontSize=3; t.x=190; t.print("8888",RED);
    t.x=10; t.y=150; t.fontSize=1; t.clear();
    assert(std::count(c.tft->pixels.begin(),c.tft->pixels.end(),RED)==0);
    std::puts("rasterized Terminal: PASS");
}
#else
class TerminalCanvas : public Canvas {
public:
    TerminalCanvas() : Canvas(TFT_PORTRAIT, BLUE) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void setup(Terminal& t, TerminalCanvas& c) {
    t.setCanvas(&c); t.x=20; t.y=30; t.show(); c.tft->ops.clear();
}
static void report(const char* label, const TerminalCanvas& c) {
    size_t fills=0, pixels=0, text=0, frame=0;
    for(const auto& o:c.tft->ops) {
        if(o.name=="fillRect") { ++fills; if(o.w>0&&o.h>0) pixels+=(size_t)o.w*o.h; }
        if(o.name.find("text:")==0) ++text;
        if(o.name=="rect") ++frame;
    }
    std::printf("%s: ops=%zu fills=%zu filled_pixels=%zu texts=%zu frames=%zu\n",
                label,c.tft->ops.size(),fills,pixels,text,frame);
}
static void benchmark() {
    TerminalCanvas c; Terminal t(120,70,TERMINAL_SCROLL_UP); setup(t,c);
    for(int i=0;i<100;++i) t.update(); report("unchanged100",c);
    c.tft->ops.clear(); t.print("alpha",RED); report("first_up",c);
    c.tft->ops.clear(); t.print("beta",GREEN); report("second_up",c);
    c.tft->ops.clear(); for(int i=0;i<100;++i) t.update(); report("unchanged_loaded100",c);
    for(int i=0;i<t.lines;++i) t.print(std::to_string(i).c_str(),RED+i);
    c.tft->ops.clear(); t.print("shift",GREEN); report("full_shift_up",c);
    c.tft->ops.clear(); t.clear(); report("clear",c);
    Terminal down(120,70,TERMINAL_SCROLL_DOWN); setup(down,c);
    c.tft->ops.clear(); down.print("alpha",RED); report("first_down",c);
    std::printf("sizeof(Terminal)=%zu lines=%u capacity=%d\n",sizeof(Terminal),t.lines,t.maxCharacters);
    std::fflush(stdout);
}
int main(int argc,char**) {
    benchmark(); if(argc>1) return 0;
#ifndef TERMINAL_BENCH_ONLY
    TerminalCanvas c; Terminal t(120,70,TERMINAL_SCROLL_UP); setup(t,c);
    t.update(); assert(c.tft->ops.empty());
    // Appending before capacity changes only its row, without frame work.
    t.print("alpha",RED);
    assert(c.tft->ops.size()==2 && c.tft->ops.back().name=="text:alpha");
    assert(c.tft->ops.back().color==RED);
    assert(c.tft->ops.front().name=="fillRect" && c.tft->ops.front().h==12);
    c.tft->ops.clear(); t.print("beta",GREEN);
    assert(c.tft->ops.size()==2 && c.tft->ops.back().name=="text:beta");
    assert(c.tft->ops.back().y==t.y+t.borderWidth+t.verticalBleed+12);
    // Software shifts keep the text/color correspondence in both directions.
    for(int dir=0;dir<2;++dir) {
        Terminal s(120,70,dir); setup(s,c);
        for(int i=0;i<s.lines;++i) s.print(std::to_string(i).c_str(),RED+i);
        c.tft->ops.clear(); s.print("new",GREEN);
        size_t texts=0;
        for(const auto& o:c.tft->ops) {
            assert(o.name!="rect");
            if(o.name=="fillRect") assert(o.h<=12);
            if(o.name.find("text:")==0) {
                int row=(o.y-s.y-s.borderWidth-s.verticalBleed)/12;
                assert(o.name==std::string("text:")+s.linesBuffer[row]);
                assert(o.color==s.linesColors[row]); ++texts;
            }
        }
        assert(texts==s.lines);
        c.tft->ops.clear(); s.update(); assert(c.tft->ops.empty());
        s.clear(); assert(!c.tft->ops.empty());
        for(const auto& o:c.tft->ops) assert(o.name.find("text:")!=0 && o.name!="rect");
        c.tft->ops.clear(); s.clear(); s.update(); assert(c.tft->ops.empty());
    }
    // Public buffer/color edits participate in last-rendered comparison.
    std::strcpy(t.linesBuffer[0],"longer"); c.tft->ops.clear(); t.update();
    assert(c.tft->ops.size()==2 && c.tft->ops.back().name=="text:longer");
    int tx=t.x+t.borderWidth+t.horizontalBleed, ty=t.y+t.borderWidth+t.verticalBleed;
    c.tft->drawPixel(tx+30,ty,WHITE);
    std::strcpy(t.linesBuffer[0],"x"); c.tft->ops.clear(); t.update();
    assert(c.tft->pixels[ty*c.tft->width()+tx+30]==t.bgColor);
    t.linesColors[0]=YELLOW; c.tft->ops.clear(); t.update();
    assert(c.tft->ops.back().color==YELLOW);
    t.linesBuffer[0][0]=0; c.tft->ops.clear(); t.update();
    assert(c.tft->ops.size()==1 && c.tft->ops.front().name=="fillRect");
    // Style, direction, and geometry changes are not suppressed by equal bytes.
    auto change=[&]() { c.tft->ops.clear(); t.update(); assert(!c.tft->ops.empty());
        c.tft->ops.clear(); t.update(); assert(c.tft->ops.empty()); };
    t.bgColor=GREEN; change(); t.fgColor=WHITE; change();
    t.borderColor=RED; change(); t.borderWidth=2; change();
    t.direction=TERMINAL_SCROLL_DOWN; change(); t.keepColors=false; change();
    t.fontSize=2; change(); t.lineSpace=3; change();
    t.horizontalBleed=1; change(); t.verticalBleed=2; change(); t.fontHeight=3; change();
    int ox=t.x, oy=t.y; c.tft->drawPixel(ox,oy,WHITE);
    t.x+=130; t.y+=40; t.w=40; t.h=30; change();
    assert(c.tft->pixels[oy*c.tft->width()+ox]==c.bgColor);
    // Pending writes do not draw while hidden, and show/force restore damage.
    t.hide(); c.tft->ops.clear(); t.print("hidden",RED); t.clear(); t.print("pending",RED);
    t.update(); assert(c.tft->ops.empty()); t.show(); assert(!c.tft->ops.empty());
    c.tft->ops.clear(); t.show(); assert(!c.tft->ops.empty());
    c.tft->ops.clear(); t.forceUpdate(); assert(!c.tft->ops.empty());
    c.tft->ops.clear(); t.update(); assert(c.tft->ops.empty());
    Terminal deferred(120,70,TERMINAL_SCROLL_UP); setup(deferred,c);
    deferred.hide(); c.tft->ops.clear(); deferred.print("deferred",YELLOW); assert(c.tft->ops.empty());
    deferred.visible=true; deferred.update();
    bool restoredFrame=false;
    for(const auto& o:c.tft->ops) if(o.name=="rect") restoredFrame=true;
    assert(restoredFrame);
    // Constructor failure never leaves partially-owned line storage.
    for(int fail=0;fail<5;++fail) {
        host_allocation_fail_after=fail; Terminal empty(120,70);
        assert(empty.lines==0);
        for(auto buffer:empty.linesBuffer) assert(buffer==nullptr);
        empty.print("inert"); empty.clear(); empty.show();
    }
    // Snapshot OOM retains data; safe uncached draw, then retry caches it.
    host_allocation_fail_after=5; Terminal fallback(120,70,TERMINAL_SCROLL_UP);
    assert(fallback.lines==5); host_allocation_fail_after=0; setup(fallback,c);
    host_allocation_fail_after=0; fallback.print("keep",RED); auto* owned=fallback.linesBuffer[0];
    int oldX=fallback.x, oldY=fallback.y; c.tft->drawPixel(oldX,oldY,WHITE);
    host_allocation_fail_after=0; fallback.x+=130; fallback.update();
    assert(c.tft->pixels[oldY*c.tft->width()+oldX]==c.bgColor);
    assert(fallback.linesBuffer[0]==owned && std::strcmp(owned,"keep")==0);
    c.tft->ops.clear(); fallback.update();
    c.tft->ops.clear(); fallback.update(); assert(c.tft->ops.empty());
    // Changing targets must not erase or otherwise touch the former driver.
    TerminalCanvas other; size_t formerOps=c.tft->ops.size();
    fallback.setCanvas(&other); fallback.update();
    assert(!other.tft->ops.empty() && c.tft->ops.size()==formerOps);
    // Effective row colors with keepColors disabled follow the legacy policy.
    Terminal policy(120,70,TERMINAL_SCROLL_DOWN); setup(policy,c);
    policy.keepColors=false; policy.print("first",RED); c.tft->ops.clear(); policy.print("next",GREEN);
    for(const auto& o:c.tft->ops) if(o.name=="text:first") assert(o.color==policy.fgColor);
    c.tft->ops.clear(); policy.scrollDown(); assert(c.tft->ops.empty()); policy.update();
    assert(!c.tft->ops.empty());
    policy.clear(); for(int i=0;i<policy.lines;++i) policy.print("same",RED);
    c.tft->ops.clear(); policy.keepColors=true; policy.update(); c.tft->ops.clear();
    // Fill with equal retained bytes/colors, so a shift changes no pixels.
    for(int i=0;i<policy.lines;++i) policy.print("same",RED);
    c.tft->ops.clear(); policy.print("same",RED); assert(c.tft->ops.empty());
    // Repeated construction/show never reallocates line storage on geometry changes.
    for(int i=0;i<30;++i) {
        Terminal again(74,34,TERMINAL_SCROLL_UP); setup(again,c);
        auto* buffer=again.linesBuffer[0];
        for(int j=0;j<3;++j) { again.w+=10; again.fontSize=j; again.show(); }
        assert(again.linesBuffer[0]==buffer && again.maxCharacters==10);
    }
    std::puts("render_terminal: PASS");
#endif
}
#endif
