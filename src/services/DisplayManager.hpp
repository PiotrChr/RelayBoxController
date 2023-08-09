#pragma once

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

class DisplayManager {
public:
    DisplayManager();
    void setup();
    void clearDisplay();
    void displayStatus(const char* action);
    void displayIP(const IPAddress &ip);
    void resetUpdateTimer();
    
private:
    Adafruit_SSD1306 display;
};
