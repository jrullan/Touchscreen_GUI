#ifndef TOUCHCALIBRATION_H
#define TOUCHCALIBRATION_H
#include <Arduino.h>

// Endpoints map to 0 and the last valid portrait pixel, after optional axis swap.
struct TouchCalibration {
    int16_t rawMinX, rawMaxX, rawMinY, rawMaxY;
    bool swapXY;
};
namespace TouchCal {
static const TouchCalibration XPT2046_DEFAULT = {3894, 261, 3891, 320, true};
}
#endif
