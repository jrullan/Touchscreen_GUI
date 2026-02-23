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
