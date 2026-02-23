// src/UI.cpp
#include "UI.h"

// Color definitions (private to this file)
#define COLOR_TURQUOISE 0x07FF
#define COLOR_WARNING   0xF800
#define COLOR_LABEL     0xC618
#define COLOR_VALUE     COLOR_TURQUOISE

#define COLOR_VERYCOLD  0x07FF
#define COLOR_COLD      0xFFFF
#define COLOR_WARM      0xFFE0
#define COLOR_HOT       0x07E0
#define COLOR_VERYHOT   0xF800

// Private UI state (updated via setters from main.ino)
static float _temp1C = -127.0f;
static float _temp1F = -127.0f;
static float _temp2C = -127.0f;
static float _temp2F = -127.0f;

static String _wifiSSID = "";
static String _wifiIP   = "";
static String _wifiMAC  = "";
static String _timeStr  = "Not synced";

// Refresh control
static unsigned long lastRefresh = 0;
static const unsigned long REFRESH_INTERVAL = 5000;

// Forward declaration for private helper function
static void drawAllInfo();

// Helper function: temperature-based color
static uint16_t getTempColor(float tempF) {
  if (tempF < 60.0f)        return COLOR_VERYCOLD;
  else if (tempF < 100.0f)  return COLOR_COLD;
  else if (tempF < 160.0f)  return COLOR_WARM;
  else if (tempF < 200.0f)  return COLOR_HOT;
  else                      return COLOR_VERYHOT;
}

// ────────────────────────────────────────────────
// Public functions (declared in UI.h)
// ────────────────────────────────────────────────

void uiInit() {
  gfx.begin();
  gfx.setRotation(1);
  gfx.fillScreen(0x0000);
  gfx.setTextColor(COLOR_TURQUOISE);
  gfx.setTextSize(3);
  gfx.setCursor(60, 100);
  gfx.print("Connecting to WiFi...");
}

void uiUpdate() {
  unsigned long now = millis();
  if (now - lastRefresh >= REFRESH_INTERVAL) {
    lastRefresh = now;
    drawAllInfo();
  }
}

// Setters – called from main.ino when values change
void uiSetTemp1(float c, float f) {
  _temp1C = c;
  _temp1F = f;
}

void uiSetTemp2(float c, float f) {
  _temp2C = c;
  _temp2F = f;
}

void uiSetWifiInfo(const String& ssid, const String& ip, const String& mac) {
  _wifiSSID = ssid;
  _wifiIP   = ip;
  _wifiMAC  = mac;
}

void uiSetTime(const String& timeStr) {
  _timeStr = timeStr;
}

// ────────────────────────────────────────────────
// Private drawing helpers
// ────────────────────────────────────────────────

static void drawAllInfo() {
  gfx.fillScreen(0x0000);

  int y = 25;
  int labelX = 35;
  int valueX = 420;
  int lineHeight = 42;

  // Title
  gfx.setTextSize(4);
  gfx.setTextColor(COLOR_TURQUOISE);
  centerText("Sauna Controller", 400, y);
  y += 55;

  // WiFi information
  gfx.setTextSize(3);
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("WiFi Network:");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(_wifiSSID);
  y += lineHeight;

  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("IP Address:");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(_wifiIP);
  y += lineHeight;

  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("MAC Address:");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(_wifiMAC);
  y += lineHeight + 20;

  // Time
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("Time (PST):");
  gfx.setTextColor(COLOR_VALUE);
  gfx.setCursor(valueX, y);
  gfx.print(_timeStr);
  y += lineHeight + 30;

  // Sensor Data header
  gfx.setTextSize(4);
  gfx.setTextColor(COLOR_TURQUOISE);
  gfx.setCursor(labelX, y);
  gfx.print("Sensor Data");
  y += lineHeight - 5;

  // Sensor #1
  gfx.setTextSize(3);
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("Temp Sensor #1:");

  if (_temp1C == -127.0f) {
    gfx.setTextColor(COLOR_WARNING);
    gfx.setCursor(valueX, y);
    gfx.print("Error");
  } else {
    uint16_t col = getTempColor(_temp1F);
    gfx.setTextColor(col);

    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f", _temp1C);
    gfx.setCursor(valueX, y);
    gfx.print(buf);

    int16_t x1; uint16_t w, h; int16_t y1;
    gfx.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    int degCX = valueX + w + 8;
    int degY = y + 4;
    gfx.fillCircle(degCX + 6, degY - 6, 6, col);
    gfx.fillCircle(degCX + 6, degY - 6, 4, 0x0000);
    gfx.setCursor(degCX + 16, y);
    gfx.print("C");

    snprintf(buf, sizeof(buf), "%.1f", _temp1F);
    int fStartX = degCX + 16 + 40;
    gfx.setCursor(fStartX, y);
    gfx.print(buf);

    gfx.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    int degFX = fStartX + w + 8;
    gfx.fillCircle(degFX + 6, degY - 6, 6, col);
    gfx.fillCircle(degFX + 6, degY - 6, 4, 0x0000);
    gfx.setCursor(degFX + 16, y);
    gfx.print("F");
  }
  y += lineHeight - 5;

  // Sensor #2
  gfx.setTextColor(COLOR_LABEL);
  gfx.setCursor(labelX, y);
  gfx.print("Temp Sensor #2:");

  if (_temp2C == -127.0f) {
    gfx.setTextColor(COLOR_WARNING);
    gfx.setCursor(valueX, y);
    gfx.print("Error");
  } else {
    uint16_t col = getTempColor(_temp2F);
    gfx.setTextColor(col);

    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f", _temp2C);
    gfx.setCursor(valueX, y);
    gfx.print(buf);

    int16_t x1; uint16_t w, h; int16_t y1;
    gfx.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    int degCX = valueX + w + 8;
    int degY = y + 4;
    gfx.fillCircle(degCX + 6, degY - 6, 6, col);
    gfx.fillCircle(degCX + 6, degY - 6, 4, 0x0000);
    gfx.setCursor(degCX + 16, y);
    gfx.print("C");

    snprintf(buf, sizeof(buf), "%.1f", _temp2F);
    int fStartX = degCX + 16 + 40;
    gfx.setCursor(fStartX, y);
    gfx.print(buf);

    gfx.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    int degFX = fStartX + w + 8;
    gfx.fillCircle(degFX + 6, degY - 6, 6, col);
    gfx.fillCircle(degFX + 6, degY - 6, 4, 0x0000);
    gfx.setCursor(degFX + 16, y);
    gfx.print("F");
  }
}

void centerText(const char* text, int x, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  gfx.setCursor(x - w / 2, y - h / 2);
  gfx.print(text);
}