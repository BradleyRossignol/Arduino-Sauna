#include "SensorManager.h"   // ← Critical: this brings in SaunaTemperatures definition

SensorManager::SensorManager() 
    : oneWire(ONE_WIRE_BUS_PIN),
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

    sensors.setResolution(12);
    sensors.setWaitForConversion(false);           // Async mode
    conversionDelay = sensors.millisToWaitForConversion(12);

    Serial.begin(115200);  // Safe to call again if already done in main
    Serial.println("Sensor init | Count: " + String(sensorCount) + " | Async delay: " + String(conversionDelay) + " ms");
}

bool SensorManager::update() {
    unsigned long now = millis();

    if (!waitingForConversion) {
        if (now - lastRead < SENSOR_READ_INTERVAL_MS) {
            return false;
        }

        Serial.println("[" + String(now) + "] Requesting temps (async)");
        sensors.requestTemperatures();
        conversionStart = now;
        waitingForConversion = true;
        return false;
    }
    else {
        if (now - conversionStart < conversionDelay) {
            return false;
        }

        Serial.println("[" + String(now) + "] Reading after " + String(now - conversionStart) + " ms");

        readTemperatures();

        lastRead = now;
        waitingForConversion = false;
        return temps.valid;
    }
}

void SensorManager::readTemperatures() {
    temps.valid = (sensorCount > 0);
    errorFlag = false;

    temps.sauna   = (sensorCount >= 1) ? sensors.getTempCByIndex(0) : INVALID_TEMPERATURE_C;
    temps.heater  = (sensorCount >= 2) ? sensors.getTempCByIndex(1) : INVALID_TEMPERATURE_C;
    temps.ambient = (sensorCount >= 3) ? sensors.getTempCByIndex(2) : INVALID_TEMPERATURE_C;

    for (uint8_t i = 0; i < sensorCount; i++) {
        float t = sensors.getTempCByIndex(i);
        if (t == DEVICE_DISCONNECTED_C || t == INVALID_TEMPERATURE_C) {
            errorFlag = true;
            temps.valid = false;
            Serial.println("Sensor " + String(i) + " invalid: " + String(t));
        } else {
            Serial.println("Sensor " + String(i) + ": " + String(t) + " C");
        }
    }

    Serial.println(temps.valid ? "Valid data set" : "Data invalid");
}

const SaunaTemperatures& SensorManager::getTemperatures() const {
    return temps;
}

float SensorManager::getAverageTemp() const {
    if (!temps.valid) return INVALID_TEMPERATURE_C;

    float sum = 0.0f;
    int count = 0;

    if (temps.sauna != INVALID_TEMPERATURE_C && temps.sauna != DEVICE_DISCONNECTED_C)   { sum += temps.sauna;   count++; }
    if (temps.heater != INVALID_TEMPERATURE_C && temps.heater != DEVICE_DISCONNECTED_C)  { sum += temps.heater;  count++; }
    if (temps.ambient != INVALID_TEMPERATURE_C && temps.ambient != DEVICE_DISCONNECTED_C) { sum += temps.ambient; count++; }

    return (count > 0) ? (sum / count) : INVALID_TEMPERATURE_C;
}

bool SensorManager::hasError() const {
    return errorFlag;
}

uint8_t SensorManager::getSensorCount() const {
    return sensorCount;
}