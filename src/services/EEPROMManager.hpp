#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "../config.hpp"


class EEPROMManager
{
public:
    EEPROMManager(int eepromSize);

    void begin();
    void writeString(int addrOffset, const char *strToWrite);
    void readString(int addrOffset, char *output, size_t maxLen);
    void saveWifiCredentials(const char *ssid, const char *password);
    void restoreWifiCredentials(char *ssid, size_t ssidLen, char *password, size_t passwordLen);

private:
    const int EEPROM_SIZE;

    static const int SSID_EEPROM_INDEX = 0;
    static const int PASSWORD_EEPROM_INDEX = 255;
};
