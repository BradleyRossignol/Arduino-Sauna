#include <Arduino_GigaDisplay_GFX.h>
#include "WiFiManager.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "UI.h"

GigaDisplay_GFX gfx;
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -28800, 60000);

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temp1C = -127.0f;
float temp1F = -127.0f;
float temp2C = -127.0f;
float temp2F = -127.0f;

void setup() {
  uiInit();  // Sets up display and shows "Connecting..."

  bool success = wifi.begin();

  gfx.fillScreen(0x0000);
  if (success && wifi.isConnected()) {
    gfx.setTextSize(3);
    gfx.setCursor(60, 180);
    gfx.print("Syncing time...");
    timeClient.begin();
    timeClient.update();

    sensors.begin();

    gfx.fillScreen(0x0000);
    gfx.setTextSize(3);
    gfx.setCursor(60, 220);
    gfx.print("DS18B20 sensors ready");
    delay(1500);
    gfx.fillScreen(0x0000);
  } else {
    gfx.setTextColor(0xF800);
    centerText("WiFi Connection Failed", 400, 240);
    while (true) delay(1000);
  }
}

void loop() {
  timeClient.update();

  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead >= 5000) {
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

// Helpers stay here
String getMacString() {
  byte mac[6];
  WiFi.macAddress(mac);
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String getFormattedDateTime() {
  if (!timeClient.isTimeSet()) return "Not synced";
  unsigned long epoch = timeClient.getEpochTime();
  // ... (keep your full date formatting code here from original)
  // For brevity, paste your original implementation
  // Example placeholder:
  char buf[32];
  snprintf(buf, sizeof(buf), "%s", timeClient.getFormattedTime().c_str());
  return String(buf);
}