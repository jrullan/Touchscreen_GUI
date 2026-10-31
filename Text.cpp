#include "Text.h"
#include <stdlib.h>
#include <string.h>

Text::Text(unsigned char textSize) : text(NULL), _textSize(textSize) {
    text = (char*)calloc((size_t)textSize + 1, 1);
}

Text::~Text() { free(text); }
char* Text::getText() { return text; }
long Text::getNum() { return text ? strtol(text, NULL, 10) : 0; }

void Text::setText(char* value) { setText((const char*)value); }
void Text::setText(const char* value) {
    if (!text) return;
    size_t length = 0;
    if (value) while (length < _textSize && value[length]) ++length;
    if (length) memmove(text, value, length);
    text[length] = 0;
}

unsigned char Text::getTextSize() {
    size_t length = 0;
    if (text) while (length < _textSize && text[length]) ++length;
    return (unsigned char)length;
}
void Text::clear() { if (text) memset(text, 0, (size_t)_textSize + 1); }
