// ============================================================
// Sample_Trend — Trend Widget Reference
// ============================================================
// Demonstrates the Trend (line graph) widget:
//   - Real-time data plotting with ring buffer
//   - Configurable Y/X scales, setpoint and limit lines
//   - Sine wave data feed via timer
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Trend.h>
#include <neotimer.h>

// ---- Pin Configuration ----
#ifdef ESP32
  #define TFT_CS 26
  #define TFT_DS 5
  #define TS_CS  17
#else
  #define TFT_CS 16
  #define TFT_DS 15
  #define TS_CS  0
#endif

// ---- Touch & Canvas ----
TouchDriver_XPT2046 touchDriver(TS_CS);
TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DS, &mapper);

// ---- Widgets ----

// Trend(width, height, scaleMin, setpoint, scaleMax, maxValues)
//   width:     TOTAL widget width (includes Y-axis scale if shown)
//   height:    TOTAL widget height (includes X-axis scale if shown)
//   scaleMin:  minimum Y scale value
//   setpoint:  setpoint marker value
//   scaleMax:  maximum Y scale value
//   maxValues: ring buffer capacity (default: MAX_TREND_VALUES=16 or 32)
Trend trend = Trend(232, 120, 0, 50, 100);

// ---- Timers ----
Neotimer dataTimer = Neotimer(150);
int angle = 0;

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  canvas.init();
  Guitft* tft = canvas.tft;

  // Title
  tft->setTextColor(WHITE);
  tft->setTextSize(1);
  tft->setCursor(4, 4);
  tft->print("Sample: Trend Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // TREND CONFIGURATION REFERENCE
  // ============================================================

  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     plot area background
  //   fgColor:     trend line color
  //   borderColor: border, scale text, and tick marks
  trend.setColors(BLACK, GREEN, WHITE);

  // setpointColor — color of the horizontal setpoint line
  trend.setpointColor = YELLOW;

  // hiLimitColor — color of the high limit horizontal line
  trend.hiLimitColor = RED;

  // lowLimitColor — color of the low limit horizontal line
  trend.lowLimitColor = CYAN;

  // setHiLimit(value, color) — high limit threshold line
  trend.setHiLimit(80, RED);

  // setLowLimit(value, color) — low limit threshold line
  trend.setLowLimit(20, CYAN);

  // borderWidth — border thickness (default: 2)
  trend.borderWidth = 2;

  // showYScale — show/hide the Y-axis scale labels and ticks (default: true)
  // When hidden, the plot area expands to fill the full width.
  // Y scale width = 4*FONT_X + 10 = 34px
  trend.showYScale = true;

  // showXScale — show/hide the X-axis scale labels and ticks (default: true)
  // When hidden, the plot area expands to fill the full height.
  // X scale height = 2*FONT_Y + 10 + borderWidth = 30px
  trend.showXScale = true;

  // setWindow(min, max) — sets the visible data window range
  // Controls how many data points are displayed on the X axis.
  // Default: (0, maxValues-1)
  trend.setWindow(0, trend.maxValues - 1);

  // enableAutoFit — auto-adjust Y scale when values approach limits
  // When true, scaleMin/scaleMax expand to keep data visible
  trend.enableAutoFit = false;

  // forceSquareWaveform — draw step/square waveform instead of lines
  // When true, horizontal then vertical segments. When false, direct lines.
  trend.forceSquareWaveform = false;

  canvas.add(&trend, 4, 22);

  // --- Additional Properties (not used above, for reference) ---
  //
  // trend.addValue(val, updateTrend)
  //   Adds a uint8_t value to the ring buffer and redraws.
  //   val:         0-255 (clamped to scaleMin..scaleMax for display)
  //   updateTrend: if true, calls update() (default: true)
  //
  // trend.getValueAt(i)
  //   Returns the value at index i (0 = most recent).
  //
  // trend.getMin(), trend.getMax()
  //   Returns the minimum/maximum value in the buffer.
  //
  // trend.maxValues
  //   Ring buffer capacity. Set via constructor parameter.
  //   Default: 16 (ESP8266) or 32 (ESP32/STM32)
  //
  // trend.autoFit(scale)
  //   Manually trigger auto-fit. If scale=true, adjusts scaleMin/Max.
  //
  // trend.forceUpdate()
  //   Forces a complete redraw regardless of dirty state.
  //
  // Width layout (showYScale=true):
  //   |<-- yScaleWidth -->|<-- border -->|<-- plot area -->|<-- border -->|
  //   |     34px          |    2px       |    varies       |    2px       |
  //   |<------------------------- w (total) ----------------------------->|

  // Labels
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(4, 155);
  tft->print("Sine wave data plotted live.");
  tft->setCursor(4, 167);
  tft->print("Yellow=setpoint, Red=hiLimit");
  tft->setCursor(4, 179);
  tft->print("Ring buffer: O(1) addValue()");

  dataTimer.start();
}

// ---- Loop ----
void loop() {
  canvas.scan();

  if (dataTimer.repeat()) {
    // Generate sine wave: 50 +/- 45
    float rad = angle * 3.14159 / 180.0;
    uint8_t val = (uint8_t)(50 + 45 * sin(rad));
    angle = (angle + 15) % 360;

    // addValue() pushes into ring buffer and redraws the trend
    trend.addValue(val);
  }
}
