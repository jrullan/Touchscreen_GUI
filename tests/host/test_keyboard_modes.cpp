#include "Keyboard.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
class ModeCanvas : public Canvas {
public:
    ModeCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void hit(Keyboard& k, int col) {
    Point p; p.x=k.x+k.w*col/10+k.w/20; p.y=k.y+k.h*4/6+k.h/12;
    host_millis+=1000; assert(!k.checkTouch(&p));
}
static unsigned callbacks=0;
static void finished(Keyboard*) { ++callbacks; }
int main() {
    {
        ModeCanvas c; Keyboard k(Button::TextStorage::None); c.add(&k,0,0,false);
        k.setText("keep"); k.setEventHandler(finished);
        for(int mode=0;mode<4;++mode) for(int col : {0,9}) {
            k.shiftActive=mode&1; k.specialActive=mode&2; k.show();
            unsigned fullFills=0;
            for(const auto& op:c.tft->ops) if(op.name=="fillRect") ++fullFills;
            c.tft->ops.clear(); const auto allocations=host_allocation_calls; hit(k,col);
            unsigned fills=0;
            for(const auto& op:c.tft->ops) if(op.name=="fillRect") ++fills;
            const unsigned expected=col==9?39:(mode&2)?1:27;
            assert(fills==expected && host_allocation_calls==allocations);
            assert(callbacks==0 && k.result()==Keyboard::Editing && std::strcmp(k.getText(),"keep")==0);
            std::printf("mode=%d toggle=%s fills=%u full=%u\n",mode,col?"Symbol":"Shift",fills,fullFills);
            c.tft->ops.clear();
        }
        c.remove(&k);
    }
    {
        ModeCanvas c; Keyboard k; k.setSize(231,197); c.add(&k,-198,0,false); k.show();
        c.tft->ops.clear(); hit(k,9);
        unsigned fills=0;
        for(const auto& op:c.tft->ops) if(op.name=="fillRect") ++fills;
        assert(fills==4 && "clipped glyph changes must not clear unchanged background fragments");
        c.remove(&k);
    }
    for (auto storage : {Button::TextStorage::Default, Button::TextStorage::None}) {
        ModeCanvas c; Keyboard k(storage); c.add(&k,3,7,false);
        k.setSize(231,197); k.setText("preserved"); k.show();
        c.tft->ops.clear(); const auto allocations=host_allocation_calls;
        hit(k,0);
        unsigned fills=0;
        for (const auto& op : c.tft->ops) {
            if (op.name!="fillRect") continue;
            ++fills;
            bool matched=false;
            for(int row=1;row<=4;++row) for(int col=0;col<10;++col) {
                if (!(row==1 || (row==2 && col<9) || (row==3 && col<7) || (row==4 && col==0))) continue;
                const int left=k.x+k.w*col/10+k.borderWidth;
                const int top=k.y+k.h*row/6+k.borderWidth;
                const int right=k.x+k.w*(col+1)/10-k.borderWidth;
                const int bottom=k.y+k.h*(row+1)/6-k.borderWidth;
                matched |= op.x==left && op.y==top && op.w==right-left && op.h==bottom-top;
            }
            assert(matched && "mode repaint must touch changed cell interiors only");
        }
        assert(fills==27 && "Shift changes 26 letters and its active background");
        assert(allocations==host_allocation_calls);
        std::printf("Shift: fills=%u sizeof(Keyboard)=%zu sizeof(Button)=%zu\n",fills,sizeof(Keyboard),sizeof(Button));
        // In symbols mode Shift changes only its highlight. RGB565 aliases
        // must compare as rendered colors, not public int identity.
        hit(k,9); k.highlightColor=k.bgColor+65536; k.update();
        c.tft->ops.clear(); hit(k,0);
        assert(c.tft->ops.empty() && "identical effective highlight is a no-op in symbols");
        k.fgColor=k.bgColor; k.highlightColor=YELLOW; k.update();
        c.tft->ops.clear(); hit(k,9);
        unsigned invisibleFills=0;
        for(const auto& op:c.tft->ops) if(op.name=="fillRect") ++invisibleFills;
        assert(invisibleFills==1 && "invisible foreground needs only mode-key background repaint");
        c.remove(&k);
    }
}
