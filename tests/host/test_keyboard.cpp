#include "Keyboard.h"
#include "Screen.h"
#include "allocation_hooks.h"
#include <cassert>
#include <cstring>
#include <string>
#include <cstdio>
#include <climits>
#include <type_traits>
static_assert(std::is_same<decltype(std::declval<const Keyboard&>().getText()), const char*>::value,"read-only view");
class KeyboardCanvas : public Canvas {
public:
    KeyboardCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static bool hit(Keyboard& k, int row, int col) {
    Point p; p.x=k.x+(col*k.w)/10+(k.w/10)/2;
    p.y=k.y+(row*k.h)/6+(k.h/6)/2;
    host_millis+=1000; return k.checkTouch(&p);
}
static void layoutTests() {
    KeyboardCanvas c; Keyboard k;
    Point p; p.x=12; p.y=48;
    assert(k.checkTouch(&p));
    c.add(&k,0,0,false); k.show();
    assert(!c.tft->ops.empty());
    assert(!hit(k,1,0)); assert(std::strcmp(k.getText(),"q")==0);
    hit(k,4,0); assert(k.shiftActive); hit(k,1,0);
    assert(std::strcmp(k.getText(),"qQ")==0);
    hit(k,4,9); assert(k.specialActive);
    hit(k,2,2); hit(k,2,5); hit(k,4,7); hit(k,3,8);
    assert(std::strcmp(k.getText(),"qQ#^<\\")==0);
    hit(k,0,9); assert(std::strcmp(k.getText(),"qQ#^<")==0);
    hit(k,5,0); assert(std::strcmp(k.getText(),"qQ#^< ")==0);
    bool reachable[127] = {}; reachable[' '] = true;
    for(int mode=0;mode<3;++mode) {
        k.specialActive=(mode==2); k.shiftActive=(mode==1);
        for(int row=1;row<=4;++row) for(int col=0;col<10;++col) {
            if(row==4 && (col==0 || col==9)) continue;
            k.clear(); hit(k,row,col);
            assert(k.getTextSize()==1); reachable[static_cast<unsigned char>(k.getText()[0])]=true;
        }
    }
    for(int ch=32;ch<=126;++ch) assert(reachable[ch]);
    k.hide(); c.tft->ops.clear(); assert(k.checkTouch(&p)); k.update();
    assert(c.tft->ops.empty());
    k.show(); k.setSize(19,6); c.tft->ops.clear(); k.drawFrame();
    assert(k.checkTouch(&p) && c.tft->ops.empty());
    k.setSize(240,192); k.fontSize=0; k.update(); assert(c.tft->ops.empty());
    k.fontSize=1; k.borderWidth=-1; k.update(); assert(c.tft->ops.empty());
    k.borderWidth=INT_MAX; k.update(); assert(c.tft->ops.empty());
    k.borderWidth=2; k.x=INT_MAX; k.y=INT_MAX; k.update(); assert(c.tft->ops.empty());
    c.remove(&k);
}
// The host records fills; real glyph/pixel equivalence is checked by the GFX runner.
static void textTouchRepaintTests() {
    KeyboardCanvas c; Keyboard k; c.add(&k,3,7,false);
    k.setSize(231,197); k.show();
    auto assertInteriorOnly = [&]() {
        assert(!c.tft->ops.empty());
        const int left=k.x+k.borderWidth, top=k.y+k.borderWidth;
        const int right=k.x+k.w*9/10-k.borderWidth;
        const int bottom=k.y+k.h/6-k.borderWidth;
        for (const auto& op : c.tft->ops) {
            assert(op.x>=left && op.y>=top);
            assert(op.x+op.w<=right && op.y+op.h<=bottom);
        }
        // Erase the entire interior, including old centered/shortened suffix glyphs.
        const auto& erase=c.tft->ops.front();
        assert(erase.x==left && erase.y==top && erase.w==right-left && erase.h==bottom-top);
        assert(erase.color==k.bgColor);
    };
    for (const auto& action : {std::pair<int,int>{1,0}, {5,0}, {0,9}}) {
        c.tft->ops.clear(); const auto before=host_allocation_calls;
        assert(!hit(k,action.first,action.second));
        assert(host_allocation_calls==before);
        assertInteriorOnly();
    }
    assert(std::strcmp(k.getText(),"q")==0);
    c.remove(&k);
}
static void noOpTouchPaintTests() {
    KeyboardCanvas c; Keyboard k; c.add(&k,0,0,false); k.show();
    auto assertNoPaint = [&](int row, int col) {
        const std::string before=k.getText();
        c.tft->ops.clear();
        assert(!hit(k,row,col));
        assert(before==k.getText() && c.tft->ops.empty());
    };
    assertNoPaint(0,9); // Empty backspace.
    assertNoPaint(0,0); // Text display, not an action.
    const std::string full(KEYBOARD_BUF_SIZE,'x');
    assert(k.setText(full.c_str())); k.update();
    assertNoPaint(1,0); // Capacity-full ordinary character.
    assertNoPaint(5,0); // Capacity-full Space.
    // Explicit repaint remains available for external damage/overlay restoration.
    c.tft->ops.clear(); k.update(); assert(!c.tft->ops.empty());
    c.remove(&k);
}
static int callbacks=0;
static Keyboard::Result observed;
static void finished(Keyboard* k) { ++callbacks; observed=k->result(); assert(k->targetButton==nullptr); }
static void deleting(Keyboard* k) { ++callbacks; delete k; }
static void sessionTests() {
    KeyboardCanvas c; Keyboard k; Button target(50,20,BLACK,WHITE,WHITE,4), unrelated;
    target.setText("old"); c.add(&k,0,0,false); c.add(&unrelated,0,200,false);
    k.setEventHandler(finished); k.show(&target);
    hit(k,1,0); assert(callbacks==0); k.setText("old");
    assert(k.result()==Keyboard::Editing && k.targetButton==&target);
    assert(std::strcmp(k.getText(),"old")==0);
    k.setText("longer"); assert(!k.accept());
    assert(k.visible && k.result()==Keyboard::Editing && callbacks==0);
    assert(std::strcmp(target.getText(),"old")==0);
    k.setText("new"); const size_t allocations=host_allocation_calls;
    assert(k.accept()); assert(host_allocation_calls==allocations);
    assert(!k.visible && observed==Keyboard::Accepted && callbacks==1);
    assert(std::strcmp(target.getText(),"new")==0 && k.targetButton==nullptr);
    assert(!k.accept()); assert(callbacks==1);
    k.show(&target); k.setText("no"); k.cancel();
    assert(observed==Keyboard::Cancelled && callbacks==2 && !k.targetButton);
    assert(std::strcmp(target.getText(),"new")==0);
    k.show(&target); k.hide(); assert(std::strcmp(k.getText(),"new")==0);
    c.tft->ops.clear(); k.hide(); assert(c.tft->ops.empty());
    k.autoremove=true;
    for(int i=0;i<20;++i) {
        c.add(&k,0,0,false); k.show(&target); k.cancel();
        assert(c.widgets.count()==1 && c.widgets[0]==&unrelated);
    }
    k.autoremove=false; c.add(&k,0,0,false);
    target.contents->text[0]='a'; target.contents->text[1]='b'; target.contents->text[2]='c';
    target.contents->text[3]='d'; target.contents->text[4]='e';
    k.show(&target); assert(!k.visible && !k.targetButton);
    target.setText("old"); k.show(&target);
    char* saved=target.contents->text; target.contents->text=nullptr;
    assert(!k.accept() && k.visible && k.result()==Keyboard::Editing);
    target.contents->text=saved; k.cancel();
    host_nothrow_new_fail_after=0; Button oom;
    host_nothrow_new_fail_after=-1;
    k.show(&oom); assert(!k.visible && !k.targetButton);
    c.remove(&k); c.remove(&unrelated);
    Keyboard* doomed=new Keyboard; c.add(doomed,0,0,false);
    doomed->autoremove=true; doomed->setEventHandler(deleting); doomed->show();
    hit(*doomed,5,8); assert(c.widgets.count()==0);
    doomed=new Keyboard; c.add(doomed,0,0,false);
    doomed->autoremove=true; doomed->setEventHandler(deleting); doomed->show();
    hit(*doomed,5,6); assert(c.widgets.count()==0);
}
static void geometryTests() {
    KeyboardCanvas c; Keyboard k; c.add(&k,0,0,false);
    k.setSize(10,6); k.show(); c.tft->ops.clear(); k.hide();
    assert(c.tft->ops.empty());
    // Adversarial public geometry/style must never overflow/narrow into drawing.
    for (int width : {-1,0,1,219,220,231,240,INT_MAX})
    for (int height : {-1,0,1,71,72,197,INT_MAX})
    for (int position : {INT_MIN,-30,0,230,INT_MAX}) {
        k.w=width; k.h=height; k.x=position; k.y=position;
        k.visible=true; c.tft->ops.clear(); k.drawFrame();
        for(const auto& op : c.tft->ops) {
            assert(op.x>=0 && op.y>=0 && op.w>=0 && op.h>=0);
            assert(op.x+op.w<=c.tft->width() && op.y+op.h<=c.tft->height());
        }
        Point p; p.x=239; p.y=319; k.checkTouch(&p);
    }
    k.x=k.y=0; k.setSize(231,197); k.fontSize=1; k.borderWidth=2; k.reset(); k.show();
    // Every x pixel in an uneven row maps to precisely the proportional drawn cell.
    const char* row="qwertyuiop";
    for(int x=0;x<231;++x) {
        k.clear(); Point p; p.x=x; p.y=40; host_millis+=1000;
        assert(!k.checkTouch(&p)); unsigned col=0;
        while(col<9 && x>=int((col+1)*231/10)) ++col;
        assert(k.getTextSize()==1 && k.getText()[0]==row[col]);
    }
    k.clear(); Point p; p.x=0; p.y=40;
    k.lastMillis=ULONG_MAX-100; host_millis=200;
    assert(!k.checkTouch(&p) && k.getTextSize()==0); // 301ms, still debounce
    host_millis=500; assert(!k.checkTouch(&p) && k.getTextSize()==1);
    assert(!k.checkTouch(&p) && k.getTextSize()==1); // held touch consumes but does not edit
    host_millis+=500; assert(!k.checkTouch(&p) && k.getTextSize()==2);
    assert(k.checkTouch(nullptr)); p.x=231; assert(k.checkTouch(&p));
    k.reset(); assert(!k.shiftActive && !k.specialActive && k.result()==Keyboard::Editing);
    c.remove(&k);
    Screen screen(&c,0,0,240,320); screen.add(&k,0,0); c.setScreen(&screen);
    k.show(); k.setText("keep"); k.hide();
    assert(!k.visible && std::strcmp(k.getText(),"keep")==0);
    c.tft->ops.clear(); k.update(); k.hide(); assert(c.tft->ops.empty());
    k.autoremove=true; k.show(); k.cancel(); assert(screen.widgets.count()==0 && !k.visible);
    screen.detach(); c.setScreen(nullptr,false);
    // No-handler completion, including actual target backing-buffer malloc failure.
    Keyboard styled(231u,197u,BLUE,GREEN,RED);
    assert(styled.w==231 && styled.h==197 && styled.bgColor==BLUE && styled.fgColor==GREEN && styled.borderColor==RED);
    Keyboard huge(UINT_MAX,UINT_MAX,BLUE,GREEN,RED);
    assert(huge.w==0 && huge.h==0);
    Keyboard standalone; standalone.show(); assert(standalone.accept());
    standalone.show(); standalone.cancel(); assert(standalone.result()==Keyboard::Cancelled);
    host_allocation_fail_after=0; Button oomText;
    host_allocation_fail_after=-1;
    standalone.show(&oomText); assert(!standalone.visible && !standalone.targetButton);
    standalone.show(); assert(!standalone.visible); // invalid session remains inert
    standalone.reset(); standalone.setText("preserve");
    standalone.show(nullptr); assert(!standalone.visible && !standalone.targetButton && !standalone.accept());
    assert(std::strcmp(standalone.getText(),"preserve")==0);
    standalone.show(&standalone); assert(!standalone.visible && !standalone.targetButton);
    Button tooLong(50,20,BLACK,WHITE,WHITE,255);
    std::string longValue(KEYBOARD_BUF_SIZE+1,'x'); tooLong.setText(longValue.c_str());
    standalone.show(&tooLong); assert(!standalone.visible && !standalone.targetButton);
    assert(std::strcmp(standalone.getText(),"preserve")==0);
    standalone.reset(); standalone.show();
    const size_t before=host_allocation_calls;
    standalone.append('a'); standalone.deleteChar(); standalone.clear(); standalone.setText("ok");
    assert(host_allocation_calls==before);
    std::printf("keyboard sizeof=%zu, base Button=%zu, owned buffer=%u\n",sizeof(Keyboard),sizeof(Button),KEYBOARD_BUF_SIZE+1);
}
int main() {
    Keyboard k;
    assert(k.getTextSize()==0);
    std::string full(KEYBOARD_BUF_SIZE,'x');
    assert(k.setText(full.c_str())); k.append('y');
    assert(std::strcmp(k.getText(),full.c_str())==0);
    assert(!k.setText((full+"z").c_str()));
    assert(std::strcmp(k.getText(),full.c_str())==0);
    k.deleteChar(); assert(k.getTextSize()==KEYBOARD_BUF_SIZE-1);
    k.clear(); k.deleteChar(); k.append('#'); k.append('^'); k.append('<'); k.append('\\');
    k.append('\0'); k.append('\n'); k.append(char(127));
    assert(std::strcmp(k.getText(),"#^<\\")==0);
    assert(!k.setText("bad\n")); assert(std::strcmp(k.getText(),"#^<\\")==0);
    assert(k.setText(nullptr)); assert(k.getTextSize()==0);
    k.setText("abc"); assert(k.setText(k.getText()+1)); assert(std::strcmp(k.getText(),"bc")==0);
    host_nothrow_new_fail_after=0;
    Keyboard oom; oom.append('a'); assert(std::strcmp(oom.getText(),"a")==0);
    host_nothrow_new_fail_after=-1;
    noOpTouchPaintTests();
    textTouchRepaintTests();
    layoutTests();
    sessionTests();
    geometryTests();
    std::puts("keyboard: PASS");
}
