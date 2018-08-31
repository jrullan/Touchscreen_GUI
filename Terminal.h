///////////////////////////////////////////////////////////
//  Terminal.h
//  Implementation of the Terminal Widget
//  Created on:      08-Aug-18 4:00 PM
//  Original author: Jose Rullan
///////////////////////////////////////////////////////////
#ifndef TERMINAL_H
#define TERMINAL_H

#include <Arduino.h>
#include <Canvas.h>
#include <Widget.h>

#define MAX_LINES 5
#define TERMINAL_SCROLL_UP 0
#define TERMINAL_SCROLL_DOWN 1

class Terminal : public Widget {
	public:
		char type = 0x15;
		
		uint8_t direction;
		int lines;
		int lineSpace;
		int fontHeight;
		int verticalBleed=1;
		int horizontalBleed=1;
		int maxCharacters;
		char* linesBuffer[MAX_LINES];
		//char** linesBuffer;
		
		Terminal(int w, int h, uint8_t dir = TERMINAL_SCROLL_DOWN, int fontSize = 1);
		~Terminal();
		void print(char* string);
		void print(const char* string){
			print((char*) string);
		}
		void print(char* string, int num);
		void print(int num);
		void clear();
		void scroll();
		void scrollDown();
		void scrollUp();
		
		//inherited methods:
		bool checkTouch(Point* p){};
		void show();
		void update();		
		
	private:
		void drawFrame();
};
#endif
