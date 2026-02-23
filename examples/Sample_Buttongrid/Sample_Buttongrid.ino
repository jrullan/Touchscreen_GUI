// ============================================================
// Sample_Buttongrid — Buttongrid Widget Reference
// ============================================================
// Demonstrates the Buttongrid (grid of buttons) widget:
//   - Configurable rows x columns grid
//   - Highlight, maintained state, custom labels
//   - Event handler receives button ID
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Buttongrid.h>
#include <Terminal.h>

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

// Buttongrid(rows, columns, configuration)
//   rows:          number of rows
//   columns:       number of columns
//   configuration: bitfield flags (can be OR'd together):
//     BUTTONGRID_HIGHLIGHT  (0b0001) — highlight pressed button
//     BUTTONGRID_MAINTAINED (0b0010) — maintain pressed state (toggle)
//     BUTTONGRID_USELABELS  (0b0100) — display custom text labels
//     BUTTONGRID_NUMLABELS  (0b1000) — display numeric labels (0, 1, 2...)
Buttongrid grid = Buttongrid(3, 4, BUTTONGRID_HIGHLIGHT | BUTTONGRID_MAINTAINED | BUTTONGRID_USELABELS);

Terminal terminal = Terminal(232, 80, TERMINAL_SCROLL_UP, 1);

// ---- Event Handler ----
// Buttongrid event handlers receive:
//   bg:  pointer to the Buttongrid
//   id:  the button ID (0-based, left-to-right, top-to-bottom)
// Use bg->getRow(id) and bg->getColumn(id) to find grid position.

void onGridPress(Buttongrid* bg, unsigned char id) {
  char buf[32];
  sprintf(buf, "Btn %d (R%d,C%d) %s", id,
          bg->getRow(id), bg->getColumn(id),
          bg->states[id] ? "ON" : "OFF");
  terminal.print(buf, bg->states[id] ? GREEN : RED);
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
  tft->print("Sample: Buttongrid Widget");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // ============================================================
  // BUTTONGRID CONFIGURATION REFERENCE
  // ============================================================

  // setColors(bgColor, fgColor, borderColor)
  //   bgColor:     button face color
  //   fgColor:     label text color
  //   borderColor: button border color
  grid.setColors(GRAY2, WHITE, WHITE);

  // setSize(width, height) — total grid dimensions
  // Individual button size = (width / columns, height / rows)
  grid.setSize(232, 140);

  // fontSize — text size for button labels
  grid.fontSize = 1;

  // highlightColor — color used to highlight pressed buttons
  // Only used when BUTTONGRID_HIGHLIGHT flag is set
  grid.highlightColor = YELLOW;

  // hGap — horizontal gap between buttons in pixels (default: 0)
  grid.hGap = 2;

  // vGap — vertical gap between buttons in pixels (default: 0)
  grid.vGap = 2;

  // setLabel(id, text) — set custom label for button at index id
  // Only used when BUTTONGRID_USELABELS flag is set
  // Labels are max 8 characters
  grid.setLabel(0,  (char*)"Do");
  grid.setLabel(1,  (char*)"Re");
  grid.setLabel(2,  (char*)"Mi");
  grid.setLabel(3,  (char*)"Fa");
  grid.setLabel(4,  (char*)"Sol");
  grid.setLabel(5,  (char*)"La");
  grid.setLabel(6,  (char*)"Si");
  grid.setLabel(7,  (char*)"Do2");
  grid.setLabel(8,  (char*)"Re2");
  grid.setLabel(9,  (char*)"Mi2");
  grid.setLabel(10, (char*)"Fa2");
  grid.setLabel(11, (char*)"Sol2");

  // setEventHandler(callback) — called when any button is pressed
  // Callback signature: void handler(Buttongrid* bg, unsigned char id)
  grid.setEventHandler(&onGridPress);

  canvas.add(&grid, 4, 22);

  // --- Terminal for feedback ---
  terminal.setColors(BLACK, GREEN, GRAY2);
  terminal.borderWidth = 1;
  canvas.add(&terminal, 4, 172);

  // --- Additional Properties (for reference) ---
  //
  // grid.rows, grid.columns — grid dimensions (read-only after construction)
  //
  // grid.lastPressed — ID of the last pressed button
  //
  // grid.states[id] — per-button state (0 or 1)
  //   Only meaningful when BUTTONGRID_MAINTAINED flag is set.
  //   Toggles on each press.
  //
  // grid.getId(row, col) — get button ID from row/column
  // grid.getRow(id) — get row from button ID
  // grid.getColumn(id) — get column from button ID
  //
  // grid.maintained — whether to redraw as maintained (1) or momentary (0)
  //
  // Configuration flags:
  //   BUTTONGRID_HIGHLIGHT  — visually highlight pressed button
  //   BUTTONGRID_MAINTAINED — toggle state on each press
  //   BUTTONGRID_USELABELS  — show custom text labels (set via setLabel())
  //   BUTTONGRID_NUMLABELS  — show numeric labels (0, 1, 2, ...)

  terminal.print("Tap grid buttons.", YELLOW);
}

// ---- Loop ----
void loop() {
  canvas.scan();
}
