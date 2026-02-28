// src/WiFiManager.cpp
#include "WiFiManager.h"
#include <WiFi.h> // GIGA uses Mbed WiFi.h
#include "Config.h" // For TimeConfig::OFFSET_SEC, NTP_SERVER, etc.
#include "arduino_secrets.h" // SECRET_SSID, SECRET_PASS

#include <NTPClient.h>   // ← Added: Defines NTPClient class (fixes "does not name a type")
#include <WiFiUdp.h>     // ← Added: Defines WiFiUDP (for ntpUDP extern)

// If using NTPClient lib (make sure it's in lib_deps or #include <NTPClient.h>)
// For now, assume it's global/extern from main.ino
extern WiFiUDP ntpUDP;
extern NTPClient timeClient; // Fix: make sure declared extern if in main.ino

WiFiManager::WiFiManager() {}

bool WiFiManager::begin() {
  // No WiFi.mode(WIFI_STA) needed on GIGA – begin() puts it in client mode
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  unsigned long start = millis();
  const unsigned long timeoutMs = 15000UL;
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(500);
  }
  bool success = (WiFi.status() == WL_CONNECTED);
  if (success) {
      // NTP setup (if using NTPClient)
    timeClient.begin();
    timeClient.setTimeOffset(TimeConfig::OFFSET_SEC);
    timeClient.update();
  }
  return success;
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::maintain() {
  static unsigned long lastCheck = 0;
  unsigned long now = millis();
  if (now - lastCheck >= 15000UL) {
      lastCheck = now;
    if (!isConnected()) {
        // No reconnect() – just call begin() again (non-blocking if credentials set)
      WiFi.begin(SECRET_SSID, SECRET_PASS);
    }
  }
}

String WiFiManager::getSSID() const {
  return WiFi.SSID();
}

String WiFiManager::getIP() const {
  return WiFi.localIP().toString();
}

String WiFiManager::getMAC() const {
  byte mac[6];
  WiFi.macAddress(mac);
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}