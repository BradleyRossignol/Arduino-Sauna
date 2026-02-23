// include/UIConfig.h
#ifndef UI_CONFIG_H
#define UI_CONFIG_H

// ────────────────────────────────────────────────
// Colors
// ────────────────────────────────────────────────
#define COLOR_TURQUOISE     0x07FF
#define COLOR_WARNING       0xF800     // Red for errors
#define COLOR_LABEL         0xC618     // Light gray-ish
#define COLOR_VALUE         COLOR_TURQUOISE

#define COLOR_VERY_COLD     0x07FF     // Cyan
#define COLOR_COLD          0xFFFF     // White
#define COLOR_WARM          0xFFE0     // Yellow
#define COLOR_HOT           0x07E0     // Green
#define COLOR_VERY_HOT      0xF800     // Red

// ────────────────────────────────────────────────
// Layout & Typography
// ────────────────────────────────────────────────
constexpr int    UI_LABEL_X         = 35;
constexpr int    UI_VALUE_X         = 420;
constexpr int    UI_LINE_HEIGHT     = 42;
constexpr int    UI_TITLE_Y         = 25;
constexpr int    UI_TITLE_SIZE      = 4;
constexpr int    UI_NORMAL_SIZE     = 3;
constexpr int    UI_SENSOR_Y_GAP    = -5;   // Slight overlap adjustment

constexpr int    UI_CENTER_X        = 400;  // Approx for centering (adjust if screen width known)

// ────────────────────────────────────────────────
// Timing
// ────────────────────────────────────────────────
constexpr unsigned long UI_REFRESH_MS = 5000UL;

// ────────────────────────────────────────────────
// Temperature thresholds (in Fahrenheit for color banding)
// ────────────────────────────────────────────────
constexpr float TEMP_THRESHOLD_VERY_COLD = 60.0f;
constexpr float TEMP_THRESHOLD_COLD      = 100.0f;
constexpr float TEMP_THRESHOLD_WARM      = 160.0f;
constexpr float TEMP_THRESHOLD_HOT       = 200.0f;

#endif // UI_CONFIG_H