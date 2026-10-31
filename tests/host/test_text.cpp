#include "Text.h"
#include "Button.h"
#include "Display.h"
#include "OptionButton.h"
#include "IconButton.h"
#include "Popup.h"
#include "Numkey.h"
#include "Terminal.h"
#include "Buttongrid.h"
#include <climits>
#include <type_traits>
#include "allocation_hooks.h"
static_assert(!std::is_copy_constructible<Text>::value, "Text owns storage and must not shallow-copy");
#include <cassert>
#include <cstring>
#include <cstdio>

class TextCanvas : public Canvas {
public:
    TextCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static bool drew(const TextCanvas& canvas, const char* name) {
    for (const auto& op : canvas.tft->ops) if (op.name == name) return true;
    return false;
}

int main() {
    Text text(4);
    text.setText("abcd");
    assert(std::strcmp(text.getText(), "abcd") == 0);
    text.setText((const char*)nullptr);
    assert(text.getText()[0] == 0);
    Button button;
    char label[] = "owned";
    button.setLabel(label);
    label[0] = 'X';
    assert(std::strcmp(button.label, "owned") == 0);
    Display display(8);
    display.setCanvas(nullptr);
    char one[] = "x";
    display.append(one);
    display.append('y');
    assert(std::strcmp(display.contents->text, "xy") == 0);
    display.append((char*)nullptr);
    display.deleteChar();
    assert(std::strcmp(display.contents->text, "x") == 0);
    display.show();
    button.show();
    button.update();
    assert(!button.checkTouch(nullptr));
    button.setLabel(button.label + 1);
    assert(std::strcmp(button.label, "wned") == 0);
    button.setLabel(nullptr);
    assert(button.label == nullptr);
    button.setText((const char*)nullptr);
    assert(button.getText()[0] == 0);
    { OptionButton option; option.show(); option.update(); }
    { IconButton icon(1, 1, nullptr, nullptr, nullptr); icon.show(); icon.update(); }
    { char message[] = "confirm"; Popup popup(message); }
    { Popup popup; popup.show(); popup.hide(); popup.processEvent(1);
      popup.targetEventHandler(); assert(!popup.checkTouch(nullptr)); }
    { Numkey keypad; char digit[] = "7"; keypad.append(digit);
      assert(std::strcmp(keypad.getText(), "7") == 0);
      keypad.append(nullptr); keypad.deleteChar();
      assert(keypad.getTextSize() == 0);
      for (int number : {INT_MIN, INT_MAX, -1, 0, 1}) {
          char expected[3 * sizeof(int) + 3];
          std::snprintf(expected, sizeof expected, "%d", number);
          expected[DISPLAY_SIZE] = 0;
          keypad.setNum(number);
          assert(std::strcmp(keypad.getText(), expected) == 0);
      }
      keypad.show(); keypad.hide(); keypad.clear(); keypad.targetEventHandler();
      assert(!keypad.checkTouch(nullptr));
    }
    { Terminal empty(-1, -1, TERMINAL_SCROLL_DOWN, 0);
      assert(empty.lines == 0 && empty.maxCharacters == 0);
      empty.print("x"); empty.printf((char*)"%d", INT_MIN);
      empty.clear(); empty.scrollUp(); empty.scrollDown(); empty.show();
    }
    { Terminal terminal(200, 200, TERMINAL_SCROLL_DOWN, 1);
      terminal.print("abc"); assert(std::strcmp(terminal.linesBuffer[0], "abc") == 0);
      terminal.printf((char*)"%d/%d!", INT_MIN);
      char expected[80]; std::snprintf(expected,sizeof expected,"%d/%d!",INT_MIN,INT_MIN);
      expected[terminal.maxCharacters] = 0;
      assert(std::strcmp(terminal.linesBuffer[0], expected) == 0);
      assert(std::strcmp(terminal.linesBuffer[1], "abc") == 0);
      terminal.print(String("string")); terminal.print((char*)nullptr);
      terminal.clear(); terminal.update();
    }
    { Buttongrid grid(2,3,BUTTONGRID_USELABELS | BUTTONGRID_MAINTAINED);
      char name[] = "last"; grid.setLabel(6,name); name[0]='X';
      assert(std::strcmp(grid.labels[6],"last") == 0);
      for (int i=0;i<=6;++i) assert(grid.states[i] == 0);
      assert(grid.getId(1,1)==1 && grid.getRow(1)==1 && grid.getColumn(1)==1);
      assert(grid.getId(2,3)==6 && grid.getRow(6)==2 && grid.getColumn(6)==3);
      assert(grid.getId(0,1)==0 && grid.getRow(0)==0 && grid.getColumn(7)==0);
      grid.setLabel(0,name); grid.setLabel(7,name);
      grid.show(); grid.clear(); assert(grid.checkTouch(nullptr));
      grid.configure(3); assert(grid.getId(3,3)==9); grid.setLabel(9,name);
    }
    { Buttongrid grid(0,0,0); grid.setSize(20,20); grid.show();
      assert(grid.getColumn(1)==0 && grid.getRow(1)==0); }
    { Buttongrid grid(255,255,0); assert(grid.rows==0 && grid.columns==0); }
    { host_allocation_fail_after = 0; Text empty(8);
      assert(empty.text == nullptr); empty.setText("x"); empty.clear();
      assert(empty.getTextSize()==0 && empty.getNum()==0);
    }
    { host_allocation_fail_after = 0; Button empty;
      empty.setText("x"); empty.fitToText(); empty.show(); assert(empty.getNum()==0);
    }
    { host_allocation_fail_after = 0; Display empty;
      empty.append(one); empty.append('x'); empty.deleteChar(); empty.show(); empty.fitToText();
    }
    { host_allocation_fail_after = 1; Numkey empty;
      assert(empty.getText()==nullptr); empty.append(one); empty.appendNum(1);
      empty.setNum(INT_MIN); empty.clear(); empty.deleteChar(); empty.fitToText(); empty.update();
      assert(empty.getNum()==0);
    }
    for (int fail=0;fail<10;++fail) {
      host_allocation_fail_after=fail; Terminal empty(200,200);
      assert(empty.lines==0); empty.print("x"); empty.clear(); empty.scrollUp(); empty.scrollDown();
    }
    for (int fail=1;fail<=2;++fail) {
      host_allocation_fail_after=fail; Buttongrid empty(2,3,BUTTONGRID_USELABELS);
      assert(empty.states==nullptr && empty.labels==nullptr);
      empty.setLabel(1,one); empty.show(); empty.setSize(5,5);
    }
    { Button owned; owned.setLabel((char*)"keep");
      host_allocation_fail_after=0; owned.setLabel((char*)"replace");
      assert(std::strcmp(owned.label,"keep")==0);
    }
    { Buttongrid owned(2,2,BUTTONGRID_USELABELS);
      owned.setLabel(4,(char*)"keep");
      host_allocation_fail_after=0; owned.setLabel(4,(char*)"replace");
      assert(std::strcmp(owned.labels[4],"keep")==0);
      host_allocation_fail_after=0; owned.configure(3);
      assert(owned.rows==2 && owned.columns==2 && std::strcmp(owned.labels[4],"keep")==0);
    }
    host_allocation_fail_after=-1;
    {
      TextCanvas canvas;
      char message[]="confirm";
      Popup popup(message); popup.setCanvas(&canvas); popup.setSize(200,160);
      message[0]='X';
      popup.show();
      assert(drew(canvas,"text:confirm"));
      canvas.tft->ops.clear(); popup.hide();
      assert(!canvas.tft->ops.empty() && !popup.visible);
      for (byte mode : {POPUP_OK,POPUP_YES_NO,POPUP_OK_CANCEL}) {
        popup.popup_type=mode; popup.show();
      }
      assert(drew(canvas,"text:confirm") && drew(canvas,"text:Cancel"));
      popup.setText((char*)"replacement"); popup.show();
      assert(drew(canvas,"text:replacement"));
      host_allocation_fail_after=0; popup.setText((char*)"OOM");
      canvas.tft->ops.clear(); popup.show(); assert(drew(canvas,"text:replacement"));
      popup.setText(nullptr); popup.hide();
      Button b((char*)"yes",(char*)"label"); b.setCanvas(&canvas); b.show();
      assert(drew(canvas,"text:yes"));
      Button wide(100,40,BLACK,WHITE,WHITE,255);
      std::string letters(200,'a'); wide.setText(letters.c_str()); wide.fitToText();
      assert(wide.w==200*FONT_X*wide.fontSize+FONT_SPACE);
      wide.setCanvas(&canvas); wide.setSize(30,300); canvas.tft->ops.clear(); wide.drawBorder();
      assert(canvas.tft->ops.front().h==300);
      b.hide(); canvas.tft->ops.clear(); b.show();
      assert(b.visible && drew(canvas,"text:yes"));
      Numkey keypad; keypad.setCanvas(&canvas); keypad.show();
      canvas.tft->ops.clear(); keypad.hide(); assert(!canvas.tft->ops.empty());
      canvas.tft->ops.clear(); keypad.show(); assert(keypad.visible && !canvas.tft->ops.empty());
      OptionButton option; option.setCanvas(&canvas); option.show(); option.touched=true; option.update();
      uint16_t bitmap[1]={RED}; IconButton icon(1,1,bitmap,nullptr,(char*)"icon");
      icon.setCanvas(&canvas); icon.update(); assert(canvas.tft->pixels[0]==RED);
      Display d(3); d.setCanvas(&canvas); d.setSize(50,30);
      d.append((char*)"abcdef"); d.append('z');
      assert(std::strcmp(d.contents->text,"abc")==0); d.show();
      assert(drew(canvas,"text:abc"));
      Buttongrid grid(2,3,BUTTONGRID_USELABELS|BUTTONGRID_MAINTAINED|BUTTONGRID_HIGHLIGHT);
      grid.setCanvas(&canvas); grid.setSize(120,100); grid.setLabel(6,(char*)"last"); grid.show();
      assert(drew(canvas,"text:last"));
      Point last; last.x=2+2*(grid.btnWidth+2)+grid.btnWidth/2;
      last.y=2+(grid.btnHeight+2)+grid.btnHeight/2;
      host_millis+=1000; grid.checkTouch(&last); assert(grid.states[6]==1 && grid.lastPressed==6);
      host_millis+=1000; grid.checkTouch(&last); assert(grid.states[6]==0);
      Buttongrid boundary(255,1,0); assert(boundary.getId(255,1)==255);
      assert(boundary.states[255]==0 && boundary.getRow(255)==255);
      Terminal up(74,34,TERMINAL_SCROLL_UP,1); up.setCanvas(&canvas);
      assert(up.lines==2 && up.maxCharacters==10);
      up.print("one", RED); up.print("two", GREEN); up.print("three", BLUE);
      assert(std::strcmp(up.linesBuffer[0],"two")==0 && std::strcmp(up.linesBuffer[1],"three")==0);
      assert(up.linesColors[0]==GREEN && up.linesColors[1]==BLUE);
      up.clear(); up.printf((char*)"a%d%d!",INT_MAX);
      assert(std::strlen(up.linesBuffer[0])==10);
      up.clear(); up.print(String("01234567890123456789"));
      assert(std::strcmp(up.linesBuffer[0],"0123456789")==0);
      up.printf((char*)"tail%",0); assert(std::strcmp(up.linesBuffer[1],"tail%")==0);
      up.show();
    }
    { Text longText(255); std::string digits(255,'9'); longText.setText(digits.c_str());
      assert(longText.getTextSize()==255 && longText.getNum()==LONG_MAX);
      longText.setText("-42.5"); assert(longText.getNum()==-42);
      longText.setText(longText.text+1); assert(std::strcmp(longText.text,"42.5")==0);
      longText.clear(); assert(longText.getTextSize()==0);
    }
    { Text zero(0); zero.setText("x"); assert(zero.getTextSize()==0 && zero.text[0]==0); }
    { Buttongrid numeric(1,2,BUTTONGRID_USELABELS|BUTTONGRID_NUMLABELS);
      numeric.setLabel(2,(char*)"ignored"); assert(numeric.labels[2]==nullptr);
      host_allocation_fail_after=1; numeric.configure(3);
      assert(numeric.rows==1 && numeric.columns==2);
    }
    { Terminal tiny(20,34); assert(tiny.maxCharacters==1); tiny.printf((char*)"%d",INT_MIN);
      assert(std::strcmp(tiny.linesBuffer[0],"-")==0); }
    { Terminal large(200,32767,TERMINAL_SCROLL_DOWN,1); assert(large.lines==MAX_LINES);
      std::string input(600,'x'); large.print(input.c_str());
      assert((int)std::strlen(large.linesBuffer[0])==large.maxCharacters);
      Terminal invalidFont(20,20,TERMINAL_SCROLL_DOWN,INT_MAX);
      assert(invalidFont.fontSize==31 && invalidFont.lines==0);
    }
    std::puts("text: PASS");
}
