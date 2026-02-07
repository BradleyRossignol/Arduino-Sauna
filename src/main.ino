/*
 * Sauna Controller - List-style Display Layout
 * °C and °F on same line, fixed crowding for °F
 * Arduino GIGA R1 WiFi + Giga Display Shield
 */
#include <Arduino_GigaDisplay_GFX.h>
#include "WiFiManager.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

GigaDisplay_GFX gfx;
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -28800, 60000); // UTC-8 = PST

#define COLOR_TURQUOISE 0x07FF
#define COLOR_WARNING   0xF800
#define COLOR_LABEL     0xC618  // Light gray/silver for labels
#define COLOR_VALUE     COLOR_TURQUOISE

// Temperature color thresholds (based on °F)
#define COLOR_VERYCOLD  0x07FF  // Turquoise  < 60 °F
#define COLOR_COLD      0xFFFF  // White      60–99 °F
#define COLOR_WARM      0xFFE0  // Yellow    100–159 °F
#define COLOR_HOT       0x07E0  // Green     160–199 °F
#define COLOR_VERYHOT   0xF800  // Red       ≥ 200 °F

// DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temp1C = -127.0;
float temp1F = -127.0;
float temp2C = -127.0;
float temp2F = -127.0;

uint16_t getTempColor(float tempF) {
  if (tempF < 60.0)        return COLOR_VERYCOLD;
  else if (tempF < 100.0)  return COLOR_COLD;
  else if (tempF < 160.0)  return COLOR_WARM;
  else if (tempF < 200.0)  return COLOR_HOT;
  else                     return COLOR_VERYHOT;
}

void setup() {
  gfx.begin();
  gfx.setRotation(1);
  gfx.fillScreen(0x0000);
  gfx.setTextColor(COLOR_TURQUOISE);
  gfx.setTextSize(3);
  gfx.setCursor(60, 100);
  gfx.print("Connecting to WiFi...");

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
    gfx.setTextColor(COLOR_WARNING);
    centerText("WiFi Connection Failed", 400, 240);
    while (true) delay(1000);
  }
}

void loop() {
  static unsigned long lastRefresh = 0;
  if (millis() - lastRefresh >= 5000) {
    lastRefresh = millis();

    sensors.requestTemperatures();
    temp1C = sensors.getTempCByIndex(0);
    temp1F = sensors.getTempFByIndex(0);
    temp2C = sensors.getTempCByIndex(1);
    temp2F = sensors.getTempFByIndex(1);

    drawAllInfo();
  }

  timeClient.update();
}

