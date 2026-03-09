// ============================================================
// ExplorerHat_HMI — Explorer Hat Pro Emulator for PiPLC
// ============================================================
// Emulates the Explorer Hat Pro on an ESP8266 touchscreen.
// 4 colored LED indicators reflect PLC output coil states (FC01 read).
// 4 momentary push buttons write input coils ON then auto-release
// after MOMENTARY_MS (FC05 write).
// Gauge reads a process value from PLC input registers (FC04).
//
// Buttons and LEDs use separate coil address ranges:
//   Buttons: Coils 0-3  → PLC O:0/0-3  (HMI writes, PLC reads)
//   LEDs:    Coils 4-7  → PLC O:0/4-7  (PLC writes, HMI reads)
//
// Hardware: Wemos D1 Mini + ILI9341 TFT + XPT2046 Touch
// Wiring:   TFT_CS=D0(16), TFT_DC=D8(15), TS_CS=D3(0)
//
// PLC side:
//   piplc-engine --modbus-server --modbus-port 1502
//   Register mapping configured via I/O Config dialog in PiPLC.
// ============================================================

// WiFi
#include <ESP8266WiFi.h>

// Modbus TCP client (modbus-esp8266 by emelianov)
#include <ModbusIP_ESP8266.h>

// Touchscreen GUI
#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <Gauge.h>
#include <Button.h>
#include <Display.h>

// ---- Pin Configuration ----
// Must be defined AFTER library includes to avoid macro clashes
// with GUI_TFT.h parameter names (same pattern as all other examples).
#define TFT_CS  16   // D0 — TFT chip select
#define TFT_DS  15   // D8 — TFT data/command (DC)
#define TS_CS    0   // D3 — XPT2046 touch chip select

// ---- Project Configuration ----
#include "config.h"

// ---- Color Definitions ----
#ifndef DARKGREY
#define DARKGREY 0x7BEF
#endif

// ---- Touch & Canvas ----
TouchDriver_XPT2046 touchDriver(TS_CS);
TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DS, &mapper);

// ---- Modbus Client ----
ModbusIP mb;
IPAddress plcIP = PLC_IP;

// ---- Widgets ----
Display dispStatus(24);                                              // Status text
Gauge gauge = Gauge(55, 120, GAUGE_MIN, GAUGE_SETPOINT, GAUGE_MAX); // Process value
Button buttons[BTN_COUNT];                                           // Momentary push buttons

// ---- LED Layout ----
#define LED_Y       60    // Center Y for all LED circles
#define LED_RADIUS  18    // LED circle radius
#define LED_SPACING 60    // Center-to-center X spacing
#define LED_X_START 30    // First LED center X

const uint16_t ledOnColors[LED_COUNT] = {BLUE, YELLOW, RED, GREEN};
const char* ledLabels[LED_COUNT] = {"Blue", "Yellow", "Red", "Green"};

// ---- LED State (from PLC coil reads) ----
bool ledState[LED_COUNT] = {false, false, false, false};
bool ledCoilValues[LED_COUNT] = {false, false, false, false};  // Read buffer for LED coils

// ---- Modbus State ----
uint16_t processValue = 0;    // Last read from PLC input register
bool mbConnected = false;     // Modbus TCP connection state

// ---- Connection Health ----
unsigned long lastSuccessTime   = 0;  // millis() of last successful response
int consecutiveFailures         = 0;  // reset on any success
int pollsInFlight               = 0;  // outstanding read transactions
#define MB_RESPONSE_TIMEOUT_MS  15000 // force reconnect if silent this long
#define MAX_FAILURES            5     // force reconnect after N timeouts

// ---- Momentary Button State ----
unsigned long btnPressTime[BTN_COUNT] = {0, 0, 0, 0};
bool btnPending[BTN_COUNT] = {false, false, false, false};

// ---- Timing (millis-based, non-blocking) ----
unsigned long lastPollTime      = 0;
unsigned long lastStatusTime    = 0;
unsigned long lastReconnectTime = 0;

// ============================================================
// LED Drawing
// ============================================================

void drawLED(int index, bool on) {
    int cx = LED_X_START + index * LED_SPACING;
    uint16_t color = on ? ledOnColors[index] : DARKGREY;
    canvas.tft->fillCircle(cx, LED_Y, LED_RADIUS, color);
    canvas.tft->drawCircle(cx, LED_Y, LED_RADIUS, WHITE);
}

// ============================================================
// Modbus Callbacks
// ============================================================

