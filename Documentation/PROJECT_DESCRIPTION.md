# Touchscreen GUI Library

## Overview

Touchscreen GUI (TGUI) is a lightweight, modular C++ GUI framework for embedded systems, designed to run on microcontrollers such as the ESP32, ESP8266, STM32, and Arduino AVR boards paired with ILI9341 TFT displays. It provides a collection of touch-enabled UI widgets for building Human-Machine Interface (HMI) applications in the Arduino ecosystem.

**Version:** 0.1 beta (since 2015)
**License:** MIT
**Target IDE:** Arduino IDE / PlatformIO

## Target Hardware

| Component | Supported Options |
|-----------|-------------------|
| **MCU** | ESP32, ESP8266, STM32F1, Arduino Mega/Due, Teensy, Udoo Neo |
| **Display** | ILI9341 TFT (240x320 / 320x240, 16-bit RGB565 color) |
| **Touch** | XPT2046 (SPI), FT6206 (I2C capacitive), STMPE610 (SPI/I2C resistive), Seeedstudio resistive |

## Architecture

The library follows a three-tier architecture with clear separation between hardware abstraction, widget logic, and application code.

```
+-----------------------------------+
|     Application (Arduino Sketch)  |   User code + event handlers
+-----------------------------------+
|     Widgets + Screens             |   Button, Dial, Slider, etc.
+-----------------------------------+
|     Canvas (Touch + Display)      |   Event dispatch & rendering
+-----------------------------------+
|  Display (ILI9341) + Touch Driver |   Hardware abstraction
+-----------------------------------+
```

### Core Classes

- **Canvas** (abstract) - Central orchestrator. Manages widget collections, handles touch event detection and dispatching, controls display orientation. Subclassed per touch chipset: `Canvas_XPT2046`, `Canvas_FT6206`, `Canvas_STMPE610`, `Canvas_SEEEDTOUCH`.
- **Widget** (abstract) - Base class for all UI elements. Defines the `checkTouch()`, `show()`, and `update()` interface. Provides common attributes (position, size, colors, borders).
- **Screen** - Container for grouping related widgets into pages, enabling multi-screen applications.
- **GUI_TFT** - Wrapper around `Adafruit_ILI9341` providing drawing utilities (text, numbers, lines).
- **Touch** (abstract) - Interface for touch driver implementations with `begin()`, `getPoint()`, `touched()`.
- **Point** - Touch coordinate representation (x, y, z) with orientation-aware transformations.

### Class Hierarchy

```
Canvas (Abstract)
  Canvas_SEEEDTOUCH
  Canvas_FT6206
  Canvas_STMPE610
  Canvas_XPT2046

Widget (Abstract)
  Button
    Numkey
    Buttongrid
    Popup
    IconButton
    OptionButton
  Display
  Terminal
  Indicator (Abstract)
    Dial
    Gauge
    Slider
    Trend

Touch (Abstract)
  SeeedstudioTouch
  Adafruit_FT6206
  Adafruit_STMPE610
  XPT2046_Touchscreen
```

## Widgets

### Input Widgets (touch-interactive)

| Widget | Description |
|--------|-------------|
| **Button** | Rectangular or round button with configurable text, colors, corner radius, and event handler |
| **Numkey** | Numeric keypad for number entry (digits, equals, clear) |
| **Buttongrid** | Configurable grid of buttons (rows x columns) |
| **Popup** | Modal dialog with 1-2 buttons (OK, Yes/No, OK/Cancel) |
| **OptionButton** | Radio-button-style selection |
| **IconButton** | Button rendered with bitmap images |
| **Slider** | Vertical slider for value selection (also an indicator) |

### Output Widgets (display-only)

| Widget | Description |
|--------|-------------|
| **Display** | Text and numeric display area |
| **Terminal** | Scrollable text output (up to 10 lines) |

### Indicator Widgets (value display with thresholds)

| Widget | Description |
|--------|-------------|
| **Dial** | Circular gauge with needle, setpoint, and hi/low limit coloring |
| **Gauge** | Vertical bar indicator with threshold zones |
| **Trend** | Time-series line chart with auto-scaling |
| **Slider** | Dual-purpose: user input and value display |