void drawAllInfo() {
  gfx.fillScreen(0x0000);

  int y = 25;           // Start a bit higher
  int labelX = 35;      // Slightly left
  int valueX = 420;     // Values stay right-aligned
  int lineHeight = 42;  // Reduced spacing

  // ── Title ──────────────────────────────────────────────
  gfx.setTextSize(4);
  gfx.setTextColor(COLOR_TURQUOISE);
  centerText("Sauna Controller", 400, y);
  y += 55;              // Reduced gap after title

  // ── WiFi Info ──────────────────────────────────────────
  gfx.setTextSize(3);
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("WiFi Network:");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(WiFi.SSID());
  y += lineHeight;

  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("IP Address:");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(WiFi.localIP().toString());
  y += lineHeight;

  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("MAC Address:");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(getMacString());
  y += lineHeight + 20; // Smaller gap before time

  // ── Time ───────────────────────────────────────────────
  gfx.setTextSize(3);
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("Time (PST):");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(getFormattedDateTime());
  y += lineHeight + 30; // Gap before sensors

  // ── Sensor Data ────────────────────────────────────────
  gfx.setTextSize(4);
  gfx.setTextColor(COLOR_TURQUOISE);
  gfx.setCursor(labelX, y);
  gfx.print("Sensor Data");
  y += lineHeight - 5;  // Tighter

  // Temp Sensor #1
  gfx.setTextSize(3);   // Smaller label for better fit
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("Temp Sensor #1:");

  if (temp1C == DEVICE_DISCONNECTED_C || temp1C == -127.0) {
    gfx.setTextColor(COLOR_WARNING);
    gfx.setTextSize(3);
    gfx.setCursor(valueX, y);
    gfx.print("Error");
  } else {
    uint16_t tempColor = getTempColor(temp1F);
    gfx.setTextColor(tempColor);
    gfx.setTextSize(3);

    char tempStr[10];
    snprintf(tempStr, sizeof(tempStr), "%.1f", temp1C);
    gfx.setCursor(valueX, y);
    gfx.print(tempStr);

    int16_t x1, y1;
    uint16_t w, h;
    gfx.getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);

    int degCX = valueX + w + 8;
    int degY = y + 4;
    gfx.fillCircle(degCX + 6, degY - 6, 6, tempColor);
    gfx.fillCircle(degCX + 6, degY - 6, 4, 0x0000);

    gfx.setCursor(degCX + 16, y);
    gfx.print("C");

    char fStr[10];
    snprintf(fStr, sizeof(fStr), "%.1f", temp1F);
    int fStartX = degCX + 16 + 40;  // Slightly tighter spacing
    gfx.setCursor(fStartX, y);
    gfx.print(fStr);

    gfx.getTextBounds(fStr, 0, 0, &x1, &y1, &w, &h);

    int degFX = fStartX + w + 8;
    gfx.fillCircle(degFX + 6, degY - 6, 6, tempColor);
    gfx.fillCircle(degFX + 6, degY - 6, 4, 0x0000);

    gfx.setCursor(degFX + 16, y);
    gfx.print("F");
  }

  y += lineHeight - 5;  // Tighter spacing between sensors

  // Temp Sensor #2
  gfx.setTextSize(3);
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("Temp Sensor #2:");

  if (temp2C == DEVICE_DISCONNECTED_C || temp2C == -127.0) {
    gfx.setTextColor(COLOR_WARNING);
    gfx.setTextSize(3);
    gfx.setCursor(valueX, y);
    gfx.print("Error");
  } else {
    uint16_t tempColor = getTempColor(temp2F);
    gfx.setTextColor(tempColor);
    gfx.setTextSize(3);

    char tempStr[10];
    snprintf(tempStr, sizeof(tempStr), "%.1f", temp2C);
    gfx.setCursor(valueX, y);
    gfx.print(tempStr);

    int16_t x1, y1;
    uint16_t w, h;
    gfx.getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);

    int degCX = valueX + w + 8;
    int degY = y + 4;
    gfx.fillCircle(degCX + 6, degY - 6, 6, tempColor);
    gfx.fillCircle(degCX + 6, degY - 6, 4, 0x0000);

    gfx.setCursor(degCX + 16, y);
    gfx.print("C");

    char fStr[10];
    snprintf(fStr, sizeof(fStr), "%.1f", temp2F);
    int fStartX = degCX + 16 + 40;
    gfx.setCursor(fStartX, y);
    gfx.print(fStr);

    gfx.getTextBounds(fStr, 0, 0, &x1, &y1, &w, &h);

    int degFX = fStartX + w + 8;
    gfx.fillCircle(degFX + 6, degY - 6, 6, tempColor);
    gfx.fillCircle(degFX + 6, degY - 6, 4, 0x0000);

    gfx.setCursor(degFX + 16, y);
    gfx.print("F");
  }

  // If you later add more elements, consider reducing lineHeight further to 38–40
}

void centerText(const char* text, int x, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  gfx.setCursor(x - w / 2, y - h / 2);
  gfx.print(text);
}

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
  unsigned long daysSince1970 = epoch / 86400UL;
  unsigned long secondsToday = epoch % 86400UL;

  unsigned long year = 1970;
  while (daysSince1970 >= 365) {
    bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    unsigned long daysInYear = leap ? 366 : 365;
    if (daysSince1970 >= daysInYear) {
      daysSince1970 -= daysInYear;
      year++;
    } else break;
  }

  const uint8_t daysInMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
  uint8_t month = 1;
  uint8_t dayOfMonth = 1;

  while (daysSince1970 > 0) {
    uint8_t daysThisMonth = daysInMonth[month-1];
    if (month == 2 && leap) daysThisMonth = 29;
    if (daysSince1970 < daysThisMonth) {
      dayOfMonth = daysSince1970 + 1;
      break;
    }
    daysSince1970 -= daysThisMonth;
    month++;
  }

  int hour = secondsToday / 3600;
  int minute = (secondsToday % 3600) / 60;
  int second = secondsToday % 60;

  char buf[32];
  const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  snprintf(buf, sizeof(buf), "%s %02d %04lu %02d:%02d:%02d",
           months[month-1], dayOfMonth, year, hour, minute, second);
  return String(buf);
}