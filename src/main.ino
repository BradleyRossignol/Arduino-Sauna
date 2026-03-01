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
#include "Debug.h"          // Phase 8.1 debug macros
#include "Arduino_GigaDisplayTouch.h"  // For touch support

GigaDisplay_GFX gfx;
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, TimeConfig::OFFSET_SEC, NTP_UPDATE_INTERVAL_MS);

SensorManager sensorManager;  // Global instance - owns all DS18B20 logic
Arduino_GigaDisplayTouch touchDetector;  // Global for touch

void setup() {
  uiInit();  // Sets up display and shows "Connecting..." (likely includes gfx.setRotation(1) or similar)
  DEBUG_INIT(115200);   // Phase 8.1

  LOG_INFO(F("Sauna Controller starting"));
  LOG_INFO(F("Build date/time: "));
  DEBUG_PRINT(__DATE__);
  DEBUG_PRINT(F(" "));
  DEBUG_PRINTLN(__TIME__);
  LOG_INFO(F("Debug level: "));
  DEBUG_PRINTLN(DEBUG_LEVEL);
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

  // Initialize touch controller
  if (touchDetector.begin()) {
    LOG_INFO(F("Touch controller initialized successfully"));
  } else {
    LOG_ERROR(F("Failed to initialize touch controller"));
  }

  gfx.fillScreen(0x0000);  // Clear for main UI
}

void loop() {
  // Phase 6: Maintain WiFi in background
  wifi.maintain();

  timeClient.update();  // Safe even if not connected

  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = millis();

    bool newReading = sensorManager.update();

    const SaunaTemperatures& temps = sensorManager.getTemperatures();

    // Convert C→F if needed (assuming uiSetTemp expects both)
    float temp1C = temps.sauna;
    float temp1F = (temp1C != TEMP_DISCONNECTED_C) ? (temp1C * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F;
    float temp2C = temps.heater;
    float temp2F = (temp2C != TEMP_DISCONNECTED_C) ? (temp2C * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F;

    uiSetTemp1(temp1C, temp1F);
    uiSetTemp2(temp2C, temp2F);

    uiSetWifiInfo(WiFi.SSID(), WiFi.localIP().toString(), getMacString());
    uiSetTime(getFormattedDateTime());

    // Optional: Reflect WiFi state or errors in UI later

    uiUpdate();
  }

    // Touch polling with corrected mapping + visual feedback dot
  static unsigned long lastTouchCheck = 0;
  static unsigned long lastDotDrawn = 0;
  static bool dotActive = false;
  static uint16_t dot_x = 0;
  static uint16_t dot_y = 0;

  if (millis() - lastTouchCheck >= 200) {
    lastTouchCheck = millis();

    uint8_t contacts;
    GDTpoint_t points[5];

    contacts = touchDetector.getTouchPoints(points);

    if (contacts > 0) {
      uint16_t raw_x = points[0].x;
      uint16_t raw_y = points[0].y;

      // Corrected mapping for landscape (setRotation(1) typical):
      // - Swap axes: raw portrait y → screen x (wide 0-799)
      // - Flip y: 479 - raw_x → screen y (top 0 to bottom 479)
      uint16_t mapped_x = raw_y;
      uint16_t mapped_y = 479 - raw_x;

      // Log with more context
      Serial.print(F("[INFO] Touch detected: "));
      Serial.print(contacts);
      Serial.println(F(" contacts"));
      Serial.print(F("  Raw (portrait): x="));
      Serial.print(raw_x);
      Serial.print(F(", y="));
      Serial.println(raw_y);
      Serial.print(F("  Mapped (landscape): x="));
      Serial.print(mapped_x);
      Serial.print(F(", y="));
      Serial.println(mapped_y);

      // Draw red feedback dot at mapped position
      gfx.fillCircle(mapped_x, mapped_y, 12, gfx.color565(255, 0, 0));  // Red, r=12
      dot_x = mapped_x;
      dot_y = mapped_y;
      dotActive = true;
      lastDotDrawn = millis();
    }
  }

  // Erase dot after 500 ms (slightly larger black circle to cover fully)
  if (dotActive && millis() - lastDotDrawn >= 500) {
    gfx.fillCircle(dot_x, dot_y, 14, 0x0000);
    dotActive = false;
  }

  // Future: heater control, touch actions, safety, etc.
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