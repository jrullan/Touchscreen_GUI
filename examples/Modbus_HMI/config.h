// ============================================================
// config.h — Modbus HMI Configuration
// ============================================================
// Edit this file to match your deployment:
//   - WiFi credentials
//   - PLC IP address and Modbus port
//   - Polling intervals and gauge range
//
// Pin assignments match the Wemos D1 Mini + ILI9341 + XPT2046
// wiring used by all Touchscreen_GUI examples.
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
// Pin Configuration (ESP8266 Wemos D1 Mini)
// ============================================================
// NOTE: Pin defines (TFT_CS, TFT_DS, TS_CS) are in the .ino file
// AFTER library includes, to avoid macro clashes with GUI_TFT.h
// parameter names. See Modbus_HMI.ino for pin assignments.

// ============================================================
// Modbus Register Map
// ============================================================
// These are Modbus-side addresses (0-based).
// The PLC-side mapping is configured via the I/O Config dialog:
//
// Current mapping (default):
//   Input Registers:   Modbus addr 0-7  <-  PLC N:0-7   (FC04, read-only)
//   Holding Registers: Modbus addr 0-7  <-> PLC N:10-17  (FC03/06/16, read-write)
//   Coils:             Modbus addr 0-15 <-> PLC O:0/0-15  (FC01/05/15)
//   Discrete Inputs:   Modbus addr 0-15 <-  PLC I:0/0-15  (FC02, read-only)

// HMI writes commands to holding registers (-> PLC N:10+)
#define MB_HR_COMMAND  0    // HR addr 0 -> PLC N:10 (command word)

// HMI reads process values from input registers (<- PLC N:0+)
#define MB_IR_PROCESS  0    // IR addr 0 <- PLC N:0 (process value)

// Command values written to MB_HR_COMMAND
#define CMD_STOP       0
#define CMD_START      1

// ============================================================
// Timing Intervals (milliseconds)
// ============================================================
#define POLL_INTERVAL_MS     200    // Process value read cycle (5 Hz)
#define STATUS_INTERVAL_MS  1000    // Connection status check cycle
#define RECONNECT_DELAY_MS  3000    // Wait before reconnect attempt

// ============================================================
// Gauge Configuration
// ============================================================
#define GAUGE_MIN       0
#define GAUGE_SETPOINT 50
#define GAUGE_MAX     100

#endif // CONFIG_H
