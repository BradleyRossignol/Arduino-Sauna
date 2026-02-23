#ifndef UI_H
#define UI_H

#include <Arduino_GigaDisplay_GFX.h>

extern GigaDisplay_GFX gfx;  // global from main.ino

void uiInit();
void uiUpdate();

// Setters for data from main
void uiSetTemp1(float c, float f);
void uiSetTemp2(float c, float f);
void uiSetWifiInfo(const String& ssid, const String& ip, const String& mac);
void uiSetTime(const String& timeStr);

// Helper (public for boot error in main)
void centerText(const char* text, int x, int y);

#endif