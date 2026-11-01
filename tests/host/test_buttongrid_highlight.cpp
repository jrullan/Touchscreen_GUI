#include "Buttongrid.h"
#include <cassert>
#include <cstdio>

class GridCanvas : public Canvas {
public:
    GridCanvas() : Canvas(TFT_PORTRAIT, BLACK) {}
    void init() override {}
    Point* getTouchedPoint() override { return nullptr; }
};
static int callbacks;
static unsigned char observed;
static void pressed(Buttongrid*, unsigned char id) { ++callbacks; observed=id; }
static void hit(Buttongrid& grid, unsigned char id, bool advance=true) {
    Point p;
    p.x=grid.x+grid.borderWidth+(grid.getColumn(id)-1)*(grid.btnWidth+grid.borderWidth)+grid.btnWidth/2;
    p.y=grid.y+grid.borderWidth+(grid.getRow(id)-1)*(grid.btnHeight+grid.borderWidth)+grid.btnHeight/2;
    if (advance) host_millis+=1000; // Beyond the legacy 500ms debounce.
    assert(grid.checkTouch(&p)); // Legacy propagation contract.
}
static void assertFill(const HostDraw& op, Buttongrid& grid, unsigned char id, int color) {
    assert(op.name=="fillRect" && op.color==color);
    assert(op.x==grid.x+grid.borderWidth+(grid.getColumn(id)-1)*(grid.btnWidth+grid.borderWidth));
    assert(op.y==grid.y+grid.borderWidth+(grid.getRow(id)-1)*(grid.btnHeight+grid.borderWidth));
    assert(op.w==grid.btnWidth && op.h==grid.btnHeight);
}
static void firstTouchHighlights() {
    GridCanvas canvas; Buttongrid grid(2,2,BUTTONGRID_HIGHLIGHT);
    grid.setSize(100,80); grid.setColors(BLUE,WHITE,RED); grid.highlightColor=YELLOW;
    grid.setEventHandler(pressed); canvas.add(&grid,10,20,false); grid.show();
    callbacks=0; canvas.tft->ops.clear();
    assert(grid.lastPressed==0); hit(grid,1);
    assert(callbacks==1 && observed==1 && grid.lastPressed==1);
    assert(canvas.tft->ops.size()==2); // Fill current cell, then redraw its label.
    assertFill(canvas.tft->ops[0],grid,1,YELLOW);
    assert(canvas.tft->ops[1].name=="text:1");
    canvas.tft->ops.clear(); hit(grid,2,false);
    assert(callbacks==1 && grid.lastPressed==1 && canvas.tft->ops.empty());
    hit(grid,2);
    assert(callbacks==2 && observed==2 && grid.lastPressed==2);
    assert(canvas.tft->ops.size()==4);
    assertFill(canvas.tft->ops[0],grid,1,BLUE);
    assert(canvas.tft->ops[1].name=="text:1");
    assertFill(canvas.tft->ops[2],grid,2,YELLOW);
    assert(canvas.tft->ops[3].name=="text:2");
    grid.clear(); assert(grid.lastPressed==0);
    canvas.tft->ops.clear(); hit(grid,3);
    assert(callbacks==3 && observed==3 && grid.lastPressed==3);
    assert(canvas.tft->ops.size()==2);
    assertFill(canvas.tft->ops[0],grid,3,YELLOW);
    assert(canvas.tft->ops[1].name=="text:3");
    canvas.remove(&grid);
}
static void maintainedAndNoHighlightContracts() {
    for (unsigned char config : {static_cast<unsigned char>(BUTTONGRID_HIGHLIGHT|BUTTONGRID_MAINTAINED),
                                 static_cast<unsigned char>(BUTTONGRID_MAINTAINED),
                                 static_cast<unsigned char>(0)}) {
        GridCanvas canvas; Buttongrid grid(2,2,config);
        grid.setSize(100,80); grid.setColors(BLUE,WHITE,RED); grid.highlightColor=YELLOW;
        grid.setEventHandler(pressed); canvas.add(&grid,10,20,false); grid.show();
        callbacks=0;
        for (int i=0; i<2; ++i) {
            canvas.tft->ops.clear(); hit(grid,1);
            assert(callbacks==i+1 && observed==1 && grid.lastPressed==1);
            assert(grid.states[1]==((config&BUTTONGRID_MAINTAINED) ? (i==0) : 0));
            if (config&BUTTONGRID_HIGHLIGHT) {
                assert(canvas.tft->ops.size()==2);
                assertFill(canvas.tft->ops[0],grid,1,i==0 ? YELLOW : BLUE);
                assert(canvas.tft->ops[1].name=="text:1");
            } else assert(canvas.tft->ops.empty());
        }
        if (config&BUTTONGRID_MAINTAINED) {
            hit(grid,1); hit(grid,2);
            assert(grid.states[1]==1 && grid.states[2]==1); // Independent toggles.
        }
        canvas.remove(&grid);
    }
}
int main() {
    firstTouchHighlights();
    maintainedAndNoHighlightContracts();
    std::puts("buttongrid highlight: PASS");
}
