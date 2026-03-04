/*
 * src/main.ino - Main entry point for Giga R1 Sauna Controller + Giga Display
 *
 * Phase 2 Step 4: Fixed "WiFi Failed" showing on successful connect
 * (use return value of wifi.begin() instead of immediate isConnected() check)
 * All previous menu + Sensor Data features preserved.
 */

#include <Arduino.h>
#include <Arduino_GigaDisplay_GFX.h>
#include <Arduino_GigaDisplayTouch.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "Config.h"
#include "Debug.h"
#include "WiFiManager.h"
#include "SensorManager.h"
#include "UI.h"
#include "TimeUtils.h"

// ── Global Objects ─────────────────────────────────────────────────────────
GigaDisplay_GFX gfx;
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, TimeConfig::OFFSET_SEC, NTP_UPDATE_INTERVAL_MS);

SensorManager sensorManager;
Arduino_GigaDisplayTouch touchDetector;

// ── UI State Machine ───────────────────────────────────────────────────────
enum UiState { MAIN, MENU, NETWORK_INFO, SENSOR_INFO };
static UiState currentState = MAIN;
static bool stateChanged = true;

// ── Hamburger Icon (turquoise, on EVERY screen) ────────────────────────────
void drawHamburgerIcon() {
  gfx.fillRect(730, 20, 50, 8, COLOR_TURQUOISE);
  gfx.fillRect(730, 40, 50, 8, COLOR_TURQUOISE);
  gfx.fillRect(730, 60, 50, 8, COLOR_TURQUOISE);
}

// ── Common header for info screens (turquoise) ─────────────────────────────
void drawInfoHeader(const char* title) {
  gfx.setTextColor(COLOR_TURQUOISE);
  gfx.setTextSize(UI_TITLE_SIZE);
  gfx.setCursor(120, 40);
  gfx.print(title);
}

// ── Ported from UI.cpp: full main-page temperature functionality ───────────
static uint16_t getTempColor(float tempF) {
  if (tempF < TEMP_THRESHOLD_VERY_COLD) return COLOR_VERY_COLD;
  else if (tempF < TEMP_THRESHOLD_COLD) return COLOR_COLD;
  else if (tempF < TEMP_THRESHOLD_WARM) return COLOR_WARM;
  else if (tempF < TEMP_THRESHOLD_HOT)  return COLOR_HOT;
  else                                  return COLOR_VERY_HOT;
}

static void drawSensorOnInfo(const char* label, float tempC, float tempF, int yPos) {
  gfx.setTextSize(UI_NORMAL_SIZE);
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(80, yPos);
  gfx.print(label);

  if (tempC == TEMP_DISCONNECTED_C) {
    gfx.setTextColor(COLOR_WARNING);
    gfx.setCursor(420, yPos);
    gfx.print("Disconnected");
    return;
  }

  uint16_t col = getTempColor(tempF);
  gfx.setTextColor(col);

  char buf[10];
  snprintf(buf, sizeof(buf), "%.1f", tempC);
  gfx.setCursor(420, yPos);
  gfx.print(buf);

  int16_t tbx, tby;
  uint16_t tbw, tbh;
  gfx.getTextBounds(buf, 0, 0, &tbx, &tby, &tbw, &tbh);
  int degCX = 420 + tbw + 8;
  int degY  = yPos + 4;
  gfx.fillCircle(degCX + 6, degY - 6, 6, col);
  gfx.fillCircle(degCX + 6, degY - 6, 4, 0x0000);
  gfx.setCursor(degCX + 16, yPos);
  gfx.print("C");

  snprintf(buf, sizeof(buf), "%.1f", tempF);
  int fStartX = degCX + 16 + 40;
  gfx.setCursor(fStartX, yPos);
  gfx.print(buf);

  gfx.getTextBounds(buf, 0, 0, &tbx, &tby, &tbw, &tbh);
  int degFX = fStartX + tbw + 8;
  gfx.fillCircle(degFX + 6, degY - 6, 6, col);
  gfx.fillCircle(degFX + 6, degY - 6, 4, 0x0000);
  gfx.setCursor(degFX + 16, yPos);
  gfx.print("F");
}

// ── Helper prototypes ──────────────────────────────────────────────────────
String getMacString();
String getFormattedDateTime();

