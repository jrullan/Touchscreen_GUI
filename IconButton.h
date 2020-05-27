///////////////////////////////////////////////////////////
//  IconButton.h
//  Implementation of the Class IconButton
//  Created on:      
///////////////////////////////////////////////////////////


/*
	//Sample image:
	// Image must be loaded as code see Image2Bitmap utility using RGB 565 conversion format (16 bits)
	
	//50x50
	const uint16_t switch_off[] = {
		0x0000, 0x0000, 0x0000, 0x7bef, 0x8410, .... // all pixels data
	};
		

	//Basic initialization:
	// Width and Heigth must match image size
	IconButton lightBulb = IconButton(50,50,lightbulb_off,lightbulb_on);

	// Basic setup...
  lightBulb.setLabel("L.Room");
  lightBulb.setEventHandler([](Button* btn){
    btn->update();
  });
  lightBulb.init();

*/


#if !defined(ICONBUTTON_H)
#define ICONBUTTON_H

#include "Canvas.h"
#include "Button.h"

#if !defined(DEBOUNCE)
#define DEBOUNCE 200
#endif

class IconButton : public Button
{

public:
	IconButton(int _width, int _height, const uint16_t* imageUnpressed, const uint16_t* imagePressed = NULL, char* label = "");
	~IconButton();
	
	void show();
	void update();
	void drawRGBABitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h, uint16_t transparentColor);
	
	const uint16_t* imageUnpressed;
	const uint16_t* imagePressed;
	uint16_t transparentColor = BLACK; // By default 0 == Black is transparent
	bool useTransparency = true;
	
private:

};
#endif
