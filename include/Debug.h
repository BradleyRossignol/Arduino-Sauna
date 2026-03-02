#pragma once

#ifndef DEBUG_H
#define DEBUG_H

// ── File Overview ──────────────────────────────────────────────────────────
// Configurable debug logging system for the Sauna Controller project.
// Uses macros so disabled levels have zero runtime or code-size cost.
//
// DEBUG_LEVEL guide (set in platformio.ini or override before include):
//   0 = Silent (no output)                  ← current default
//   1 = ERROR only
//   2 = WARN + ERROR
//   3 = INFO + lower
//   4 = VERBOSE + lower (timing, sensor data)
//   5 = TRACE + lower (very detailed)
//
// All LOG_ macros are safe to leave in production code.
// Legacy DEBUG_INIT / DEBUG_PRINT macros are kept for main.ino compatibility.
// ───────────────────────────────────────────────────────────────────────────

#include <Arduino.h>

// ── Master debug level (can be overridden in platformio.ini) ───────────────
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0  // 0 = off (exact default from committed code)
#endif

// ── Legacy debug macros (kept for full backward compatibility) ─────────────
#if DEBUG_LEVEL > 0
    #define DEBUG_INIT(baud)    Serial.begin(baud); while(!Serial) { }
    #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
    #define DEBUG_INIT(baud)
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
#endif

// ── Modern leveled logging macros (preferred) ──────────────────────────────
#define LOG_ERROR(...)   do { if (DEBUG_LEVEL >= 1) { Serial.print(F("ERROR: "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_WARN(...)    do { if (DEBUG_LEVEL >= 2) { Serial.print(F("WARN : "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_INFO(...)    do { if (DEBUG_LEVEL >= 3) { Serial.print(F("INFO : "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_VERBOSE(...) do { if (DEBUG_LEVEL >= 4) { Serial.print(F("VERB : "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_TRACE(...)   do { if (DEBUG_LEVEL >= 5) { Serial.print(F("TRACE: "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)

// ── Startup helper (used in multiple places) ───────────────────────────────
#define LOG_STARTUP(msg) LOG_INFO(F("Startup: ") msg)

#endif // DEBUG_H