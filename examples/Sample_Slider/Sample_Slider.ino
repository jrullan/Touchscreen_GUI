// ============================================================
// Sample_Slider — Slider Widget Reference
// ============================================================
// Demonstrates the Slider (interactive vertical bar) widget:
//   - Touch-draggable vertical slider
//   - Event handler reports value changes
//   - Linked Display shows current value
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Slider.h>
#include <Display.h>

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

// Slider() — default constructor
// Defaults: 40x120, scale 0-100, setpoint=50, hiLimit=90, lowLimit=10
Slider slider = Slider();

// Display to show the slider's current value
Display dispValue = Display(8);
Display dispLabel = Display(12);

// ---- Event Handler ----
// Slider event handlers receive a pointer to the Slider.
// Read slider->currentValue to get the current position.

void onSliderChange(Slider* s) {
  char buf[8];
  sprintf(buf, "%d", s->currentValue);
  dispValue.setText(buf);
  dispValue.update();

  Serial.print("Slider: ");
  Serial.println(s->currentValue);
}

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  canvas.init();
  Guitft* tft = canvas.tft;

  // Title
  tft->setTextColor(WHITE);
  tft->setTextSize(1);
  tft->setCursor(4, 4);
  tft->print("Sample: Slider Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // SLIDER CONFIGURATION REFERENCE
  // ============================================================

  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     unfilled area color
  //   fgColor:     filled bar color
  //   borderColor: border color
  slider.setColors(BLACK, BLUE, WHITE);

  // setSize(width, height) — slider dimensions
  // Taller = more precision. Width determines the bar thickness.
  slider.setSize(60, 200);

  // setLimits(scaleMin, setpoint, scaleMax) — value range
  slider.setLimits(0, 50, 100);

  // setHiLimit(value, color) — high limit marker
  slider.setHiLimit(80, RED);

  // setLowLimit(value, color) — low limit marker
  slider.setLowLimit(20, CYAN);

  // setSP(value, color) — setpoint marker
  slider.setSP(50, YELLOW);

  // borderWidth — border thickness (default: 2)
  slider.borderWidth = 2;

  // setCV(value, show) — set initial value
  slider.setCV(50, false);

  // setEventHandler(callback) — called on each touch/drag
  // Callback signature: void handler(Slider* s)
  slider.setEventHandler(&onSliderChange);

  // setDebounce(ms) — minimum time between touch events (default: 200ms)
  // Lower = smoother tracking. Higher = fewer updates.
  slider.setDebounce(50);

  // block — when true, touch stops propagating to widgets below
  slider.block = true;

  canvas.add(&slider, 90, 30);

  // --- Value Display ---
  dispLabel.setColors(BLACK, GRAY1, BLACK);
  dispLabel.fontSize = 1;
  dispLabel.borderWidth = 0;
  dispLabel.setSize(60, 12);
  dispLabel.setText("VALUE:");
  canvas.add(&dispLabel, 90, 238);

  dispValue.setColors(BLACK, GREEN, BLACK);
  dispValue.fontSize = 2;
  dispValue.borderWidth = 0;
  dispValue.setSize(60, 20);
  dispValue.setText("50");
  canvas.add(&dispValue, 90, 254);

  // --- Additional Properties (for reference) ---
  //
  // slider.currentValue — current value (read-only, set via setCV())
  // slider.previousValue — previous value (for change detection)
  // slider.scaleMin, slider.scaleMax — value range
  //
  // The slider maps the touch Y position to scaleMin..scaleMax.
  // Touch at top = scaleMax, touch at bottom = scaleMin.
  //
  // Delta rendering: only the strip between old and new position
  // is redrawn, making updates very fast.

  // Labels
  tft->setTextColor(GRAY1);
  tft->setTextSize(1);
  tft->setCursor(4, 290);
  tft->print("Touch and drag the slider.");
}

// ---- Loop ----
void loop() {
  // canvas.scan() polls touch and calls slider's event handler
  // when the touch falls within the slider's bounds.
  canvas.scan();
}
