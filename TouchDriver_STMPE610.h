#ifndef TOUCHDRIVER_STMPE610_H
#define TOUCHDRIVER_STMPE610_H

#include "TouchDriver.h"
#include "Adafruit_STMPE610.h"

// Polling hardware-SPI adapter; owns the bundled controller without allocation.
class TouchDriver_STMPE610 : public TouchDriver {
public:
    // CS selects hardware SPI, never I2C. beginAddress is forwarded unchanged
    // to bundled begin(uint8_t i2caddr), which ignores it for SPI.
    // CS must fit the bundled signed pin storage (0..127); begin rejects others.
    explicit TouchDriver_STMPE610(uint8_t cs = STMPE_CS, uint8_t beginAddress = STMPE_ADDR)
        : hardware_(cs), beginAddress_(beginAddress), validCs_(cs <= 127), ready_(false) {}
    bool begin() override {
        // Bundled controller stores CS in int8_t; 255 becomes its I2C sentinel.
        if (!validCs_) return false;
        if (!ready_) ready_ = hardware_.begin(beginAddress_);
        return ready_;
    }
    bool touched() override {
        return ready_ && hardware_.touched() != 0 && !hardware_.bufferEmpty();
    }
    // FIFO records are successive events, not repeated measurements of one event.
    bool needsFiltering() override { return false; }
    void readRaw(int16_t& x, int16_t& y, int16_t& z) override {
        x = y = z = 0;
        if (!ready_ || hardware_.bufferEmpty()) return;
        // Bundled getPoint() drains the entire FIFO and is undefined when empty.
        // readData() pops exactly one 12-bit XY / 8-bit pressure record.
        uint16_t rawX = 0, rawY = 0;
        uint8_t rawZ = 0;
        hardware_.readData(&rawX, &rawY, &rawZ);
        x = static_cast<int16_t>(rawX);
        y = static_cast<int16_t>(rawY);
        z = rawZ;
    }
private:
    Adafruit_STMPE610 hardware_;
    const uint8_t beginAddress_;
    const bool validCs_;
    bool ready_;
};
#endif
