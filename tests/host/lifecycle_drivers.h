#pragma once
// Replace hardware-only Touch subclasses, not Canvas/core implementations.
#include "Touch.h"
#define _XPT2046_Touchscreen_h_
#define ADAFRUIT_FT6206_LIBRARY
#define _ADAFRUIT_STMPE610H_
#define SEEEDSTUDIOTOUCH_H
#define FT62XX_DEFAULT_THRESHOLD 128
#define STMPE_CS 16
#define STMPE_ADDR 0x41
#define STMPE_MINX 240
#define STMPE_MAXX 3775
#define STMPE_MINY 240
#define STMPE_MAXY 3700
#define TS_MINX 232
#define TS_MAXX 1780
#define TS_MINY 166
#define TS_MAXY 1826
class LifecycleDriver : public Touch {
public:
    static int live;
    static int constructed;
    static int pointCalls;
    static int touchCalls;
    static bool beginResult;
    static LifecycleDriver* latest;
    int begins = 0;
    LifecycleDriver() { ++live; ++constructed; latest = this; }
    ~LifecycleDriver() override { if (latest == this) latest = nullptr; --live; }
    bool begin(uint8_t) override { ++begins; return beginResult; }
    Point getPoint() override { ++pointCalls; return Point(); }
    uint8_t touched() override { ++touchCalls; return 0; }
};
inline int LifecycleDriver::live = 0;
inline int LifecycleDriver::constructed = 0;
inline int LifecycleDriver::pointCalls = 0;
inline int LifecycleDriver::touchCalls = 0;
inline bool LifecycleDriver::beginResult = true;
inline LifecycleDriver* LifecycleDriver::latest = nullptr;
class XPT2046_Touchscreen : public LifecycleDriver { public: explicit XPT2046_Touchscreen(int) {} };
class Adafruit_FT6206 : public LifecycleDriver {};
class Adafruit_STMPE610 : public LifecycleDriver { public: explicit Adafruit_STMPE610(int) {} };
class SeeedstudioTouch : public LifecycleDriver { public: SeeedstudioTouch(int,int,int,int) {} };
#include "Canvas_XPT2046.h"
#include "Canvas_FT6206.h"
#include "Canvas_STMPE610.h"
#include "Canvas_SEEEDTOUCH.h"
