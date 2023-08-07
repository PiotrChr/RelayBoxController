#include "ESPAsyncWebServer.h"
#include "CircularBuffer.h"
#include "Wire.h"
#include "EEPROM.h"
#include "Regexp.h"
#include "WiFi.h"
#include "SPI.h"
#include "FS.h"
#include "SD.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include <functional>

void setup();
void loop();
void setupButtons();
void setupWifi();
void setupDisplay();
void setupRelays();
void displayStatus(const char* action);
void checkRelays();
void addTask(const char* name, unsigned long dueDate, std::function<void()> job, bool completionCondition ());
void wifiConnect();
void checkWifi();
void performTasks();
void setupServer();
void checkButton(int buttonPin, std::function<void(unsigned long)> handler, unsigned long *lastLock, int *buttonState, String message);
void checkButtons();
void restoreSettingsFromEEPROM();
void saveSettingsToEEPROM();
void writeStringToEEPROM(int addrOffset, const char* strToWrite);
void readStringFromEEPROM(int addrOffset, char* output, size_t maxLen);
bool getWifiConfigFromSD();
bool scanStringAndSetConfig(const char* content);
void readFile(fs::FS &fs, const char *path, String *contents);
void openCircuit(unsigned int circuit);
void closeCircuit(unsigned int circuit);
bool isIdle();