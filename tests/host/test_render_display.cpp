#include "Display.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

// Layout-only baseline for exact host ABI overhead; never instantiated.
struct OriginalDisplayLayout : Widget { Text* contents; char charPos; };
class RenderCanvas : public Canvas {
public:
    RenderCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void setup(Display& d, RenderCanvas& c) {
    d.setCanvas(&c); d.x=20; d.y=30; d.setSize(120,40);
    d.borderWidth=0; d.setColors(BLACK,WHITE,WHITE);
}
static void workload(bool changed) {
    RenderCanvas c; Display d(8); setup(d,c); d.setText((char*)"1234",true);
    c.tft->ops.clear();
    for(int i=0;i<100;++i) d.setText((char*)(changed && i%2==0 ? "5678":"1234"),true);
    std::printf("display workload %s: %zu operations\n",changed?"changed":"same",c.tft->ops.size());
}
int main(int argc,char**) {
    workload(false); workload(true); std::fflush(stdout);
    if(argc>1) return 0;
    RenderCanvas c; Display d(4); setup(d,c);
    char value[]="1234tail"; d.setText(value,true);
    assert(std::strcmp(d.contents->text,"1234")==0);
    c.tft->ops.clear(); d.setText((char*)"1234different",true);
    assert(c.tft->ops.empty());
    // Public style changes must not be hidden by incoming-text equality.
    auto changed = [&]() { c.tft->ops.clear(); d.setText(d.contents->text,true); assert(!c.tft->ops.empty()); };
    d.fgColor=RED; changed(); assert(c.tft->ops.back().color==RED);
    d.fontSize=1; changed(); assert(c.tft->ops.back().h==8);
    d.bgColor=BLUE; changed(); assert(c.tft->ops.front().color==BLUE);
    d.x+=10; changed(); d.y+=10; changed(); d.w+=10; changed(); d.h+=10; changed();
    d.borderWidth=1; changed();
    d.borderColor=GREEN; changed();
    bool greenBorder=false;
    for(const auto& op:c.tft->ops) if(op.name=="rect" && op.color==GREEN) greenBorder=true;
    assert(greenBorder);
    // Deferred updates don't replace the last-rendered snapshot.
    c.tft->ops.clear(); d.setText((char*)"X",false); d.setText((char*)"Y",false);
    assert(c.tft->ops.empty()); d.update(); assert(!c.tft->ops.empty());
    c.tft->ops.clear(); d.setText((char*)"Y",true); assert(c.tft->ops.empty());
    // A shorter centered value must erase the old, differently centered footprint.
    d.borderWidth=0; d.fontSize=2; d.setSize(120,40); d.setText((char*)"1234",true);
    int oldX=d.getCenterTextX(d.x,d.w,4), oldY=d.getCenterTextY(d.y,d.h);
    c.tft->drawPixel(oldX,oldY,RED); c.tft->ops.clear(); d.setText((char*)"7",true);
    assert(c.tft->pixels[oldY*c.tft->width()+oldX]==BLUE);
    assert(c.tft->ops.back().name=="text:7");
    assert(c.tft->ops.back().x==d.getCenterTextX(d.x,d.w,1));
    c.tft->ops.clear(); d.setText(nullptr,true);
    assert(!c.tft->ops.empty()); for(const auto& op:c.tft->ops) assert(op.name.find("text:")!=0);
    // Moving/shrinking cleans the old widget footprint, not the new coordinates.
    d.setText((char*)"1234",true); int priorX=d.x, priorY=d.y;
    c.tft->drawPixel(priorX,priorY,RED); d.x+=30; d.setSize(40,20); changed();
    assert(c.tft->pixels[priorY*c.tft->width()+priorX]==c.bgColor);
    // Old text may extend beyond the widget; shrinking font must erase that too.
    d.x=100; d.y=100; d.setSize(30,20); d.fontSize=2; d.setText((char*)"1234",true);
    oldX=d.getCenterTextX(d.x,d.w,4); oldY=d.getCenterTextY(d.y,d.h);
    c.tft->drawPixel(oldX,oldY,RED); d.fontSize=1; d.setText((char*)"7",true);
    assert(c.tft->pixels[oldY*c.tft->width()+oldX]==BLUE);
    // Overflow cleanup must restore any border it crosses.
    d.setSize(30,20); d.fontSize=2; d.borderWidth=1; d.borderColor=GREEN;
    d.setText((char*)"1234",true); c.tft->ops.clear(); d.setText((char*)"7",true);
    greenBorder=false;
    for(const auto& op:c.tft->ops) if(op.name=="rect" && op.color==GREEN) greenBorder=true;
    assert(greenBorder);
    // Explicit repaint repairs external damage even with an identical signature.
    d.setSize(100,40); d.borderWidth=1; d.show();
    c.tft->drawPixel(d.x+2,d.y+2,RED); c.tft->drawPixel(d.x,d.y,RED);
    c.tft->ops.clear(); d.forceUpdate(); assert(!c.tft->ops.empty());
    assert(c.tft->pixels[(d.y+2)*c.tft->width()+d.x+2]==BLUE);
    assert(c.tft->pixels[d.y*c.tft->width()+d.x]==GREEN);
    c.tft->ops.clear(); d.update(); assert(!c.tft->ops.empty());
    d.hide(); c.tft->ops.clear(); d.setText((char*)"hide",true); assert(c.tft->ops.empty());
    d.show(); assert(!c.tft->ops.empty());
    c.tft->ops.clear(); d.show(); assert(!c.tft->ops.empty());
    // Direct content mutation and aliases are compared to the last render.
    d.contents->text[0]='Z'; c.tft->ops.clear(); d.setText(d.contents->text,true);
    assert(!c.tft->ops.empty()); d.setText(d.contents->text+1,true);
    assert(std::strcmp(d.contents->text,"ide")==0);
    RenderCanvas other; d.setCanvas(&other); d.setText(d.contents->text,true);
    assert(!other.tft->ops.empty());
    { Display unattached(4); unattached.setText((char*)"1234",true);
      unattached.setCanvas(&c); unattached.setSize(100,40); c.tft->ops.clear();
      unattached.setText((char*)"1234",true); assert(!c.tft->ops.empty()); }
    // Text OOM and snapshot OOM retain ownership and keep repainting safely.
    { host_allocation_fail_after=0; Display empty(8); setup(empty,c);
      empty.setText((char*)"safe",true); empty.update(); assert(!empty.contents->text); }
    { host_allocation_fail_after=1; Display fallback(8); setup(fallback,c);
      assert(fallback.contents->text); fallback.setText((char*)"safe",true);
      c.tft->ops.clear(); fallback.setText((char*)"safe",true);
      assert(!c.tft->ops.empty()); assert(std::strcmp(fallback.contents->text,"safe")==0); }
    { Display zero(0); setup(zero,c); zero.setText((char*)"ignored",true);
      c.tft->ops.clear(); zero.setText(nullptr,true); assert(c.tft->ops.empty()); }
    { Display maximum(255); setup(maximum,c); maximum.setSize(220,40); maximum.fontSize=1;
      std::string longText(255,'x'); maximum.setText(&longText[0],true);
      c.tft->ops.clear(); maximum.setText(&longText[0],true); assert(c.tft->ops.empty()); }
    std::printf("sizeof(Display)=%zu baseline=%zu added=%zu bytes + capacity+1 heap bytes\n",
                sizeof(Display),sizeof(OriginalDisplayLayout),sizeof(Display)-sizeof(OriginalDisplayLayout));
    std::puts("render_display: PASS");
}
