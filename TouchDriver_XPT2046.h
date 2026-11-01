#ifndef TOUCHDRIVER_XPT2046_H
#define TOUCHDRIVER_XPT2046_H

#include "TouchDriver.h"
#include "XPT2046_Touchscreen.h"

// Own the hardware by value: no allocation, publication of partial startup,
// or leak on retry. The mapper borrows this adapter and must die first.
class TouchDriver_XPT2046 : public TouchDriver {
public:
    explicit TouchDriver_XPT2046(uint8_t cs, uint8_t irq = 255)
        : hardware_(cs), requestedIrq_(irq), ready_(false) {}
    bool begin() override {
        // Bundled IRQ registration outlives its object; this adapter is polling-only.
        // Reject before any hardware startup, while retaining constructor compatibility.
        if (requestedIrq_ != 255) return false;
        if (!ready_) ready_ = hardware_.begin(0);
        return ready_;
    }
    bool touched() override { return ready_ && hardware_.touched(); }
    // Bundled driver already selects the closest pair and caches samples for 3 ms.
    bool needsFiltering() override { return false; }
    void readRaw(int16_t& x, int16_t& y, int16_t& z) override {
        x = y = z = 0;
        if (!ready_) return;
        const Point point = hardware_.getPoint();
        x = point.x; y = point.y; z = point.z;
    }
private:
    XPT2046_Touchscreen hardware_;
    const uint8_t requestedIrq_;
    bool ready_;
};
#endif
