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

#define MAX_LINES 10
#define TERMINAL_SCROLL_UP 0
#define TERMINAL_SCROLL_DOWN 1

class Terminal : public Widget {
	public:
		char type = 0x15;
		
		uint8_t direction;
		uint8_t keepColors;
		uint8_t lines;
		uint8_t lineSpace;
		uint8_t fontHeight;
		uint8_t verticalBleed=1;
		uint8_t horizontalBleed=1;
		int maxCharacters;
		uint8_t linesIndex = 0;
		uint16_t highlightColor;
		uint16_t linesColors[MAX_LINES];
		char* linesBuffer[MAX_LINES];
		//char** linesBuffer;
		
		Terminal(int w, int h, uint8_t dir = TERMINAL_SCROLL_DOWN, int fontSize = 1);
		~Terminal();
		void print(char* string, uint16_t highColor = NULL);
		void printf(char* string, int num, uint16_t highColor = NULL);
		void print(const char* string, uint16_t highColor = NULL){
			print((char*) string, highColor);
		}
		void print(String str, uint16_t highColor = NULL){
			char buffer[str.length()];
			str.toCharArray(buffer,str.length()+1);
			print(buffer,highColor);
		}
		//void print(char* string, int num,uint16_t highColor = NULL);
		//void print(int num,uint16_t highColor = NULL);
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
