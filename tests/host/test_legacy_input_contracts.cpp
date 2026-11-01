#include "Numkey.h"
#include "Dial.h"
#include <cassert>
#include <cstdio>
#include <cstring>

class LegacyCanvas : public Canvas {
public:
    LegacyCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static int targetCalls, numkeyCalls;
static Button* expectedTarget;
static void targetCallback(Button* button) { assert(button==expectedTarget); ++targetCalls; }
static void entered(Numkey* key) {
    ++numkeyCalls;
    assert(key->entry==42 && key->getNum()==42);
}
static void explicitlyBridge(Numkey* key) {
    entered(key);
    key->targetEventHandler();
}
static void equalsTouch(Numkey& key, bool advance=true) {
    Point p; p.x=key.x+5*(key.w/3)/2; p.y=key.y+9*(key.h/5)/2;
    if (advance) host_millis+=1000;
    assert(!key.checkTouch(&p));
}
static void numkeyTargetIsNotAnAcceptSession() {
    LegacyCanvas canvas; Button target; Numkey key;
    expectedTarget=&target; targetCalls=numkeyCalls=0;
    target.setText("17"); target.setEventHandler(targetCallback);
    canvas.add(&key,5,10,false); key.autoremove=false;
    key.setNum(42); key.setEventHandler(entered);
    key.show(&target);
    assert(key.targetButton==&target && key.visible);
    assert(std::strcmp(key.getText(),"42")==0); // No import from target.
    assert(std::strcmp(target.getText(),"17")==0);
    assert(numkeyCalls==0 && targetCalls==0);
    equalsTouch(key);
    assert(numkeyCalls==1 && targetCalls==0 && key.entry==42);
    assert(key.visible && key.targetButton==&target);
    assert(std::strcmp(target.getText(),"17")==0); // No export on '='.
    equalsTouch(key,false); assert(numkeyCalls==1 && targetCalls==0); // Debounced.
    key.targetEventHandler();
    assert(targetCalls==1 && numkeyCalls==1);
    assert(std::strcmp(target.getText(),"17")==0);
    key.setEventHandler(explicitlyBridge); equalsTouch(key);
    assert(numkeyCalls==2 && targetCalls==2);
    assert(std::strcmp(target.getText(),"17")==0);
    key.reset(); assert(key.entry==0 && key.targetButton==&target);
    assert(std::strcmp(key.getText(),"42")==0);
    key.targetButton=nullptr; key.targetEventHandler(); assert(targetCalls==2);
    target.setEventHandler(nullptr); key.targetButton=&target;
    key.targetEventHandler(); assert(targetCalls==2);
    canvas.remove(&key);
}
static void dialColorRoles() {
    LegacyCanvas canvas; Dial dial;
    canvas.add(&dial,100,100,false); dial.setSize(40); dial.borderWidth=3;
    dial.showVal=false; dial.setpoint=0;
    dial.setCV(50,false); // Avoid alarm colors/markers: isolate ring and tick roles.
    dial.setColors(BLACK,GREEN,RED);
    const auto verify = [&](int ringColor, int tickColor) {
        canvas.tft->ops.clear(); dial.show();
        const auto& ops=canvas.tft->ops;
        assert(ops.size()>2);
        assert(ops[0].name=="fillCircle" && ops[0].x==dial.x && ops[0].y==dial.y);
        assert(ops[0].w==dial.radius && ops[0].color==ringColor);
        assert(ops[1].name=="fillCircle" && ops[1].w==dial.radius-dial.borderWidth);
        assert(ops[1].color==BLACK);
        unsigned runs=0, labels=0;
        for (const auto& op : ops) {
            if (op.name=="hline" || op.name=="vline") { ++runs; assert(op.color==tickColor); }
            if (op.name.rfind("text:",0)==0) { ++labels; assert(op.color==tickColor); }
        }
        assert(runs>0 && labels==2);
        // Pixel witnesses complement operation colors; glyphs are not rasterized by this seam.
        const int stride=canvas.tft->width();
        assert(canvas.tft->pixels[(dial.y+int(dial.radius))*stride+dial.x]==ringColor);
        assert(canvas.tft->pixels[dial.y*stride+dial.x+int(dial.radius)-dial.borderWidth]==tickColor);
    };
    verify(GREEN,RED);
    dial.setColors(BLACK,RED,GREEN); verify(RED,GREEN); // Contrasting swapped palette.
    canvas.remove(&dial);
}
int main() {
    numkeyTargetIsNotAnAcceptSession();
    dialColorRoles();
    std::puts("legacy input contracts: PASS");
}
