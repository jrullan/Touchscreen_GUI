///////////////////////////////////////////////////////////
//  Display.h
//  Implementation of the Class Display
//  Created on:      11-Mar-2015 9:12 AM
///////////////////////////////////////////////////////////

#ifndef DISPLAY_H
#define DISPLAY_H

#include "Canvas.h"
#include "Widget.h"
#include "Text.h"

class Display : public Widget
{
public:
	Display(unsigned char textLength = 8);
	Display(unsigned int width, unsigned int height, int backgroundColor, int textColor, int borderColor,unsigned char textLength = 8);
	virtual ~Display();

	// Attributes	
	// Owned Text object; do not delete/rebind. May be NULL on OOM.
	Text* contents;		

	// Methods
	void init();
	void drawFrame();
	void drawText(int color);
	void fitToText();
	//void setNum(int num, bool force = false); -- See Text.cpp --
	// force requests immediate render-if-changed, not an unconditional repaint.
	// update(), show(), and forceUpdate() explicitly repair external damage.
	void setText(char* _text, bool force = false);
	void forceUpdate() { update(); }
	void append(char* c);
	
	//new testing out...
	void append(char c);
	
	void deleteChar();
	
	//Overriden methods
	virtual bool checkTouch(Point* p);
	virtual void show();
	virtual void update();

private:
	char charPos;
	// Exact retained-text snapshot; allocation failure disables suppression safely.
	char* renderedText = NULL;
	struct RenderState {
		Canvas* canvas;
		Guitft* driver;
		int x, y, w, h, borderWidth, bg, fg, border;
		unsigned char font, length;
		int16_t textX, textY;
		uint16_t textW, textH;
	} rendered;
	bool renderedValid = false;
	void render(bool force);
	bool matchesRendered() const;
	void rememberRender();
};
#endif // !defined(EA_B5B4E3E3_B9E7_43df_9DCA_EF77F36BA339__INCLUDED_)
