// Matrix System Analysis Terminal
// Futuristic control panel with WiFi + NTP clock (AST)
// ESP8266 + ILI9341 + XPT2046

#include <ESP8266WiFi.h>
#include <time.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Display.h>
#include <Dial.h>
#include <Gauge.h>
#include <Trend.h>
#include <Terminal.h>
#include <Button.h>
#include <neotimer.h>

// WiFi
const char* WIFI_SSID = "Linksys03507_Ext";
const char* WIFI_PASS = "4antgxupkm";

// Pins
#define TFT_CS 16
#define TFT_DS 15
#define TS_CS  0

// Matrix RGB565 colors
#define MX_GREEN    0x07E0
#define MX_DGREEN   0x03E0
#define MX_DIMGREEN 0x0280
#define MX_CYAN     0x07FF
#define MX_BG       0x0000

// Touch + Canvas
TouchDriver_XPT2046 touchDriver(TS_CS);
TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, MX_BG, TFT_CS, TFT_DS, &mapper);

// Widgets
Display dispTime    = Display(10);
Display dispStatus  = Display(24);
Dial    dial        = Dial(35, 0, 50, 100);
Gauge   gauge       = Gauge(55, 88, 0, 50, 100);
Trend   trend       = Trend(228, 56, 0, 50, 100);
Terminal terminal   = Terminal(238, 76, TERMINAL_SCROLL_UP, 1);
Button  btnScan     = Button();
Button  btnPurge    = Button();

// Timers
Neotimer clockTimer = Neotimer(1000);
Neotimer dataTimer  = Neotimer(600);
Neotimer termTimer  = Neotimer(2500);
Neotimer trendTimer = Neotimer(100);

// State
uint8_t cpuLoad = 45;
uint8_t memLoad = 30;
bool scanning = false;
bool wifiConnected = false;

// Fake terminal messages
const char* termMsgs[] = {
  "Scanning subnet...",
  "Node 10.0.0.%d active",
  "Firewall rule #%d OK",
  "Packet scan: %d threats",
  "Entropy pool: %d bits",
  "Trace route: %d hops",
  "Cipher: AES-%d",
  "Port %d: FILTERED",
  "Hash verified [OK]",
  "Buffer check [PASS]",
  "Neural sync: %d%%",
  "Quantum state: nominal",
};
const uint8_t MSG_COUNT = 12;

// Forward declarations
void onScan(Button* btn);
void onPurge(Button* btn);

