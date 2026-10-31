#include "Terminal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Terminal::Terminal(int width, int height, uint8_t dir, int size) {
    w = width > 0 ? width : 0;
    h = height > 0 ? height : 0;
    // Keep public byte-sized font geometry representable.
    fontSize = size < 1 ? 1 : (size > 31 ? 31 : size);
    bgColor = BLACK; fgColor = borderColor = GRAY1;
    direction = dir == TERMINAL_SCROLL_UP ? TERMINAL_SCROLL_UP : TERMINAL_SCROLL_DOWN;
    keepColors = true; borderWidth = 1; highlightColor = 0;
    horizontalBleed = fontSize * FONT_X;
    verticalBleed = fontSize * FONT_Y / 2;
    fontHeight = fontSize * FONT_Y;
    lineSpace = fontHeight / 2;
    memset(linesBuffer, 0, sizeof linesBuffer);
    for (int i = 0; i < MAX_LINES; ++i) linesColors[i] = fgColor;
    int usableWidth = w - 2 * borderWidth - 2 * horizontalBleed;
    int usableHeight = h - 2 * borderWidth - 2 * verticalBleed;
    maxCharacters = usableWidth > 0 ? usableWidth / (fontSize * FONT_X) : 0;
    int count = usableHeight > 0 ? usableHeight / (fontHeight + lineSpace) : 0;
    lines = maxCharacters > 0 ? (count > MAX_LINES ? MAX_LINES : count) : 0;
    for (int i = 0; i < lines; ++i) {
        linesBuffer[i] = (char*)calloc((size_t)maxCharacters + 1, 1);
        if (!linesBuffer[i]) {
            // Atomic initialization: an OOM terminal is empty and inert.
            for (int j = 0; j < i; ++j) { free(linesBuffer[j]); linesBuffer[j] = NULL; }
            lines = 0;
            break;
        }
    }
}
Terminal::~Terminal() {
    for (int i = 0; i < MAX_LINES; ++i) free(linesBuffer[i]);
}
void Terminal::print(char* string, uint16_t highColor) {
    writeLine(string, 0, false, highColor);
}
void Terminal::printf(char* string, int num, uint16_t highColor) {
    writeLine(string, num, true, highColor);
}
void Terminal::writeLine(const char* string, int num, bool formatted, uint16_t highColor) {
    if (!lines || maxCharacters <= 0) return;
    if (!string) string = "";
    highlightColor = highColor;
    int index = direction == TERMINAL_SCROLL_DOWN ? 0 : lines - 1;
    if (direction == TERMINAL_SCROLL_UP && linesIndex < lines) index = linesIndex++;
    else scroll();
    char* output = linesBuffer[index];
    if (!output) return;
    linesColors[index] = highColor ? highColor : fgColor;
    char number[3 * sizeof(int) + 3];
    if (formatted) snprintf(number, sizeof number, "%d", num);
    size_t used = 0, capacity = (size_t)maxCharacters;
    while (*string && used < capacity) {
        if (formatted && string[0] == '%' && string[1] == 'd') {
            for (size_t i = 0; number[i] && used < capacity; ++i) output[used++] = number[i];
            string += 2;
        } else output[used++] = *string++;
    }
    output[used] = 0;
    update();
}
void Terminal::scroll() {
    if (!lines) return;
    if (canDraw()) myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,w-2*borderWidth,h-2*borderWidth,bgColor);
    if (direction == TERMINAL_SCROLL_DOWN) scrollDown(); else scrollUp();
}
void Terminal::scrollDown() {
    for (int line = (int)lines - 1; line > 0; --line) {
        if (linesBuffer[line] && linesBuffer[line - 1])
            memmove(linesBuffer[line], linesBuffer[line - 1], (size_t)maxCharacters + 1);
        linesColors[line] = linesColors[line - 1];
    }
}
void Terminal::scrollUp() {
    for (int line = 0; line + 1 < lines; ++line) {
        if (linesBuffer[line] && linesBuffer[line + 1])
            memmove(linesBuffer[line], linesBuffer[line + 1], (size_t)maxCharacters + 1);
        linesColors[line] = linesColors[line + 1];
    }
}
void Terminal::clear() {
    for (int i = 0; i < lines; ++i) {
        if (linesBuffer[i]) linesBuffer[i][0] = 0;
        linesColors[i] = fgColor;
    }
    linesIndex = 0;
    if (canDraw() && w > 2*borderWidth && h > 2*borderWidth)
        myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,w-2*borderWidth,h-2*borderWidth,bgColor);
}
void Terminal::drawFrame() {
    if (!canDraw()) return;
    int px=x, py=y, width=w, height=h;
    for (int i=0; i<borderWidth && width>0 && height>0; ++i) {
        myCanvas->tft->drawRect(px++,py++,width,height,borderColor);
        width-=2; height-=2;
    }
}
void Terminal::show() {
    visible = true;
    if (!canDraw()) return;
    drawFrame();
    if (w > 2*borderWidth && h > 2*borderWidth)
        myCanvas->tft->fillRect(x+borderWidth,y+borderWidth,w-2*borderWidth,h-2*borderWidth,bgColor);
    update();
}
void Terminal::update() {
    if (!canDraw()) return;
    int lineX=x+borderWidth+horizontalBleed, lineY=y+borderWidth+verticalBleed;
    int current = direction == TERMINAL_SCROLL_DOWN ? 0 : (int)lines-1;
    for (int i=0; i<lines; ++i) {
        uint16_t color = i == current || keepColors ? linesColors[i] : fgColor;
        if (linesBuffer[i]) myCanvas->tft->drawString(linesBuffer[i],lineX,
            lineY+i*(fontSize*FONT_Y+lineSpace),fontSize,color);
    }
}
