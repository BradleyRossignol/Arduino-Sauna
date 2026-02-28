#include "SensorManager.h"

SensorManager::SensorManager() : oneWire(ONE_WIRE_BUS), sensors(&oneWire) {}

void SensorManager::init() {
    sensors.begin();
    sensorCount = sensors.getDeviceCount();
    if (sensorCount > MAX_SENSORS) sensorCount = MAX_SENSORS;

    for (uint8_t i = 0; i < sensorCount; i++) {
        if (!sensors.getAddress(addresses[i], i)) {
            errorFlag = true;
        }
    }

    sensors.setResolution(12);  // High res
    sensors.setWaitForConversion(true);
}

bool SensorManager::update() {
    unsigned long now = millis();
    if (now - lastRead < SENSOR_READ_INTERVAL_MS) return false;
    lastRead = now;

    sensors.requestTemperatures();

    temps.valid = (sensorCount > 0);
    errorFlag = false;

    // Map sensors by index (customize based on your wiring/roles)
    temps.sauna = (sensorCount >= 1) ? sensors.getTempCByIndex(0) : TEMP_INVALID;
    temps.heater = (sensorCount >= 2) ? sensors.getTempCByIndex(1) : TEMP_INVALID;
    temps.ambient = (sensorCount >= 3) ? sensors.getTempCByIndex(2) : TEMP_INVALID;

    // Error checks (expand on your 1.035 improvements: CRC, disconnects, retries?)
    for (uint8_t i = 0; i < sensorCount; i++) {
        float t = sensors.getTempCByIndex(i);
        if (t == DEVICE_DISCONNECTED_C || t == TEMP_INVALID) {
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
    if (!temps.valid) return TEMP_INVALID;
    float sum = 0;
    int count = 0;
    if (temps.sauna != TEMP_INVALID) { sum += temps.sauna; count++; }
    if (temps.heater != TEMP_INVALID) { sum += temps.heater; count++; }
    if (temps.ambient != TEMP_INVALID) { sum += temps.ambient; count++; }
    return (count > 0) ? sum / count : TEMP_INVALID;
}

bool SensorManager::hasError() const { return errorFlag; }
uint8_t SensorManager::getSensorCount() const { return sensorCount; }