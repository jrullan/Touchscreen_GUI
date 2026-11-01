#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "Button.h"
#ifndef KEYBOARD_BUF_SIZE
#define KEYBOARD_BUF_SIZE 48
#endif
#define KEYBOARD_COLS 10
#define KEYBOARD_ROWS 6
static_assert(KEYBOARD_BUF_SIZE >= 1 && KEYBOARD_BUF_SIZE <= 255,
              "KEYBOARD_BUF_SIZE must be 1..255");

class Keyboard : public Button {
public:
    Keyboard();
    Keyboard(unsigned int width, unsigned int height, int backgroundColor,
             int textColor, int borderColor);
    void init();
    void append(char c);
    void deleteChar();
    void clear();
    void reset();
    bool setText(const char* text);
    const char* getText() const { return buffer_; }
    uint8_t getTextSize() const { return length_; }
    enum Result { Editing, Accepted, Cancelled };
    Result result() const { return result_; }
    bool accept();
    void cancel();
    void setEventHandler(void (*handler)(Keyboard*));
    void show(Button* target);
    bool autoremove = false;
    Button* targetButton = nullptr; // Borrowed until accept/cancel; must outlive session.
    void show() override;
    void hide();
    void update() override;
    void drawFrame();
    bool checkTouch(Point* p) override;
    bool shiftActive = false;
    bool specialActive = false;
    int highlightColor = YELLOW;
private:
    enum class Action : uint8_t { None, Character, Backspace, Shift, Special, Space, Cancel, Accept };
    struct Cell { Action action; char character; const char* label; };
    Cell cell(unsigned int row, unsigned int col) const;
    bool layoutValid() const;
    void paintRect(int64_t left, int64_t top, int64_t width, int64_t height, int color);
    void paintCell(unsigned int row, unsigned int col, unsigned int span, const char* label,
                   bool active = false, bool frame = true);
    void paintTextField(bool frame);
    char buffer_[KEYBOARD_BUF_SIZE + 1];
    uint8_t length_;
    Result result_ = Editing;
    bool sessionValid_ = true;
    void (*handler_)(Keyboard*) = nullptr;
    void finish(Result result);
    void restoreBackground(Canvas* canvas);

};
#endif
