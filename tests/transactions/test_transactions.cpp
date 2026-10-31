#include <Adafruit_ILI9341.h>
// White-box boundary injection avoids billions of startWrite calls; no
// production test hooks or reduced-width production counters.
#define private public
#include "GUI_TFT.h"
#undef private
#include <cassert>
#include <cstdio>
#include <type_traits>
static void earlyReturn(Guitft &panel) {
  Guitft::ScopedWrite render(panel);
  panel.drawString("early",8,8,1,WHITE);
  return;
}
static void fullDraw(Guitft &panel) {
  Guitft::ScopedWrite render(panel);
  panel.fillRect(1,1,50,50,BLACK);
  panel.drawRect(1,1,50,50,WHITE);
  { Guitft::ScopedWrite inner(panel);
    panel.fillCircle(25,25,8,GREEN);
    panel.drawLine(2,3,42,20,RED);
    panel.drawString("SPI",3,4,2,YELLOW);
    panel.drawNumber(42,3,25,1,BLUE);
  }
  assert(panel.selected);
}
int main() {
  Guitft panel(1,2);
  Adafruit_GFX &gfx=panel;
  // Real GFX drawRect calls startWrite, writeFast{H,V}Line and endWrite.
  // Its writePixel fallback reaches the instrumented panel drawPixel.
  gfx.drawRect(2,3,12,9,0x1234);
  assert(panel.writes>0);
  assert(panel.begins==1 && panel.ends==1);
  assert(!panel.selected && panel.writesOutside==0);
  std::puts("PASS real GFX drawRect virtual transaction dispatch");
  panel.endWrite(); // Unmatched end must leave the next real draw functional.
  panel.endWrite();
  gfx.drawCircle(40,40,8,0xabcd);
  assert(panel.begins==2 && panel.ends==2);
  assert(!panel.selected && panel.writesOutside==0);
  std::puts("PASS unmatched end does not underflow");
  Guitft saturated(1,2);
  saturated.startWrite();
  saturated.transactionDepth_=UINT32_MAX;
  saturated.startWrite(); // Unrepresentable nesting must latch, not wrap.
  saturated.endWrite();
  assert(saturated.transactionDepth_==UINT32_MAX);
  assert(saturated.begins==1 && saturated.ends==0 && saturated.selected);
  saturated.drawRect(0,0,4,4,0x8888);
  assert(saturated.begins==1 && saturated.ends==0 && saturated.selected);
  std::puts("PASS overflow fails closed without early CS release");
  Guitft boundary(1,2);
  boundary.startWrite();
  boundary.transactionDepth_=UINT32_MAX-1;
  boundary.startWrite();
  boundary.endWrite(); // Exactly representable maximum is not overflow.
  assert(boundary.transactionDepth_==UINT32_MAX-1 && boundary.ends==0);
  boundary.transactionDepth_=1; // Undo the artificial boundary injection.
  boundary.endWrite();
  assert(boundary.ends==1 && !boundary.selected);
  Adafruit_ILI9341 baseline(1,2);
  Guitft nested(1,2);
  baseline.drawRect(2,3,12,9,0x1234);
  nested.drawRect(2,3,12,9,0x1234);
  assert(baseline.pixels==nested.pixels && baseline.writes==nested.writes);
  assert(baseline.begins>nested.begins && nested.begins==1 && nested.ends==1);
  std::printf("COUNT BENCH real GFX drawRect: baseline=%u/%u nested=%u/%u writes=%u (same pixels)\n",
              baseline.begins,baseline.ends,nested.begins,nested.ends,nested.writes);
  static_assert(!std::is_copy_constructible<Guitft::ScopedWrite>::value,"no double end");
  static_assert(!std::is_move_constructible<Guitft::ScopedWrite>::value,"scope bound");
  Guitft scoped(1,2);
  earlyReturn(scoped);
  assert(scoped.begins==1 && scoped.ends==1 && !scoped.selected);
  fullDraw(scoped);
  assert(scoped.begins==2 && scoped.ends==2 && !scoped.selected);
  assert(scoped.writes>0 && scoped.writesOutside==0);
  scoped.startWrite();
  for(unsigned i=0;i<1000;++i) scoped.startWrite();
  scoped.drawRect(0,0,5,5,WHITE);
  for(unsigned i=0;i<1000;++i) scoped.endWrite();
  assert(scoped.begins==3 && scoped.ends==2 && scoped.selected);
  scoped.endWrite();
  assert(scoped.begins==3 && scoped.ends==3 && !scoped.selected);
  std::printf("PASS scoped early return, nested full draw, 1001 levels: %u begins/%u ends\n",scoped.begins,scoped.ends);
}
