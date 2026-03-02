#include "WiFiManager.h"
#include "Debug.h"
#include "Config.h"
#include "arduino_secrets.h"

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// External NTP objects (declared in main.ino)
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

WiFiManager::WiFiManager() {}

bool WiFiManager::begin() {
    LOG_INFO("WiFi connecting to SSID: " + String(SECRET_SSID));

    WiFi.begin(SECRET_SSID, SECRET_PASS);

    unsigned long start = millis();
    const unsigned long timeoutMs = 15000UL;

    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        delay(500);
    }

    bool success = (WiFi.status() == WL_CONNECTED);

    if (success) {
        LOG_INFO("WiFi connected | IP: " + WiFi.localIP().toString());
        timeClient.begin();
        timeClient.setTimeOffset(TimeConfig::OFFSET_SEC);
        timeClient.update();
        LOG_INFO("NTP client started (offset " + String(TimeConfig::OFFSET_SEC) + " s)");
    } else {
        LOG_WARN("WiFi connection timeout after 15 s");
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
            LOG_WARN("WiFi disconnected → attempting reconnect");
            WiFi.begin(SECRET_SSID, SECRET_PASS);
        } else {
            LOG_VERBOSE("WiFi still connected");
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