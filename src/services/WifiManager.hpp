#pragma once

#include <Arduino.h>
#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#endif

class WiFiManager {
public:
    WiFiManager();

    bool connect(const char* ssid, const char* password, uint32_t timeoutMs = 10000);
    void disconnect();
    bool isConnected();
    String getCurrentSSID();
    IPAddress getLocalIP();
};
