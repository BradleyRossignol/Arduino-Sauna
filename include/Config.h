#pragma once
#include <stdint.h>

#ifndef CONFIG_H
#define CONFIG_H

// ────────────────────────────────────────────────
// Hardware Pins
// ────────────────────────────────────────────────
constexpr uint8_t ONE_WIRE_BUS_PIN = 2; // DS18B20 data pin

// ────────────────────────────────────────────────
// Timing & Intervals
// ────────────────────────────────────────────────
constexpr unsigned long SENSOR_READ_INTERVAL_MS = 5000UL; // Read temps every 5 seconds
constexpr unsigned long STARTUP_MESSAGE_DELAY_MS = 1500UL; // Pause after "sensors ready"
constexpr unsigned long UI_REFRESH_MS = 5000UL; // UI refresh interval

// ────────────────────────────────────────────────
// Network / Time
// ────────────────────────────────────────────────
constexpr const char* NTP_SERVER = "pool.ntp.org";
constexpr unsigned long NTP_UPDATE_INTERVAL_MS = 60000UL; // Poll NTP every 60s
constexpr const char* TIME_FORMAT_PATTERN = "%04d-%02d-%02d %02d:%02d:%02d";

namespace TimeConfig {
    constexpr long OFFSET_SEC = -28800L; // PST = UTC-8 (change for your timezone)
}

// ────────────────────────────────────────────────
// Temperature Configuration
// ────────────────────────────────────────────────
constexpr int MAX_SENSORS = 5;
constexpr float TEMP_DISCONNECTED_C = -127.0f; // DS18B20 disconnected/error value
constexpr float TEMP_DISCONNECTED_F = -196.6f; // Approx -127°C in °F

// ────────────────────────────────────────────────
// Display Layout (Startup)
// ────────────────────────────────────────────────
constexpr int STARTUP_TEXT_SIZE = 3;
constexpr int STARTUP_CURSOR_X = 60;
constexpr int SYNC_TIME_Y = 180;
constexpr int SENSORS_READY_Y = 220;
constexpr int CENTER_TEXT_X = 400; // Approx horizontal center for 800x480
constexpr int CENTER_TEXT_Y = 240;

// ────────────────────────────────────────────────
// UI Colors (Consolidated)
// ────────────────────────────────────────────────
constexpr uint16_t COLOR_TURQUOISE = 0x07FF;
constexpr uint16_t COLOR_WARNING = 0xF800; // Red for errors (was also ERROR_TEXT_COLOR_565)
constexpr uint16_t COLOR_LABEL = 0xC618; // Light gray-ish
constexpr uint16_t COLOR_VALUE = COLOR_TURQUOISE;

constexpr uint16_t COLOR_VERY_COLD = 0x07FF; // Cyan
constexpr uint16_t COLOR_COLD = 0xFFFF; // White
constexpr uint16_t COLOR_WARM = 0xFFE0; // Yellow
constexpr uint16_t COLOR_HOT = 0x07E0; // Green
constexpr uint16_t COLOR_VERY_HOT = 0xF800; // Red

// ────────────────────────────────────────────────
// UI Layout & Typography
// ────────────────────────────────────────────────
constexpr int UI_LABEL_X = 35;
constexpr int UI_VALUE_X = 420;
constexpr int UI_LINE_HEIGHT = 42;
constexpr int UI_TITLE_Y = 25;
constexpr int UI_TITLE_SIZE = 4;
constexpr int UI_NORMAL_SIZE = 3;
constexpr int UI_SENSOR_Y_GAP = -5; // Slight overlap adjustment
constexpr int UI_CENTER_X = 400; // Approx for centering

// ────────────────────────────────────────────────
// UI Temperature Thresholds (in Fahrenheit for color banding)
// ────────────────────────────────────────────────
constexpr float TEMP_THRESHOLD_VERY_COLD = 60.0f;
constexpr float TEMP_THRESHOLD_COLD = 100.0f;
constexpr float TEMP_THRESHOLD_WARM = 160.0f;
constexpr float TEMP_THRESHOLD_HOT = 200.0f;

constexpr float INVALID_TEMPERATURE_C = -127.0f;  // Matches DS18B20 disconnected
// Optionally remove or deprecate TEMP_INVALID = -999.0f if not used elsewhere

#endif // CONFIG_H