/*
 * Wrapper class for TFT display class
 *
 */
 
#if !defined(GUITFT_H)
#define GUITFT_H

//#include "SPI.h"
#include <Adafruit_GFX_AS.h>    // Core graphics library, with extra fonts.
#include <Adafruit_ILI9341_STM.h> // STM32 DMA Hardware-specific library

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

//Touchscreen values
#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2

class Guitft : public Adafruit_ILI9341_STM{
	public:
	
		Guitft(int8_t TFT_CS, int8_t TFT_DC, int8_t TFT_RST):Adafruit_ILI9341_STM(TFT_CS, TFT_DC, -1){};
		~Guitft();
			
		void fillRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color);
		void drawRectangle(uint16_t poX, uint16_t poY, uint16_t length,uint16_t width,uint16_t color);
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
		
		uint8_t layoutMode;
};



#endif

extern Guitft Tft;
