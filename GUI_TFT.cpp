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
	print(long_num);
}

// Needed to declare in GUI_TFT.h as extern
// and instance it here
// to avoid compiler issues with redeclarations
#if defined(ESP32)
	Guitft Tft = Guitft(5, 26); // Use hardware SPI
#else
	Guitft Tft = Guitft(5, 6); // Use hardware SPI
#endif