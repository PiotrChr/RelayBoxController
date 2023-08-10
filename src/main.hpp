#pragma once

#include "ESPAsyncWebServer.h"
#include "Wire.h"
#include "Regexp.h"
#include "SPI.h"
#include "config.hpp"
#include <functional>

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
void openCircuit(unsigned int circuit);
void closeCircuit(unsigned int circuit);
bool isIdle();
void handleRelayApiRoute(AsyncWebServerRequest *request, bool enable);
void handleStatusRoute(AsyncWebServerRequest *request);
void checkButtonsForRelay(Relay &relay);
void setRelayState(unsigned int circuit, int state);