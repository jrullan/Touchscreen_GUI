#include "Terminal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace {
// Match the actual backend font/wrap metrics. Older fixed-cell backends use
// conservative cells; raster regressions exercise the real GFX bounds API.
template<class Driver>
auto terminalBounds(Driver* driver, const char* text, int16_t x, int16_t y,
                    uint8_t, int16_t& bx, int16_t& by, uint16_t& bw, uint16_t& bh, int)
    -> decltype(driver->getTextBounds(text,x,y,&bx,&by,&bw,&bh), void()) {
    driver->getTextBounds(text,x,y,&bx,&by,&bw,&bh);
}
template<class Driver>
void terminalBounds(Driver*, const char* text, int16_t x, int16_t y,
                    uint8_t font, int16_t& bx, int16_t& by, uint16_t& bw, uint16_t& bh, long) {
    bx=x; by=y;
    const unsigned size = font ? font : 1;
    size_t width = strlen(text)*FONT_X*size;
    bw = width>65535 ? 65535 : width; bh=FONT_Y*size;
}
}

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
    if (lines) renderedText = (char*)calloc(lines, (size_t)maxCharacters + 1);
}
Terminal::~Terminal() {
    free(renderedText);
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
    update();
}
void Terminal::drawFrame() {
    if (!canDraw() || borderWidth<=0 || w<=0 || h<=0) return;
    Guitft::ScopedWrite guard(*myCanvas->tft);
    int px=x, py=y, width=w, height=h;
    for (int i=0; i<borderWidth && width>0 && height>0; ++i) {
        myCanvas->tft->drawRect(px++,py++,width,height,borderColor);
        width-=2; height-=2;
    }
}
void Terminal::show() {
    visible = true;
    render(true);
}
Terminal::RenderState Terminal::state() const {
    return {myCanvas,myCanvas->tft,x,y,w,h,borderWidth,bgColor,fgColor,borderColor,
            myCanvas->tft->width(),myCanvas->tft->height(),fontSize,direction,keepColors,
            lineSpace,fontHeight,horizontalBleed,verticalBleed};
}
bool Terminal::matchesStyle(const RenderState& s) const {
    return renderedValid && rendered.canvas==s.canvas && rendered.driver==s.driver &&
        rendered.x==s.x && rendered.y==s.y && rendered.w==s.w && rendered.h==s.h &&
        rendered.borderWidth==s.borderWidth && rendered.bg==s.bg && rendered.fg==s.fg &&
        rendered.border==s.border && rendered.panelW==s.panelW && rendered.panelH==s.panelH &&
        rendered.font==s.font && rendered.direction==s.direction && rendered.keepColors==s.keepColors &&
        rendered.lineSpace==s.lineSpace && rendered.fontHeight==s.fontHeight &&
        rendered.horizontalBleed==s.horizontalBleed && rendered.verticalBleed==s.verticalBleed;
}
void Terminal::update() { render(false); }
void Terminal::render(bool force) {
    if (!visible) restorePending = true;
    if (!canDraw()) return;
    RenderState next = state();
    bool full = force || restorePending || !matchesStyle(next);
    if (lines && !renderedText) {
        // Cache OOM must not touch owned text or discard committed erase bounds.
        renderedText = (char*)calloc(lines, (size_t)maxCharacters + 1);
        full = true;
    }
    const bool sameTarget = renderedValid && rendered.canvas==next.canvas && rendered.driver==next.driver;
    const int lineX=x+borderWidth+horizontalBleed, lineY=y+borderWidth+verticalBleed;
    const int pitch=(fontSize ? fontSize : 1)*FONT_Y+lineSpace;
    const int current = direction == TERMINAL_SCROLL_DOWN ? 0 : (int)lines-1;
    bool dirty[MAX_LINES] = {};
    Footprint bounds[MAX_LINES] = {};
    bool changed=full, overflow=false;
    myCanvas->tft->setTextSize(fontSize);
    for (int i=0; i<lines; ++i) {
        const char* text=linesBuffer[i] ? linesBuffer[i] : "";
        const char* saved=renderedText ? renderedText+i*((size_t)maxCharacters+1) : NULL;
        uint16_t color = i==current || keepColors ? linesColors[i] : fgColor;
        Footprint& b=bounds[i];
        if (*text) terminalBounds(myCanvas->tft,text,(int16_t)(uint16_t)lineX,
            (int16_t)(uint16_t)(lineY+i*pitch),fontSize,b.x,b.y,b.w,b.h,0);
        const Footprint& old=renderedBounds[i];
        dirty[i]=full || !saved || strcmp(saved,text)!=0 ||
            (*text && renderedColors[i]!=color) ||
            b.x!=old.x || b.y!=old.y || b.w!=old.w || b.h!=old.h;
        changed=changed || dirty[i];
        // Wrapped, overlapping, or outlying glyphs need a complete composition:
        // erasing one such row could otherwise remove another row's pixels.
        const int top=lineY+i*pitch;
        auto outside=[&](const Footprint& r) {
            return r.w && r.h && (r.x<x+borderWidth || r.y<top ||
                (int)r.x+r.w>x+w-borderWidth || (int)r.y+r.h>min(top+pitch,y+h-borderWidth));
        };
        overflow=overflow || outside(b) || (sameTarget && outside(old));
    }
    if (!changed) return;
    full=full || overflow;
    if (full) {
        if (sameTarget) {
            // Retain actual old glyph bounds even when the optional text cache
            // could not be allocated. Never draw on a former Canvas/driver.
            for (int i=0; i<MAX_LINES; ++i) {
                const Footprint& b=renderedBounds[i];
                if (b.w && b.h) {
                    Guitft::ScopedWrite guard(*myCanvas->tft);
                    myCanvas->tft->fillRect(b.x,b.y,b.w,b.h,myCanvas->bgColor);
                }
            }
            if ((rendered.x!=x || rendered.y!=y || rendered.w!=w || rendered.h!=h) &&
                rendered.w>0 && rendered.h>0) {
                Guitft::ScopedWrite guard(*myCanvas->tft);
                myCanvas->tft->fillRect(rendered.x,rendered.y,rendered.w,rendered.h,myCanvas->bgColor);
            }
        }
        if (w>0 && h>0) {
            Guitft::ScopedWrite guard(*myCanvas->tft);
            myCanvas->tft->fillRect(x,y,w,h,bgColor);
        }
        drawFrame();
    }
    for (int i=0; i<lines; ++i) {
        if (!full && !dirty[i]) continue;
        uint16_t color = i==current || keepColors ? linesColors[i] : fgColor;
        char* saved = renderedText ? renderedText+i*((size_t)maxCharacters+1) : NULL;
        const char* text=linesBuffer[i] ? linesBuffer[i] : "";
        const int ly=lineY+i*pitch;
        const bool eraseRow = !full && w>2*borderWidth && ly<y+h-borderWidth;
        if (eraseRow || *text) {
            // Display work only, one retained row per transaction. Allocation,
            // bounds preparation, and snapshot copies stay outside the guard.
            Guitft::ScopedWrite guard(*myCanvas->tft);
            if (eraseRow) myCanvas->tft->fillRect(x+borderWidth,ly,w-2*borderWidth,
                                               min(pitch,y+h-borderWidth-ly),bgColor);
            if (*text) myCanvas->tft->drawString(text,lineX,ly,fontSize,color);
        }
        if (saved) strcpy(saved,text);
        renderedColors[i]=color;
        renderedBounds[i]=bounds[i];
    }
    rendered=next;
    renderedValid=true;
    restorePending=false;
}
