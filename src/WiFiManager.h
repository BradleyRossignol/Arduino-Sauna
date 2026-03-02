#pragma once

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// ── File Overview ──────────────────────────────────────────────────────────
// WiFi connection manager for Arduino Giga R1 (Mbed WiFi stack).
// Handles initial connect with timeout, periodic maintenance/reconnect,
// and provides status strings for the Network Info screen.
// NTPClient is initialized on successful connect (extern objects from main.ino).
// ───────────────────────────────────────────────────────────────────────────

#include <Arduino.h>

class WiFiManager {
public:
    WiFiManager();

    bool begin();                    // Connect with 15 s timeout
    bool isConnected();
    void maintain();                 // Periodic check/retry

    String getSSID() const;
    String getIP() const;
    String getMAC() const;
};

#endif