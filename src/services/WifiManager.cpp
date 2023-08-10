#include "WifiManager.hpp"

WiFiManager::WiFiManager() {}

void WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    LOG_PRINTLN("WiFiManager initialized");
}

bool WiFiManager::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
    LOG_PRINTLN("Connecting to WiFi...");        
    WiFi.begin(ssid, password);

    uint32_t startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > timeoutMs) {
            LOG_PRINTLN("Connection timed out");
            return false;
        }
        LOG_PRINT('.');
        delay(100);
    }

    return true;
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getCurrentSSID() {
    return WiFi.SSID();
}

IPAddress WiFiManager::getLocalIP() {
    return WiFi.localIP();
}