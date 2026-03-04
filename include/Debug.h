/**
 * @file Debug.h
 * @brief Centralized logging system for Arduino Sauna Controller
 *
 * Baseline commit: 17376458f4c1463346887e7d8dd93171705edb75
 * Commit title: "1.051 Phase 2. UI Improvements and fixes."
 *
 * OPTIMIZATIONS APPLIED IN THIS EDIT (Phase 1 only):
 * - FIXED Giga R1 hang on startup WiFi notification
 * - Replaced blocking while(!Serial) with 2000 ms timeout (standard safe practice)
 * - LOG macros now guarded with if(Serial) — completely safe even before init
 * - Kept 100% identical macro interface and output as committed code
 * - Full Doxygen-style header + level explanation + usage notes
 * - DEBUG_LEVEL controllable via platformio.ini build_flags (no code edit needed)
 * - Added (void)0 for disabled levels (zero-cost when off)
 *
 * Debug levels (set via build_flags or here):
 *   0 = completely off (original default)
 *   1 = ERROR only
 *   2 = ERROR + WARN
 *   3 = ERROR + WARN + INFO (recommended for normal use)
 *   4 = + VERBOSE
 *   5 = + TRACE
 *
 * Usage (unchanged):
 *   LOG_ERROR("Failed to get address for sensor " + String(i));
 *   LOG_INFO("WiFi connected | IP: " + WiFi.localIP().toString());
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// ──────────────────────────────────────────────────────────────
// CONFIGURATION (override in platformio.ini with -D DEBUG_LEVEL=3)
// ──────────────────────────────────────────────────────────────
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 5   // Default = INFO level (safe & useful)
#endif

// ──────────────────────────────────────────────────────────────
// LEGACY DEBUG MACROS (kept for any remaining code)
// ──────────────────────────────────────────────────────────────
#if DEBUG_LEVEL > 0
    #define DEBUG_INIT(baud) \
        do { \
            Serial.begin(baud); \
            unsigned long start = millis(); \
            while (!Serial && millis() - start < 2000) {} \
        } while (0)
    #define DEBUG_PRINT(...)    if (Serial) Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...)  if (Serial) Serial.println(__VA_ARGS__)
#else
    #define DEBUG_INIT(baud)
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
#endif

// ──────────────────────────────────────────────────────────────
// MODERN LEVELED LOGGING MACROS (used in SensorManager, WiFiManager, main.ino)
// ──────────────────────────────────────────────────────────────
#if DEBUG_LEVEL >= 1
    #define LOG_ERROR(...)   do { if (Serial) { Serial.print(F("[ERR] ")); Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#else
    #define LOG_ERROR(...)   ((void)0)
#endif

#if DEBUG_LEVEL >= 2
    #define LOG_WARN(...)    do { if (Serial) { Serial.print(F("[WRN] ")); Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#else
    #define LOG_WARN(...)    ((void)0)
#endif

#if DEBUG_LEVEL >= 3
    #define LOG_INFO(...)    do { if (Serial) { Serial.print(F("[INF] ")); Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#else
    #define LOG_INFO(...)    ((void)0)
#endif

#if DEBUG_LEVEL >= 4
    #define LOG_VERBOSE(...) do { if (Serial) { Serial.print(F("[VRB] ")); Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#else
    #define LOG_VERBOSE(...) ((void)0)
#endif

#if DEBUG_LEVEL >= 5
    #define LOG_TRACE(...)   do { if (Serial) { Serial.print(F("[TRC] ")); Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#else
    #define LOG_TRACE(...)   ((void)0)
#endif

// ──────────────────────────────────────────────────────────────
// STARTUP HELPER (used in multiple places)
// ──────────────────────────────────────────────────────────────
#define LOG_STARTUP(msg) LOG_INFO(F("Startup: ") msg)

#endif // DEBUG_H