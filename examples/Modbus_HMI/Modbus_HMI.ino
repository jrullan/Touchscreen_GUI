// ============================================================
// Modbus_HMI — Remote Touchscreen HMI for PiPLC
// ============================================================
// Connects an ESP8266 + ILI9341 + XPT2046 touchscreen to a
// PiPLC softPLC over Modbus TCP / WiFi.
//
// Features:
//   - START / STOP buttons write commands to PLC holding registers
//   - Gauge reads and displays a process value from PLC input registers
//   - Connection status display with auto-reconnect
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

// ---- Touch & Canvas ----
TouchDriver_XPT2046 touchDriver(TS_CS);
TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BLACK, TFT_CS, TFT_DS, &mapper);

// ---- Modbus Client ----
ModbusIP mb;
IPAddress plcIP = PLC_IP;

// ---- Widgets ----
Display dispStatus(24);                                              // Status text (24-char buffer)
Gauge gauge = Gauge(70, 160, GAUGE_MIN, GAUGE_SETPOINT, GAUGE_MAX); // Process value
Button btnStart = Button();                                          // START command
Button btnStop  = Button();                                          // STOP command

// ---- Modbus State ----
uint16_t processValue = 0;    // Last read from PLC input register
bool mbConnected = false;     // Modbus TCP connection state

// Modbus transaction callback — debug read results
bool cbReadIreg(Modbus::ResultCode event, uint16_t transactionId, void* data) {
    if (event == Modbus::EX_SUCCESS) {
        Serial.printf("MB read OK: PV=%d\n", processValue);
    } else {
        Serial.printf("MB read FAIL: 0x%02X\n", event);
    }
    return true;
}

// ---- Timing (millis-based, non-blocking) ----
unsigned long lastPollTime      = 0;
unsigned long lastStatusTime    = 0;
unsigned long lastReconnectTime = 0;

// ============================================================
// Button Event Handlers
// ============================================================

void onStart(Button* btn) {
    Serial.println("HMI: START pressed");
    // Visual feedback on status display
    char msg[] = "CMD: START";
    dispStatus.setText(msg);
    dispStatus.update();
    if (mbConnected) {
        mb.writeHreg(plcIP, MB_HR_COMMAND, (uint16_t)CMD_START, nullptr, PLC_UNIT_ID);
        Serial.println("HMI: START sent to PLC");
    } else {
        Serial.println("HMI: Not connected, write skipped");
    }
}

void onStop(Button* btn) {
    Serial.println("HMI: STOP pressed");
    // Visual feedback on status display
    char msg[] = "CMD: STOP";
    dispStatus.setText(msg);
    dispStatus.update();
    if (mbConnected) {
        mb.writeHreg(plcIP, MB_HR_COMMAND, (uint16_t)CMD_STOP, nullptr, PLC_UNIT_ID);
        Serial.println("HMI: STOP sent to PLC");
    } else {
        Serial.println("HMI: Not connected, write skipped");
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
    return mb.connect(plcIP, PLC_PORT);
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
    Serial.println("\n=== PiPLC Modbus HMI ===");

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
    tft->print("PiPLC Modbus HMI");
    tft->drawFastHLine(0, 14, 240, WHITE);

    // ---- Status Display ----
    dispStatus.setColors(BLACK, CYAN, BLACK);
    dispStatus.fontSize = 1;
    dispStatus.borderWidth = 0;
    dispStatus.setSize(232, 12);
    char initMsg[] = "Connecting...";
    dispStatus.setText(initMsg);
    canvas.add(&dispStatus, 4, 18);

    // ---- Gauge (process value from PLC) ----
    gauge.setColors(BLACK, GREEN, WHITE);
    gauge.setpointColor = YELLOW;
    gauge.hiLimitColor = RED;
    gauge.lowLimitColor = BLUE;
    gauge.setHiLimit(80, RED);
    gauge.setLowLimit(20, BLUE);
    gauge.borderWidth = 2;
    gauge.setCV(0, false);
    canvas.add(&gauge, 85, 36);

    // ---- START Button ----
    btnStart.init();
    btnStart.setColors(GREEN, BLACK, WHITE);
    btnStart.setSize(110, 50);
    btnStart.fontSize = 2;
    btnStart.setText("START");
    btnStart.setEventHandler(&onStart);
    btnStart.setDebounce(300);
    canvas.add(&btnStart, 4, 210);

    // ---- STOP Button ----
    btnStop.init();
    btnStop.setColors(RED, WHITE, WHITE);
    btnStop.setSize(110, 50);
    btnStop.fontSize = 2;
    btnStop.setText("STOP");
    btnStop.setEventHandler(&onStop);
    btnStop.setDebounce(300);
    canvas.add(&btnStop, 126, 210);

    // ---- Init timers ----
    lastPollTime = millis();
    lastStatusTime = millis();
    lastReconnectTime = millis();

    Serial.println("Setup complete.");
    Serial.printf("PLC target: %s:%d\n",
                  plcIP.toString().c_str(), PLC_PORT);
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

    // (3) Poll process value from PLC (every POLL_INTERVAL_MS)
    if (now - lastPollTime >= POLL_INTERVAL_MS) {
        lastPollTime = now;
        if (mbConnected) {
            // Async read: queues request — mb.task() fills processValue on response
            mb.readIreg(plcIP, MB_IR_PROCESS, &processValue, 1, cbReadIreg, PLC_UNIT_ID);
            Serial.printf("PV read: %d\n", processValue);
        }
        // Update gauge with latest value (may be from previous cycle's response)
        gauge.setCV(processValue);
    }

    // (4) Connection status check (every STATUS_INTERVAL_MS)
    if (now - lastStatusTime >= STATUS_INTERVAL_MS) {
        lastStatusTime = now;
        updateStatus();
    }
}
