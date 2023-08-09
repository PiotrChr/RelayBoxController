#include "SDManager.hpp"

SDManager::SDManager() : isSDInitialized(false) {
    initializeSD();
}

void SDManager::initializeSD() {
    if (!SD.begin(SD_CARD_READER_PIN)) {
        Serial.println("Card Mount Failed");
        isSDInitialized = false;
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        isSDInitialized = false;
        return;
    }
    isSDInitialized = true;
}

bool SDManager::readFileContents(const char *path, String &contents) {
    if (!isSDInitialized) {
        return false;
    }
    
    return readFile(SD, path, contents);
}

bool SDManager::readFile(fs::FS &fs, const char *path, String &contents) {
    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        return false;
    }

    // Read contents from the file
    contents = file.readString();
    file.close();
    return true;
}
