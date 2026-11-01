#include <cassert>
#include <cstdio>
#include <type_traits>
#include "Touch.h"
#include "allocation_hooks.h"

// Only replace the bundled hardware boundary; use the real adapter and mapper.
#define _ADAFRUIT_STMPE610H_
#define STMPE_CS 16
#define STMPE_ADDR 0x41
class Adafruit_STMPE610 : public Touch {
public:
    static int live, constructed, begins;
    static uint8_t lastCs, lastAddress;
    static bool result;
    static uint8_t down;
    static int touches, empties, reads;
    static Point fifo[4];
    static unsigned head, tail;
    static void resetSamples() { touches=empties=reads=0; head=tail=0; down=0; }
    static void enqueue(Point p) { assert(tail<4); fifo[tail++]=p; }
    explicit Adafruit_STMPE610(uint8_t cs) { ++live; ++constructed; lastCs=cs; }
    ~Adafruit_STMPE610() override { --live; }
    bool begin(uint8_t address=STMPE_ADDR) override {
        ++begins; lastAddress=address; return result;
    }
    uint8_t touched() override { ++touches; return down; }
    bool bufferEmpty() { ++empties; return head==tail; }
    void readData(uint16_t* x,uint16_t* y,uint8_t* z) {
        assert(head<tail); ++reads;
        const Point p=fifo[head++]; *x=p.x; *y=p.y; *z=p.z;
    }
    Point getPoint() override { assert(false && "bundled getPoint drains FIFO"); return Point(); }
};
int Adafruit_STMPE610::live=0, Adafruit_STMPE610::constructed=0;
int Adafruit_STMPE610::begins=0;
uint8_t Adafruit_STMPE610::lastCs=0, Adafruit_STMPE610::lastAddress=0;
bool Adafruit_STMPE610::result=false;
uint8_t Adafruit_STMPE610::down=0;
int Adafruit_STMPE610::touches=0, Adafruit_STMPE610::empties=0, Adafruit_STMPE610::reads=0;
Point Adafruit_STMPE610::fifo[4];
unsigned Adafruit_STMPE610::head=0, Adafruit_STMPE610::tail=0;
#include "TouchDriver_STMPE610.h"
#include "TouchMapper.h"

static void startup() {
    using HW=Adafruit_STMPE610;
    static_assert(!std::is_copy_constructible<TouchDriver_STMPE610>::value, "hardware identity");
    static_assert(!std::is_convertible<uint8_t,TouchDriver_STMPE610>::value, "explicit CS");
    host_nothrow_new_fail_after=0;
    {
        TouchDriver_STMPE610 driver;
        assert(host_nothrow_new_fail_after==0);
        assert(HW::constructed==1 && HW::live==1 && HW::lastCs==STMPE_CS);
        int16_t x=99,y=99,z=99;
        assert(!driver.touched() && driver.touchCount()==0);
        driver.readRaw(x,y,z);
        assert(x==0 && y==0 && z==0 && HW::begins==0);
        assert(HW::touches==0 && HW::empties==0 && HW::reads==0);
        assert(!driver.begin() && HW::begins==1 && HW::lastAddress==STMPE_ADDR);
        assert(!driver.touched() && driver.touchCount()==0);
        x=y=z=99; driver.readRaw(x,y,z);
        assert(x==0 && y==0 && z==0);
        assert(HW::touches==0 && HW::empties==0 && HW::reads==0);
        HW::result=true;
        assert(driver.begin() && HW::begins==2);
        assert(driver.begin() && HW::begins==2 && HW::constructed==1);
    }
    host_nothrow_new_fail_after=-1;
    assert(HW::live==0);
    {
        TouchDriver_STMPE610 driver(7,0x44);
        assert(HW::lastCs==7 && driver.begin() && HW::lastAddress==0x44);
    }
}
static void invalidCs() {
    using HW=Adafruit_STMPE610;
    const int begins=HW::begins;
    for (uint8_t cs : {uint8_t(128),uint8_t(255)}) {
        TouchDriver_STMPE610 driver(cs);
        assert(!driver.begin() && !driver.begin() && HW::begins==begins);
        assert(!driver.touched() && driver.touchCount()==0);
        int16_t x=1,y=1,z=1; driver.readRaw(x,y,z);
        assert(x==0 && y==0 && z==0);
    }
}
static void contact() {
    using HW=Adafruit_STMPE610;
    HW::resetSamples(); HW::result=true;
    TouchDriver_STMPE610 driver;
    assert(driver.begin());
    assert(!driver.touched() && HW::touches==1 && HW::empties==0);
    assert(driver.touchCount()==0 && HW::touches==2 && HW::reads==0);
    HW::down=0x80;
    assert(!driver.touched() && HW::empties==1); // Contact alone is not a sample.
    HW::enqueue(Point(4095,0,255));
    assert(driver.touched() && HW::touches==4 && HW::empties==2);
    assert(driver.touchCount()==1 && HW::touches==5 && HW::reads==0 && HW::head==0);
}
static void fifoRaw() {
    using HW=Adafruit_STMPE610;
    HW::resetSamples(); HW::result=true;
    TouchDriver_STMPE610 driver; assert(driver.begin());
    int16_t x=99,y=99,z=99;
    driver.readRaw(x,y,z);
    assert(x==0 && y==0 && z==0 && HW::reads==0 && HW::empties==1);
    HW::enqueue(Point(4095,0,255)); HW::enqueue(Point(0,4095,0));
    driver.readRaw(x,y,z);
    assert(x==4095 && y==0 && z==255 && HW::reads==1 && HW::head==1);
    driver.readRaw(x,y,z);
    assert(x==0 && y==4095 && z==0 && HW::reads==2 && HW::head==2);
    x=y=z=99; driver.readRaw(x,y,z);
    assert(x==0 && y==0 && z==0 && HW::reads==2 && HW::touches==0);
}
static void mapperSamplesOnce() {
    using HW=Adafruit_STMPE610;
    HW::resetSamples(); HW::result=true; HW::down=0x80;
    HW::enqueue(Point(100,20,7)); HW::enqueue(Point(10,200,255));
    HW::enqueue(Point(200,10,0));
    TouchDriver_STMPE610 driver;
    const TouchCalibration identity={0,239,0,319,false};
    TouchMapper mapper(&driver,identity);
    mapper.poll();
    assert(HW::touches==0 && HW::empties==0 && HW::reads==0);
    assert(mapper.begin());
    const Point expected[]={Point(100,20,7),Point(10,200,255),Point(200,10,0)};
    for (unsigned i=0;i<3;++i) {
        mapper.poll();
        assert(HW::reads==int(i+1) && HW::head==i+1 && HW::touches==int(i+1));
        assert(mapper.currentPoint()==expected[i]);
        assert(mapper.state()==(i==0 ? TOUCH_PRESSED : TOUCH_HELD));
    }
    assert(!driver.needsFiltering());
    mapper.poll();
    assert(mapper.state()==TOUCH_RELEASED && HW::reads==3);
    HW::down=0; mapper.poll();
    assert(mapper.state()==TOUCH_IDLE && HW::reads==3);
}
int main() {
    startup(); invalidCs(); contact(); fifoRaw(); mapperSamplesOnce();
    std::puts("touch stmpe610: PASS");
}
