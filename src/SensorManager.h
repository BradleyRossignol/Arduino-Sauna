#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"           // For pins, MAX_SENSORS, SENSOR_READ_INTERVAL_MS, etc.

// ── Temperature data structure ─────────────────────────────────────────────
struct SaunaTemperatures {
    float sauna   = INVALID_TEMPERATURE_C;
    float heater  = INVALID_TEMPERATURE_C;
    float ambient = INVALID_TEMPERATURE_C;
    bool  valid   = false;
};

class SensorManager {
public:
    SensorManager();

    void init();
    bool update();                               // true = new valid data ready
    const SaunaTemperatures& getTemperatures() const;
    float getAverageTemp() const;
    bool hasError() const;
    uint8_t getSensorCount() const;

    // ── New: UI feedback for async health ───────────────────────────────────
    bool hadRecentFallback() const;              // true if fallback used recently

private:
    OneWire oneWire;
    DallasTemperature sensors;
    DeviceAddress addresses[MAX_SENSORS];
    uint8_t sensorCount = 0;
    SaunaTemperatures temps;
    bool errorFlag = false;
    unsigned long lastRead = 0;

    // ── Phase 8: Async support ──────────────────────────────────────────────
    unsigned long conversionStart = 0;
    unsigned long conversionDelay = 750;         // Set accurately in init()
    bool waitingForConversion = false;

    // ── Stability enhancements ──────────────────────────────────────────────
    int consecutiveFailures = 0;
    const int MAX_CONSECUTIVE_FAILURES = 3;
    bool useBlockingFallbackThisCycle = false;

    // ── For UI status ───────────────────────────────────────────────────────
    unsigned long lastFallbackTime = 0;
    static const unsigned long FALLBACK_RECENT_MS = 30000;  // 30 seconds

    void readTemperatures();                     // Shared read logic
};

#endif