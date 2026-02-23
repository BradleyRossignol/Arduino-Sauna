#ifndef WiFi_MANAGER_H
#define WiFi_MANAGER_H

#include <WiFi.h>

class WiFiManager {
public:
  WiFiManager();             // constructor

  bool begin();              // attempts connection, returns true if successful
  bool isConnected() const;  // simple status check
};

#endif