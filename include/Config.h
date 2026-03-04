#pragma once

#ifndef CONFIG_H
#define CONFIG_H

// ── File Overview ──────────────────────────────────────────────────────────
// Central configuration file for the Arduino Giga R1 Sauna Controller.
// Contains all hardware pins, timing intervals, display constants, 
// color palette, temperature thresholds, and network/time settings.
//
// All values are constexpr for compile-time optimization.
// Edit this file to tune behavior without changing logic elsewhere.
// ───────────────────────────────────────────────────────────────────────────

#include <stdint.h>

// ────────────────────────────────────────────────
// Hardware Pins
// ────────────────────────────────────────────────
constexpr uint8_t ONE_WIRE_BUS_PIN = 2; // DS18B20 data pin

// ────────────────────────────────────────────────
// Timing & Intervals
// ────────────────────────────────────────────────
constexpr unsigned long SENSOR_READ_INTERVAL_MS   = 5000UL;   // Read temps every 5 seconds
constexpr unsigned long UI_REFRESH_MS             = 5000UL;   // UI refresh interval
constexpr unsigned long NTP_UPDATE_INTERVAL_MS    = 60000UL;  // Poll NTP every 60s

// ────────────────────────────────────────────────
// Network / Time
// ────────────────────────────────────────────────
constexpr const char* NTP_SERVER          = "pool.ntp.org";
constexpr const char* TIME_FORMAT_PATTERN = "%04d-%02d-%02d %02d:%02d:%02d";

namespace TimeConfig {
    constexpr long OFFSET_SEC = -28800L; // PST = UTC-8 (change for your timezone)
}

// ────────────────────────────────────────────────
// Temperature Configuration
// ────────────────────────────────────────────────
constexpr int   MAX_SENSORS           = 5;
constexpr float TEMP_DISCONNECTED_C   = -127.0f;   // DS18B20 error value
constexpr float TEMP_DISCONNECTED_F   = -196.6f;   // -127°C in °F
constexpr float INVALID_TEMPERATURE_C = -127.0f;   // Alias for consistency

// ────────────────────────────────────────────────
// Display Layout
// ────────────────────────────────────────────────
constexpr int SCREEN_WIDTH          = 800;
constexpr int SCREEN_HEIGHT         = 480;

// ────────────────────────────────────────────────
// UI Colors
// ────────────────────────────────────────────────
constexpr uint16_t COLOR_BACKGROUND   = 0x0000;     // Black
constexpr uint16_t COLOR_TURQUOISE    = 0x07FF;
constexpr uint16_t COLOR_WARNING      = 0xF800;     // Red
constexpr uint16_t COLOR_LABEL        = 0xC618;     // Light gray
constexpr uint16_t COLOR_VALUE        = COLOR_TURQUOISE;

// Temperature color banding (Fahrenheit)
constexpr uint16_t COLOR_VERY_COLD    = 0x07FF;     // Cyan
constexpr uint16_t COLOR_COLD         = 0xFFFF;     // White
constexpr uint16_t COLOR_WARM         = 0xFFE0;     // Yellow
constexpr uint16_t COLOR_HOT          = 0x07E0;     // Green
constexpr uint16_t COLOR_VERY_HOT     = 0xF800;     // Red

// ────────────────────────────────────────────────
// UI Layout & Typography
// ────────────────────────────────────────────────
constexpr int UI_LABEL_X        = 35;
constexpr int UI_VALUE_X        = 420;
constexpr int UI_LINE_HEIGHT    = 42;
constexpr int UI_TITLE_Y        = 25;
constexpr int UI_TITLE_SIZE     = 4;
constexpr int UI_NORMAL_SIZE    = 3;
constexpr int UI_SENSOR_Y_GAP   = -5;     // Slight overlap adjustment
constexpr int UI_CENTER_X       = 400;

// ────────────────────────────────────────────────
// UI Temperature Thresholds (Fahrenheit)
// ────────────────────────────────────────────────
constexpr float TEMP_THRESHOLD_VERY_COLD = 60.0f;
constexpr float TEMP_THRESHOLD_COLD      = 100.0f;
constexpr float TEMP_THRESHOLD_WARM      = 160.0f;
constexpr float TEMP_THRESHOLD_HOT       = 200.0f;

#endif // CONFIG_H