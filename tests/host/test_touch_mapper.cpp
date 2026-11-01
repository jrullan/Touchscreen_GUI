#include <cassert>
#include <cstdio>
#include <type_traits>
#include "TouchMapper.h"

class FakeTouch : public TouchDriver {
public:
    bool success = false, down = false, filter = false;
    int begins = 0, touches = 0, reads = 0;
    int16_t x = 0, y = 0, z = 7;
    bool begin() override { ++begins; return success; }
    bool touched() override { ++touches; return down; }
    void readRaw(int16_t& rx, int16_t& ry, int16_t& rz) override {
        ++reads; rx = x; ry = y; rz = z;
    }
    bool needsFiltering() override { return filter; }
};
static const TouchCalibration linear{0,239,0,319,false};
static void zero(Point p) { assert(p.x == 0 && p.y == 0 && p.z == 0); }
static void startup() {
    static_assert(!std::is_copy_constructible<TouchDriver>::value, "borrowed hardware is not copied");
    static_assert(!std::is_copy_assignable<TouchDriver>::value, "borrowed hardware is not assigned");
    static_assert(!std::is_copy_constructible<TouchMapper>::value, "gesture owner is not copied");
    TouchMapper absent(nullptr, linear);
    assert(!absent.isReady() && !absent.begin()); absent.poll();
    assert(absent.state() == TOUCH_IDLE && absent.pressDuration() == 0);
    zero(absent.currentPoint()); zero(absent.pressPoint());
    FakeTouch driver;
    {
        TouchMapper mapper(&driver, linear);
        mapper.poll();
        assert(!mapper.isReady() && !mapper.begin()); mapper.poll();
        assert(driver.begins == 1 && driver.touches == 0 && driver.reads == 0);
        driver.success = true;
        assert(mapper.begin() && mapper.isReady());
        assert(mapper.begin() && driver.begins == 2);
        assert(mapper.state() == TOUCH_IDLE); zero(mapper.currentPoint());
    }
    assert(driver.begin()); // Mapper destruction must not destroy the borrowed driver.
}
static void gesture() {
    FakeTouch d; d.success = true;
    TouchMapper m(&d, linear); assert(m.begin());
    m.poll(); assert(m.state() == TOUCH_IDLE && d.reads == 0);
    host_millis = ULONG_MAX - 9;
    d.down = true; d.x = 20; d.y = 30; d.z = 40;
    m.poll(); assert(m.state() == TOUCH_PRESSED);
    assert(m.currentPoint() == Point(20,30,40));
    assert(m.pressPoint() == Point(20,30,40));
    assert(m.pressDuration() == 0);
    host_millis = 5; d.x = 21; m.poll();
    assert(m.state() == TOUCH_HELD && m.pressDuration() == 15);
    assert(m.currentPoint().x == 21 && m.pressPoint().x == 20);
    m.poll(); assert(m.state() == TOUCH_HELD);
    const int reads = d.reads;
    d.down = false; m.poll();
    assert(m.state() == TOUCH_RELEASED && d.reads == reads);
    assert(m.currentPoint().z == 40 && m.pressDuration() == 15);
    // A new press immediately after RELEASED must not be discarded.
    d.down = true; host_millis = 8; d.x = 50; m.poll();
    assert(m.state() == TOUCH_PRESSED && m.pressPoint().x == 50);
    assert(m.pressDuration() == 0);
    d.down = false; host_millis = 10; m.poll();
    assert(m.state() == TOUCH_RELEASED && m.pressDuration() == 2);
    m.poll(); assert(m.state() == TOUCH_IDLE && m.pressDuration() == 0);
}
static void calibrationValidation() {
    FakeTouch d; d.success = true; d.down = true;
    TouchCalibration badX{1,1,0,319,false}, badY{0,239,-7,-7,false};
    TouchMapper m(&d, badX);
    assert(!m.begin()); m.poll();
    assert(d.begins == 0 && d.touches == 0 && d.reads == 0);
    assert(!m.setCalibration(badY) && !m.begin());
    assert(m.setCalibration(linear) && m.begin());
    d.x = 55; d.y = 66; m.poll();
    assert(m.state() == TOUCH_PRESSED);
    assert(!m.setCalibration(badX));
    assert(m.state() == TOUCH_PRESSED && m.currentPoint() == Point(55,66,7));
    assert(!m.setCalibration(badY));
    m.poll(); assert(m.state() == TOUCH_HELD && m.currentPoint().x == 55);
    assert(m.setCalibration(linear) && m.isReady());
    assert(m.state() == TOUCH_IDLE && m.pressDuration() == 0);
    zero(m.currentPoint()); zero(m.pressPoint());
    m.poll(); assert(m.state() == TOUCH_PRESSED && d.begins == 1);
    assert(TouchCal::XPT2046_DEFAULT.rawMinX == 3894);
    assert(TouchCal::XPT2046_DEFAULT.rawMaxX == 261);
    assert(TouchCal::XPT2046_DEFAULT.rawMinY == 3891);
    assert(TouchCal::XPT2046_DEFAULT.rawMaxY == 320);
    assert(TouchCal::XPT2046_DEFAULT.swapXY);
}
static void mapping() {
    FakeTouch d; d.success = true; d.down = true;
    TouchMapper m(&d, linear); assert(m.begin());
    auto expect = [&](TouchCalibration cal, int16_t x, int16_t y, int ex, int ey) {
        assert(m.setCalibration(cal)); d.x = x; d.y = y; m.poll();
        assert(m.currentPoint() == Point(ex,ey,7));
    };
    expect(linear, -32768, -32768, 0, 0);
    expect(linear, 32767, 32767, 239, 319);
    TouchCalibration descending{1000,-1000,2000,-2000,false};
    expect(descending,1000,2000,0,0);
    expect(descending,-1000,-2000,239,319);
    expect(descending,0,0,119,159);
    expect(descending,32767,-32768,0,319);
    TouchCalibration full{-32768,32767,32767,-32768,false};
    expect(full,-32768,32767,0,0);
    expect(full,32767,-32768,239,319);
    expect(full,0,0,119,159);
    TouchCalibration narrow{32766,32767,-32767,-32768,false};
    expect(narrow,-32768,32767,0,0);
    expect(narrow,32767,-32768,239,319);
    expect(TouchCal::XPT2046_DEFAULT,3891,3894,0,0);
    expect(TouchCal::XPT2046_DEFAULT,320,261,239,319);
    expect(TouchCal::XPT2046_DEFAULT,-32768,32767,0,319);
}
static void rotations() {
    FakeTouch d; d.success = true; d.down = true;
    TouchMapper m(&d, linear); assert(m.begin());
    const int raw[4][2] = {{0,0},{239,0},{0,319},{239,319}};
    const int expected[4][4][2] = {
        {{0,0},{239,0},{0,319},{239,319}},
        {{0,239},{0,0},{319,239},{319,0}},
        {{239,319},{0,319},{239,0},{0,0}},
        {{319,0},{319,239},{0,0},{0,239}}
    };
    for (uint8_t r = 0; r < 4; ++r) {
        m.setOrientation(r);
        assert(m.state() == TOUCH_IDLE); zero(m.currentPoint()); zero(m.pressPoint());
        for (int c = 0; c < 4; ++c) {
            d.x = raw[c][0]; d.y = raw[c][1]; m.poll();
            assert(m.currentPoint() == Point(expected[r][c][0],expected[r][c][1],7));
        }
    }
    host_millis = 100; m.setOrientation(5); // modulo four, changing while held resets.
    assert(m.isReady() && m.state() == TOUCH_IDLE && m.pressDuration() == 0);
    d.x = 20; d.y = 30; m.poll();
    assert(m.state() == TOUCH_PRESSED && m.pressPoint() == Point(30,219,7));
    m.poll(); assert(m.state() == TOUCH_HELD);
    m.setOrientation(1); // Same effective orientation does not cancel a gesture.
    assert(m.state() == TOUCH_HELD);
    d.x = -32768; d.y = 32767; m.poll();
    assert(m.currentPoint() == Point(319,239,7));
}
class NoisyTouch : public TouchDriver {
public:
    int reads = 0;
    bool begin() override { return true; }
    bool touched() override { return true; }
    int16_t samples[3][3] = {{230,30,300},{20,300,10},{40,10,100}};
    void readRaw(int16_t& x, int16_t& y, int16_t& z) override {
        const int i = reads++ % 3;
        x = samples[i][0]; y = samples[i][1]; z = samples[i][2];
    }
};
class PartialTouch : public NoisyTouch {
public:
    void readRaw(int16_t& x, int16_t& y, int16_t&) override { ++reads; x = 50; y = 60; }
};
static void filtering() {
    NoisyTouch d;
    assert(d.needsFiltering() && d.touchCount() == 1);
    TouchMapper m(&d, linear); assert(m.begin()); m.poll();
    assert(d.reads == 3 && m.currentPoint() == Point(40,30,100));
    m.poll(); assert(d.reads == 6 && m.state() == TOUCH_HELD);
    // All permutations, ties and signed extremes exercise each median branch.
    const int values[8][3] = {{1,2,3},{1,3,2},{2,1,3},{2,3,1},
                             {3,1,2},{3,2,1},{2,2,2},{-32768,2,32767}};
    for (const auto& v : values) {
        for (int i = 0; i < 3; ++i)
            for (int axis = 0; axis < 3; ++axis) d.samples[i][axis] = v[i];
        m.poll(); assert(m.currentPoint() == Point(2,2,2));
    }
    FakeTouch filtered; filtered.success = filtered.down = true;
    filtered.x = 11; filtered.y = 12; filtered.z = -123;
    TouchMapper once(&filtered, linear); assert(once.begin()); once.poll();
    assert(filtered.reads == 1 && once.currentPoint() == Point(11,12,-123));
    filtered.down = false; assert(filtered.touchCount() == 0);
    PartialTouch partial;
    TouchMapper defined(&partial,linear); assert(defined.begin()); defined.poll();
    assert(defined.currentPoint() == Point(50,60,0));
}
int main() { startup(); gesture(); calibrationValidation(); mapping(); rotations(); filtering(); std::puts("touch_mapper: PASS"); }
