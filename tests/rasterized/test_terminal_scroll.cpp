#include "Terminal.h"
#include "../host/allocation_hooks.h"
#include <Fonts/FreeSans9pt7b.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>

class Panel : public Guitft {
public:
    size_t writes=0;
    Panel():Guitft(-1,-1) {}
    void drawPixel(int16_t x,int16_t y,uint16_t c) override {
        ++writes; Adafruit_ILI9341::drawPixel(x,y,c);
    }
};
class TestCanvas : public Canvas {
public:
    TestCanvas():Canvas(TFT_PORTRAIT,BLUE) { delete tft; tft=new Panel; tft->fillScreen(BLUE); }
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
    Panel& panel() { return *static_cast<Panel*>(tft); }
};
static size_t frames=0;
static void compare(Terminal& a,TestCanvas& ca,Terminal& b,TestCanvas& cb) {
    assert(a.lines==b.lines && a.linesIndex==b.linesIndex);
    for(int i=0;i<a.lines;++i) {
        assert(!std::memcmp(a.linesBuffer[i],b.linesBuffer[i],a.maxCharacters+1));
        assert(a.linesColors[i]==b.linesColors[i]);
    }
    assert(ca.tft->pixels==cb.tft->pixels); ++frames;
    size_t writes=cb.panel().writes;
    cb.tft->begins=cb.tft->ends=0;
    b.update();
    assert(writes==cb.panel().writes && !cb.tft->begins && !cb.tft->ends);
    assert(!ca.tft->selected && !cb.tft->selected && !ca.tft->writesOutside && !cb.tft->writesOutside);
}
int main() {
    for(int dir=0;dir<2;++dir) for(int size : {1,2,3}) for(bool custom : {false,true}) for(bool wrap : {false,true}) {
        TestCanvas ca,cb;
        Terminal a(160,140,dir,size), b(160,140,dir,size,Terminal::ScrollStorage::RotateRows);
        a.setCanvas(&ca); b.setCanvas(&cb);
        a.x=b.x=wrap?210:12; a.y=b.y=30;
        ca.tft->setFont(custom?&FreeSans9pt7b:nullptr); cb.tft->setFont(custom?&FreeSans9pt7b:nullptr);
        ca.tft->setTextWrap(wrap); cb.tft->setTextWrap(wrap);
        a.show(); b.show(); compare(a,ca,b,cb);
        for(int i=0;i<16;++i) {
            const char* text=i%4==0?"A\nB":i%4==1?"8888":i%4==2?"Wj":"";
            a.print(text,i%2?RED:GREEN); b.print(text,i%2?RED:GREEN); compare(a,ca,b,cb);
        }
        // Actual glyphs must exist: a fake text seam cannot satisfy this test.
        a.print("8",RED); b.print("8",RED); compare(a,ca,b,cb);
        assert(std::count(cb.tft->pixels.begin(),cb.tft->pixels.end(),RED)>0);
        a.scrollDown(); b.scrollDown(); a.update(); b.update(); compare(a,ca,b,cb);
        a.scrollUp(); b.scrollUp(); a.update(); b.update(); compare(a,ca,b,cb);
        a.keepColors=b.keepColors=false; a.update(); b.update(); compare(a,ca,b,cb);
        std::strcpy(a.linesBuffer[0],"x"); std::strcpy(b.linesBuffer[0],"x");
        a.update(); b.update(); compare(a,ca,b,cb);
        a.hide(); b.hide(); size_t writes=cb.panel().writes;
        for(int i=0;i<12;++i) { a.print("hide",YELLOW); b.print("hide",YELLOW); }
        assert(cb.panel().writes==writes);
        a.show(); b.show(); compare(a,ca,b,cb);
        // Existing geometry setters change drawing only; allocation geometry is fixed.
        a.setSize(90,80); b.setSize(90,80); a.fontSize=b.fontSize=1;
        a.x=b.x=8; a.y=b.y=170; a.update(); b.update(); compare(a,ca,b,cb);
        a.clear(); b.clear(); compare(a,ca,b,cb);
        assert(std::count(cb.tft->pixels.begin(),cb.tft->pixels.end(),RED)==0);
        a.forceUpdate(); b.forceUpdate(); compare(a,ca,b,cb);
    }
    // Snapshot allocation may fail at construction and repeatedly while rows rotate.
    // Use identical fault schedules and verify eventual caching suppresses all work.
    for(int dir=0;dir<2;++dir) {
        TestCanvas ca,cb;
        host_allocation_fail_after=5; Terminal a(120,70,dir);
        host_allocation_fail_after=5; Terminal b(120,70,dir,1,Terminal::ScrollStorage::RotateRows);
        a.setCanvas(&ca); b.setCanvas(&cb);
        host_allocation_fail_after=0; a.show();
        host_allocation_fail_after=0; b.show();
        for(int i=0;i<12;++i) {
            host_allocation_fail_after=0; a.print("cache",RED+i);
            host_allocation_fail_after=0; b.print("cache",RED+i);
            assert(ca.tft->pixels==cb.tft->pixels); ++frames;
        }
        host_allocation_fail_after=-1; a.update(); b.update(); compare(a,ca,b,cb);
        a.clear(); b.clear(); compare(a,ca,b,cb);
    }
    std::printf("terminal scroll real GFX: %zu full-panel comparisons PASS (24 font/direction/wrap scenarios + 2 cache-OOM)\n",frames);
}
