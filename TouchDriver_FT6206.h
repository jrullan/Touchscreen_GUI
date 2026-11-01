#ifndef TOUCHDRIVER_FT6206_H
#define TOUCHDRIVER_FT6206_H

#include "TouchDriver.h"
#include "Adafruit_FT6206.h"

// Own the bundled controller by value. Mappers borrow this adapter and must
// be destroyed first; construction performs no I2C access or allocation.
class TouchDriver_FT6206 : public TouchDriver {
public:
    explicit TouchDriver_FT6206(uint8_t threshold = FT62XX_DEFAULT_THRESHOLD)
        : threshold_(threshold), ready_(false) {}
    bool begin() override {
        if (!ready_) ready_ = hardware_.begin(threshold_);
        return ready_;
    }
    // Capacitive controller already reports resolved coordinates, not ADC noise.
    bool needsFiltering() override { return false; }
    bool touched() override { return touchCount() != 0; }
    uint8_t touchCount() override { return ready_ ? hardware_.touched() : 0; }
    void readRaw(int16_t& x, int16_t& y, int16_t& z) override {
        x = y = z = 0;
        if (!ready_) return;
        // Explicit index avoids the bundled default-argument/no-argument overload ambiguity.
        // Bundled coordinates are 12-bit (0..4095), safely representable as int16_t.
        const Point point = hardware_.getPoint(uint8_t(0));
        x = point.x; y = point.y; z = point.z;
    }
private:
    Adafruit_FT6206 hardware_;
    const uint8_t threshold_;
    bool ready_;
};
#endif
