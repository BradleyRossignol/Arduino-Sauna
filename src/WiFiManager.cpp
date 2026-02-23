#include "WiFiManager.h"
#include "arduino_secrets.h"   // Must include here for SECRET_SSID / SECRET_PASS
#include <SPI.h>               // Sometimes required for GIGA

WiFiManager::WiFiManager() {}

bool WiFiManager::begin() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("WiFi module not found!");
    return false;
  }

  Serial.print("Connecting to: ");
  Serial.println(SECRET_SSID);

  int localStatus = WL_IDLE_STATUS;

  unsigned long timeout = millis() + 30000UL;

  while (localStatus != WL_CONNECTED && millis() < timeout) {
    Serial.print("Attempting WPA connection to: ");
    Serial.println(SECRET_SSID);

    localStatus = WiFi.begin(SECRET_SSID, SECRET_PASS);

    delay(5000);
  }

  if (localStatus == WL_CONNECTED) {
    Serial.println("Connected!");
    return true;
  }

  Serial.println("Connection failed.");
  return false;
}

bool WiFiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}