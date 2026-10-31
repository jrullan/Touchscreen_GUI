///////////////////////////////////////////////////////////
//  Text.h
//  Implementation of the Class Text
//  Created on:      11-Mar-2015 9:12 AM
///////////////////////////////////////////////////////////

#ifndef TEXT_H
#define TEXT_H

#include <Arduino.h>

class Text
{
	public:
		
		Text(unsigned char textSize = 8);
		~Text();
        Text(const Text&) = delete;
        Text& operator=(const Text&) = delete;
		
        // Owned capacity+1 buffer. May be NULL on OOM. Do not free/rebind;
        // direct edits must stay within _textSize and keep a trailing NUL.
        char* text;
		
		//=====Getters and Setters======
		//void setNum(int num); -- Had to be removed, see cpp --
		void setText(char* _text);
		void setText(const char* _text);
		long getNum();	
		char* getText();
		unsigned char getTextSize();
		
		//=====Utilities======
		void clear();
		
        // Allocation capacity (read-only after construction).
        unsigned char _textSize;
};
#endif
