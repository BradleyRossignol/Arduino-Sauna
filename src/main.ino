/*
 * main.ino - Entry point for Giga R1 Sauna Controller with Giga Display
 */

#include <Arduino_GigaDisplay_GFX.h>
#include "WiFiManager.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "UI.h"
#include "Config.h"
#include "SensorManager.h"
#include "TimeUtils.h"
#include "Debug.h"
#include "Arduino_GigaDisplayTouch.h"

GigaDisplay_GFX gfx;
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, TimeConfig::OFFSET_SEC, NTP_UPDATE_INTERVAL_MS);

SensorManager sensorManager;
Arduino_GigaDisplayTouch touchDetector;

// UI states
enum UiState { MAIN, MENU, NETWORK_INFO, SENSOR_INFO };
static UiState currentState = MAIN;
static bool stateChanged = true;

// Persistent menu icon
void drawMenuIcon() {
  gfx.fillRect(730, 20, 50, 8, 0xFFFF);
  gfx.fillRect(730, 40, 50, 8, 0xFFFF);
  gfx.fillRect(730, 60, 50, 8, 0xFFFF);
}

// Turquoise color constant
#define COLOR_TURQUOISE gfx.color565(0, 255, 255)

// Common header + back button in turquoise
void drawInfoHeader(const char* title) {
  gfx.setTextColor(COLOR_TURQUOISE);
  gfx.setTextSize(3);                 // Adjust size to match your preference
  gfx.setCursor(120, 40);             // Centered-ish at top
  gfx.print(title);

  // Back button in turquoise
  gfx.fillRoundRect(200, 400, 400, 60, 15, COLOR_TURQUOISE);
  gfx.setTextColor(0x0000);           // Black text on turquoise for contrast
  gfx.setTextSize(3);
  gfx.setCursor(320, 420);
  gfx.print("Back");
}

void setup() {
  uiInit();
  DEBUG_INIT(115200);

  LOG_INFO(F("Sauna Controller starting"));
  LOG_INFO(F("Build date/time: "));
  DEBUG_PRINT(__DATE__);
  DEBUG_PRINT(F(" "));
  DEBUG_PRINTLN(__TIME__);
  LOG_INFO(F("Debug level: "));
  DEBUG_PRINTLN(DEBUG_LEVEL);

  wifi.begin();

  gfx.fillScreen(0x0000);

  if (wifi.isConnected()) {
    gfx.setTextSize(STARTUP_TEXT_SIZE);
    gfx.setCursor(STARTUP_CURSOR_X, SYNC_TIME_Y);
    gfx.print("Syncing time...");
    timeClient.begin();
    timeClient.update();

    sensorManager.init();

    gfx.fillScreen(0x0000);
    gfx.setTextSize(STARTUP_TEXT_SIZE);
    gfx.setCursor(STARTUP_CURSOR_X, SENSORS_READY_Y);
    gfx.print("DS18B20 sensors ready");
    delay(STARTUP_MESSAGE_DELAY_MS);
  } else {
    gfx.setTextColor(COLOR_WARNING);
    centerText("WiFi Failed - Retrying...", CENTER_TEXT_X, CENTER_TEXT_Y);
    delay(STARTUP_MESSAGE_DELAY_MS);
    sensorManager.init();
  }

  if (touchDetector.begin()) {
    LOG_INFO(F("Touch controller initialized successfully"));
  } else {
    LOG_ERROR(F("Failed to initialize touch controller"));
  }

  gfx.fillScreen(0x0000);
}

