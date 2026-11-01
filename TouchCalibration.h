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
// Nominal FT6206 panel endpoints, preserving legacy portrait axis directions.
// Runtime calibration remains necessary for different panel wiring/dimensions.
static const TouchCalibration FT6206_DEFAULT = {239, 0, 319, 0, false};
// Legacy STMPE610 experimental raw endpoints, with portrait X reversed.
static const TouchCalibration STMPE610_DEFAULT = {3775, 240, 240, 3700, false};
}
#endif