void setup() {
  uiInit();
  DEBUG_INIT(115200);

  LOG_INFO(String(F("Sauna Controller starting")));
  LOG_INFO(String(F("Build: ")) + String(__DATE__) + " " + String(__TIME__));
  LOG_INFO(String(F("Debug level: ")) + String(DEBUG_LEVEL));

  // PHASE 2 Step 4: Use return value of begin() to avoid race condition
  bool wifiSuccess = wifi.begin();

  gfx.fillScreen(0x0000);

  if (wifiSuccess) {
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

  // ── ALWAYS update sensors (required for live Sensor Info screen) ──────────
  sensorManager.update();

  // ── Sensor & UI updates (MAIN screen only) ───────────────────────────────
  if (currentState == MAIN) {
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
      lastSensorRead = millis();

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
      drawHamburgerIcon();
    }
  }

  // ── Live refresh for SENSOR_INFO screen ──────────────────────────────────
  static unsigned long lastSensorInfoRefresh = 0;
  if (currentState == SENSOR_INFO && millis() - lastSensorInfoRefresh >= UI_REFRESH_MS) {
    lastSensorInfoRefresh = millis();

    gfx.fillRect(0, 100, SCREEN_WIDTH, 280, 0x0000);

    const SaunaTemperatures& temps = sensorManager.getTemperatures();

    drawSensorOnInfo("Temp Sensor #1:", temps.sauna,
                     (temps.sauna != TEMP_DISCONNECTED_C) ? (temps.sauna * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F, 130);

    drawSensorOnInfo("Temp Sensor #2:", temps.heater,
                     (temps.heater != TEMP_DISCONNECTED_C) ? (temps.heater * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F, 200);

    LOG_VERBOSE(F("SENSOR_INFO live refresh"));
  }

  // ── Touch handling with landscape coordinate mapping ─────────────────────
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

      if (mapped_x > 700 && mapped_y < 90) {
        if (currentState == MENU) {
          currentState = MAIN;
          stateChanged = true;
          LOG_INFO(F("Hamburger → MAIN"));
        } else {
          currentState = MENU;
          stateChanged = true;
          LOG_INFO(F("Hamburger opened MENU"));
        }
      }
      else if (currentState == MENU) {
        if (mapped_x < 400 && mapped_y > 140 && mapped_y < 240) {
          currentState = NETWORK_INFO;
          stateChanged = true;
          LOG_INFO(F("Network Info selected"));
        } else if (mapped_x > 400 && mapped_y > 140 && mapped_y < 240) {
          currentState = SENSOR_INFO;
          stateChanged = true;
          LOG_INFO(F("Sensor Data selected"));
        }
      } else if (currentState == NETWORK_INFO || currentState == SENSOR_INFO) {
        if (mapped_y > 400 && mapped_x > 200 && mapped_x < 600) {
          currentState = MENU;
          stateChanged = true;
          LOG_INFO(F("Back to MENU"));
        }
      }
    }
  }

  if (dotActive && millis() - lastDotDrawn >= 500) {
    gfx.fillCircle(dot_x, dot_y, 10, 0x0000);
    dotActive = false;
  }

  // ── Redraw on state change ───────────────────────────────────────────────
  if (stateChanged) {
    gfx.fillScreen(0x0000);

    if (currentState == MAIN) {
      uiUpdate();
      drawHamburgerIcon();
      LOG_INFO(F("MAIN redrawn + hamburger"));
    }
    else if (currentState == MENU) {
      gfx.setTextColor(COLOR_TURQUOISE);
      gfx.setTextSize(UI_TITLE_SIZE);
      gfx.setCursor(280, 40);
      gfx.print("MENU");

      gfx.fillRoundRect(80, 140, 300, 100, 20, gfx.color565(0, 120, 255));
      gfx.setTextColor(0xFFFF);
      gfx.setTextSize(UI_NORMAL_SIZE);
      gfx.setCursor(120, 180);
      gfx.print("Network Info");

      gfx.fillRoundRect(420, 140, 300, 100, 20, gfx.color565(0, 180, 0));
      gfx.setCursor(470, 180);
      gfx.print("Sensor Data");

      drawHamburgerIcon();
      LOG_INFO(F("MENU redrawn - turquoise title"));
    }
    else if (currentState == NETWORK_INFO) {
      drawInfoHeader("Network Information");
      drawHamburgerIcon();

      gfx.setTextColor(0xFFFF);
      gfx.setTextSize(UI_NORMAL_SIZE);
      int y = 130;

      gfx.setCursor(80, y); y += 45;
      gfx.print("SSID: "); gfx.print(WiFi.SSID());

      gfx.setCursor(80, y); y += 45;
      gfx.print("IP: "); gfx.print(WiFi.localIP());

      gfx.setCursor(80, y); y += 45;
      gfx.print("MAC: "); gfx.print(getMacString());

      gfx.setCursor(80, y); y += 45;
      gfx.print("Status: ");
      gfx.setTextColor(wifi.isConnected() ? COLOR_TURQUOISE : gfx.color565(255, 100, 0));
      gfx.print(wifi.isConnected() ? "Connected" : "Disconnected");

      gfx.fillRoundRect(200, 400, 400, 60, 15, COLOR_TURQUOISE);
      gfx.setTextColor(0x0000);
      gfx.setTextSize(UI_NORMAL_SIZE);
      gfx.setCursor(320, 420);
      gfx.print("Back");

      LOG_INFO(F("NETWORK_INFO redrawn"));
    }
    else if (currentState == SENSOR_INFO) {
      drawInfoHeader("Sensor Data");
      drawHamburgerIcon();

      const SaunaTemperatures& temps = sensorManager.getTemperatures();

      drawSensorOnInfo("Temp Sensor #1:", temps.sauna,
                       (temps.sauna != TEMP_DISCONNECTED_C) ? (temps.sauna * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F, 130);

      drawSensorOnInfo("Temp Sensor #2:", temps.heater,
                       (temps.heater != TEMP_DISCONNECTED_C) ? (temps.heater * 9.0/5.0 + 32.0) : TEMP_DISCONNECTED_F, 200);

      gfx.fillRoundRect(200, 400, 400, 60, 15, COLOR_TURQUOISE);
      gfx.setTextColor(0x0000);
      gfx.setTextSize(UI_NORMAL_SIZE);
      gfx.setCursor(320, 420);
      gfx.print("Back");

      LOG_INFO(F("SENSOR_INFO initial draw"));
    }

    stateChanged = false;
  }
}

// ── Helpers ────────────────────────────────────────────────────────────────
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