#include "WiFiManager.h"
#include "arduino_secrets.h"  // SECRET_SSID, SECRET_PASS
#include "Config.h"           // If you have constants like NTP_SERVER there

WiFiManager::WiFiManager() 
    : timeClient(ntpUDP, NTP_SERVER, NTP_TIME_OFFSET_SEC, NTP_UPDATE_INTERVAL_MS) {
    // Constructor
}

bool WiFiManager::begin() {
    Serial.println("Initializing WiFi...");

    // NO WiFi.mode(WIFI_STA);  <-- Remove this line! Not supported on Giga

    WiFi.begin(SECRET_SSID, SECRET_PASS);

    unsigned long start = millis();
    const unsigned long timeout = 12000;  // 12 seconds max

    while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
        delay(400);
        Serial.print(".");
    }
    Serial.println();

    bool connected = (WiFi.status() == WL_CONNECTED);
    if (connected) {
        Serial.println("WiFi connected! IP: " + WiFi.localIP().toString());
        syncNTP();
        wasConnected = true;
    } else {
        Serial.println("Initial WiFi connection failed - will retry later");
        wasConnected = false;
    }

    lastAttempt = millis();
    return connected;
}

void WiFiManager::maintain() {
    unsigned long now = millis();

    if (WiFi.status() != WL_CONNECTED) {
        if (wasConnected) {
            Serial.println("WiFi disconnected!");
            // Optional: uiSetWifiConnected(false); or show warning icon
            wasConnected = false;
        }

        if (now - lastAttempt >= 15000UL) {  // Retry every 15 seconds
            lastAttempt = now;
            Serial.println("Reconnecting to WiFi...");
            WiFi.disconnect();           // <-- Fixed: no bool argument
            WiFi.begin(SECRET_SSID, SECRET_PASS);
            // begin() may block briefly on fail; acceptable at this interval
        }
    } else {
        if (!wasConnected) {
            Serial.println("WiFi reconnected! IP: " + WiFi.localIP().toString());
            syncNTP();
            // Optional: uiSetWifiConnected(true);
            wasConnected = true;
        }
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::syncNTP() {
    if (isConnected()) {
        timeClient.begin();
        if (timeClient.update()) {
            Serial.println("NTP synced: " + timeClient.getFormattedTime());
            // If you set RTC or system time here, do it
        } else {
            Serial.println("NTP update failed");
        }
    }
}