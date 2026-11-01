#include <cassert>
#include <cstdio>
#include <type_traits>
#include "Touch.h"
// Match this repository's bundled API (including its two getPoint overloads).
// Only the I2C boundary is replaced; the adapter and mapper remain production.
#define ADAFRUIT_FT6206_LIBRARY
#define FT62XX_DEFAULT_THRESHOLD 128
class Adafruit_FT6206 : public Touch {
public:
    static int live, constructed, destroyed, begins, touches, reads;
    static bool result;
    static uint8_t threshold, count;
    static Point point;
    Adafruit_FT6206() { ++live; ++constructed; }
    ~Adafruit_FT6206() override { --live; ++destroyed; }
    bool begin(uint8_t thresh = FT62XX_DEFAULT_THRESHOLD) override {
        ++begins; threshold = thresh; return result;
    }
    uint8_t touched() override { ++touches; return count > 2 ? 0 : count; }
    Point getPoint(uint8_t n = 0) {
        assert(n == 0); ++reads; return point;
    }
    Point getPoint() override { return getPoint(uint8_t(0)); }
};
int Adafruit_FT6206::live=0, Adafruit_FT6206::constructed=0, Adafruit_FT6206::destroyed=0;
int Adafruit_FT6206::begins=0, Adafruit_FT6206::touches=0, Adafruit_FT6206::reads=0;
bool Adafruit_FT6206::result=false;
uint8_t Adafruit_FT6206::threshold=0, Adafruit_FT6206::count=0;
Point Adafruit_FT6206::point(0,0,0);
#include "TouchDriver_FT6206.h"
#include "TouchMapper.h"
#include "allocation_hooks.h"
using HW = Adafruit_FT6206;
static void lifecycle() {
    assert(!std::is_copy_constructible<TouchDriver_FT6206>::value);
    assert(!std::is_move_constructible<TouchDriver_FT6206>::value);
    const int starts=HW::constructed, stops=HW::destroyed, attempts=HW::begins;
    {
        host_nothrow_new_fail_after=0;
        TouchDriver_FT6206 driver;
        assert(host_nothrow_new_fail_after==0);
        host_nothrow_new_fail_after=-1;
        assert(HW::constructed==starts+1 && HW::live==1 && HW::begins==attempts);
        int16_t x=99,y=99,z=99;
        const int touches=HW::touches, reads=HW::reads;
        assert(!driver.touched() && driver.touchCount()==0);
        driver.readRaw(x,y,z);
        assert(x==0 && y==0 && z==0 && HW::touches==touches && HW::reads==reads);
        HW::result=false;
        assert(!driver.begin() && HW::begins==attempts+1);
        x=y=z=99;
        assert(!driver.touched() && driver.touchCount()==0);
        driver.readRaw(x,y,z);
        assert(x==0 && y==0 && z==0 && HW::touches==touches && HW::reads==reads);
        assert(!driver.begin() && HW::begins==attempts+2);
        HW::result=true;
        assert(driver.begin() && HW::threshold==FT62XX_DEFAULT_THRESHOLD);
        assert(driver.begin() && driver.begin() && HW::begins==attempts+3);
        assert(HW::constructed==starts+1);
    }
    assert(HW::live==0 && HW::destroyed==stops+1);
    {
        TouchDriver_FT6206 driver(37);
        assert(driver.begin() && HW::threshold==37);
    }
    assert(HW::live==0 && HW::destroyed==stops+2);
}
static void samples() {
    TouchDriver_FT6206 driver;
    assert(driver.begin());
    assert(!driver.needsFiltering());
    for (uint8_t count=0; count<=3; ++count) {
        HW::count=count;
        const int before=HW::touches;
        assert(driver.touchCount()==(count<=2 ? count : 0));
        assert(driver.touched()==(count==1 || count==2));
        assert(HW::touches==before+2); // Each query reads the live count, no cached events.
    }
    const Point points[]={Point(0,0,1),Point(239,319,1),Point(4095,4095,1),Point(0,0,0)};
    for (const Point& p : points) {
        HW::point=p;
        int16_t x=-1,y=-1,z=-1;
        const int reads=HW::reads, touches=HW::touches;
        driver.readRaw(x,y,z);
        assert(x==p.x && y==p.y && z==p.z);
        assert(HW::reads==reads+1 && HW::touches==touches);
    }
}
static void mapperIntegration() {
    const TouchCalibration proposed={239,0,319,0,false};
    TouchDriver_FT6206 driver;
    const int stops=HW::destroyed;
    {
        TouchMapper mapper(&driver,proposed);
        assert(mapper.begin());
        HW::count=1;
        const int rawX[]={239,0,0,239}, rawY[]={319,319,0,0};
        const int portraitX[]={0,239,239,0}, portraitY[]={0,0,319,319};
        for (uint8_t r=0; r<4; ++r) {
            mapper.setOrientation(r);
            for (int corner=0; corner<4; ++corner) {
                HW::point=Point(rawX[corner],rawY[corner],1);
                const int reads=HW::reads, touches=HW::touches;
                mapper.poll();
                assert(HW::reads==reads+1 && HW::touches==touches+1);
                const int px=portraitX[corner], py=portraitY[corner];
                const int xs[]={px,py,239-px,319-py};
                const int ys[]={py,239-px,319-py,px};
                const Point p=mapper.currentPoint();
                assert(p.x==xs[r] && p.y==ys[r] && p.z==1);
                assert(mapper.state()==(corner==0 ? TOUCH_PRESSED : TOUCH_HELD));
            }
            HW::count=0;
            const int reads=HW::reads;
            mapper.poll(); assert(mapper.state()==TOUCH_RELEASED);
            mapper.poll(); assert(mapper.state()==TOUCH_IDLE);
            assert(HW::reads==reads); // Never manufacture a sample when count is zero.
            HW::count=2;
            mapper.poll(); assert(mapper.state()==TOUCH_PRESSED);
            HW::count=1;
        }
    }
    // Mapper borrowed this adapter; destruction cannot delete it or its controller.
    assert(HW::destroyed==stops && HW::live==1 && driver.begin());
}
int main() {
    lifecycle();
    samples();
    mapperIntegration();
    assert(HW::live==0);
    std::puts("touch ft6206: PASS");
}
