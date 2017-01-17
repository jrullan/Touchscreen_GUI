///////////////////////////////////////////////////////////
//  Widget.h
//  Implementation of the Class Widget
//  Created on:      02-Mar-2015 9:17:00 PM
//  Original author: Invision
///////////////////////////////////////////////////////////

#if !defined(EA_B6B323D4_C08E_4473_BE8D_AEA9F30B3DA6__INCLUDED_)
#define EA_B6B323D4_C08E_4473_BE8D_AEA9F30B3DA6__INCLUDED_

#ifndef WIDGET_H
#define WIDGET_H

#ifndef FONT_SPACE
#define FONT_SPACE 6
#endif
#ifndef FONT_X
#define FONT_X 8
#endif
#ifndef FONT_Y
#define FONT_Y 8
#endif

#define DISPLAY_SIZE 6

// Forward declarations
class Canvas;
class Point;

/**
 * A widget represents a button, a field box, a drop-down list, a list, etc. Any
 * GUI element that will be part of the GUI. There are input widgets and display
 * widgets. Input widgets are buttons, entry box, selection lists (dropdown and
 * normal). An output widget could be a text display, numeric display, color-
 * changing button or indicator.
 */
class Widget
{

public:
	Widget();
	virtual ~Widget();
	
	char type = 0x00;
	/*
	 * Widgets types:
	 * 
	 * Widget - 		0x00
	 * 	Display - 		0x10
	 * 	Indicator - 	0x20
	 * 		Dial -		0x21
	 * 		Gauge -		0x22
	 * 		Trend -		0x23
	 * 	Button -		0x30
	 * 		Buttongrid-	0x31
	 * 		Numkey -	0x32
	 * 		Popup - 	0x33
	 * 		Option - 	0x34
	 */ 
	int bgColor;
	int borderColor;
	int borderWidth;
	int fgColor;
	int h;
	int w;
	int x;
	int y;
	bool visible = true;
	bool isButton = false;
	unsigned char fontSize = 2;

	//template <typename T> void update(T* c);
	void hide();
	void setCanvas(Canvas* c);
	void setColors(int bgColor,int fgColor, int borderColor);
	void setSize(int width, int height);
	static char* getType(Widget* w);
	
	//Pure Virtual methods
	virtual bool checkTouch(Point* p)=0;
	//virtual bool isButton() =0;
	virtual void show() =0;
	virtual void update() =0;
	
protected:
	Canvas* myCanvas;
	
};
#endif // WIDGET_H
#endif // !defined(EA_B6B323D4_C08E_4473_BE8D_AEA9F30B3DA6__INCLUDED_)
