#include "TouchMapper.h"

namespace {
int16_t median3(int16_t a, int16_t b, int16_t c) {
    if (a > b) { const int16_t t = a; a = b; b = t; }
    if (b > c) { b = c; }
    return a > b ? a : b;
}
// Widen before subtracting: supports opposite signed and descending endpoints.
int mapAxis(int16_t raw, int16_t first, int16_t last, int extent) {
    const int64_t mapped = (int64_t(raw) - int64_t(first)) * int64_t(extent)
                         / (int64_t(last) - int64_t(first));
    return mapped < 0 ? 0 : (mapped > extent ? extent : int(mapped));
}
}

TouchMapper::TouchMapper(TouchDriver* driver, const TouchCalibration& cal)
    : _driver(driver), _cal(cal), _ready(false), _state(TOUCH_IDLE),
      _current(0, 0, 0), _pressStart(0, 0, 0) {}

bool TouchMapper::begin() {
    if (_ready) return true;
    if (!_driver || !valid(_cal)) return false;
    _ready = _driver->begin();
    return _ready;
}
bool TouchMapper::valid(const TouchCalibration& cal) {
    return cal.rawMinX != cal.rawMaxX && cal.rawMinY != cal.rawMaxY;
}
void TouchMapper::resetGesture() {
    _state = TOUCH_IDLE;
    _current = Point(0, 0, 0);
    _pressStart = Point(0, 0, 0);
    _pressStartTime = 0;
}
bool TouchMapper::setCalibration(const TouchCalibration& cal) {
    if (!valid(cal)) return false;
    _cal = cal;
    resetGesture();
    return true;
}
void TouchMapper::setOrientation(uint8_t rotation) {
    rotation %= 4;
    if (_rotation == rotation) return;
    _rotation = rotation;
    resetGesture();
}
void TouchMapper::poll() {
    if (!_ready) return;
    if (_driver->touched()) {
        int16_t x = 0, y = 0, z = 0;
        if (_driver->needsFiltering()) {
            int16_t xs[3] = {}, ys[3] = {}, zs[3] = {};
            for (uint8_t i = 0; i < 3; ++i) _driver->readRaw(xs[i], ys[i], zs[i]);
            x = median3(xs[0], xs[1], xs[2]);
            y = median3(ys[0], ys[1], ys[2]);
            z = median3(zs[0], zs[1], zs[2]);
        } else {
            _driver->readRaw(x, y, z);
        }
        const int px = mapAxis(_cal.swapXY ? y : x, _cal.rawMinX, _cal.rawMaxX, 239);
        const int py = mapAxis(_cal.swapXY ? x : y, _cal.rawMinY, _cal.rawMaxY, 319);
        switch (_rotation) {
            case 1: _current = Point(py, 239 - px, z); break;
            case 2: _current = Point(239 - px, 319 - py, z); break;
            case 3: _current = Point(319 - py, px, z); break;
            default: _current = Point(px, py, z); break;
        }
        if (_state == TOUCH_IDLE || _state == TOUCH_RELEASED) {
            _pressStart = _current;
            _pressStartTime = millis();
            _state = TOUCH_PRESSED;
        } else {
            _state = TOUCH_HELD;
        }
    } else if (_state == TOUCH_PRESSED || _state == TOUCH_HELD) {
        _state = TOUCH_RELEASED;
    } else {
        _state = TOUCH_IDLE;
    }
}
unsigned long TouchMapper::pressDuration() const {
    return _state == TOUCH_IDLE ? 0 : millis() - _pressStartTime;
}
