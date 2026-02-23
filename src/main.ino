/*
 * main.ino - Entry point for Giga R1 Sauna Controller with Giga Display
 *
 * Responsibilities:
 *   - Initialize hardware (display, WiFi, NTP, DS18B20)
 *   - Handle WiFi connection with on-screen error if failed
 *   - Periodically read temperatures and update UI
 *   - Delegate all drawing / touch to UI module
 */



#include <Arduino_GigaDisplay_GFX.h>
#include "WiFiManager.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "UI.h"
#include "Config.h" // For configuration constants


GigaDisplay_GFX gfx;
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_TIME_OFFSET_SEC, NTP_UPDATE_INTERVAL_MS);

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

float temp1C = INVALID_TEMPERATURE_C;
float temp1F = INVALID_TEMPERATURE_F;
float temp2C = INVALID_TEMPERATURE_C;
float temp2F = INVALID_TEMPERATURE_F;

void setup() {
  uiInit();  // Sets up display and shows "Connecting..."

  bool success = wifi.begin();

  gfx.fillScreen(0x0000);
  if (success && wifi.isConnected()) {
    gfx.setTextSize(STARTUP_TEXT_SIZE);
    gfx.setCursor(STARTUP_CURSOR_X, SYNC_TIME_Y);
    gfx.print("Syncing time...");
    timeClient.begin();
    timeClient.update();

    sensors.begin();

    gfx.fillScreen(0x0000);
    gfx.setTextSize(STARTUP_TEXT_SIZE);
    gfx.setCursor(STARTUP_CURSOR_X, SENSORS_READY_Y);
    gfx.print("DS18B20 sensors ready");
    delay(STARTUP_MESSAGE_DELAY_MS);
    gfx.fillScreen(0x0000);
  } else {
    gfx.setTextColor(ERROR_TEXT_COLOR_565);
    centerText("WiFi Connection Failed", CENTER_TEXT_X, CENTER_TEXT_Y);
    while (true) delay(1000);
  }
}

void loop() {
  timeClient.update();

  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = millis();

    sensors.requestTemperatures();
    temp1C = sensors.getTempCByIndex(0);
    temp1F = sensors.getTempFByIndex(0);
    temp2C = sensors.getTempCByIndex(1);
    temp2F = sensors.getTempFByIndex(1);

    // Feed to UI
    uiSetTemp1(temp1C, temp1F);
    uiSetTemp2(temp2C, temp2F);
    uiSetWifiInfo(WiFi.SSID(), WiFi.localIP().toString(), getMacString());
    uiSetTime(getFormattedDateTime());

    uiUpdate();
  }
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

  char buf[20];  // Enough for "YYYY-MM-DD HH:MM:SS\0"
  snprintf(buf, sizeof(buf), TIME_FORMAT_PATTERN, year, month, day, hour, minute, second);

  return String(buf);
}

// Helper: Convert Unix epoch seconds to broken-down date/time (UTC, then offset applied by NTPClient)
void epochToDateTime(unsigned long epoch, int& year, int& month, int& day, int& hour, int& minute, int& second) {
  second = epoch % 60;
  epoch /= 60;
  minute = epoch % 60;
  epoch /= 60;
  hour   = epoch % 24;
  epoch /= 24;

  year = 1970;
  while (true) {
    unsigned long daysInYear = 365;
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
      daysInYear = 366;
    }
    if (epoch < daysInYear) break;
    epoch -= daysInYear;
    year++;
  }

  static const uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  month = 0;
  while (true) {
    uint8_t dim = daysInMonth[month];
    if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
      dim = 29;  // February leap
    }
    if (epoch < dim) break;
    epoch -= dim;
    month++;
  }
  month++;  // 1-based
  day = epoch + 1;  // 1-based
}