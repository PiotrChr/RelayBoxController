#include "main.h"

#define LOG 1

#define RELAY_PIN_1 16
#define RELAY_PIN_2 17
#define RELAY_PIN_3 2
#define RELAY_PIN_4 15

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150;
unsigned long lastButtonLockTime = millis();
const unsigned long buttonLockTime = 3000;

#define SSD1306_NO_SPLASH
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C /// < See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

struct Relay {
  char name[50];
  int pin;
  int state;
  int btnOnPin;
  int btnOffPin;
  int btnOnState;
  int btnOffState;
};

Relay relays [4] = {
  {"Circuit 1", 16, HIGH, 36, 39, HIGH, HIGH},
  {"Circuit 2", 17, HIGH, 34, 35, HIGH, HIGH},
  {"Circuit 3", 2, HIGH, 32, 33, HIGH, HIGH},
  {"Circuit 4", 15, HIGH, 4, 0, HIGH, HIGH}
 };

AsyncWebServer server(80);
AsyncEventSource events("/events"); // event source (Server-Sent events)

// Wifi config
#define SD_CARD_READER_PIN 5
const char* wifiConfigFile = "/wifi.txt";
const char* wifiConfigFileReplacement = "/wifi_old.txt";

// String fileContents;
IPAddress localIp;

char wifiSsid[50] = "";
char wifiPassword[50] = "";

#define EEPROM_SIZE 512
#define SSID_EEPROM_INDEX 0
#define PASSWORD_EEPROM_INDEX 255

unsigned const long wifiConnectionTimeout = 20000;
unsigned const long wifiCheckInterval = 100000;
unsigned long wifiCheckLock = 0;

// Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long lastDisplayUpdate = millis();
int displayUpdateTimeout = 15000;

// Task runner config
typedef std::function<void()> Job;
typedef bool (*RunCondition)();

struct Task {
  const char* name;
  unsigned long dueDate;
  bool finished;
  Job job;
  RunCondition runCondition;
};

const int taskLimit = 20;
CircularBuffer <Task, taskLimit> tasks;

void setup() {
  if (LOG) {
    Serial.begin(9600);
    while (!Serial) {
      ;
    }
  }
  delay(100);

  setupRelays();

  setupDisplay();
  delay(3000);

  setupButtons();

  displayStatus("Reading SD...");
  displayStatus("Restoring from EEPROM...");
  restoreSettingsFromEEPROM();
  delay(1000);
  
  if (LOG) {
    Serial.println("Wifi SSID: ");
    Serial.println(wifiSsid);
    Serial.println("Wifi Password: ");
    Serial.println(wifiPassword);
  }
  
  if (getWifiConfigFromSD()) {
    displayStatus("Found connection details on SD...");
  }
  delay(1000);
  
  displayStatus("Setting up Wifi...");
  setupWifi();
  delay(1000);
  
  if (wifiSsid[0] != '\0') {
    displayStatus("Saving to EEPROM...");
    saveSettingsToEEPROM();

    delay(1000);

    char wifiText[200] = "Connecting to: ";
    strcat (wifiText, wifiSsid);
    displayStatus(wifiText);
    wifiConnect();

    delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED) {
    displayStatus("Setting up HTTP server...");
    setupServer();
    delay(1000);
  }

  displayStatus("OK");
  if (LOG) {
    Serial.println("Setup complete");
  }
}

void setupEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
}

void closeCircuit(unsigned int circuit, String message) {
  relays[circuit].state = LOW;

  //show message
}

void openCircuit(unsigned int circuit, String message) {
  relays[circuit].state = HIGH;

  //show message
}

void setupButtons() {
  for (auto& relay : relays) {
    pinMode(relay.btnOnPin, INPUT);
    pinMode(relay.btnOffPin, INPUT);
  }
  
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);
}

void setupDisplay() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { 
    Serial.println(F("SSD1306 allocation failed"));
  }

  display.clearDisplay();
  display.display();
}

void setupRelays() {
  for (auto& relay : relays) {
    pinMode(relay.pin, OUTPUT);
  }
}

void displayStatus(const char* action) {
  display.clearDisplay();

  display.setTextSize(1);             
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);            
  display.println(localIp);
  if (action != "") {
    display.setCursor(0,15);         
    display.println(action);
    }
  
  display.display();

  lastDisplayUpdate = millis();
  } 

void checkRelays() {
  for (auto& relay : relays) {
    digitalWrite(relay.pin, relay.state);
  }
}

void addTask(const char* name, unsigned long dueDate, std::function<void()> job, bool completionCondition ()) {
  struct Task task = {name, dueDate, false, job, completionCondition};
  tasks.push(task);
}

void wifiConnect() {
  if (LOG) {
    Serial.println("Connecting to: ");
    Serial.println(wifiSsid);
    Serial.println(wifiPassword);
  }

  WiFi.begin(wifiSsid, wifiPassword);
  unsigned long currentMillis = millis();
  
  while (WiFi.status() != WL_CONNECTED) {
    if (LOG) Serial.print('.');
    delay(1000);
    if (currentMillis > wifiConnectionTimeout) {
      if (LOG) Serial.println("Connection timeout.");
      return;
      }
  }

  localIp = WiFi.localIP();
  if (LOG) {
    Serial.println("Connected to WiFi.");
    Serial.print("IP address: ");
    Serial.println(localIp);
  }
}

void checkWifi() {
  unsigned long currentMillis = millis();
  
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - wifiCheckLock >= wifiCheckInterval)) {
    if (LOG) Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    wifiCheckLock = currentMillis;
    localIp = WiFi.localIP();
    if (LOG) {
      Serial.print("IP address: ");
      Serial.println(localIp);
    }
  }
}