void setup() {
  Serial.begin(115200);

  // WiFi + NTP (AST = UTC-4)
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  configTime(-4 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  // Canvas
  canvas.init();
  Guitft* tft = canvas.tft;

  // ---- Title bar ----
  tft->setTextColor(MX_GREEN);
  tft->setTextSize(1);
  tft->setCursor(4, 3);
  tft->print("SYS ANALYSIS v2.1");
  tft->drawFastHLine(0, 13, 240, MX_DGREEN);

  // ---- Time display (top right) ----
  dispTime.setColors(MX_BG, MX_GREEN, MX_BG);
  dispTime.fontSize = 1;
  dispTime.borderWidth = 0;
  dispTime.setSize(52, 10);
  dispTime.setText("--:--:--");
  canvas.add(&dispTime, 186, 2);

  // ---- Status bar ----
  dispStatus.setColors(MX_BG, MX_DIMGREEN, MX_BG);
  dispStatus.fontSize = 1;
  dispStatus.borderWidth = 0;
  dispStatus.setSize(232, 10);
  dispStatus.setText("CONNECTING...");
  canvas.add(&dispStatus, 4, 16);
  tft->drawFastHLine(0, 28, 240, MX_DGREEN);

  // ---- Dial (CPU load, left side) ----
  dial.setColors(MX_BG, MX_GREEN, MX_DGREEN);
  dial.setpointColor = MX_CYAN;
  dial.hiLimitColor = RED;
  dial.lowLimitColor = MX_DIMGREEN;
  dial.showTicks = true;
  dial.showVal = true;
  dial.valFontSize = 1;
  dial.setCV(cpuLoad, false);
  canvas.add(&dial, 40, 70);

  // ---- Gauge (MEM, right side) ----
  gauge.setColors(MX_BG, MX_GREEN, MX_DGREEN);
  gauge.hiLimitColor = RED;
  gauge.lowLimitColor = MX_DIMGREEN;
  gauge.setCV(memLoad, false);
  canvas.add(&gauge, 185, 32);

  // ---- Labels ----
  tft->setTextColor(MX_DIMGREEN);
  tft->setTextSize(1);
  tft->setCursor(18, 122);
  tft->print("CPU LOAD");
  tft->setCursor(217, 122);
  tft->print("MEM");

  // Decorative center panel (centered between dial and gauge)
  tft->drawRect(92, 48, 72, 58, MX_DGREEN);
  tft->setTextColor(MX_GREEN);
  tft->setCursor(102, 54);
  tft->print("SYSTEM");
  tft->setCursor(99, 66);
  tft->print("METRICS");
  tft->setTextColor(MX_DIMGREEN);
  tft->setCursor(97, 82);
  tft->print("[ACTIVE]");

  tft->drawFastHLine(0, 132, 240, MX_DGREEN);

  // ---- Trend (data stream) ----
  trend.setColors(MX_BG, MX_GREEN, MX_DGREEN);
  trend.setpointColor = MX_CYAN;
  trend.borderWidth = 1;
  trend.showXScale = false;
  canvas.add(&trend, 6, 134);

  tft->drawFastHLine(0, 194, 240, MX_DGREEN);

  // ---- Terminal ----
  terminal.setColors(MX_BG, MX_GREEN, MX_DGREEN);
  terminal.borderWidth = 1;
  canvas.add(&terminal, 1, 197);

  tft->drawFastHLine(0, 286, 240, MX_DGREEN);

  // ---- Buttons ----
  btnScan.setColors(MX_DGREEN, MX_GREEN, MX_GREEN);
  btnScan.setSize(112, 26);
  btnScan.fontSize = 1;
  btnScan.setText("[ SCAN ]");
  btnScan.setEventHandler(&onScan);
  canvas.add(&btnScan, 3, 290);

  btnPurge.setColors(MX_DGREEN, RED, MX_GREEN);
  btnPurge.setSize(112, 26);
  btnPurge.fontSize = 1;
  btnPurge.setText("[ PURGE ]");
  btnPurge.setEventHandler(&onPurge);
  canvas.add(&btnPurge, 125, 290);

  // ---- Boot sequence ----
  terminal.print("MATRIX SYS ANALYSIS v2.1");
  terminal.print("Initializing network...");

  clockTimer.start();
}

void loop() {
  canvas.scan();

  if(clockTimer.repeat()) {
    updateClock();
    checkWifi();
  }

  if(dataTimer.repeat()) {
    int d = random(-5, 6);
    cpuLoad = constrain(cpuLoad + d + (scanning ? 3 : 0), 10, scanning ? 98 : 85);
    dial.setCV(cpuLoad);

    d = random(-3, 4);
    memLoad = constrain(memLoad + d, 15, 88);
    gauge.setCV(memLoad);
  }

  if(trendTimer.repeat()) {
    //uint8_t v = random(1, scanning ? 95 : 20);
    uint8_t v = random(-5, 6);
    trend.addValue(cpuLoad+v);
  }

  if(termTimer.repeat()) {
    printTermMsg();
  }
}

void updateClock() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  if(t->tm_year > 100) {
    char buf[12];
    sprintf(buf, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    dispTime.setText(buf);
    dispTime.update();
  }
}

void checkWifi() {
  bool connected = (WiFi.status() == WL_CONNECTED);
  if(connected && !wifiConnected) {
    wifiConnected = true;
    terminal.print(">> NETWORK ONLINE <<", MX_CYAN);
    char ipbuf[32];
    sprintf(ipbuf, "IP: %s", WiFi.localIP().toString().c_str());
    terminal.print(ipbuf, MX_CYAN);
  }
  char buf[28];
  if(connected) {
    sprintf(buf, "ONLINE | %s", WiFi.localIP().toString().c_str());
    dispStatus.setText(buf);
  } else {
    dispStatus.setText("CONNECTING...");
  }
  dispStatus.update();
}

void printTermMsg() {
  uint8_t idx = random(0, MSG_COUNT);
  const char* msg = termMsgs[idx];
  int val = random(1, 256);

  // Check if message has %d format
  bool hasFmt = false;
  for(const char* p = msg; *p; p++) {
    if(*p == '%' && *(p+1) == 'd') { hasFmt = true; break; }
  }

  uint16_t hl = (random(0, 4) == 0) ? MX_CYAN : 0;
  if(hasFmt)
    terminal.printf((char*)msg, val, hl);
  else
    terminal.print((char*)msg, hl);
}

void onScan(Button* btn) {
  scanning = !scanning;
  if(scanning) {
    btn->setColors(MX_GREEN, MX_BG, MX_GREEN);
    btn->setText("[ ACTIVE]");
    terminal.print(">> DEEP SCAN STARTED <<", RED);
    termTimer.set(800);
  } else {
    btn->setColors(MX_DGREEN, MX_GREEN, MX_GREEN);
    btn->setText("[ SCAN ]");
    terminal.print(">> SCAN HALTED <<", MX_CYAN);
    termTimer.set(2500);
  }
  btn->show();
}

void onPurge(Button* btn) {
  terminal.print(">> PURGING BUFFERS <<", RED);
  cpuLoad = 20;
  memLoad = 15;
  dial.setCV(cpuLoad);
  gauge.setCV(memLoad);
  terminal.print(">> SYSTEM NOMINAL <<", MX_CYAN);
}
