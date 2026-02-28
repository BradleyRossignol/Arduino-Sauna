// src/SensorManager.cpp
#include "SensorManager.h"
#include "Config.h"          // Ensure this is included for ONE_WIRE_BUS_PIN, TEMP_DISCONNECTED_C, etc.
#include <DallasTemperature.h>  // For DEVICE_DISCONNECTED_C

SensorManager::SensorManager() 
    : oneWire(ONE_WIRE_BUS_PIN),   // ← Updated: use consistent pin constant from Config.h
      sensors(&oneWire) 
{}

void SensorManager::init() {
    sensors.begin();
    sensorCount = sensors.getDeviceCount();
    if (sensorCount > MAX_SENSORS) sensorCount = MAX_SENSORS;

    for (uint8_t i = 0; i < sensorCount; i++) {
        if (!sensors.getAddress(addresses[i], i)) {
            errorFlag = true;
        }
    }

    sensors.setResolution(12);          // High resolution
    sensors.setWaitForConversion(true); // Blocking mode for now (async later in Phase 8)
}

bool SensorManager::update() {
    unsigned long now = millis();
    if (now - lastRead < SENSOR_READ_INTERVAL_MS) return false;
    lastRead = now;

    sensors.requestTemperatures();  // Start conversion

    temps.valid = (sensorCount > 0);
    errorFlag = false;

    // Assign by index (custom mapping - adjust if sensors are reordered)
    temps.sauna   = (sensorCount >= 1) ? sensors.getTempCByIndex(0) : DEVICE_DISCONNECTED_C;
    temps.heater  = (sensorCount >= 2) ? sensors.getTempCByIndex(1) : DEVICE_DISCONNECTED_C;
    temps.ambient = (sensorCount >= 3) ? sensors.getTempCByIndex(2) : DEVICE_DISCONNECTED_C;

    // Check for errors/disconnects
    for (uint8_t i = 0; i < sensorCount; i++) {
        float t = sensors.getTempCByIndex(i);
        if (t == DEVICE_DISCONNECTED_C || t == INVALID_TEMPERATURE_C) {  // ← Use library + config constant
            errorFlag = true;
            temps.valid = false;
        }
    }

    return temps.valid;
}

const SaunaTemperatures& SensorManager::getTemperatures() const {
    return temps;
}

float SensorManager::getAverageTemp() const {
    if (!temps.valid) return INVALID_TEMPERATURE_C;  // ← Consistent error return

    float sum = 0.0f;
    int count = 0;

    if (temps.sauna != DEVICE_DISCONNECTED_C && temps.sauna != INVALID_TEMPERATURE_C) {
        sum += temps.sauna;
        count++;
    }
    if (temps.heater != DEVICE_DISCONNECTED_C && temps.heater != INVALID_TEMPERATURE_C) {
        sum += temps.heater;
        count++;
    }
    if (temps.ambient != DEVICE_DISCONNECTED_C && temps.ambient != INVALID_TEMPERATURE_C) {
        sum += temps.ambient;
        count++;
    }

    return (count > 0) ? (sum / count) : INVALID_TEMPERATURE_C;
}

bool SensorManager::hasError() const {
    return errorFlag;
}

uint8_t SensorManager::getSensorCount() const {
    return sensorCount;
}