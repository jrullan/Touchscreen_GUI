// TFT display only — Adafruit + Guitft, no Touchscreen_GUI widgets
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <GUI_TFT.h>

#define TFT_CS 16
#define TFT_DS 15

Guitft tft = Guitft(TFT_CS, TFT_DS);

void setup() {
  Serial.begin(115200);
  tft.begin();
}
void loop() {}
