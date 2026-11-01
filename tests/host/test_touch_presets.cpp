#include <cassert>
#include <cstdio>
#include "TouchMapper.h"
class RawContact : public TouchDriver {
public:
    int16_t x=0,y=0;
    bool begin() override { return true; }
    bool touched() override { return true; }
    bool needsFiltering() override { return false; }
    void readRaw(int16_t& rx,int16_t& ry,int16_t& rz) override { rx=x;ry=y;rz=1; }
};
static void verify(const TouchCalibration& cal) {
    RawContact contact;
    TouchMapper mapper(&contact,cal);
    assert(mapper.begin());
    const int expected[4][4][2]={
        {{0,0},{239,0},{0,319},{239,319}},
        {{0,239},{0,0},{319,239},{319,0}},
        {{239,319},{0,319},{239,0},{0,0}},
        {{319,0},{319,239},{0,0},{0,239}}
    };
    for(int rotation=0;rotation<4;++rotation){
        mapper.setOrientation(rotation);
        for(int corner=0;corner<4;++corner){
            contact.x=(corner&1)?cal.rawMaxX:cal.rawMinX;
            contact.y=(corner&2)?cal.rawMaxY:cal.rawMinY;
            mapper.poll();Point p=mapper.currentPoint();
            assert(p.x==expected[rotation][corner][0]);
            assert(p.y==expected[rotation][corner][1]);
        }
    }
}
int main(){
    const TouchCalibration ft=TouchCal::FT6206_DEFAULT;
    const TouchCalibration stm=TouchCal::STMPE610_DEFAULT;
    assert(ft.rawMinX==239 && ft.rawMaxX==0 && ft.rawMinY==319 && ft.rawMaxY==0 && !ft.swapXY);
    assert(stm.rawMinX==3775 && stm.rawMaxX==240 && stm.rawMinY==240 && stm.rawMaxY==3700 && !stm.swapXY);
    verify(ft);verify(stm);
    puts("adapter calibration presets: PASS 32 rotated corners (nominal defaults, not hardware calibration)");
}
