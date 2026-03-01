// include/Debug.h
// Phase 8.1 - Configurable serial debug levels (printf-free version for broad compatibility)

#pragma once

#include <Arduino.h>

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0  // 0 = off, 1=error, 2=warn, 3=info, 4=verbose, 5=trace
#endif

#if DEBUG_LEVEL > 0
    #define DEBUG_INIT(baud)    Serial.begin(baud); while(!Serial) { }
    #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
    #define DEBUG_INIT(baud)
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
#endif

#define LOG_ERROR(...)   do { if (DEBUG_LEVEL >= 1) { Serial.print(F("ERROR: "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_WARN(...)    do { if (DEBUG_LEVEL >= 2) { Serial.print(F("WARN : "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_INFO(...)    do { if (DEBUG_LEVEL >= 3) { Serial.print(F("INFO : "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_VERBOSE(...) do { if (DEBUG_LEVEL >= 4) { Serial.print(F("VERB : "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)
#define LOG_TRACE(...)   do { if (DEBUG_LEVEL >= 5) { Serial.print(F("TRACE: "));   Serial.print(__VA_ARGS__); Serial.println(); } } while(0)

// Startup helpers
#define LOG_STARTUP(msg) LOG_INFO(F("Startup: ") msg)