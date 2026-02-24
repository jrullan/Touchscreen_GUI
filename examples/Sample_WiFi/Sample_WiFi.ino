// ============================================================
// Sample_WiFi — WiFi Connection with Keyboard & Buttongrid
// ============================================================
// Scans for WiFi networks, shows the 3 strongest SSIDs on a
// Buttongrid for selection, then uses the Keyboard widget to
// enter the password. On successful connection, displays the
// IP address and current time via NTP.
//
// Hardware: ESP8266 or ESP32 + ILI9341 TFT + XPT2046 Touch
// ============================================================

#include <ESP8266WiFi.h>
#include <time.h>
#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Keyboard.h>
#include <Buttongrid.h>
#include <Display.h>
#include <Button.h>
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
Buttongrid grid = Buttongrid(3, 1, BUTTONGRID_USELABELS | BUTTONGRID_HIGHLIGHT);
Keyboard keyboard = Keyboard();
Display dispStatus = Display(32);
Display dispIP     = Display(20);
Display dispTime   = Display(12);
Button btnScan     = Button();

// ---- State ----
char ssids[3][33];       // 3 SSIDs, max 32 chars each
int  rssiVals[3];
byte ssidCount = 0;
byte selectedSSID = 0;
bool connecting = false;
bool connected = false;

Neotimer clockTimer = Neotimer(1000);

// ---- Forward declarations ----
void onScan(Button* btn);
void onSSIDSelect(Buttongrid* bg, unsigned char id);
void onPasswordDone(Keyboard* kb);
void scanNetworks();
void updateClock();

// ---- Event Handlers ----

void onScan(Button* btn) {
  scanNetworks();
}

void onSSIDSelect(Buttongrid* bg, unsigned char id) {
  if(id < 1 || id > ssidCount) return;
  selectedSSID = id - 1;

  // Show keyboard for password entry
  char buf[40];
  sprintf(buf, "Password for: %s", ssids[selectedSSID]);
  dispStatus.setText(buf);
  dispStatus.update();

  canvas.add(&keyboard, 0, 140);
  keyboard.show();
}

void onPasswordDone(Keyboard* kb) {
  char* password = kb->getText();

  kb->hide();

  // Start WiFi connection
  char buf[40];
  sprintf(buf, "Connecting to %s...", ssids[selectedSSID]);
  dispStatus.setText(buf);
  dispStatus.update();

  WiFi.begin(ssids[selectedSSID], password);
  connecting = true;
  kb->reset();
}

void scanNetworks() {
  dispStatus.setText("Scanning...");
  dispStatus.update();

  int n = WiFi.scanNetworks();
  if(n == 0) {
    dispStatus.setText("No networks found.");
    dispStatus.update();
    return;
  }

  // Sort by RSSI (strongest first), pick top 3
  // Simple selection of up to 3 strongest
  ssidCount = 0;
  bool used[32];
  memset(used, 0, sizeof(used));
  int limit = (n < 3) ? n : 3;

  for(int pick = 0; pick < limit; pick++) {
    int best = -1;
    int bestRSSI = -999;
    for(int i = 0; i < n && i < 32; i++) {
      if(!used[i] && WiFi.RSSI(i) > bestRSSI) {
        bestRSSI = WiFi.RSSI(i);
        best = i;
      }
    }
    if(best >= 0) {
      used[best] = true;
      strncpy(ssids[pick], WiFi.SSID(best).c_str(), 32);
      ssids[pick][32] = 0;
      rssiVals[pick] = bestRSSI;
      ssidCount++;
    }
  }

  WiFi.scanDelete();

  // Update grid labels with SSID names
  for(byte i = 0; i < 3; i++) {
    if(i < ssidCount) {
      grid.setLabel(i + 1, ssids[i]);
    } else {
      grid.setLabel(i + 1, (char*)"---");
    }
  }
  grid.show();

  char buf[32];
  sprintf(buf, "Found %d networks. Select one:", ssidCount);
  dispStatus.setText(buf);
  dispStatus.update();
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

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  canvas.init();
  Guitft* tft = canvas.tft;

  // Title
  tft->setTextColor(WHITE);
  tft->setTextSize(1);
  tft->setCursor(4, 4);
  tft->print("Sample: WiFi Connection");
  tft->drawFastHLine(0, 14, 240, WHITE);

  // Status display
  dispStatus.setColors(BLACK, CYAN, BLACK);
  dispStatus.fontSize = 1;
  dispStatus.borderWidth = 0;
  dispStatus.setSize(232, 12);
  dispStatus.setText("Tap SCAN to find networks.");
  canvas.add(&dispStatus, 4, 20);

  // Scan button
  btnScan.init();
  btnScan.setColors(BLUE, WHITE, WHITE);
  btnScan.setSize(232, 30);
  btnScan.fontSize = 2;
  btnScan.setText("SCAN");
  btnScan.setEventHandler(&onScan);
  canvas.add(&btnScan, 4, 36);

  // SSID selection grid (3 rows × 1 col)
  grid.init();
  grid.setColors(GRAY2, WHITE, WHITE);
  grid.setSize(232, 90);
  grid.fontSize = 1;
  grid.highlightColor = CYAN;
  grid.setLabel(1, (char*)"---");
  grid.setLabel(2, (char*)"---");
  grid.setLabel(3, (char*)"---");
  grid.setEventHandler(&onSSIDSelect);
  canvas.add(&grid, 4, 72);

  // IP display
  dispIP.setColors(BLACK, GREEN, BLACK);
  dispIP.fontSize = 1;
  dispIP.borderWidth = 0;
  dispIP.setSize(232, 12);
  dispIP.setText("");
  canvas.add(&dispIP, 4, 168);

  // Time display
  dispTime.setColors(BLACK, YELLOW, BLACK);
  dispTime.fontSize = 2;
  dispTime.borderWidth = 1;
  dispTime.setSize(120, 28);
  dispTime.setText("--:--:--");
  canvas.add(&dispTime, 60, 186);

  // Keyboard config (starts hidden)
  keyboard.init();
  keyboard.setColors(GRAY2, WHITE, WHITE);
  keyboard.setSize(240, 180);
  keyboard.fontSize = 1;
  keyboard.setEventHandler(&onPasswordDone);

  // NTP (AST = UTC-4)
  configTime(-4 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  clockTimer.start();
}

// ---- Loop ----
void loop() {
  canvas.scan();

  // Check WiFi connection progress
  if(connecting) {
    if(WiFi.status() == WL_CONNECTED) {
      connecting = false;
      connected = true;

      char buf[40];
      sprintf(buf, "Connected to %s", ssids[selectedSSID]);
      dispStatus.setText(buf);
      dispStatus.update();

      char ipbuf[24];
      sprintf(ipbuf, "IP: %s", WiFi.localIP().toString().c_str());
      dispIP.setText(ipbuf);
      dispIP.update();

      Serial.println(buf);
      Serial.println(ipbuf);
    } else if(WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_NO_SSID_AVAIL) {
      connecting = false;
      dispStatus.setText("Connection failed.");
      dispStatus.update();
    }
  }

  // Update clock when connected
  if(connected && clockTimer.repeat()) {
    updateClock();
  }
}
