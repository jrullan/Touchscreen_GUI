#include "Display.h"
#include "../host/allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <Fonts/FreeMono9pt7b.h>
class DeltaCanvas : public Canvas {
public:
    DeltaCanvas():Canvas(TFT_PORTRAIT,BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void setup(Display& d, DeltaCanvas& c) {
    d.setCanvas(&c); d.x=20; d.y=30; d.setSize(180,40);
    d.borderWidth=2; d.fontSize=2; d.setColors(BLACK,WHITE,GREEN);
}
static void clockCases() {
    const char* values[]={"12:34:56","12:34:57","12:34:59","12:35:00",
        "12:59:59","13:00:00","23:59:59","00:00:00","00:00:00"};
    for(int rotation=0;rotation<4;++rotation) for(int scale=1;scale<=3;++scale)
        for(int border : {0,2}) {
            DeltaCanvas c, oracle; Display d(8), full(8);
            setup(d,c); setup(full,oracle);
            c.tft->setRotation(rotation); oracle.tft->setRotation(rotation);
            d.fontSize=full.fontSize=scale; d.borderWidth=full.borderWidth=border;
            d.setText((char*)values[0],true); full.setText((char*)values[0],true);
            for(unsigned n=1;n<sizeof(values)/sizeof(*values);++n) {
                c.tft->resetWrites();
                // Dirty draws must establish print state even after unrelated printing.
                c.tft->setTextColor(RED,BLUE); c.tft->setTextSize(4); c.tft->setCursor(0,0);
                d.setText((char*)values[n],true);
                full.setText((char*)values[n],false); full.update();
                const int tx=d.getCenterTextX(d.x,d.w,8), ty=d.getCenterTextY(d.y,d.h);
                unsigned dirty=0;
                for(int i=0;i<8;++i) dirty+=values[n][i]!=values[n-1][i];
                assert(c.tft->pixelWrites==dirty*48u*scale*scale);
                for(int y=0;y<c.tft->height();++y) for(int x=0;x<c.tft->width();++x) {
                    const int i=(x-tx)/(6*scale);
                    bool changed=x>=tx && x<tx+8*6*scale && y>=ty && y<ty+8*scale &&
                        values[n][i]!=values[n-1][i];
                    assert(c.tft->writes[y*c.tft->width()+x]==unsigned(changed));
                }
                assert(c.tft->pixels==oracle.tft->pixels);
                if(dirty) {
                    assert(c.tft->getCursorX()==oracle.tft->getCursorX());
                    assert(c.tft->getCursorY()==oracle.tft->getCursorY());
                    assert(c.tft->foreground()==WHITE && c.tft->background()==WHITE);
                    assert(c.tft->textSize()==scale);
                }
            }
        }
    std::puts("clock: four rotations, three scales, border/no border, rollover, noop PASS");
}
static void fallbackCases() {
    for(int mode=0;mode<8;++mode) {
        DeltaCanvas c, oracle; Display d(8), full(8); setup(d,c); setup(full,oracle);
        if(mode==0) { d.fontSize=full.fontSize=0; }
        if(mode==1) { d.x=full.x=210; d.setSize(30,20); full.setSize(30,20); }
        if(mode==2) { d.x=full.x=-30; }
        if(mode==3) { d.y=full.y=-15; }
        if(mode==4) { c.tft->setFont(&FreeMono9pt7b); oracle.tft->setFont(&FreeMono9pt7b); }
        if(mode==5) { d.setSize(40,20); full.setSize(40,20); }
        if(mode==6) { d.fontSize=full.fontSize=1; c.tft->setTextWrap(false); oracle.tft->setTextWrap(false); }
        if(mode==7) { d.borderWidth=full.borderWidth=0; }
        const char* values[]={"88:88:88","88:88:87","8","88\n88","87\n88","","8888",""};
        for(const char* value:values) {
            d.setText((char*)value,true); full.setText((char*)value,false); full.update();
            assert(c.tft->pixels==oracle.tft->pixels);
#ifndef DISPLAY_DELTAS_BASELINE
            assert(c.tft->textLayout().font==oracle.tft->textLayout().font);
            assert(c.tft->textLayout().flags==oracle.tft->textLayout().flags);
#endif
        }
    }
    // Shared driver state changes must invalidate identical text suppression.
    DeltaCanvas c, oracle; Display d(8), full(8); setup(d,c); setup(full,oracle);
    d.setText((char*)"88888888",true); full.setText((char*)"88888888",true);
    for(int step=0;step<10;++step) {
        if(step==0) { c.tft->setTextWrap(false); oracle.tft->setTextWrap(false); }
        if(step==1) { c.tft->setFont(&FreeMono9pt7b); oracle.tft->setFont(&FreeMono9pt7b); }
        if(step==2) { c.tft->setFont(nullptr); oracle.tft->setFont(nullptr); }
        if(step==3) { d.fontSize=full.fontSize=1; }
        if(step==4) { d.fgColor=full.fgColor=RED; }
        if(step==5) { d.bgColor=full.bgColor=BLUE; }
        if(step==6) { d.borderColor=full.borderColor=YELLOW; }
        if(step==7) { d.borderWidth=full.borderWidth=0; }
        if(step==8) { d.x=full.x=50; d.y=full.y=100; }
        if(step==9) { d.setSize(100,30); full.setSize(100,30); }
        c.tft->resetWrites();
        d.setText(d.contents->text,true); full.setText(full.contents->text,false); full.update();
        assert(c.tft->pixelWrites>0 && c.tft->pixels==oracle.tft->pixels);
    }
    // Repeated show and explicit repairs restore pixels outside glyphs/border.
    for(int repair=0;repair<3;++repair) {
        c.tft->drawPixel(d.x+2,d.y+2,GREEN);
        if(repair==0) d.update(); else if(repair==1) d.forceUpdate(); else d.show();
        assert(c.tft->pixels==oracle.tft->pixels);
    }
    d.hide(); c.tft->resetWrites(); d.setText((char*)"hidden",true);
    assert(c.tft->pixelWrites==0); d.show();
    full.setText((char*)"hidden",false); full.update(); assert(c.tft->pixels==oracle.tft->pixels);
    d.setText((char*)"hidden",false); c.tft->resetWrites();
    d.setText((char*)"hiddex",false); assert(c.tft->pixelWrites==0);
    d.update(); full.setText((char*)"hiddex",false); full.update(); assert(c.tft->pixels==oracle.tft->pixels);
    std::puts("fallback: font0/custom/wrap/clip/control/shorten/style/move/show/repair/defer PASS");
}
static void ownershipCases() {
    for(int border : {0,2}) for(int failure=0;failure<=2;++failure) {
        DeltaCanvas c, oracle; Display full(8); setup(full,oracle); full.borderWidth=border;
        host_allocation_fail_after=failure; Display d(8); host_allocation_fail_after=-1;
        setup(d,c); d.borderWidth=border;
        const char* values[]={"12:34:56","12:34:57","7","","12:59:59"};
        for(const char* value:values) {
            c.tft->resetWrites(); d.setText((char*)value,true);
            full.setText((char*)(failure==0?"":value),false); full.update();
            assert(c.tft->pixels==oracle.tft->pixels);
            // Text or snapshot failure remains a safe full repaint, every time.
            if(failure<2) assert(c.tft->pixelWrites>=unsigned((d.w-2*border)*(d.h-2*border)));
        }
    }
    DeltaCanvas c, oracle; Display d(8), full(8); setup(d,c); setup(full,oracle);
    d.setText((char*)"12:34:56",true);
    d.contents->text[7]='7'; c.tft->resetWrites(); d.setText(d.contents->text,true);
    full.setText((char*)"12:34:57",true);
    assert(c.tft->pixelWrites==192 && c.tft->pixels==oracle.tft->pixels);
    d.setText(d.contents->text+1,true); full.setText((char*)"2:34:57",false); full.update();
    assert(c.tft->pixels==oracle.tft->pixels);
    d.setText(nullptr,true); full.setText(nullptr,false); full.update(); assert(c.tft->pixels==oracle.tft->pixels);
    // Rotation and encoding changes are layout changes even for identical bytes.
    d.setText((char*)"\xB0\xB1",true); full.setText((char*)"\xB0\xB1",false); full.update();
    c.tft->cp437(true); oracle.tft->cp437(true); c.tft->resetWrites();
    d.setText(d.contents->text,true); full.update();
    assert(c.tft->pixelWrites>192 && c.tft->pixels==oracle.tft->pixels);
    c.tft->setRotation(2); oracle.tft->setRotation(2); c.tft->resetWrites();
    d.setText(d.contents->text,true); full.update();
    assert(c.tft->pixelWrites>192 && c.tft->pixels==oracle.tft->pixels);
    std::puts("ownership: OOM text/snapshot/first render, border/no border, aliases/null, encoding/rotation PASS");
}
int main(int argc,char**) {
    DeltaCanvas c; Display d(8); setup(d,c);
    d.setText((char*)"12:34:56",true);
    c.tft->resetWrites(); d.setText((char*)"12:34:57",true);
    std::printf("one-cell pixel writes=%zu budget=192 sizeof(Display)=%zu\n",c.tft->pixelWrites,sizeof(Display));
    std::fflush(stdout);
    if(argc>1) return 0; // Measurement-only, including git-show baseline.
    assert(c.tft->pixelWrites==192);
    const int tx=d.getCenterTextX(d.x,d.w,8), ty=d.getCenterTextY(d.y,d.h);
    for(int y=0;y<c.tft->height();++y) for(int x=0;x<c.tft->width();++x)
        assert(c.tft->writes[y*c.tft->width()+x] == unsigned(x>=tx+7*12 && x<tx+8*12 && y>=ty && y<ty+16));
    DeltaCanvas oracle; Display full(8); setup(full,oracle);
    full.setText((char*)"12:34:57",true);
    assert(c.tft->pixels==oracle.tft->pixels);
    assert(c.tft->getCursorX()==oracle.tft->getCursorX());
    assert(c.tft->getCursorY()==oracle.tft->getCursorY());
    assert(c.tft->foreground()==WHITE && c.tft->background()==WHITE && c.tft->textSize()==2);
    // Equal fg/bg in GFX means transparent drawChar, but a delta must still
    // replace the entire changed cell, including its spacing column.
    d.setColors(BLACK,BLACK,GREEN); d.setText((char*)"12:34:57",true);
    for(int y=ty;y<ty+16;++y) for(int x=tx+7*12;x<tx+8*12;++x)
        c.tft->drawPixel(x,y,RED);
    c.tft->resetWrites(); d.setText((char*)"12:34:58",true);
    assert(c.tft->pixelWrites==192);
    for(int y=ty;y<ty+16;++y) for(int x=tx+7*12;x<tx+8*12;++x)
        assert(c.tft->pixels[y*c.tft->width()+x]==BLACK);
    // Another widget can change the driver's font without changing fontSize.
    d.setColors(BLACK,WHITE,GREEN); d.setText((char*)"12:34:58",true);
    c.tft->setFont(&FreeMono9pt7b); c.tft->resetWrites();
    d.setText((char*)"12:34:58",true);
    assert(c.tft->pixelWrites>192);
    oracle.tft->setFont(&FreeMono9pt7b); full.setText((char*)"12:34:58",true);
    assert(c.tft->pixels==oracle.tft->pixels);
#ifndef DISPLAY_DELTAS_BASELINE
    assert(c.tft->textLayout().font==&FreeMono9pt7b);
#endif
    clockCases(); fallbackCases(); ownershipCases();
    std::puts("display_deltas raster: PASS");
}
