#include "SensorManager.h"

// ── Toggle detailed debug output ────────────────────────────────────────────
// Set to 1 for verbose logging (every sensor value on success), 0 for minimal
#define SENSOR_DEBUG 1

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
            Serial.println("Failed to get address for sensor " + String(i));
        }
    }

    sensors.setResolution(12);
    sensors.setWaitForConversion(false);           // Async mode
    conversionDelay = sensors.millisToWaitForConversion(12);

    // Serial.begin(115200);  // Assume called in main.ino
    Serial.println("Sensor init | Count: " + String(sensorCount) + " | Async delay: " + String(conversionDelay) + " ms");
}

bool SensorManager::update() {
    unsigned long now = millis();

    if (useBlockingFallbackThisCycle) {
        Serial.println("[" + String(now) + "] Blocking fallback cycle");
        sensors.setWaitForConversion(true);
        sensors.requestTemperatures();
        readTemperatures();
        sensors.setWaitForConversion(false);  // Restore async

        lastRead = now;
        consecutiveFailures = 0;
        lastFallbackTime = now;  // Record for UI
        useBlockingFallbackThisCycle = false;
        return temps.valid;
    }

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

        Serial.print("[" + String(now) + "] Reading after " + String(now - conversionStart) + " ms → ");

        readTemperatures();

        lastRead = now;
        waitingForConversion = false;

        if (!temps.valid) {
            consecutiveFailures++;
            Serial.println("Async read failed (" + String(consecutiveFailures) + "/" + String(MAX_CONSECUTIVE_FAILURES) + ")");
            if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
                useBlockingFallbackThisCycle = true;
                Serial.println("→ Switching to blocking fallback next cycle");
            }
        } else {
            consecutiveFailures = 0;
#if SENSOR_DEBUG
            Serial.println("OK");
#else
            Serial.println("OK (valid)");
#endif
        }

        return temps.valid;
    }
}

void SensorManager::readTemperatures() {
    temps.valid = (sensorCount > 0);
    errorFlag = false;

    temps.sauna   = (sensorCount >= 1) ? sensors.getTempCByIndex(0) : INVALID_TEMPERATURE_C;
    temps.heater  = (sensorCount >= 2) ? sensors.getTempCByIndex(1) : INVALID_TEMPERATURE_C;
    temps.ambient = (sensorCount >= 3) ? sensors.getTempCByIndex(2) : INVALID_TEMPERATURE_C;

#if SENSOR_DEBUG
    for (uint8_t i = 0; i < sensorCount; i++) {
        float t = sensors.getTempCByIndex(i);
        if (t == DEVICE_DISCONNECTED_C || t == INVALID_TEMPERATURE_C) {
            errorFlag = true;
            temps.valid = false;
            Serial.println("  Sensor " + String(i) + " invalid: " + String(t));
        } else {
            Serial.println("  Sensor " + String(i) + ": " + String(t) + " C");
        }
    }
#else
    for (uint8_t i = 0; i < sensorCount; i++) {
        float t = sensors.getTempCByIndex(i);
        if (t == DEVICE_DISCONNECTED_C || t == INVALID_TEMPERATURE_C) {
            errorFlag = true;
            temps.valid = false;
        }
    }
#endif

    if (!temps.valid) {
        Serial.println("Data invalid");
    }
}

bool SensorManager::hadRecentFallback() const {
    unsigned long now = millis();
    return (lastFallbackTime > 0) && (now - lastFallbackTime < FALLBACK_RECENT_MS);
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