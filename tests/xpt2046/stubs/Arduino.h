#pragma once
// Reuse time/GPIO boundary without editing the shared host stub.
#include "../../host/stubs/Arduino.h"
#define FALLING 2
inline void (*irqCallback)() = nullptr;
inline unsigned irqAttachments = 0, irqDetachments = 0;
inline void attachInterrupt(int, void (*callback)(), int) {
    irqCallback = callback;
    ++irqAttachments;
}
inline void detachInterrupt(int) {
    irqCallback = nullptr;
    ++irqDetachments;
}
