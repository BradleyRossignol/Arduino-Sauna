#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"

struct SaunaTemperatures {
    float sauna;    // Primary sauna temp
    float heater;   // Near heater/element for safety
    float ambient;  // Room/outside (if present)
    // Expand for more sensors (e.g., humidity later)
    bool valid = false;
};

class SensorManager {
public:
    SensorManager();
    void init();
    bool update();  // Returns true if new valid reading available
    const SaunaTemperatures& getTemperatures() const;
    float getAverageTemp() const;
    bool hasError() const;
    uint8_t getSensorCount() const;

private:
    OneWire oneWire;
    DallasTemperature sensors;
    SaunaTemperatures temps;
    DeviceAddress addresses[MAX_SENSORS];
    uint8_t sensorCount = 0;
    bool errorFlag = false;
    unsigned long lastRead = 0;
};

#endif