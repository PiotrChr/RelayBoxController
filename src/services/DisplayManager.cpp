#include "DisplayManager.hpp"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C //  See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

void DisplayManager::setup() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (1);  // hang here
    }
    clearDisplay();
    display.display();
}

void DisplayManager::clearDisplay() {
    display.clearDisplay();
    display.display();
}

void DisplayManager::displayStatus(const char* action) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 15);
    display.println(action);
    display.display();
}

void DisplayManager::displayIP(const IPAddress &ip) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(ip.toString());
    display.display();
}

void DisplayManager::resetUpdateTimer() {
    // lastDisplayUpdate = millis(); // if you have any functionality related to display update timing
}