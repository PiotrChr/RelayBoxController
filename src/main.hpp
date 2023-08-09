#include "ESPAsyncWebServer.h"
#include "Wire.h"
#include "Regexp.h"
#include "SPI.h"
#include "config.hpp"
#include <functional>

#if LOG == 1
  #define LOG_PRINT(x) Serial.print(x)
  #define LOG_PRINTLN(x) Serial.println(x)
#else
  #define LOG_PRINT(x) 
  #define LOG_PRINTLN(x) 
#endif

void setup();
void loop();
void setupSerial();
void setupButtons();
void setupRelays();
void checkRelays();
void setupServer();
void checkButton(int buttonPin, int &buttonState, const char *action, std::function<void()> handler);
void checkButtons();
bool getWifiConfigFromSD();
bool scanStringAndSetConfig(const char* content);
void readFile(fs::FS &fs, const char *path, String *contents);
void openCircuit(unsigned int circuit);
void closeCircuit(unsigned int circuit);
bool isIdle();
void handleRelayApiRoute(AsyncWebServerRequest *request, bool enable);
void handleStatusRoute(AsyncWebServerRequest *request);
void checkButtonsForRelay(Relay &relay);
void setRelayState(unsigned int circuit, int state);