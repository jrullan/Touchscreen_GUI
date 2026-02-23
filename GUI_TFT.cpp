#include "GUI_TFT.h"

Guitft::~Guitft(){
	
}

/*
 * Wrapper for drawing strings on the canvas
 */
void Guitft::drawString(char* string,uint16_t poX, uint16_t poY,uint16_t size,uint16_t fgcolor){
	setCursor(poX,poY);
	setTextSize(size);
	setTextColor(fgcolor);
	print(string);
}

/*
 * Wrapper method for drawing vertical lines
 */
void Guitft::drawVerticalLine(uint16_t poX, uint16_t poY,uint16_t length,uint16_t color){
	drawFastVLine(poX,poY,length,color);
}

/*
 * Wrapper method for drawing horizontal lines
 */
void Guitft::drawHorizontalLine(uint16_t poX, uint16_t poY,uint16_t length,uint16_t color){
	drawFastHLine(poX,poY,length,color);
}

/*
 * Wrapper method for drawing numbers
 */
uint8_t Guitft::drawNumber(long long_num,uint16_t poX, uint16_t poY,uint16_t size,uint16_t fgcolor){
	setCursor(poX,poY);
	setTextSize(size);
	setTextColor(fgcolor);
	return print(long_num);
}

/*
 * Sets the ILI9341 vertical scroll area (command 0x33)
 */
void Guitft::setScrollArea(uint16_t topFixed, uint16_t scrollLines, uint16_t bottomFixed) {
	uint8_t data[6] = {
		(uint8_t)(topFixed >> 8), (uint8_t)(topFixed & 0xFF),
		(uint8_t)(scrollLines >> 8), (uint8_t)(scrollLines & 0xFF),
		(uint8_t)(bottomFixed >> 8), (uint8_t)(bottomFixed & 0xFF)
	};
	sendCommand(0x33, data, 6);
}

/*
 * Sets the ILI9341 vertical scroll start address (command 0x37)
 */
void Guitft::setScrollStart(uint16_t line) {
	uint8_t data[2] = {
		(uint8_t)(line >> 8), (uint8_t)(line & 0xFF)
	};
	sendCommand(0x37, data, 2);
}

// =============================================================
// SPI Fast Path — ESP8266/ESP32 only
// Bypasses Adafruit_SPITFT::writeColor() which has delay(1)
// on ESP8266. Uses SPI.writeBytes() for FIFO-based bulk writes
// (32 pixels per FIFO fill). ~27x faster than Adafruit path.
// =============================================================

#if defined(ESP8266) || defined(ESP32)

#include <SPI.h>

void Guitft::writeColorFast(uint16_t color, uint32_t len) {
	if (!len) return;
	// Pre-fill a 64-byte buffer (32 pixels) and blast via writeBytes.
	// writeBytes uses the hardware FIFO directly — faster than per-pixel write16.
	uint8_t hi = color >> 8, lo = color & 0xFF;
	uint8_t buf[64];
	for (uint8_t i = 0; i < 64; i += 2) {
		buf[i]   = hi;
		buf[i+1] = lo;
	}
	while (len >= 32) {
		hwspi._spi->writeBytes(buf, 64);
		len -= 32;
	}
	if (len) {
		hwspi._spi->writeBytes(buf, len * 2);
	}
}

// Transaction-less overrides (called within startWrite/endWrite context)

void Guitft::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                           uint16_t color) {
	if (w && h) {
		if (w < 0) { x += w + 1; w = -w; }
		if (x < _width) {
			if (h < 0) { y += h + 1; h = -h; }
			if (y < _height) {
				int16_t x2 = x + w - 1;
				if (x2 >= 0) {
					int16_t y2 = y + h - 1;
					if (y2 >= 0) {
						if (x < 0) { x = 0; w = x2 + 1; }
						if (y < 0) { y = 0; h = y2 + 1; }
						if (x2 >= _width)  { w = _width - x; }
						if (y2 >= _height) { h = _height - y; }
						setAddrWindow(x, y, w, h);
						writeColorFast(color, (uint32_t)w * h);
					}
				}
			}
		}
	}
}

void Guitft::writeFastHLine(int16_t x, int16_t y, int16_t w,
                            uint16_t color) {
	if ((y >= 0) && (y < _height) && w) {
		if (w < 0) { x += w + 1; w = -w; }
		if (x < _width) {
			int16_t x2 = x + w - 1;
			if (x2 >= 0) {
				if (x < 0)  { x = 0; w = x2 + 1; }
				if (x2 >= _width) { w = _width - x; }
				setAddrWindow(x, y, w, 1);
				writeColorFast(color, (uint32_t)w);
			}
		}
	}
}

void Guitft::writeFastVLine(int16_t x, int16_t y, int16_t h,
                            uint16_t color) {
	if ((x >= 0) && (x < _width) && h) {
		if (h < 0) { y += h + 1; h = -h; }
		if (y < _height) {
			int16_t y2 = y + h - 1;
			if (y2 >= 0) {
				if (y < 0)  { y = 0; h = y2 + 1; }
				if (y2 >= _height) { h = _height - y; }
				setAddrWindow(x, y, 1, h);
				writeColorFast(color, (uint32_t)h);
			}
		}
	}
}

// Transaction-wrapped overrides (standalone public API calls)

void Guitft::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                      uint16_t color) {
	if (w && h) {
		if (w < 0) { x += w + 1; w = -w; }
		if (x < _width) {
			if (h < 0) { y += h + 1; h = -h; }
			if (y < _height) {
				int16_t x2 = x + w - 1;
				if (x2 >= 0) {
					int16_t y2 = y + h - 1;
					if (y2 >= 0) {
						if (x < 0) { x = 0; w = x2 + 1; }
						if (y < 0) { y = 0; h = y2 + 1; }
						if (x2 >= _width)  { w = _width - x; }
						if (y2 >= _height) { h = _height - y; }
						startWrite();
						setAddrWindow(x, y, w, h);
						writeColorFast(color, (uint32_t)w * h);
						endWrite();
					}
				}
			}
		}
	}
}

void Guitft::drawFastHLine(int16_t x, int16_t y, int16_t w,
                           uint16_t color) {
	if ((y >= 0) && (y < _height) && w) {
		if (w < 0) { x += w + 1; w = -w; }
		if (x < _width) {
			int16_t x2 = x + w - 1;
			if (x2 >= 0) {
				if (x < 0)  { x = 0; w = x2 + 1; }
				if (x2 >= _width) { w = _width - x; }
				startWrite();
				setAddrWindow(x, y, w, 1);
				writeColorFast(color, (uint32_t)w);
				endWrite();
			}
		}
	}
}

void Guitft::drawFastVLine(int16_t x, int16_t y, int16_t h,
                           uint16_t color) {
	if ((x >= 0) && (x < _width) && h) {
		if (h < 0) { y += h + 1; h = -h; }
		if (y < _height) {
			int16_t y2 = y + h - 1;
			if (y2 >= 0) {
				if (y < 0)  { y = 0; h = y2 + 1; }
				if (y2 >= _height) { h = _height - y; }
				startWrite();
				setAddrWindow(x, y, 1, h);
				writeColorFast(color, (uint32_t)h);
				endWrite();
			}
		}
	}
}

#endif // ESP8266 || ESP32
