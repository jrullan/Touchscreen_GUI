// ============================================================
// Sample_Dial — Dial Widget Reference
// ============================================================
// Demonstrates the Dial (circular gauge) widget:
//   - Needle indicator with configurable range
//   - Tick marks, value display, setpoint and limit lines
//   - Animated sweep using timer
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Dial.h>
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

// Dial(radius, scaleMin, setpoint, scaleMax)
//   radius:   dial radius in pixels (x,y position = center)
//   scaleMin: minimum scale value
//   setpoint: setpoint marker value
//   scaleMax: maximum scale value
Dial dial = Dial(55, 0, 50, 100);

// ---- Timers ----
Neotimer sweepTimer = Neotimer(50);
int value = 0;
int increment = 1;

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  canvas.init();
  Guitft* tft = canvas.tft;

  // Title
  tft->setTextColor(WHITE);
  tft->setTextSize(1);
  tft->setCursor(4, 4);
  tft->print("Sample: Dial Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // DIAL CONFIGURATION REFERENCE
  // ============================================================

  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     dial face fill
  //   fgColor:     needle color (changes to limit colors when value exceeds limits)
  //   borderColor: tick marks and scale text color
  dial.setColors(BLACK, GREEN, WHITE);

  // setpointColor — color of the setpoint marker line on the dial face
  dial.setpointColor = YELLOW;

  // hiLimitColor — needle turns this color when currentValue >= hiLimit
  dial.hiLimitColor = RED;

  // lowLimitColor — needle turns this color when currentValue <= lowLimit
  dial.lowLimitColor = BLUE;

  // setHiLimit(value, color) — set high limit value and its marker color
  // When currentValue >= hiLimit, the needle color changes to hiLimitColor
  dial.setHiLimit(80, RED);

  // setLowLimit(value, color) — set low limit value and its marker color
  // When currentValue <= lowLimit, the needle color changes to lowLimitColor
  dial.setLowLimit(20, BLUE);

  // showTicks — draw tick marks around the dial face (default: true)
  // When false, only min and max ticks are drawn
  dial.showTicks = true;

  // tickDegree — angle between tick marks in degrees (default: 45)
  // Smaller values = more ticks. The dial spans from 315 to 585 degrees.
  dial.tickDegree = 45;

  // tickSize — length of tick marks in pixels (default: 10)
  dial.tickSize = 10;

  // gap — gap between tick marks and needle tip in pixels (default: 5)
  dial.gap = 5;

  // showVal — display the numeric value inside the dial (default: true)
  dial.showVal = true;

  // valFontSize — font size for the value display (default: 2)
  // Use 1 for small dials (radius < 40) to avoid overflow
  dial.valFontSize = 2;

  // setCV(value, show) — set the current value
  //   value: new value (clamped to scaleMin..scaleMax)
  //   show:  if true, immediately redraws (default: true)
  //          pass false during setup to avoid drawing before canvas.add()
  dial.setCV(value, false);

  // Position is the CENTER of the dial circle, not the top-left corner.
  // Place at least radius pixels from all screen edges.
  // Here: center at (120, 100), radius=55 → top edge at y=45, fits below title
  canvas.add(&dial, 120, 100);

  // --- Additional Properties (not used above, for reference) ---
  //
  // dial.setSize(radius);
  //   Changes the dial radius after construction. Sets w=h=2*radius.
  //
  // dial.forceUpdate();
  //   Forces a redraw even if the value hasn't changed.
  //   Useful after changing visual properties.
  //
  // dial.minDegree, dial.maxDegree
  //   Internal degree range (default: 585 to 315).
  //   Normally not changed by the user.

  // Labels
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(90, 160);
  tft->print("VALUE");

  tft->setCursor(4, 190);
  tft->print("Dial sweeps 0-100 automatically.");
  tft->setCursor(4, 202);
  tft->print("Needle changes color at limits.");
  tft->setCursor(4, 214);
  tft->print("Blue < 20 | Green | Red > 80");

  sweepTimer.start();
}

// ---- Loop ----
void loop() {
  canvas.scan();

  if (sweepTimer.repeat()) {
    value += increment;
    if (value >= 100 || value <= 0) increment = -increment;

    // setCV() marks dirty; update() redraws needle and value
    dial.setCV(value);
  }
}
