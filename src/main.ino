/*
 * main.ino - Entry point for Giga R1 Sauna Controller with Giga Display
 *
 * Responsibilities:
 *   - Initialize hardware (display, WiFi, NTP, sensors via SensorManager)
 *   - Handle WiFi connection with graceful retry (Phase 6: no blocking halt on failure)
 *   - Periodically update sensors via SensorManager and feed to UI
 *   - Delegate all drawing / touch to UI module
 */

#include <Arduino_GigaDisplay_GFX.h>
#include "WiFiManager.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "UI.h"
#include "Config.h"         // For configuration constants
#include "SensorManager.h"  // NEW: Sensor handling extracted
#include "TimeUtils.h"      // Extracted time helper

GigaDisplay_GFX gfx;
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, TimeConfig::OFFSET_SEC, NTP_UPDATE_INTERVAL_MS);

SensorManager sensorManager;  // Global instance - owns all DS18B20 logic

void setup() {
  uiInit();  // Sets up display and shows "Connecting..."

  wifi.begin();  // Attempt connection (timed/returns success)

  gfx.fillScreen(0x0000);

  if (wifi.isConnected()) {
    gfx.setTextSize(STARTUP_TEXT_SIZE);
    gfx.setCursor(STARTUP_CURSOR_X, SYNC_TIME_Y);
    gfx.print("Syncing time...");
    timeClient.begin();
    timeClient.update();

    // Initialize sensors (moved from here)
    sensorManager.init();

    gfx.fillScreen(0x0000);
    gfx.setTextSize(STARTUP_TEXT_SIZE);
    gfx.setCursor(STARTUP_CURSOR_X, SENSORS_READY_Y);
    gfx.print("DS18B20 sensors ready");
    delay(STARTUP_MESSAGE_DELAY_MS);
  } else {
    // Phase 6: Show warning but CONTINUE - no infinite loop
    gfx.setTextColor(COLOR_WARNING);
    centerText("WiFi Failed - Retrying...", CENTER_TEXT_X, CENTER_TEXT_Y);
    delay(STARTUP_MESSAGE_DELAY_MS);  // Brief display of message

    // Still initialize sensors so core functions work offline
    sensorManager.init();
  }

  gfx.fillScreen(0x0000);  // Clear for main UI
}

void loop() {
  // Phase 6: Maintain WiFi in background (checks status, retries if disconnected)
  wifi.maintain();

  timeClient.update();  // Safe even if not connected

  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = millis();

    bool newReading = sensorManager.update();

    const SaunaTemperatures& temps = sensorManager.getTemperatures();

    // Feed to UI (convert C→F if needed - assuming uiSetTemp expects both)
    float temp1C = temps.sauna;
    float temp1F = (temp1C != TEMP_DISCONNECTED_C) ? (temp1C * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F;
    float temp2C = temps.heater;
    float temp2F = (temp2C != TEMP_DISCONNECTED_C) ? (temp2C * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F;

    uiSetTemp1(temp1C, temp1F);
    uiSetTemp2(temp2C, temp2F);

    uiSetWifiInfo(WiFi.SSID(), WiFi.localIP().toString(), getMacString());
    uiSetTime(getFormattedDateTime());

    // Optional: Reflect WiFi state in UI
    // uiSetWifiConnected(wifi.isConnected());

    // Optional: Add error feedback to UI
    if (sensorManager.hasError()) {
      // e.g., uiShowError("Sensor Error"); or flash warning
    }

    uiUpdate();
  }

  // Future additions: heater control, touch handling, safety checks, etc.
}

// Helpers (unchanged)
String getMacString() {
  byte mac[6];
  WiFi.macAddress(mac);
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String getFormattedDateTime() {
  if (!timeClient.isTimeSet()) {
    return "Time not synced";
  }

  unsigned long epoch = timeClient.getEpochTime();

  int year, month, day, hour, minute, second;
  epochToDateTime(epoch, year, month, day, hour, minute, second);

  char buf[20];
  snprintf(buf, sizeof(buf), TIME_FORMAT_PATTERN, year, month, day, hour, minute, second);

  return String(buf);
}