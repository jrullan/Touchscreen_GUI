#include "Text.h"
#include "Widget.h"

Text::Text(unsigned char textSize){
	// Reserve memory for the text, and initialize to empty characters (0)
	_textSize = textSize;
	if(this->text = (char *)malloc(textSize+1)){
		memset(this->text,0,textSize+1);
	}
}

Text::~Text(){
	if(this->text) free(this->text);
}

char* Text::getText(){
  return this->text;
}

long Text::getNum(){
	char size = getTextSize();
	long result = 0;
	for(int i = 0; i<size; i++){
		if(this->text[i] == '.') break;  // Only process integer side
		result = result * 10 + this->text[i]-'0';
	}
	return result;
}


/*
 * This function had to be deprecated because it was originally
 * designed to fill a char[] with the digits of a number, however
 * later the internal text property was modified to be a pointer
 * to an existing char[], instead of the actual array.
 * This is the reason it is crashing when trying to fill the text array
 * (I think)
 */
/*
void Text::setNum(int num){
	clear();
	char numChar[_textSize];
	char chars = 0;
	
	// Get the last digit for each power of ten
	while(num >= 10)
	{
		numChar[chars++] = num%10;
		num /= 10;
	}
	
	numChar[chars++] = num;
	
	for(int j = 0; j < chars; j++)
	{
		this->text[chars-1-j] = '0'+numChar[j];
	}
	
	this->text[chars]=0;
}
*/
void Text::setText(char* _text){
	for(int i=0; i<_textSize;i++){
		this->text[i] = _text[i];
		if(_text[i] == 0) break;
	}
}

unsigned char Text::getTextSize(){
	return Widget::getTextLength(this->text);
}

void Text::clear(){
	unsigned char textSize = getTextSize();
	if(textSize){
		for(int i = textSize-1; i >= 0; i--)
		{
			this->text[i] = 0;
		}
	}
}