void performTasks() {
  for (int i = 0; i < tasks.size(); i++) {
    if (!tasks[i].finished && tasks[i].dueDate < millis() && tasks[i].runCondition() == true) {
      struct Task task = tasks.shift();
      displayStatus(task.name);
      if (LOG) Serial.println(task.name);
      task.job();
      task.finished = true;
    } else {
      return;
    }
  }
}

void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", "Hello");
  });

  server.begin();
}

void checkButton(int buttonPin, std::function<void(unsigned long)> handler, unsigned long *lastLock, int *buttonState, String message)
{
  int reading = digitalRead(buttonPin);

  unsigned long now = millis();

  if ((now - *lastLock) < buttonLockTime)
  {
    return;
  }

  if ((now - lastDebounceTime) > debounceDelay)
  {
    lastDebounceTime = now;
    
    if (LOG) {
      Serial.println("Reading:");
      Serial.println(reading); 

      Serial.println("Button state:");
      Serial.println(*buttonState);
    }

    if (reading != *buttonState)
    {
      if (LOG) Serial.println("Got button readout for: " + message);
      
      *buttonState = reading;
      if (*buttonState == LOW)
      {
        handler(now);
      }

      *lastLock = now;
    }
  }
}

void checkButtons()
{
  for (unsigned int i = 0; i < sizeof(relays)/sizeof(Relay); i++)
  {
    checkButton(
      relays[i].btnOnPin,
      [i](unsigned long initTime){ 
        addTask("Enable", initTime, [i](){ openCircuit(i, "Enabling circuit"); },[](){ return isIdle(); });
      },
      &lastButtonLockTime,
      &relays[i].btnOnState,
      "Enabling circuit"
    );

    checkButton(
      relays[i].btnOffPin,
      [i](unsigned long initTime){ 
        addTask("Disable", initTime, [i](){ closeCircuit(i, "Disabling circuit"); },[](){ return isIdle(); });
       },
      &lastButtonLockTime,
      &relays[i].btnOffState,
      "Disabling circuit"
    );
  }

}

bool isIdle() {
  return true;
}

void restoreSettingsFromEEPROM() {
  readStringFromEEPROM(SSID_EEPROM_INDEX, wifiSsid, sizeof(wifiSsid));
  readStringFromEEPROM(PASSWORD_EEPROM_INDEX, wifiPassword, sizeof(wifiPassword));
}

void saveSettingsToEEPROM() {
  writeStringToEEPROM(SSID_EEPROM_INDEX, wifiSsid);
  writeStringToEEPROM(PASSWORD_EEPROM_INDEX, wifiPassword);
}

void writeStringToEEPROM(int addrOffset, const char* strToWrite)
{
  byte len = strlen(strToWrite);
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }

  EEPROM.commit();
}

void readStringFromEEPROM(int addrOffset, char* output, size_t maxLen)
{
  int newStrLen = EEPROM.read(addrOffset);
  newStrLen = newStrLen < maxLen ? newStrLen : maxLen - 1; // prevent buffer overflow

  if (newStrLen <= 0) {
    output[0] = '\0';
    return;
  }
  
  for (int i = 0; i < newStrLen; i++)
  {
    output[i] = EEPROM.read(addrOffset + 1 + i);
  }
  output[newStrLen] = '\0';
}

bool getWifiConfigFromSD() {
  if (!SD.begin(SD_CARD_READER_PIN, SPI, 400000))
  {
    if (LOG) Serial.println("Card Mount Failed");
    SD.end();
    return false;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    if (LOG) Serial.println("No SD card attached");
    return false;
  }
  
  String fileContents;
  readFile(SD, wifiConfigFile, &fileContents);

  if (fileContents[0] == '\0')
  {
    return false;
  }

  SD.end();
  
  return scanStringAndSetConfig(fileContents.c_str());
}

void readFile(fs::FS &fs, const char *path, String* contents)
{
  if (LOG) Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    if (LOG) Serial.println("Failed to open file for reading");
    return;
  }

  memset(contents, '\0', sizeof(contents));

  if (LOG) Serial.print("Read from file: ");
  while (file.available())
  {
    char currentChar = file.read();
    if (LOG) Serial.print(currentChar);
    *contents += currentChar;
  }
  file.close();
  if (LOG) Serial.print("Finished reading");
}

bool scanStringAndSetConfig(const char* content) {
  char* modifiable_str = strdup(content);
  MatchState ms;
  const char ssidRegex[] = "ssid=([A-Za-z0-9_@.\\/#&+-]*)";
  const char passwordRegex[] = "password=([A-Za-z0-9_@.\\/#&+-]*)";

  ms.Target(modifiable_str);

  if (ms.Match(ssidRegex) == REGEXP_NOMATCH)
  {
    if (LOG) {
      Serial.println("No match for ssid");
      Serial.println(content);
    }
    
    return false;
  }

  strncpy(wifiSsid, ms.GetCapture(modifiable_str, 0), sizeof(wifiSsid));
  wifiSsid[sizeof(wifiSsid) - 1] = '\0';  // Ensure null-termination

  if (ms.Match(passwordRegex) == REGEXP_NOMATCH)
  {
    if (LOG) {
      Serial.println("No match for password");
      Serial.println(content);
    }
    
    return false;
  }

  strncpy(wifiPassword, ms.GetCapture(modifiable_str, 0), sizeof(wifiPassword));
  wifiPassword[sizeof(wifiPassword) - 1] = '\0';  // Ensure null-termination

  free(modifiable_str);
  return true;
}

void loop() {
  checkButtons();
  checkRelays();
  performTasks();
}
