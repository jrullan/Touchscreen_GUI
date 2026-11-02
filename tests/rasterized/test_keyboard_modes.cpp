#include "Keyboard.h"
#include <Fonts/FreeSans18pt7b.h>
#include <cassert>
#include <cstdio>
#include <cstring>
class ModeCanvas : public Canvas {
public:
    ModeCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void hit(Keyboard& k,int col) {
    Point p; p.x=k.x+k.w*col/10+k.w/20; p.y=k.y+k.h*4/6+k.h/12;
    host_millis+=1000; assert(!k.checkTouch(&p));
}
static void matrix() {
    unsigned transitions=0;
    for(int rotation=0;rotation<4;++rotation)
    for(bool custom : {false,true})
    for(int size : {1,2})
    for(int border : {0,2})
    for(int placement=0;placement<4;++placement) {
        ModeCanvas c, reference; c.tft->setRotation(rotation); reference.tft->setRotation(rotation);
        Keyboard k(Button::TextStorage::None), oracle(Button::TextStorage::None);
        const int w=size==1?231:440, h=size==1?197:205;
        const int x=placement==0?0:placement==1?c.tft->width()-w:placement==2?-198:15;
        const int y=placement==3?c.tft->height()-h+15:0;
        c.add(&k,x,y,false); reference.add(&oracle,x,y,false);
        for(Keyboard* keyboard : {&k,&oracle}) {
            keyboard->setSize(w,h); keyboard->fontSize=size; keyboard->borderWidth=border;
            keyboard->setColors(BLACK,WHITE,RED); keyboard->setText("unchanged buffer tail ABC 123");
            keyboard->visible=true;
        }
        for(auto* tft : {c.tft,reference.tft}) {
            tft->setFont(custom?&FreeSans18pt7b:nullptr); tft->setTextWrap(true);
            tft->setTextSize(2,3); tft->setTextColor(GREEN,BLUE); tft->setCursor(5,25);
        }
        for(int oldMode=0;oldMode<4;++oldMode) for(int col : {0,9}) {
            const int px=x+w*col/10+w/20, py=y+h*4/6+h/12;
            if(px<0 || px>=c.tft->width() || py<0 || py>=c.tft->height()) continue;
            k.shiftActive=oldMode&1; k.specialActive=oldMode&2;
            c.tft->fillScreen(0x1234); k.drawFrame();
            const auto before=c.tft->pixels;
            oracle.shiftActive=k.shiftActive^(col==0); oracle.specialActive=k.specialActive^(col==9);
            reference.tft->fillScreen(0x1234); oracle.drawFrame();
            std::fill(c.tft->pixelWrites.begin(),c.tft->pixelWrites.end(),0);
            hit(k,col);
            assert(c.tft->pixels==reference.tft->pixels && "incremental mode pixels differ from full redraw");
            assert(std::strcmp(k.getText(),oracle.getText())==0);
            // Derive changed cells from actual old/new raster pixels, independently
            // of production's mode tables and invalidation predicates.
            std::vector<bool> allowed(c.tft->pixels.size(),false);
            for(int row=1;row<=4;++row) for(int column=0;column<10;++column) {
                const int left=std::max(0,x+w*column/10+border);
                const int right=std::min(int(c.tft->width()),x+w*(column+1)/10-border);
                const int top=std::max(0,y+h*row/6+border);
                const int bottom=std::min(int(c.tft->height()),y+h*(row+1)/6-border);
                bool changed=false;
                for(int yy=top;yy<bottom;++yy) for(int xx=left;xx<right;++xx) {
                    const auto i=yy*c.tft->width()+xx;
                    changed |= before[i]!=reference.tft->pixels[i];
                }
                if(changed) for(int yy=top;yy<bottom;++yy) for(int xx=left;xx<right;++xx)
                    allowed[yy*c.tft->width()+xx]=true;
            }
            for(size_t i=0;i<allowed.size();++i)
                assert((!c.tft->pixelWrites[i] || allowed[i]) && "write outside changed cell interiors");
            assert(c.tft->begins==c.tft->ends && !c.tft->selected && !c.tft->writesOutside);
            assert(c.tft->getCursorX()==5 && c.tft->getCursorY()==25);
            ++transitions;
        }
        // Real subsequent rendering proves the inherited custom/classic font,
        // anisotropic size, color, wrap and cursor survived keyboard rendering.
        GFXcanvas16 untouched(c.tft->width(),c.tft->height());
        untouched.setFont(custom?&FreeSans18pt7b:nullptr); untouched.setTextWrap(true);
        untouched.setTextSize(2,3); untouched.setTextColor(GREEN,BLUE); untouched.setCursor(5,25);
        c.tft->fillScreen(0x1234); untouched.fillScreen(0x1234);
        const char* probe="WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";
        c.tft->print(probe); untouched.print(probe);
        const std::vector<uint16_t> expectedState(untouched.getBuffer(),untouched.getBuffer()+c.tft->pixels.size());
        assert(c.tft->pixels==expectedState);
        assert(c.tft->getCursorX()==untouched.getCursorX());
        assert(c.tft->getCursorY()==untouched.getCursorY());
        c.remove(&k); reference.remove(&oracle);
    }
    std::printf("mode raster matrix: %u transitions, four rotations, classic/custom, size 1/2, border 0/2, clipped edges PASS\n",transitions);
}
int main() {
    matrix();
    ModeCanvas c; Keyboard k(Button::TextStorage::None); c.add(&k,0,100,false);
    k.setColors(BLACK,WHITE,RED); k.setText("preserved"); k.show();
    const auto writes=c.tft->writes;
    hit(k,0);
    const auto incremental=c.tft->pixels;
    const auto delta=c.tft->writes-writes;
    c.tft->fillScreen(BLACK); const auto fullStart=c.tft->writes; k.drawFrame();
    assert(incremental==c.tft->pixels && "selective Shift equals clean full redraw");
    const auto full=c.tft->writes-fullStart;
    std::printf("Shift real-GFX writes selective=%u full=%u\n",delta,full);
    assert(delta<full/2 && "Shift must not repaint unchanged chrome/text/keys");
    hit(k,9); k.highlightColor=k.bgColor+65536; k.update();
    const auto unchanged=c.tft->pixels; const auto noOpStart=c.tft->writes;
    hit(k,0);
    assert(c.tft->pixels==unchanged);
    assert(c.tft->writes==noOpStart && "effective highlight equality must write nothing");
    k.fgColor=k.bgColor; k.highlightColor=YELLOW; k.update();
    const auto invisibleStart=c.tft->writes; hit(k,9);
    assert(c.tft->writes-invisibleStart<1000 && "invisible glyph changes must not repaint letters");
    k.fgColor=WHITE;
    for(int mode=0;mode<4;++mode) for(int col : {0,9}) {
        k.shiftActive=mode&1; k.specialActive=mode&2; k.update();
        const auto start=c.tft->writes; hit(k,col);
        const auto selective=c.tft->writes-start;
        const auto expected=c.tft->pixels;
        c.tft->fillScreen(BLACK); const auto fullStart=c.tft->writes; k.drawFrame();
        assert(c.tft->pixels==expected);
        std::printf("mode=%d toggle=%s real-GFX writes=%u full=%u\n",mode,col?"Symbol":"Shift",selective,c.tft->writes-fullStart);
    }
    c.remove(&k);
}
