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
        // Owned buffers: do not free/rebind entries. Direct edits must remain
        // NUL-terminated within maxCharacters. lines/maxCharacters are read-only
        // allocation geometry after construction (use a new Terminal to resize).
        char* linesBuffer[MAX_LINES];
		//char** linesBuffer;
		
		Terminal(int w, int h, uint8_t dir = TERMINAL_SCROLL_DOWN, int fontSize = 1);
		~Terminal();
		void print(char* string, uint16_t highColor = 0);
		void printf(char* string, int num, uint16_t highColor = 0);
		void print(const char* string, uint16_t highColor = 0){
			print((char*) string, highColor);
		}
		void print(String str, uint16_t highColor = 0){
            print(str.c_str(), highColor);
		}
		//void print(char* string, int num,uint16_t highColor = 0);
		//void print(int num,uint16_t highColor = 0);
		void clear();
		void scroll();
		void scrollDown();
		void scrollUp();
		
		//inherited methods:
		bool checkTouch(Point*) { return true; }
		void show();
		void update();
        // Explicit repaint after overlays/screens damage the panel.
        void forceUpdate() { render(true); }
		
	private:
        char* renderedText = NULL;
        uint16_t renderedColors[MAX_LINES] = {};
        struct Footprint { int16_t x, y; uint16_t w, h; };
        Footprint renderedBounds[MAX_LINES] = {};
        bool renderedValid = false;
        bool restorePending = false;
        struct RenderState {
            Canvas* canvas;
            Guitft* driver;
            int x, y, w, h, borderWidth, bg, fg, border;
            int panelW, panelH;
            uint8_t font, direction, keepColors, lineSpace, fontHeight, horizontalBleed, verticalBleed;
        } rendered = {};
        void render(bool force);
        RenderState state() const;
        bool matchesStyle(const RenderState& s) const;
		void drawFrame();
        void writeLine(const char* string, int num, bool formatted, uint16_t highColor);
};
#endif
