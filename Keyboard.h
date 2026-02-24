///////////////////////////////////////////////////////////
//  Keyboard.h
//  QWERTY keyboard widget for text entry
//  Provides lowercase, uppercase, and special character modes
//  with ENTER key event handler
///////////////////////////////////////////////////////////

#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifndef KEYBOARD_BUF_SIZE
#define KEYBOARD_BUF_SIZE 48
#endif

#define KEYBOARD_COLS 10
#define KEYBOARD_ROWS 6  // display row + 4 char rows + bottom row

#include "Canvas.h"
#include "Button.h"

class Keyboard : public Button
{

public:
	Keyboard();
	Keyboard(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor);
	virtual ~Keyboard();

	void init();
	void append(char c);
	void deleteChar();
	void clear();
	char* getText();
	byte getTextSize();
	void drawFrame();

	void setEventHandler(void (*functionPointer)(Keyboard*));
	void show(Button* btn);
	void hide();
	void reset();

	// Overridden methods
	virtual bool checkTouch(Point* p);
	virtual void show();
	virtual void update();

	bool autoremove;
	Button* targetButton;
	bool shiftActive;
	bool specialActive;
	int highlightColor = YELLOW;

private:
	char* text;
	byte textPos;
	void (*eventHandler)(Keyboard*);

	int keyW();
	int keyH();
	void drawKeys();
	void drawDisplayArea();
	char getKeyChar(byte row, byte col);

	static const char keysLower[];
	static const char keysUpper[];
	static const char keysSpecial[];
};

#endif
