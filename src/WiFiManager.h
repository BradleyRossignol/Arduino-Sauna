#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class WiFiManager {
public:
    WiFiManager();
    bool begin();                  // Returns true if connected after attempt
    void maintain();               // Call in loop() for background reconnects
    bool isConnected();
    void syncNTP();
    // Add your existing methods (e.g. getIP() if any)

private:
    unsigned long lastAttempt = 0;
    bool wasConnected = false;
    WiFiUDP ntpUDP;
    NTPClient timeClient;
};

extern WiFiManager wifi;  // Assuming global instance as in your main.ino

#endif