#ifndef TOUCHDRIVER_H
#define TOUCHDRIVER_H
#include <Arduino.h>

// Hardware identity is noncopyable. Mappers borrow this object; it must outlive them.
class TouchDriver {
public:
    TouchDriver() = default;
    virtual ~TouchDriver() = default;
    TouchDriver(const TouchDriver&) = delete;
    TouchDriver& operator=(const TouchDriver&) = delete;
    virtual bool begin() = 0;
    virtual bool touched() = 0;
    virtual void readRaw(int16_t& x, int16_t& y, int16_t& z) = 0;
    virtual bool needsFiltering() { return true; }
    virtual uint8_t touchCount() { return touched() ? 1 : 0; }
};
#endif
