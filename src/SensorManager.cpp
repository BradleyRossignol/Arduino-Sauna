#include "SensorManager.h"
#include "Debug.h"  // ADDED: For leveled logging macros

// ── Removed: SENSOR_DEBUG define (integrated into LOG_VERBOSE) ──────────────

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
            LOG_ERROR("Failed to get address for sensor " + String(i));  // CHANGED: To LOG_ERROR
        }
    }

    sensors.setResolution(12);
    sensors.setWaitForConversion(false);           // Async mode
    conversionDelay = sensors.millisToWaitForConversion(12);

    // Serial.begin(115200);  // Assume called in main.ino (via DEBUG_INIT if needed)
    LOG_INFO("Sensor init | Count: " + String(sensorCount) + " | Async delay: " + String(conversionDelay) + " ms");  // CHANGED: To LOG_INFO
}

bool SensorManager::update() {
    unsigned long now = millis();

    if (useBlockingFallbackThisCycle) {
        LOG_WARN("[" + String(now) + "] Blocking fallback cycle");  // CHANGED: To LOG_WARN (recovery event)
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

        LOG_VERBOSE("[" + String(now) + "] Requesting temps (async)");  // CHANGED: To LOG_VERBOSE (detailed timing)
        sensors.requestTemperatures();
        conversionStart = now;
        waitingForConversion = true;
        return false;
    }
    else {
        if (now - conversionStart < conversionDelay) {
            return false;
        }

        LOG_VERBOSE("[" + String(now) + "] Reading after " + String(now - conversionStart) + " ms → ");  // CHANGED: To LOG_VERBOSE

        readTemperatures();

        lastRead = now;
        waitingForConversion = false;

        if (!temps.valid) {
            consecutiveFailures++;
            LOG_WARN("Async read failed (" + String(consecutiveFailures) + "/" + String(MAX_CONSECUTIVE_FAILURES) + ")");  // CHANGED: To LOG_WARN
            if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
                useBlockingFallbackThisCycle = true;
                LOG_WARN("→ Switching to blocking fallback next cycle");  // CHANGED: To LOG_WARN
            }
        } else {
            consecutiveFailures = 0;
            LOG_VERBOSE("OK (valid)");  // CHANGED: To LOG_VERBOSE (unified, was conditional on SENSOR_DEBUG)
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

    // ── Unified validation loop (always runs, details via LOG_VERBOSE) ────── // ADDED: Comment
    for (uint8_t i = 0; i < sensorCount; i++) {
        float t = sensors.getTempCByIndex(i);
        if (t == DEVICE_DISCONNECTED_C || t == INVALID_TEMPERATURE_C) {
            errorFlag = true;
            temps.valid = false;
            LOG_VERBOSE("  Sensor " + String(i) + " invalid: " + String(t));  // CHANGED: To LOG_VERBOSE (was under SENSOR_DEBUG)
        } else {
            LOG_VERBOSE("  Sensor " + String(i) + ": " + String(t) + " C");  // CHANGED: To LOG_VERBOSE (was under SENSOR_DEBUG)
        }
    }

    if (!temps.valid) {
        LOG_WARN("Data invalid");  // CHANGED: To LOG_WARN
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