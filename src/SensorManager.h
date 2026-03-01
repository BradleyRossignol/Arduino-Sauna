#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"           // For pins, MAX_SENSORS, SENSOR_READ_INTERVAL_MS, etc.

// ── Temperature data structure (defined here as in original commit) ────────
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

    void readTemperatures();                     // Shared read logic
};

#endif