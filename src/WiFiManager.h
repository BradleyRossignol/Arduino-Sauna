#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

class WiFiManager {
public:
  WiFiManager();

  bool begin();                    // Returns true if connected within timeout
  bool isConnected();
  void maintain();                 // Periodic check/retry

  String getSSID() const;
  String getIP() const;
  String getMAC() const;
};

#endif