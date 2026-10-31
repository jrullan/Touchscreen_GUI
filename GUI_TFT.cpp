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
 * Fast diagonal line: run-length sliced.
 *
 * Adafruit GFX's drawLine() plots a diagonal as one setAddrWindow()+write
 * per pixel (~30us/pixel on ESP8266 at 80MHz). This override collapses
 * consecutive same-row (shallow) or same-column (steep) pixels into single
 * H/V runs, each drawn with ONE setAddrWindow + a block fill. For a shallow
 * line the row changes at most |dy| times, so SPI transactions drop from
 * |dx|+1 pixel-writes to ~|dy|+1 run-writes — a big win on trend widgets.
 *
 * Run boundaries: close each run INCLUSIVE of the turning column at the old
 * Y (so the start/end pixels always render), then resume on the next column.
 * This reproduces a Bresenham-equivalent stair-step pixel set that is
 * 8-connected (no gaps) with identical endpoints and zero overdraw --
 * verified by simulation across hundreds of thousands of random lines.
 */
void Guitft::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                      uint16_t color){
	// Axis-aligned: delegate to the (already fast) H/V line primitives.
	if(x0 == x1){
		drawVerticalLine(x0, y0<y1?y0:y1, (uint16_t)((y0<y1?y1-y0:y0-y1)+1), color);
		return;
	}
	if(y0 == y1){
		drawHorizontalLine(x0<x1?x0:x1, y0, (uint16_t)((x0<x1?x1-x0:x0-x1)+1), color);
		return;
	}

	// Canonicalize to increasing dominant axis (Bresenham numerics);
	// track steepness so we can emit the correct orientation.
	bool steep = (y1-y0 > 0) ? (y1-y0 > (x1>x0?x1-x0:x0-x1))
	                         : (y0-y1 > (x1>x0?x1-x0:x0-x1));
	int16_t sx=x0, sy=y0, ex=x1, ey=y1;
	if(steep){ int16_t t=sx; sx=sy; sy=t; t=ex; ex=ey; ey=t; }
	if(sx > ex){ int16_t t=sx; sx=ex; ex=t; t=sy; sy=ey; ey=t; }

	int16_t dx = ex - sx;
	int16_t dy = ey>=sy ? ey-sy : sy-ey;
	int16_t err = dx/2;
	int8_t  ystep = ey>=sy ? 1 : -1;

	int16_t runStart = sx;
	int16_t runY     = sy;
	int16_t x = sx;

	while(x <= ex){
		int16_t oldY = sy;
		err -= dy;
		if(err < 0){ err += dx; sy += ystep; }
		if(sy != oldY){                 // Y turned: close run INCLUSIVE of x
			if(steep){
				drawVerticalLine(runY, runStart, (uint16_t)(x-runStart+1), color);
			}else{
				drawHorizontalLine(runStart, runY, (uint16_t)(x-runStart+1), color);
			}
			runStart = x + 1;
			runY     = sy;
		}
		x++;
	}
	// final run (always reachable, guarantees end pixel)
	if(steep){
		drawVerticalLine(runY, runStart, (uint16_t)(x-runStart), color);
	}else{
		drawHorizontalLine(runStart, runY, (uint16_t)(x-runStart), color);
	}
}

/*
 * Wrapper method for drawing numbers
 */
uint8_t Guitft::drawNumber(long long_num,uint16_t poX, uint16_t poY,uint16_t size,uint16_t fgcolor){
	setCursor(poX,poY);
	setTextSize(size);
	setTextColor(fgcolor);
	print(long_num);
	return 0;
}

// Needed to declare in GUI_TFT.h as extern
// and instance it here
// to avoid compiler issues with redeclarations
//Guitft Tft = Guitft(15, 33);

/*
#if defined(ESP32)
	//Guitft Tft = Guitft(5, 26); // Use hardware SPI
	Guitft Tft = Guitft(15, 33); // Use hardware SPI
#else
	Guitft Tft = Guitft(5, 6); // Use hardware SPI
#endif
*/
