// ============================================================
// config.h — Explorer Hat Pro Emulator Configuration
// ============================================================
// Edit this file to match your deployment:
//   - WiFi credentials
//   - PLC IP address and Modbus port
//   - Coil addresses for LEDs and buttons
// ============================================================

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// WiFi Configuration
// ============================================================
#define WIFI_SSID     "Linksys03507"
#define WIFI_PASSWORD "4antgxupkm"

// ============================================================
// PLC Modbus Server Configuration
// ============================================================
// IP address of the Raspberry Pi running piplc-engine
// Start piplc-engine with:
//   piplc-engine --modbus-server --modbus-port 1502
// Register mapping is configured via the I/O Config dialog in PiPLC.
#define PLC_IP        IPAddress(192, 168, 1, 120)
#define PLC_PORT      1502
#define PLC_UNIT_ID   1      // Modbus unit/slave ID (server default is 1)

// ============================================================
// Coil Addresses — Explorer Hat Pro Emulation
// ============================================================
// Buttons and LEDs use separate coil address ranges.
// Default I/O Config maps Coils addr 0-15 → PLC O:0/0-15.
//
// Buttons (HMI → PLC): Momentary pulse via FC05 (Write Single Coil)
//   Coil 0 → O:0/0 — Button 1 (Blue)
//   Coil 1 → O:0/1 — Button 2 (Yellow)
//   Coil 2 → O:0/2 — Button 3 (Red)
//   Coil 3 → O:0/3 — Button 4 (Green)
#define MB_COIL_BTN_BASE  0    // First button coil address
#define BTN_COUNT         4    // Number of push buttons
#define MOMENTARY_MS    200    // Momentary button pulse duration (ms)

// LEDs (PLC → HMI): Read via FC01 (Read Coils)
//   Coil 4 → O:0/4 — Blue LED
//   Coil 5 → O:0/5 — Yellow LED
//   Coil 6 → O:0/6 — Red LED
//   Coil 7 → O:0/7 — Green LED
#define MB_COIL_LED_BASE  4    // First LED coil address
#define LED_COUNT         4    // Number of LED indicators

// ============================================================
// Input Register — Process Value (Gauge)
// ============================================================
// Default I/O Config maps IR addr 0-7 → PLC N:0-7.
#define MB_IR_PROCESS  0    // IR addr 0 ← PLC N:0 (process value)

// ============================================================
// Timing Intervals (milliseconds)
// ============================================================
#define POLL_INTERVAL_MS     200    // Coil/register read cycle (5 Hz)
#define STATUS_INTERVAL_MS  1000    // Connection status check cycle
#define RECONNECT_DELAY_MS  3000    // Wait before reconnect attempt

// ============================================================
// Gauge Configuration
// ============================================================
#define GAUGE_MIN       0
#define GAUGE_SETPOINT 50
#define GAUGE_MAX     100

#endif // CONFIG_H
