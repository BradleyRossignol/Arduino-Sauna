#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// ============== ROBUST DEBUG CONFIG (edit here or override in Config.h) ==============
#define DEBUG_LEVEL 4   // 0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=VERBOSE
// ==================================================================================

#define DEBUG_TIMESTAMP() Serial.printf("[%06lu] ", millis())

#if DEBUG_LEVEL >= 1
#define LOG_ERROR(...) do { DEBUG_TIMESTAMP(); Serial.print(F("[ERROR] ")); Serial.printf(__VA_ARGS__); Serial.println(); Serial.flush(); } while(0)
#else
#define LOG_ERROR(...) 
#endif

#if DEBUG_LEVEL >= 2
#define LOG_WARN(...)  do { DEBUG_TIMESTAMP(); Serial.print(F("[WARN]  ")); Serial.printf(__VA_ARGS__); Serial.println(); Serial.flush(); } while(0)
#else
#define LOG_WARN(...)  
#endif

#if DEBUG_LEVEL >= 3
#define LOG_INFO(...)  do { DEBUG_TIMESTAMP(); Serial.print(F("[INFO]  ")); Serial.printf(__VA_ARGS__); Serial.println(); } while(0)
#else
#define LOG_INFO(...)  
#endif

#if DEBUG_LEVEL >= 4
#define LOG_DEBUG(...) do { DEBUG_TIMESTAMP(); Serial.print(F("[DEBUG] ")); Serial.printf(__VA_ARGS__); Serial.println(); } while(0)
#else
#define LOG_DEBUG(...) 
#endif

#if DEBUG_LEVEL >= 5
#define LOG_VERBOSE(...) do { DEBUG_TIMESTAMP(); Serial.print(F("[VERBOSE] ")); Serial.printf(__VA_ARGS__); Serial.println(); } while(0)
#else
#define LOG_VERBOSE(...) 
#endif

// Function tracing for every function – perfect for troubleshooting call flow
#define DEBUG_ENTER() LOG_VERBOSE(F(">> %s"), __func__)
#define DEBUG_EXIT()  LOG_VERBOSE(F("<< %s"), __func__)

// Optional status dump helper (call from anywhere)
#define DEBUG_DUMP_STATUS(msg) LOG_DEBUG(F("STATUS: %s"), msg)

#endif