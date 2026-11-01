#include <cassert>
#include <cstdio>
#include <cstring>
#include "TouchDriver_XPT2046.h"
#include "TouchMapper.h"

static void irqLifetime() {
    auto* driver = new TouchDriver_XPT2046(3, 4);
    const bool started = driver->begin();
    delete driver;
    // Intentionally exercise any surviving registration: the pre-fix adapter
    // reaches the real bundled ISR's freed object under ASan, not a fake driver.
    if (irqCallback) irqCallback();
    assert(!started);
    assert(SPI.begins == 0 && irqAttachments == 0 && !irqCallback);
    TouchDriver_XPT2046 rejected(5, 6);
    assert(!rejected.begin() && !rejected.begin());
    int16_t x = 1, y = 1, z = 1;
    rejected.readRaw(x, y, z);
    assert(!rejected.touched() && x == 0 && y == 0 && z == 0);
    TouchMapper mapper(&rejected, TouchCal::XPT2046_DEFAULT);
    assert(!mapper.begin());
    mapper.poll();
    assert(!mapper.isReady() && mapper.state() == TOUCH_IDLE);
    // Different polling lifetimes must never publish a global ISR target.
    auto* first = new TouchDriver_XPT2046(3);
    auto* second = new TouchDriver_XPT2046(5, 255);
    assert(first->begin() && first->begin() && second->begin());
    assert(SPI.begins == 2);
    delete first;
    assert(second->touched());
    delete second;
    assert(irqAttachments == 0 && irqDetachments == 0 && !irqCallback);
    assert(SPI.transactions == SPI.ends);
    puts("PASS IRQ rejection before hardware begin; polling lifetimes register no callback");
}
// Instrument adapter entry only; acquisition/filter/cache still run in real XPT source.
class CountedDriver : public TouchDriver_XPT2046 {
public:
    CountedDriver() : TouchDriver_XPT2046(3) {}
    unsigned reads = 0;
    void readRaw(int16_t& x, int16_t& y, int16_t& z) override {
        ++reads;
        TouchDriver_XPT2046::readRaw(x, y, z);
    }
};
static void mapperCorners() {
    CountedDriver driver;
    TouchMapper mapper(&driver, TouchCal::XPT2046_DEFAULT);
    assert(mapper.begin());
    const int raw[4][2] = {{3891,3894},{3891,261},{320,3894},{320,261}};
    const int expected[4][4][2] = {
        {{0,0},{239,0},{0,319},{239,319}},
        {{0,239},{0,0},{319,239},{319,0}},
        {{239,319},{0,319},{239,0},{0,0}},
        {{319,0},{319,239},{0,0},{0,239}}
    };
    for (unsigned rotation = 0; rotation < 4; ++rotation) {
        mapper.setOrientation(rotation);
        for (unsigned corner = 0; corner < 4; ++corner) {
            SPI.point(raw[corner][0], raw[corner][1]);
            host_millis += 3;
            const unsigned transactions = SPI.transactions, reads = driver.reads;
            mapper.poll();
            const Point point = mapper.currentPoint();
            assert(point.x == expected[rotation][corner][0]);
            assert(point.y == expected[rotation][corner][1] && point.z == 4095);
            assert(SPI.transactions == transactions + 1);
            assert(driver.reads == reads + 1); // no median of repeated cached reads
        }
    }
    assert(!driver.needsFiltering());
    assert(SPI.transactions == SPI.ends && !irqCallback && irqAttachments == 0);
    puts("PASS actual bundled XPT: 16 rotated mapper corners, one readRaw per poll");
}
static void cachedFiltering() {
    TouchDriver_XPT2046 driver(3);
    assert(driver.begin());
    SPI.x[0] = 1000; SPI.x[1] = 1004; SPI.x[2] = 3000;
    SPI.y[0] = 2000; SPI.y[1] = 100; SPI.y[2] = 2006;
    int16_t x = 0, y = 0, z = 0;
    assert(driver.touched());
    driver.readRaw(x, y, z);
    assert(x == 1002 && y == 2003 && z == 4095);
    assert(SPI.transactions == 1); // touched acquired; readRaw reused the cache
    SPI.point(320, 261);
    host_millis += 2;
    assert(driver.touched());
    driver.readRaw(x, y, z);
    assert(x == 1002 && y == 2003 && SPI.transactions == 1);
    ++host_millis;
    driver.readRaw(x, y, z);
    assert(x == 320 && y == 261 && z == 4095);
    assert(SPI.transactions == 2 && SPI.ends == 2);
    puts("PASS real best-two average rejects outliers; cache holds <3ms, refreshes at 3ms");
}
int main(int argc, char** argv) {
    assert(argc == 2);
    if (!std::strcmp(argv[1], "irq")) irqLifetime();
    else if (!std::strcmp(argv[1], "corners")) mapperCorners();
    else if (!std::strcmp(argv[1], "cache")) cachedFiltering();
    else return 2;
}
