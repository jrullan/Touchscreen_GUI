// ============================================================
// Sample_Gauge — Gauge Widget Reference
// ============================================================
// Demonstrates the Gauge (vertical bar) widget:
//   - Vertical fill bar with Y-axis scale
//   - Setpoint, high limit, and low limit markers
//   - Delta rendering (only redraws changed portion)
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Gauge.h>
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

// Gauge(width, height, scaleMin, setpoint, scaleMax)
//   width:    TOTAL widget width including Y-axis scale labels (28px) + border + bar
//             Minimum usable width ~ 35px (28 scale + 4 border + some bar)
//   height:   total widget height
//   scaleMin: minimum scale value
//   setpoint: setpoint marker value
//   scaleMax: maximum scale value
Gauge gauge = Gauge(70, 160, 0, 50, 100);

// ---- Timers ----
Neotimer animTimer = Neotimer(100);
int value = 30;
int increment = 2;

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  canvas.init();
  Guitft* tft = canvas.tft;

  // Title
  tft->setTextColor(WHITE);
  tft->setTextSize(1);
  tft->setCursor(4, 4);
  tft->print("Sample: Gauge Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // GAUGE CONFIGURATION REFERENCE
  // ============================================================

  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     gauge background
  //   fgColor:     fill bar color (changes when value exceeds limits)
  //   borderColor: border and scale text color
  gauge.setColors(BLACK, GREEN, WHITE);

  // setpointColor — color of the setpoint marker on the Y scale
  gauge.setpointColor = YELLOW;

  // hiLimitColor — bar turns this color when value >= hiLimit
  gauge.hiLimitColor = RED;

  // lowLimitColor — bar turns this color when value <= lowLimit
  gauge.lowLimitColor = BLUE;

  // setHiLimit(value, color) — high limit threshold and marker color
  gauge.setHiLimit(80, RED);

  // setLowLimit(value, color) — low limit threshold and marker color
  gauge.setLowLimit(20, BLUE);

  // borderWidth — border thickness (default: 2)
  // The bar area = width - yScaleWidth - 2*borderWidth
  gauge.borderWidth = 2;

  // yScaleWidth — width of the Y-axis scale area (default: 3*FONT_X + 10 = 28px)
  // Contains: 3-digit text labels + tick lines
  // The gauge width parameter is TOTAL width (scale + border + bar).
  // Bar width = width - yScaleWidth - 2*borderWidth
  // For gauge(70,...): bar = 70 - 28 - 4 = 38px

  // setCV(value, show) — set current value
  //   show: if true, immediately redraws (default: true)
  gauge.setCV(value, false);

  // Position is top-left corner of the widget.
  // Scale labels draw at x, bar draws at x + yScaleWidth + borderWidth.
  // Right edge at x + width. For right-aligned: x = 240 - width.
  canvas.add(&gauge, 85, 30);

  // --- Additional Properties (not used above, for reference) ---
  //
  // gauge.yScaleWidth
  //   Read/write. Width of the Y-axis label area in pixels.
  //   Default: 3*FONT_X + 10 = 28. Changing this shifts the bar position.
  //
  // Width layout:
  //   |<-- yScaleWidth -->|<-- border -->|<-- bar -->|<-- border -->|
  //   |     28px          |    2px       |  varies   |    2px       |
  //   |<----------------------- w (total) ----------------------->|

  // Labels
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(4, 200);
  tft->print("Gauge animates 0-100.");
  tft->setCursor(4, 212);
  tft->print("Bar color changes at limits:");
  tft->setCursor(4, 224);
  tft->print("Blue < 20 | Green | Red > 80");
  tft->setCursor(4, 244);
  tft->print("Width = scale + border + bar");
  tft->setCursor(4, 256);
  tft->print("Delta render: only changed");
  tft->setCursor(4, 268);
  tft->print("portion is redrawn.");

  animTimer.start();
}

// ---- Loop ----
void loop() {
  canvas.scan();

  if (animTimer.repeat()) {
    value += increment;
    if (value >= 100 || value <= 0) increment = -increment;

    // setCV() marks dirty; update() uses delta rendering
    // (only fills the strip between old and new value)
    gauge.setCV(value);
  }
}