void onMbFailure() {
    if (++consecutiveFailures >= MAX_FAILURES) {
        Serial.printf("MB: %d consecutive failures, disconnecting\n", consecutiveFailures);
        mb.disconnect(plcIP);
        mbConnected = false;
        pollsInFlight = 0;
        consecutiveFailures = 0;
    }
}

bool cbReadLedCoils(Modbus::ResultCode event, uint16_t transactionId, void* data) {
    pollsInFlight--;
    if (event == Modbus::EX_SUCCESS) {
        consecutiveFailures = 0;
        lastSuccessTime = millis();
        for (int i = 0; i < LED_COUNT; i++) {
            if (ledCoilValues[i] != ledState[i]) {
                ledState[i] = ledCoilValues[i];
                drawLED(i, ledState[i]);
            }
        }
    } else {
        Serial.printf("MB LED read FAIL: 0x%02X\n", event);
        onMbFailure();
    }
    return true;
}

bool cbReadIreg(Modbus::ResultCode event, uint16_t transactionId, void* data) {
    pollsInFlight--;
    if (event == Modbus::EX_SUCCESS) {
        consecutiveFailures = 0;
        lastSuccessTime = millis();
        gauge.setCV(processValue);
    } else {
        onMbFailure();
    }
    return true;
}

// ============================================================
// Button Event Handler
// ============================================================

void onButtonPress(Button* btn) {
    for (int i = 0; i < BTN_COUNT; i++) {
        if (btn == &buttons[i]) {
            if (btnPending[i]) break;  // ignore repeat fires while coil is still ON
            Serial.printf("HMI: Button %d pressed\n", i + 1);
            if (mbConnected) {
                mb.writeCoil(plcIP, MB_COIL_BTN_BASE + i, true, nullptr, PLC_UNIT_ID);
                btnPressTime[i] = millis();
                btnPending[i] = true;
            } else {
                Serial.println("HMI: Not connected, write skipped");
            }
            break;
        }
    }
}

// ============================================================
// WiFi & Modbus Connection Management
// ============================================================

bool checkWiFi() {
    return WiFi.status() == WL_CONNECTED;
}

bool connectModbus() {
    if (!checkWiFi()) return false;
    if (mb.isConnected(plcIP)) return true;
    bool ok = mb.connect(plcIP, PLC_PORT);
    if (ok) {
        lastSuccessTime = millis();  // grace period for first response
        consecutiveFailures = 0;
        pollsInFlight = 0;
    }
    return ok;
}

void updateStatus() {
    bool wifiOk = checkWiFi();
    unsigned long now = millis();
    char buf[24];

    if (!wifiOk) {
        snprintf(buf, sizeof(buf), "WiFi: Disconnected");
        dispStatus.setText(buf);
        dispStatus.update();
        mbConnected = false;
        if (now - lastReconnectTime >= RECONNECT_DELAY_MS) {
            lastReconnectTime = now;
            WiFi.reconnect();
            Serial.println("WiFi: Reconnecting...");
        }
        return;
    }

    // WiFi is up — check Modbus
    mbConnected = mb.isConnected(plcIP);

    if (!mbConnected) {
        snprintf(buf, sizeof(buf), "Modbus: Connecting...");
        dispStatus.setText(buf);
        dispStatus.update();
        if (now - lastReconnectTime >= RECONNECT_DELAY_MS) {
            lastReconnectTime = now;
            connectModbus();
            Serial.println("Modbus: Connecting...");
        }
        return;
    }

    // All connected
    snprintf(buf, sizeof(buf), "PLC OK  %s",
             WiFi.localIP().toString().c_str());
    dispStatus.setText(buf);
    dispStatus.update();
}

