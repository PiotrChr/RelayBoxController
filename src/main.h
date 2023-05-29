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
void displayStatus(const String action);
void checkRelays();
void addTask(String name, unsigned long dueDate, std::function<void()> job, bool completionCondition ());
void wifiConnect();
void checkWifi();
void performTasks();
void setupServer();
void checkButton(int buttonPin, std::function<void(unsigned long)> handler, unsigned long *lastLock, int *buttonState, String message);
void checkButtons();
void restoreSettingsFromEEPROM();
void saveSettingsToEEPROM();
void writeStringToEEPROM(int addrOffset, const String &strToWrite);
String readStringFromEEPROM(int addrOffset);
bool getWifiConfigFromSD();
bool scanStringAndSetConfig(String content);
void readFile(fs::FS &fs, const char *path, String *contents);
void openCircuit(unsigned int circuit, String message);
void closeCircuit(unsigned int circuit, String message);
bool isIdle();