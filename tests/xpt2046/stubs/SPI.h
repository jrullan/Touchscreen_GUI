#pragma once
#include <cassert>
#include <cstdint>
#define MSBFIRST 1
#define SPI_MODE0 0
struct SPISettings { SPISettings(int, int, int) {} };
struct SPIProbe {
    unsigned begins = 0, transactions = 0, ends = 0, index = 0;
    int x[3] = {3891,3891,3891}, y[3] = {3894,3894,3894};
    void begin() { ++begins; }
    void beginTransaction(SPISettings) { index = 0; ++transactions; }
    void endTransaction() { assert(index == 9); ++ends; }
    uint8_t transfer(uint8_t command) { assert(command == 0xB1); return 0; }
    uint16_t transfer16(uint16_t command) {
        const uint16_t commands[] = {0xC1,0x91,0x91,0xD1,0x91,0xD1,0x91,0xD0,0};
        const int values[] = {1000,1000,0,x[0],y[0],x[1],y[1],x[2],y[2]};
        assert(index < 9 && command == commands[index]);
        return uint16_t(values[index++] << 3);
    }
    void point(int rawX, int rawY) {
        for (int i = 0; i < 3; ++i) { x[i] = rawX; y[i] = rawY; }
    }
};
inline SPIProbe SPI;