// ============================================================
// Setup
// ============================================================

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== PiPLC Explorer Hat HMI ===");

    // WiFi (non-blocking)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("WiFi: Connecting to %s\n", WIFI_SSID);

    // Modbus client
    mb.client();

    // Initialize display + touch
    canvas.init();
    Guitft* tft = canvas.tft;

    // ---- Title bar ----
    tft->setTextColor(WHITE);
    tft->setTextSize(1);
    tft->setCursor(4, 4);
    tft->print("PiPLC Explorer Hat HMI");
    tft->drawFastHLine(0, 14, 240, WHITE);

    // ---- Status Display ----
    dispStatus.setColors(BLACK, CYAN, BLACK);
    dispStatus.fontSize = 1;
    dispStatus.borderWidth = 0;
    dispStatus.setSize(232, 12);
    char initMsg[] = "Connecting...";
    dispStatus.setText(initMsg);
    canvas.add(&dispStatus, 4, 18);

    // ---- Draw LED indicators (initial state: OFF) ----
    tft->setTextColor(WHITE);
    tft->setTextSize(1);
    for (int i = 0; i < LED_COUNT; i++) {
        drawLED(i, false);
        // Center label below each LED circle
        int cx = LED_X_START + i * LED_SPACING;
        int labelWidth = strlen(ledLabels[i]) * 6;
        tft->setCursor(cx - labelWidth / 2, LED_Y + LED_RADIUS + 4);
        tft->print(ledLabels[i]);
    }

    // ---- Button colors matching their LED ----
    const uint16_t btnBg[BTN_COUNT] = {BLUE, YELLOW, RED, GREEN};
    const uint16_t btnFg[BTN_COUNT] = {WHITE, BLACK, WHITE, BLACK};

    // ---- Momentary Push Buttons ----
    for (int i = 0; i < BTN_COUNT; i++) {
        buttons[i].init();
        buttons[i].setColors(btnBg[i], btnFg[i], WHITE);
        buttons[i].setSize(52, 50);
        buttons[i].fontSize = 2;
        char label[2];
        label[0] = '1' + i;
        label[1] = '\0';
        buttons[i].setText(label);
        buttons[i].setEventHandler(&onButtonPress);
        buttons[i].setDebounce(300);
        canvas.add(&buttons[i], 6 + i * 58, 110);
    }

    // ---- Gauge (process value from PLC) ----
    gauge.setColors(BLACK, GREEN, WHITE);
    gauge.setpointColor = YELLOW;
    gauge.hiLimitColor = RED;
    gauge.lowLimitColor = BLUE;
    gauge.setHiLimit(80, RED);
    gauge.setLowLimit(20, BLUE);
    gauge.borderWidth = 2;
    gauge.setCV(0, false);
    canvas.add(&gauge, 90, 185);

    // ---- Init timers ----
    lastPollTime = millis();
    lastStatusTime = millis();
    lastReconnectTime = millis();

    Serial.println("Setup complete.");
    Serial.printf("PLC target: %s:%d\n",
                  plcIP.toString().c_str(), PLC_PORT);
    Serial.printf("Button coils: %d-%d  LED coils: %d-%d\n",
                  MB_COIL_BTN_BASE, MB_COIL_BTN_BASE + BTN_COUNT - 1,
                  MB_COIL_LED_BASE, MB_COIL_LED_BASE + LED_COUNT - 1);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
}

// ============================================================
// Main Loop
// ============================================================

void loop() {
    // (1) Touch input — must run every iteration
    canvas.scan();

    // (2) Modbus processing — must run every iteration
    mb.task();

    unsigned long now = millis();

    // (3) Momentary button release — auto-clear coils after MOMENTARY_MS
    for (int i = 0; i < BTN_COUNT; i++) {
        if (btnPending[i] && (now - btnPressTime[i] >= MOMENTARY_MS)) {
            mb.writeCoil(plcIP, MB_COIL_BTN_BASE + i, false, nullptr, PLC_UNIT_ID);
            btnPending[i] = false;
            Serial.printf("HMI: Button %d released (coil OFF)\n", i + 1);
        }
    }

    // (4) Stale connection watchdog — no response for MB_RESPONSE_TIMEOUT_MS
    if (mbConnected && lastSuccessTime > 0 &&
        now - lastSuccessTime > MB_RESPONSE_TIMEOUT_MS) {
        Serial.println("MB: Response timeout, forcing reconnect");
        mb.disconnect(plcIP);
        mbConnected = false;
        pollsInFlight = 0;
        consecutiveFailures = 0;
    }

    // (5) Poll LED coils + process value — only when no reads are in flight
    if (pollsInFlight == 0 && now - lastPollTime >= POLL_INTERVAL_MS) {
        lastPollTime = now;
        if (mbConnected) {
            pollsInFlight = 2;  // two reads issued; each callback decrements
            // Read LED coils — callback updates LED indicators
            mb.readCoil(plcIP, MB_COIL_LED_BASE, ledCoilValues, LED_COUNT,
                        cbReadLedCoils, PLC_UNIT_ID);
            // Read process value — callback updates gauge
            mb.readIreg(plcIP, MB_IR_PROCESS, &processValue, 1,
                        cbReadIreg, PLC_UNIT_ID);
        }
    }

    // (6) Connection status check (every STATUS_INTERVAL_MS)
    if (now - lastStatusTime >= STATUS_INTERVAL_MS) {
        lastStatusTime = now;
        updateStatus();
    }
}
