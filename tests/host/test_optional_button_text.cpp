#include "IconButton.h"
#include "Keyboard.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstring>
static void nullSafe(Button& b) {
    assert(b.getText()==nullptr && b.getNum()==0);
    const int w=b.w,h=b.h;
    b.setText("ignored"); char text[]="ignored"; b.setText(text);
    b.fitToText(); b.drawText(); b.show(); b.update();
    assert(b.getText()==nullptr && b.w==w && b.h==h);
}
static void defaultsAndOOM() {
    Button plain; assert(plain.contents && plain.contents->_textSize==8);
    Keyboard ordinary, styled(231,197,BLUE,GREEN,RED);
    for (Button* b : {static_cast<Button*>(&ordinary),static_cast<Button*>(&styled)}) {
        assert(b->contents && b->contents->_textSize==8 && std::strcmp(b->getText(),"Button")==0);
        b->setText("123"); assert(b->getNum()==123);
    }
    Button sized(20,30,BLACK,WHITE,RED), round(8,BLACK,WHITE,RED);
    char initial[]="hello"; Button named(initial);
    assert(sized.contents->_textSize==8 && round.contents->_textSize==8);
    assert(std::strcmp(sized.getText(),"Button")==0 && std::strcmp(round.getText(),"Button")==0);
    assert(std::strcmp(named.getText(),"hello")==0);
    for(int mode=0;mode<2;++mode) {
        // Default constructors remain safe if Text new or its calloc fails.
        if(mode==0) host_nothrow_new_fail_after=0; else host_allocation_fail_after=0;
        { IconButton oom(1,1,nullptr,nullptr,nullptr); nullSafe(oom); }
        if(mode==0) host_nothrow_new_fail_after=0; else host_allocation_fail_after=0;
        { Keyboard oom; nullSafe(oom); assert(oom.setText("still edits")); assert(std::strcmp(oom.getText(),"still edits")==0); }
        host_nothrow_new_fail_after=host_allocation_fail_after=-1;
    }
    // Opt-out must not even attempt new/calloc, including init's setText("Button").
    host_nothrow_new_fail_after=host_allocation_fail_after=0;
    {
        IconButton lean(1,1,nullptr,nullptr,nullptr,Button::TextStorage::None);
        Keyboard keyboard(Button::TextStorage::None);
        nullSafe(lean); nullSafe(keyboard); lean.init(); keyboard.Button::init();
        assert(!lean.contents && !keyboard.contents);
        assert(host_nothrow_new_fail_after==0 && host_allocation_fail_after==0);
    }
    host_nothrow_new_fail_after=host_allocation_fail_after=-1;
    // Labels remain separately owned, with their established OOM semantics.
    host_allocation_fail_after=0;
    { IconButton lean(1,1,nullptr,nullptr,(char*)"Lamp",Button::TextStorage::None); assert(!lean.label); }
    Button* dynamic=new Keyboard(Button::TextStorage::None); nullSafe(*dynamic); delete dynamic;
    dynamic=new IconButton(1,1,nullptr,nullptr,(char*)"Lamp",Button::TextStorage::None); nullSafe(*dynamic); delete dynamic;
}
class OptionalCanvas : public Canvas {
public:
    OptionalCanvas() : Canvas(TFT_PORTRAIT,BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static void touchSession() {
    OptionalCanvas canvas; Keyboard k(Button::TextStorage::None); Button target;
    target.setText("old"); canvas.add(&k,0,0,false); k.show(&target);
    auto hit=[&](int row,int col) {
        Point p; p.x=k.x+col*k.w/10+k.w/20; p.y=k.y+row*k.h/6+k.h/12;
        host_millis+=1000; assert(!k.checkTouch(&p));
    };
    hit(0,9); hit(0,9); hit(0,9); // Backspace existing target.
    hit(1,0); hit(4,0); hit(1,0); hit(4,9); hit(2,2); hit(5,0);
    assert(std::strcmp(k.getText(),"qQ# ")==0);
    hit(5,8); assert(k.result()==Keyboard::Accepted && !k.visible && !k.targetButton);
    assert(std::strcmp(target.getText(),"qQ# ")==0);
    k.show(&target); hit(1,1); hit(5,6);
    assert(k.result()==Keyboard::Cancelled && !k.visible && !k.targetButton);
    assert(std::strcmp(target.getText(),"qQ# ")==0);
    IconButton noTextTarget(1,1,nullptr,nullptr,nullptr,Button::TextStorage::None);
    k.show(&noTextTarget); assert(!k.visible && !k.targetButton && !k.accept());
    canvas.remove(&k);
}
int main() {
    defaultsAndOOM(); touchSession();
    IconButton legacy(1,1,nullptr);
    assert(legacy.contents && legacy.contents->_textSize==8);
    assert(std::strcmp(legacy.getText(),"Button")==0);
    IconButton lean(1,1,nullptr,nullptr,nullptr,Button::TextStorage::None);
    assert(lean.contents==nullptr);
    Button* base=&lean;
    assert(base->getText()==nullptr && base->getNum()==0);
    const int w=lean.w, h=lean.h;
    base->setText("ignored"); char mutableText[]="ignored"; base->setText(mutableText);
    base->fitToText(); base->drawText(); base->init();
    assert(lean.contents==nullptr && lean.w==100 && lean.h==40);
    (void)w; (void)h;
    Keyboard keyboard(Button::TextStorage::None);
    assert(keyboard.contents==nullptr);
    assert(keyboard.setText("edit")); keyboard.append('!'); keyboard.deleteChar();
    assert(std::strcmp(keyboard.getText(),"edit")==0);
    Button* inherited=&keyboard;
    assert(inherited->getText()==nullptr && inherited->getNum()==0);
    inherited->setText("ignored"); inherited->fitToText(); inherited->drawText();
    assert(keyboard.contents==nullptr);
    Button target; target.setText("old");
    keyboard.show(&target); keyboard.setText("new"); assert(keyboard.accept());
    assert(std::strcmp(target.getText(),"new")==0);
    keyboard.show(&target); keyboard.setText("cancel"); keyboard.cancel();
    assert(std::strcmp(target.getText(),"new")==0);
    Keyboard styled(231,197,BLUE,GREEN,RED,Button::TextStorage::None);
    assert(styled.contents==nullptr && styled.w==231 && styled.h==197);
    assert(styled.bgColor==BLUE && styled.fgColor==GREEN && styled.borderColor==RED);
}
