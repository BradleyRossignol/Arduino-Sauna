#pragma once
#include <stdint.h>
// ────────────────────────────────────────────────
// Sauna Controller Configuration
// Hardware, timing, display, network settings
// ────────────────────────────────────────────────

// ── Hardware Pins ─────────────────────────────────
constexpr uint8_t ONE_WIRE_BUS_PIN = 2;  // DS18B20 data pin

// ── Timing & Intervals ────────────────────────────
constexpr unsigned long SENSOR_READ_INTERVAL_MS   = 5000UL;   // Read temps every 5 seconds
constexpr unsigned long STARTUP_MESSAGE_DELAY_MS  = 1500UL;   // Pause after "sensors ready"

// ── Network / Time ────────────────────────────────
constexpr const char* NTP_SERVER          = "pool.ntp.org";
constexpr long        NTP_TIME_OFFSET_SEC = -28800L;   // PST = UTC-8 (change for your timezone)
constexpr unsigned long NTP_UPDATE_INTERVAL_MS = 60000UL;  // Poll NTP every 60s
constexpr const char* TIME_FORMAT_STR = "%04d-%02d-%02d %02d:%02d:%02d";  // For future easy changes
constexpr const char* TIME_FORMAT_PATTERN = "%04d-%02d-%02d %02d:%02d:%02d";

// ── Display / Colors ──────────────────────────────
constexpr uint16_t ERROR_TEXT_COLOR_565 = 0xF800;      // Bright red in RGB565

// ── Temperature Initialization ────────────────────
constexpr float INVALID_TEMPERATURE_C = -127.0f;       // DS18B20 error value
constexpr float INVALID_TEMPERATURE_F = -196.6f;       // Approx -127°C in °F

// ── Display Layout Assumptions (may move to UI later) ──
constexpr int STARTUP_TEXT_SIZE = 3;
constexpr int STARTUP_CURSOR_X  = 60;
constexpr int SYNC_TIME_Y       = 180;
constexpr int SENSORS_READY_Y   = 220;
constexpr int CENTER_TEXT_X     = 400;   // Approx horizontal center for 800x480?
constexpr int CENTER_TEXT_Y     = 240;
