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
		
		unsigned char _textSize;
};
#endif
