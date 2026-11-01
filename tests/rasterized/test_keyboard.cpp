#include "Keyboard.h"
#include "Screen.h"
#include <Fonts/FreeSans18pt7b.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr uint16_t sentinel = 0x1234;
class RasterCanvas : public Canvas {
public:
    RasterCanvas() : Canvas(TFT_PORTRAIT, sentinel) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
void require(bool value, const std::string& message) {
    if (!value) throw std::runtime_error(message);
}
void setup(RasterCanvas& c, Keyboard& k, int x=0, int y=100, int w=240, int h=192) {
    c.tft->fillScreen(sentinel);
    k.setSize(w,h); k.borderWidth=2; k.fontSize=1;
    k.setColors(BLACK,WHITE,RED);
    c.add(&k,x,y,false);
}
void samePixels(const std::vector<uint16_t>& actual, const std::vector<uint16_t>& expected,
                const std::string& context) {
    require(actual.size()==expected.size(), context+": framebuffer size");
    for (size_t i=0; i<actual.size(); ++i) {
        if (actual[i]!=expected[i]) {
            char detail[160];
            std::snprintf(detail,sizeof(detail)," at (%zu,%zu): actual=0x%04x expected=0x%04x",
                          i%240,i/240,actual[i],expected[i]);
            require(false,context+detail);
        }
    }
}
void outsideUnchanged(const RasterCanvas& c, const Keyboard& k,
                      const std::vector<uint16_t>& before, const char* context) {
    for (int y=0; y<c.tft->height(); ++y) for (int x=0; x<c.tft->width(); ++x) {
        if (x>=k.x && x<static_cast<int64_t>(k.x)+k.w &&
            y>=k.y && y<static_cast<int64_t>(k.y)+k.h) continue;
        const size_t i=y*c.tft->width()+x;
        if (c.tft->pixels[i]!=before[i]) {
            char detail[160];
            std::snprintf(detail,sizeof(detail),"%s: stray pixel outside keyboard (%d,%d), color=0x%04x",
                          context,x,y,c.tft->pixels[i]);
            require(false,detail);
        }
    }
}
// Independent raster oracle: declared public key labels, rendered by real GFX.
// No Keyboard private methods or mocked drawString/print implementations.
std::vector<uint16_t> expectedFrame(const Keyboard& k) {
    GFXcanvas16 reference(240,320); reference.fillScreen(sentinel);
    reference.setTextColor(WHITE); reference.setTextSize(k.fontSize); reference.setTextWrap(false);
    auto cell = [&](int row,int col,int span,const std::string& text,bool active=false) {
        const int x=k.x+k.w*col/10, y=k.y+k.h*row/6;
        const int w=k.w*(col+span)/10-k.w*col/10;
        const int h=k.h*(row+1)/6-k.h*row/6;
        reference.fillRect(x,y,w,h,RED);
        reference.fillRect(x+2,y+2,w-4,h-4,active?k.highlightColor:BLACK);
        const int tw=text.size()*6*k.fontSize, th=8*k.fontSize;
        const int tx=x+(w-tw)/2, ty=y+(h-th)/2;
        if (tw && tw<=w-4 && th<=h-4 && tx>=0 && ty>=0 && tx+tw<=240 && ty+th<=320) {
            reference.setCursor(tx,ty); reference.print(text.c_str());
        }
    };
    const std::string text=k.getText();
    const size_t fit=std::min(text.size(),static_cast<size_t>((k.w*9/10-4)/(6*k.fontSize)));
    cell(0,0,9,text.substr(text.size()-fit)); cell(0,9,1,"<");
    const char* lower[]={"qwertyuiop","asdfghjkl`","zxcvbnm,./"," -_=~?!:; "};
    const char* symbols[]={"1234567890","!@#$%^&*()","-_=+[]{}\\|"," ;:'\",.<> "};
    for (int row=1; row<=4; ++row) for (int col=0; col<10; ++col) {
        if (row==4 && col==0) { cell(row,col,1,"^",k.shiftActive); continue; }
        if (row==4 && col==9) { cell(row,col,1,"#",k.specialActive); continue; }
        char ch=(k.specialActive?symbols:lower)[row-1][col];
        if (!k.specialActive && k.shiftActive && ch>='a' && ch<='z') ch+='A'-'a';
        cell(row,col,1,std::string(1,ch));
    }
    cell(5,0,6,"Space"); cell(5,6,2,"Cancel"); cell(5,8,2,"OK");
    return std::vector<uint16_t>(reference.getBuffer(),reference.getBuffer()+240*320);
}
void assertFrame(RasterCanvas& c, Keyboard& k, const char* context) {
    samePixels(c.tft->pixels,expectedFrame(k),context);
    require(c.tft->begins==c.tft->ends && !c.tft->selected && !c.tft->writesOutside,
            "unbalanced display transaction");
}
void hit(Keyboard& k,int row,int col) {
    Point p; p.x=k.x+k.w*col/10+k.w/20; p.y=k.y+k.h*row/6+k.h/12;
    host_millis+=1000;
    require(!k.checkTouch(&p),"key touch must be consumed");
}
void labels() {
    for (int width : {220,231,240}) {
        RasterCanvas c; Keyboard k; setup(c,k,(240-width)/2,90,width,197);
        require(k.setText("abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKL"),"seed text");
        const auto before=c.tft->pixels; k.show();
        outsideUnchanged(c,k,before,"normal labels"); assertFrame(c,k,"all labels and buffer tail");
        // Explicit non-vacuity: each command label has actual foreground pixels.
        for (const auto& range : std::vector<std::pair<int,int>>{{0,6},{6,8},{8,10}}) {
            unsigned ink=0;
            for (int y=k.y+k.h*5/6+2; y<k.y+k.h-2; ++y)
                for (int x=k.x+k.w*range.first/10+2; x<k.x+k.w*range.second/10-2; ++x)
                    ink+=c.tft->pixels[y*240+x]==WHITE;
            require(ink>0,"Space/Cancel/OK label must be visible");
        }
        k.setText("q"); k.update(); assertFrame(c,k,"shorter buffer removes old glyphs");
        k.clear(); k.update(); assertFrame(c,k,"empty buffer removes old glyphs");
        c.remove(&k);
    }
}
void restore() {
    for (bool screenOwned : {false,true}) {
        RasterCanvas c;
        Screen screen(&c,7,20,226,280); screen.bgColor=BLUE;
        Button canvasChild(60,25,GREEN,WHITE,RED), shown(70,30,YELLOW,WHITE,RED), hidden(50,25,RED,WHITE,RED);
        Keyboard k;
        canvasChild.fontSize=shown.fontSize=hidden.fontSize=1;
        canvasChild.setText("canvas"); shown.setText("visible"); hidden.setText("hidden");
        c.tft->fillScreen(c.bgColor); c.setScreen(&screen,false);
        screen.add(&shown,20,100); screen.add(&hidden,100,120);
        c.tft->fillRect(screen.x,screen.y,screen.w,screen.h,screen.bgColor);
        shown.visible=true; shown.show(); hidden.visible=false;
        c.add(&canvasChild,15,235,true);
        const auto baseline=c.tft->pixels;
        k.borderWidth=2; k.setColors(BLACK,WHITE,RED);
        if (screenOwned) screen.add(&k,-7,80); else c.add(&k,0,100,false);
        for (int repeat=0; repeat<3; ++repeat) {
            k.show(); require(c.tft->pixels!=baseline,"keyboard must paint over baseline");
            k.hide();
            require(!k.visible && !hidden.visible && shown.visible && canvasChild.visible,
                    "hide resurrected a hidden Screen child or changed visibility");
            samePixels(c.tft->pixels,baseline,"hide must restore complete background/visible widgets");
            const auto writes=c.tft->writes; k.update(); k.hide();
            require(c.tft->writes==writes,"hidden keyboard must be inert");
        }
        if (screenOwned) screen.remove(&k); else c.remove(&k);
        c.remove(&canvasChild); screen.remove(&shown); screen.remove(&hidden);
        screen.detach(); c.setScreen(nullptr,false);
    }
}
void edges() {
    for (bool wrap : {false,true}) for (const auto& pos : std::vector<std::pair<int,int>>{
            {0,128},{20,128},{23,150},{-15,140},{210,280},{-210,-160}}) {
        RasterCanvas c; Keyboard k; setup(c,k,pos.first,pos.second,231,192);
        c.tft->setTextWrap(wrap); k.setText("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
        const auto before=c.tft->pixels; k.show();
        outsideUnchanged(c,k,before,"edge draw/wrap"); assertFrame(c,k,"edge clipped labels");
        // Probe actual backend wrap behavior, not just Keyboard's own cursor.
        c.tft->setFont(nullptr); c.tft->setTextSize(1); c.tft->setCursor(239,10); c.tft->print("A");
        require(c.tft->getCursorY()==(wrap?18:10),"keyboard changed caller wrap setting");
        c.remove(&k);
    }
}
void modes() {
    RasterCanvas c; Keyboard k; setup(c,k); k.show();
    hit(k,1,0); hit(k,4,0); hit(k,1,0);
    require(std::strcmp(k.getText(),"qQ")==0,"normal/shift literal typing");
    assertFrame(c,k,"shift raster and typed text");
    hit(k,4,9); hit(k,2,2); hit(k,2,5); hit(k,4,7); hit(k,3,8);
    require(std::strcmp(k.getText(),"qQ#^<\\")==0,"symbols must append literal # ^ < backslash");
    assertFrame(c,k,"special raster and literal text");
    hit(k,0,9); hit(k,5,0);
    require(std::strcmp(k.getText(),"qQ#^< ")==0,"dedicated backspace and Space actions");
    assertFrame(c,k,"backspace/Space buffer raster"); c.remove(&k);
}
void customfont() {
    RasterCanvas c; Keyboard k; setup(c,k,10,70,220,192);
    c.tft->setFont(&FreeSans18pt7b); c.tft->setTextWrap(true);
    k.setText("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
    const auto before=c.tft->pixels; k.show();
    outsideUnchanged(c,k,before,"FreeSans18pt7b inherited by keyboard");
    require(std::count(c.tft->pixels.begin(),c.tft->pixels.end(),WHITE)>0,
            "inherited custom font must not make the keyboard blank/inert");
    for (const auto& range : std::vector<std::pair<int,int>>{{0,6},{6,8},{8,10}}) {
        unsigned ink=0;
        for (int y=k.y+k.h*5/6+2; y<k.y+k.h-2; ++y)
            for (int x=k.x+k.w*range.first/10+2; x<k.x+k.w*range.second/10-2; ++x)
                ink+=c.tft->pixels[y*240+x]==WHITE;
        require(ink>0,"custom-font Space/Cancel/OK label must stay visible");
    }
    // Regardless of font policy, text ink may not overwrite any cell border.
    for (int row=1; row<=4; ++row) for (int col=0; col<10; ++col) {
        const int x=k.x+k.w*col/10, y=k.y+k.h*row/6;
        const int right=k.x+k.w*(col+1)/10, bottom=k.y+k.h*(row+1)/6;
        for (int py=y; py<bottom; ++py) for (int px=x; px<right; ++px)
            if (px<x+2 || px>=right-2 || py<y+2 || py>=bottom-2)
                require(c.tft->pixels[py*240+px]==RED,"custom-font glyph escaped its key interior");
    }
    k.hide(); require(c.tft->pixels==before,"custom-font hide left glyph pixels");
    c.remove(&k);
}
void font0() {
    RasterCanvas c; Keyboard k; setup(c,k); k.fontSize=0;
    const auto before=c.tft->pixels; const auto writes=c.tft->writes;
    k.show(); k.update(); k.hide();
    samePixels(c.tft->pixels,before,"documented fontSize=0 inert draw");
    require(c.tft->writes==writes,"fontSize=0 must not paint"); c.remove(&k);
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2,"select one test case");
        const std::string test=argv[1];
        if (test=="labels") labels(); else if (test=="restore") restore();
        else if (test=="edges") edges(); else if (test=="modes") modes();
        else if (test=="customfont") customfont(); else if (test=="font0") font0();
        else require(false,"unknown case");
        std::printf("keyboard real-GFX %s: PASS\n",argv[1]); return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr,"FAIL: %s\n",error.what()); return 1;
    }
}
