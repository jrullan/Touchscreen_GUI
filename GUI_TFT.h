/*
 * Wrapper class for TFT display class
 * 
 * This class wraps the Adafruit_ILI9341_STM.h
 * calls using the calls already defined in the Touchscreen_Gui library
 * 
 * Author: José Rullán
 * Date: 2017-01-10
 */
 
#if !defined(GUITFT_H)
#define GUITFT_H

//#include "SPI.h"
#if defined(__STM32F1__)
	#include <Adafruit_GFX_AS.h>    	// Core graphics library, with extra fonts.
	#include <Adafruit_ILI9341_STM.h> 	// STM32 DMA Hardware-specific library
#else
	#include <Adafruit_GFX.h>
	#include <Adafruit_ILI9341.h>
#endif
 
//Basic Colors
#define RED		0xf800
#define GREEN	0x07e0
#define BLUE	0x001f
#define BLACK	0x0000
#define YELLOW	0xffe0
#define WHITE	0xffff

//Other Colors
#define CYAN		0x07ff	
#define BRIGHT_RED	0xf810	
#define GRAY1		0x8410  
#define GRAY2		0x4208  

//Orientation
#define TFT_PORTRAIT 0
#define TFT_LANDSCAPE 1
#define TFT_PORTRAIT2 2
#define TFT_LANDSCAPE2 3

/*
//Font Dimensions (based on the standard 5x7 font included in Adafruit_GFX
#define FONT_SPACE 6
#define FONT_X 6
#define FONT_Y 8
*/

// Inherit from STM version if using an STM mcu
#if defined(__STM32F1__)


class Guitft : public Adafruit_ILI9341_STM{
	public:
	Guitft(int8_t TFT_CS, int8_t TFT_DC):Adafruit_ILI9341_STM(TFT_CS, TFT_DC, -1){};
//Otherwise inherit from standard library
#else


class Guitft : public Adafruit_ILI9341{
	public:
	Guitft(int8_t TFT_CS, int8_t TFT_DC):Adafruit_ILI9341(TFT_CS, TFT_DC, -1){};
#endif	

	~Guitft();

	// Nesting counter for SPI transactions — prevents redundant
	// CS toggles and SPI begin/end when nested draw calls (e.g.
	// fillTriangle inside a startWrite/endWrite block) invoke
	// startWrite/endWrite internally.
	void startWrite(void) {
		if(_nestCount++ == 0) {
#if defined(__STM32F1__)
			Adafruit_ILI9341_STM::startWrite();
#else
			Adafruit_ILI9341::startWrite();
#endif
		}
	}
	void endWrite(void) {
		if(--_nestCount == 0) {
#if defined(__STM32F1__)
			Adafruit_ILI9341_STM::endWrite();
#else
			Adafruit_ILI9341::endWrite();
#endif
		}
	}

	void drawString(char *string,uint16_t poX, uint16_t poY,uint16_t size,uint16_t fgcolor);
	void drawString(const char *string,uint16_t poX, uint16_t poY,uint16_t size,uint16_t fgcolor)
	{
	  drawString((char*)string, poX, poY, size, fgcolor);
	}
	void drawVerticalLine(uint16_t poX, uint16_t poY,uint16_t length,uint16_t color);
	void drawHorizontalLine(uint16_t poX, uint16_t poY,uint16_t length,uint16_t color);
	uint8_t drawNumber(long long_num,uint16_t poX, uint16_t poY,uint16_t size,uint16_t fgcolor);
	uint8_t Get_Digits(uint8_t x){
		return (x < 10 ? 1 :   
			(x < 100 ? 2 :   
			(x < 1000 ? 3 :   
			(x < 10000 ? 4 :   
			(x < 100000 ? 5 :   
			(x < 1000000 ? 6 :   
			(x < 10000000 ? 7 :  
			(x < 100000000 ? 8 :  
			(x < 1000000000 ? 9 :  
			10)))))))));
	}
	
	void setScrollArea(uint16_t topFixed, uint16_t scrollLines, uint16_t bottomFixed);
	void setScrollStart(uint16_t line);

#if defined(ESP8266) || defined(ESP32)
	// SPI fast path — overrides Adafruit_GFX virtual methods to bypass
	// writeColor() which has delay(1) on ESP8266. Uses SPI.writeBytes()
	// for bulk FIFO writes at full SPI clock speed.
	void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
#endif

	uint8_t layoutMode;

private:
	uint8_t _nestCount = 0;
#if defined(ESP8266) || defined(ESP32)
	void writeColorFast(uint16_t color, uint32_t len);
#endif
};
#endif

// Needed to declare here as extern
// and instance it in the GUI_TFT.cpp
// to avoid compiler issues with redeclarations
//extern Guitft Tft;