void loop() {
  wifi.maintain();
  timeClient.update();

  // Sensor & UI updates ONLY on MAIN
  if (currentState == MAIN) {
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
      lastSensorRead = millis();

      sensorManager.update();

      const SaunaTemperatures& temps = sensorManager.getTemperatures();

      float temp1C = temps.sauna;
      float temp1F = (temp1C != TEMP_DISCONNECTED_C) ? (temp1C * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F;
      float temp2C = temps.heater;
      float temp2F = (temp2C != TEMP_DISCONNECTED_C) ? (temp2C * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F;

      uiSetTemp1(temp1C, temp1F);
      uiSetTemp2(temp2C, temp2F);
      uiSetWifiInfo(WiFi.SSID(), WiFi.localIP().toString(), getMacString());
      uiSetTime(getFormattedDateTime());

      uiUpdate();
      drawMenuIcon();
    }
  }

  // Touch polling
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

      uint16_t mapped_x = raw_y;
      uint16_t mapped_y = 479 - raw_x;

      gfx.fillCircle(mapped_x, mapped_y, 8, gfx.color565(255, 100, 0));
      dot_x = mapped_x;
      dot_y = mapped_y;
      dotActive = true;
      lastDotDrawn = millis();

      if (currentState == MAIN) {
        if (mapped_x > 700 && mapped_y < 80) {
          currentState = MENU;
          stateChanged = true;
          Serial.println(F("[INFO] Menu icon → MENU"));
        }
      } else if (currentState == MENU) {
        if (mapped_y > 400 && mapped_x > 200 && mapped_x < 600) {
          currentState = MAIN;
          stateChanged = true;
          Serial.println(F("[INFO] Back → MAIN"));
        } else if (mapped_x < 400 && mapped_y > 100 && mapped_y < 250) {
          currentState = NETWORK_INFO;
          stateChanged = true;
          Serial.println(F("[INFO] Network Info selected"));
        } else if (mapped_x > 400 && mapped_y > 100 && mapped_y < 250) {
          currentState = SENSOR_INFO;
          stateChanged = true;
          Serial.println(F("[INFO] Sensor Data selected"));
        }
      } else if (currentState == NETWORK_INFO || currentState == SENSOR_INFO) {
        if (mapped_y > 400 && mapped_x > 200 && mapped_x < 600) {
          currentState = MENU;
          stateChanged = true;
          Serial.println(F("[INFO] Back to MENU"));
        }
      }
    }
  }

  if (dotActive && millis() - lastDotDrawn >= 500) {
    gfx.fillCircle(dot_x, dot_y, 10, 0x0000);
    dotActive = false;
  }

  // Redraw on state change
  if (stateChanged) {
    gfx.fillScreen(0x0000);  // Always start with black background

    if (currentState == MAIN) {
      uiUpdate();
      drawMenuIcon();
      Serial.println(F("[INFO] MAIN redrawn + icon"));
    }
    else if (currentState == MENU) {
      gfx.setTextColor(0xFFFF);
      gfx.setTextSize(3);
      gfx.setCursor(280, 40);
      gfx.print("MENU");

      gfx.fillRoundRect(80, 100, 300, 120, 20, gfx.color565(0, 120, 255));
      gfx.setCursor(120, 145);
      gfx.setTextColor(0xFFFF);
      gfx.print("Network Info");

      gfx.fillRoundRect(420, 100, 300, 120, 20, gfx.color565(0, 180, 0));
      gfx.setCursor(460, 145);
      gfx.print("Sensor Data");

      gfx.fillRoundRect(200, 400, 400, 60, 15, gfx.color565(200, 0, 0));
      gfx.setCursor(320, 420);
      gfx.setTextColor(0xFFFF);
      gfx.print("Back");

      Serial.println(F("[INFO] MENU redrawn"));
    }
    else if (currentState == NETWORK_INFO) {
      drawInfoHeader("Network Information");

      gfx.setTextColor(0xFFFF);           // White data text (or match your main UI)
      gfx.setTextSize(2);                 // Match typical main UI size for info
      int y = 120;                        // Start below header

      gfx.setCursor(80, y); y += 40;
      gfx.print("SSID: "); gfx.print(WiFi.SSID());

      gfx.setCursor(80, y); y += 40;
      gfx.print("IP: "); gfx.print(WiFi.localIP());

      gfx.setCursor(80, y); y += 40;
      gfx.print("MAC: "); gfx.print(getMacString());

      gfx.setCursor(80, y); y += 40;
      gfx.print("Status: ");
      gfx.setTextColor(wifi.isConnected() ? COLOR_TURQUOISE : gfx.color565(255, 100, 0));
      gfx.print(wifi.isConnected() ? "Connected" : "Disconnected");

      Serial.println(F("[INFO] NETWORK_INFO redrawn - black + turquoise header"));
    }
    else if (currentState == SENSOR_INFO) {
      drawInfoHeader("Sensor Data");

      const SaunaTemperatures& temps = sensorManager.getTemperatures();

      gfx.setTextColor(0xFFFF);
      gfx.setTextSize(3);                 // Larger for emphasis, or match main size
      int y = 120;                        // Higher position after header

      gfx.setCursor(80, y); y += 60;
      gfx.print("Sauna: ");
      gfx.setTextColor(COLOR_TURQUOISE);
      gfx.print(temps.sauna, 1); gfx.print(" C");

      gfx.setTextColor(0xFFFF);
      gfx.setCursor(80, y); y += 60;
      gfx.print("Heater: ");
      gfx.setTextColor(COLOR_TURQUOISE);
      gfx.print(temps.heater, 1); gfx.print(" C");

      Serial.println(F("[INFO] SENSOR_INFO redrawn - black + turquoise header"));
    }

    stateChanged = false;
  }
}

// Helpers unchanged
String getMacString() {
  byte mac[6];
  WiFi.macAddress(mac);
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String getFormattedDateTime() {
  if (!timeClient.isTimeSet()) return "Time not synced";

  unsigned long epoch = timeClient.getEpochTime();
  int year, month, day, hour, minute, second;
  epochToDateTime(epoch, year, month, day, hour, minute, second);

  char buf[20];
  snprintf(buf, sizeof(buf), TIME_FORMAT_PATTERN, year, month, day, hour, minute, second);
  return String(buf);
}