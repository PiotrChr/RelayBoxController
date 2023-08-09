#include "WifiManager.hpp"

WiFiManager::WiFiManager() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
}

bool WiFiManager::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
    WiFi.begin(ssid, password);

    uint32_t startTime = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > timeoutMs) {
            return false; // Connection timed out
        }
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