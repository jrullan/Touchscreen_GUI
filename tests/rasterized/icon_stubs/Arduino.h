#pragma once
#include "../stubs/Arduino.h"
#include <cassert>
#include <cstddef>
inline const uint16_t* iconReadBase = nullptr;
inline size_t iconReadCapacity = 0, iconReads = 0;
inline uint16_t iconReadWord(const void* address) {
    const auto* p = static_cast<const uint16_t*>(address);
    if (iconReadBase) {
        assert(p >= iconReadBase && p < iconReadBase + iconReadCapacity);
        ++iconReads;
    }
    return *p;
}
#undef pgm_read_word
#define pgm_read_word(p) iconReadWord(p)
