#ifndef TOUCHMAPPER_H
#define TOUCHMAPPER_H
#include <Arduino.h>
#include "Point.h"
#include "TouchDriver.h"
#include "TouchCalibration.h"

enum TouchState : uint8_t { TOUCH_IDLE, TOUCH_PRESSED, TOUCH_HELD, TOUCH_RELEASED };

// Borrows driver (which must outlive this mapper). No heap allocation or deletion.
class TouchMapper {
public:
    TouchMapper(TouchDriver* driver, const TouchCalibration& cal);
    TouchMapper(const TouchMapper&) = delete;
    TouchMapper& operator=(const TouchMapper&) = delete;
    bool begin();
    void poll();
    bool setCalibration(const TouchCalibration& cal);
    void setOrientation(uint8_t rotation);
    bool isReady() const { return _ready; }
    TouchState state() const { return _state; }
    Point currentPoint() const { return _current; }
    Point pressPoint() const { return _pressStart; }
    unsigned long pressDuration() const;
private:
    TouchDriver* _driver;
    TouchCalibration _cal;
    uint8_t _rotation = 0;
    bool _ready;
    TouchState _state;
    Point _current, _pressStart;
    unsigned long _pressStartTime = 0;
    void resetGesture();
    static bool valid(const TouchCalibration& cal);
};
#endif