## Touch Input Pipeline

```
Hardware Touch Event
  -> Touch Driver (chipset-specific)
  -> Canvas::scan()           [called in loop()]
  -> Canvas::getTouchedPoint() [coordinate mapping + debounce]
  -> Canvas::touchWidgets()    [route to widgets]
  -> Widget::checkTouch()      [hit detection]
  -> eventHandler(Widget*)     [user callback]
```

Touch events are debounced (default 200ms), sampled at 25ms intervals, and averaged over a 5-element buffer. Coordinate transformation handles portrait/landscape orientations automatically.

## Usage Pattern

```cpp
#include <Canvas_XPT2046.h>
#include <Button.h>
#include <Display.h>

#define TFT_CS 26
#define TFT_DS 5
#define TS_CS  17

Canvas_XPT2046 canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DS, TS_CS);
Button button = Button(80, 40, GRAY1, WHITE, GRAY);
Display display = Display();

void buttonHandler(Button* btn) {
    display.setText("Pressed!");
}

void setup() {
    button.setText("Press Me");
    button.setEventHandler(&buttonHandler);
    button.init();
    display.init();

    canvas.add(&button, 80, 100);
    canvas.add(&display, 50, 200);
}

void loop() {
    canvas.scan();
}
```

## Project Structure

```
Touchscreen_GUI/
  *.h / *.cpp          Core library source (widgets, canvas, utilities)
  Documentation/
    main.md            Detailed user guide with code examples
    *.pptx             Architecture presentation
  examples/            14 Arduino sketch examples
    TGUI_Sample_*      Individual widget demos
    Touchscreen_GUI_Demo/  Comprehensive multi-screen demo
    maker_faire_pip/       Maker Faire booth display
  images/              Architecture diagrams and screenshots
  Utilities/
    Image2Bitmap.exe   Tool for converting images to bitmap arrays
  keywords.txt         Arduino IDE syntax highlighting definitions
  README.md            Setup and overview
  Todo.txt             Development notes
```

## Examples Included

1. **TGUI_Sample_Button** - Basic button with event handler
2. **TGUI_Sample_Display** - Text display widget
3. **TGUI_Sample_Dial** - Circular gauge indicator
4. **TGUI_Sample_Slider** - Value slider control
5. **TGUI_Sample_Terminal** - Scrollable text output
6. **TGUI_Sample_Terminal_2** - Advanced terminal usage
7. **TGUI_Sample_Numkey** - Numeric keypad entry
8. **TGUI_Sample_Screens** - Multi-page navigation
9. **TGUI_Sample_Button_Terminal** - Button + terminal combination
10. **TGUI_Sample_IconButton** - Image-based buttons
11. **TGUI_Sample_OptionButton** - Radio button option selection
12. **TGUI_Sample_ButtonGrid** - Grid of configurable buttons
13. **Touchscreen_GUI_Demo** - Full demo: multi-screen, navigation, password entry, real-time values
14. **maker_faire_pip** - Maker Faire booth display

## Design Patterns

- **Abstract Factory** - Canvas subclasses encapsulate chipset-specific initialization
- **Strategy** - Interchangeable touch driver implementations behind the Touch interface
- **Observer** - Function-pointer event handlers for user interaction callbacks
- **Composite** - Screens contain collections of widgets
- **Template Method** - Widget base class defines the show/update/checkTouch contract

## Dependencies

- [Adafruit_ILI9341](https://github.com/adafruit/Adafruit_ILI9341) (display driver)
- [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library) (graphics primitives)
- Touch driver library matching your hardware (XPT2046_Touchscreen, Adafruit_FT6206, Adafruit_STMPE610, or SeeedTouchScreen)

## Memory Considerations

The library is designed for constrained environments:
- Pointer-based text storage to minimize per-widget overhead
- Stack-based widget collections (`StackArray<T>`)
- Hierarchical type codes for efficient widget classification
- Platform-specific conditional compilation to reduce binary size
