#include "EEPROMManager.hpp"
#include <EEPROM.h>

EEPROMManager::EEPROMManager(int eepromSize)
    : EEPROM_SIZE(eepromSize)
{
    EEPROM.begin(EEPROM_SIZE);
}

void EEPROMManager::writeString(int addrOffset, const char *strToWrite)
{
    byte len = strlen(strToWrite);
    EEPROM.write(addrOffset, len);
    for (int i = 0; i < len; i++)
    {
        EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
    }

    EEPROM.commit();
}

void EEPROMManager::readString(int addrOffset, char *output, size_t maxLen)
{
    int newStrLen = EEPROM.read(addrOffset);
    newStrLen = newStrLen < maxLen ? newStrLen : maxLen - 1; // prevent buffer overflow

    if (newStrLen <= 0)
    {
        output[0] = '\0';
        return;
    }

    for (int i = 0; i < newStrLen; i++)
    {
        output[i] = EEPROM.read(addrOffset + 1 + i);
    }
    output[newStrLen] = '\0';
}

void EEPROMManager::saveWifiCredentials(const char *ssid, const char *password)
{
    writeString(SSID_EEPROM_INDEX, ssid);
    writeString(PASSWORD_EEPROM_INDEX, password);
}

void EEPROMManager::restoreWifiCredentials(char *ssid, size_t ssidLen, char *password, size_t passwordLen)
{
    readString(SSID_EEPROM_INDEX, ssid, ssidLen);
    readString(PASSWORD_EEPROM_INDEX, password, passwordLen);
}
