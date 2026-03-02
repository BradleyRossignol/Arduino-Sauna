#include "UI.h"
#include "Debug.h"
#include "Config.h"             // For SCREEN_WIDTH, colors, UI constants, TEMP_*, etc.
#include <Arduino.h>
#include <DallasTemperature.h>  // For DEVICE_DISCONNECTED_C

#include "SensorManager.h"      // For hadRecentFallback()
extern SensorManager sensorManager;  // Global from main.ino

// ── Private UI state ───────────────────────────────────────────────────────
static float _temp1C = TEMP_DISCONNECTED_C;
static float _temp1F = TEMP_DISCONNECTED_C;
static float _temp2C = TEMP_DISCONNECTED_C;
static float _temp2F = TEMP_DISCONNECTED_C;

static String _wifiSSID = "";
static String _wifiIP   = "";
static String _wifiMAC  = "";
static String _timeStr  = "Not synced";

static unsigned long lastRefresh = 0;

// ── Private helpers ────────────────────────────────────────────────────────

static uint16_t getTempColor(float tempF) {
    if (tempF < TEMP_THRESHOLD_VERY_COLD) return COLOR_VERY_COLD;
    else if (tempF < TEMP_THRESHOLD_COLD) return COLOR_COLD;
    else if (tempF < TEMP_THRESHOLD_WARM) return COLOR_WARM;
    else if (tempF < TEMP_THRESHOLD_HOT)  return COLOR_HOT;
    else                                  return COLOR_VERY_HOT;
}

static void drawSensor(const char* label, float tempC, float tempF, int yPos) {
    gfx.setTextSize(UI_NORMAL_SIZE);
    gfx.setTextColor(COLOR_LABEL);
    gfx.setCursor(UI_LABEL_X, yPos);
    gfx.print(label);

    if (tempC == TEMP_DISCONNECTED_C) {
        gfx.setTextColor(COLOR_WARNING);
        gfx.setCursor(UI_VALUE_X, yPos);
        gfx.print("Disconnected");
        return;
    }

    uint16_t col = getTempColor(tempF);
    gfx.setTextColor(col);

    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f", tempC);
    gfx.setCursor(UI_VALUE_X, yPos);
    gfx.print(buf);

    // Degree C symbol
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    gfx.getTextBounds(buf, 0, 0, &tbx, &tby, &tbw, &tbh);
    int degCX = UI_VALUE_X + tbw + 8;
    int degY  = yPos + 4;
    gfx.fillCircle(degCX + 6, degY - 6, 6, col);
    gfx.fillCircle(degCX + 6, degY - 6, 4, 0x0000);
    gfx.setCursor(degCX + 16, yPos);
    gfx.print("C");

    // Fahrenheit
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

static void drawAllInfo() {
    gfx.fillScreen(0x0000);

    int y = UI_TITLE_Y;

    // Title
    gfx.setTextSize(UI_TITLE_SIZE);
    gfx.setTextColor(COLOR_TURQUOISE);
    centerText("Sauna Controller", UI_CENTER_X, y);
    y += 55;

    // WiFi information
    gfx.setTextSize(UI_NORMAL_SIZE);
    gfx.setTextColor(COLOR_LABEL);
    gfx.setCursor(UI_LABEL_X, y);
    gfx.print("WiFi Network:");
    gfx.setTextColor(COLOR_VALUE);
    gfx.setCursor(UI_VALUE_X, y);
    gfx.print(_wifiSSID);
    y += UI_LINE_HEIGHT;

    gfx.setTextColor(COLOR_LABEL);
    gfx.setCursor(UI_LABEL_X, y);
    gfx.print("IP Address:");
    gfx.setTextColor(COLOR_VALUE);
    gfx.setCursor(UI_VALUE_X, y);
    gfx.print(_wifiIP);
    y += UI_LINE_HEIGHT;

    gfx.setTextColor(COLOR_LABEL);
    gfx.setCursor(UI_LABEL_X, y);
    gfx.print("MAC Address:");
    gfx.setTextColor(COLOR_VALUE);
    gfx.setCursor(UI_VALUE_X, y);
    gfx.print(_wifiMAC);
    y += UI_LINE_HEIGHT + 20;

    // Time
    gfx.setTextColor(COLOR_LABEL);
    gfx.setCursor(UI_LABEL_X, y);
    gfx.print("Time (PST):");
    gfx.setTextColor(COLOR_VALUE);
    gfx.setCursor(UI_VALUE_X, y);
    gfx.print(_timeStr);
    y += UI_LINE_HEIGHT + 30;

    // Sensor Data header
    gfx.setTextSize(UI_TITLE_SIZE);
    gfx.setTextColor(COLOR_TURQUOISE);
    gfx.setCursor(UI_LABEL_X, y);
    gfx.print("Sensor Data");
    y += UI_LINE_HEIGHT + UI_SENSOR_Y_GAP;

    drawSensor("Temp Sensor #1:", _temp1C, _temp1F, y);
    y += UI_LINE_HEIGHT + UI_SENSOR_Y_GAP;

    drawSensor("Temp Sensor #2:", _temp2C, _temp2F, y);

    // ── Async sensor health indicator (top-right corner) ─────────────────────
    const int statusX = SCREEN_WIDTH - 160;
    const int statusY = 15;
    const int statusW = 150;
    const int statusH = 25;

    if (sensorManager.hadRecentFallback()) {
        gfx.setTextSize(UI_NORMAL_SIZE - 1);
        gfx.setTextColor(COLOR_WARNING);
        gfx.setCursor(statusX, statusY);
        gfx.print("Sensor Recovery");
    } else {
        gfx.fillRect(statusX, statusY - 5, statusW, statusH + 10, 0x0000);
    }
}

// ── Public API ─────────────────────────────────────────────────────────────

void uiInit() {
    gfx.begin();
    gfx.setRotation(1);
    gfx.fillScreen(0x0000);
    gfx.setTextColor(COLOR_TURQUOISE);
    gfx.setTextSize(UI_NORMAL_SIZE);
    gfx.setCursor(60, 100);
    gfx.print("Connecting to WiFi...");

    LOG_INFO("UI initialized (landscape rotation set)");
}

void uiUpdate() {
    unsigned long now = millis();
    if (now - lastRefresh >= UI_REFRESH_MS) {
        lastRefresh = now;
        LOG_VERBOSE("UI refresh cycle");
        drawAllInfo();
    }
}

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

void centerText(const char* text, int x, int y) {
    int16_t x1, y1;
    uint16_t w, h;
    gfx.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    gfx.setCursor(x - w / 2, y - h / 2);
    gfx.print(text);
}