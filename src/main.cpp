#define LOG 1

#include "main.hpp"
#include "services/EEPROMManager.hpp"
#include "services/WifiManager.hpp"
#include "services/TaskManager.hpp"
#include "services/DisplayManager.hpp"
#include "services/SDManager.hpp"

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150;
unsigned long lastButtonLockTime = millis();
const unsigned long buttonLockTime = 2000;

#define SSD1306_NO_SPLASH
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C //  See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

bool stateChanged = false;

AsyncWebServer server(80);
AsyncEventSource events("/events");


const char *wifiConfigFile = "/wifi.txt";
const char *wifiConfigFileReplacement = "/wifi_old.txt";
SDManager sdManager;

#define EEPROM_SIZE 512
EEPROMManager eepromManager(EEPROM_SIZE);

IPAddress localIp;
char wifiSsid[50] = "";
char wifiPassword[50] = "";
unsigned const long wifiConnectionTimeout = 20000;
unsigned const long wifiCheckInterval = 100000;
unsigned long wifiCheckLock = 0;
WiFiManager wifiManager;

DisplayManager displayManager;
unsigned long lastDisplayUpdate = millis();
int displayUpdateTimeout = 15000;

TaskManager tasks;

void setup()
{
  setupSerial();
  setupRelays();
  displayManager.setup();
  delay(1000);

  setupButtons();
  
  displayManager.displayStatus("Restoring from EEPROM...");
  
  eepromManager.restoreWifiCredentials(wifiSsid, sizeof(wifiSsid), wifiPassword, sizeof(wifiPassword));
  delay(1000);

  if (getWifiConfigFromSD())
  {
    displayManager.displayStatus("Found connection details on SD...");
  }
  delay(1000);

  if (wifiSsid[0] != '\0')
  {
    displayManager.displayStatus("Saving to EEPROM...");
    eepromManager.saveWifiCredentials(wifiSsid, wifiPassword);

    delay(500);

    char wifiText[200] = "Connecting to: ";
    strcat(wifiText, wifiSsid);
    displayManager.displayStatus(wifiText);

    wifiManager.connect(wifiSsid, wifiPassword, wifiConnectionTimeout);

    delay(1000);
  }

  if (wifiManager.isConnected())
  {
    displayManager.displayStatus("Setting up HTTP server...");
    setupServer();
    delay(1000);
  }

  displayManager.displayStatus("OK");
  LOG_PRINTLN("Setup complete");
}

void setupSerial() {
  if (LOG)
  {
    Serial.begin(9600);
    while (!Serial)
    {
      ;
    }
  }
  delay(100);
}

void closeCircuit(unsigned int circuit)
{
  setRelayState(circuit, LOW);
}

void openCircuit(unsigned int circuit)
{
  setRelayState(circuit, HIGH);
}

void setRelayState(unsigned int circuit, int state) {
    if (relays[circuit].state != state) {
        relays[circuit].state = state;
        stateChanged = true;
    }
}

void setupButtons()
{
  for (auto &relay : relays)
  {
    pinMode(relay.btnOnPin, INPUT_PULLUP);
    pinMode(relay.btnOffPin, INPUT_PULLUP);
  }
}

void setupRelays()
{
  for (auto &relay : relays)
  {
    pinMode(relay.pin, OUTPUT);
  }
}

void checkRelays()
{
  for (auto &relay : relays)
  {
    digitalWrite(relay.pin, relay.state);
  }
}

void onRequest(AsyncWebServerRequest *request)
{
  request->send(404);
}

void handleStatusRoute(AsyncWebServerRequest *request)
{
  String status = "{";
  for (auto &relay : relays)
  {
    status += "\"";
    status += relay.name;
    status += "\":";
    status += relay.state == HIGH ? "false" : "true";
    status += ",";
  }
  status += "}";

  request->send(200, "text/plain", status);
}

void handleRelayApiRoute(AsyncWebServerRequest *request, bool enable) {
    tasks.addTask(enable ? "API Enable" : "API Disable", millis(), [request, enable]() {
        if (!request->hasParam("circuit")) {
            request->send_P(400, "text/plain", "Missing circuit param");
            return;
        }
        int circuit = request->arg("circuit").toInt();
        if (circuit < 0 || circuit > 3) {
            request->send_P(400, "text/plain", "Invalid circuit param");
            return;
        }
        if (enable) {
            closeCircuit(circuit);
        } else {
            openCircuit(circuit);
        }
    }, []() { return isIdle(); });

    request->send_P(200, "text/plain", enable ? "Enabling circuit" : "Disabling circuit");
}

void setupServer()
{
    server.onNotFound(onRequest);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", "Hello");
    });
    server.on("/enable", HTTP_GET, [](AsyncWebServerRequest *request) { handleRelayApiRoute(request, true); });
    server.on("/disable", HTTP_GET, [](AsyncWebServerRequest *request) { handleRelayApiRoute(request, false); });
    server.on("/status", HTTP_GET, handleStatusRoute);

    server.begin();
}

void checkButtonsForRelay(Relay &relay) {
    checkButton(relay.btnOnPin, relay.btnOnState, "Enable", [&]() { closeCircuit(&relay - relays); });
    checkButton(relay.btnOffPin, relay.btnOffState, "Disable", [&]() { openCircuit(&relay - relays); });
}

void checkButton(int buttonPin, int &buttonState, const char *action, std::function<void()> handler)
{
  int reading = digitalRead(buttonPin);
    unsigned long now = millis();

    if ((now - lastButtonLockTime) < buttonLockTime) {
        return;
    }

    if (reading != buttonState) {
        LOG_PRINTLN("Got button readout for: " + String(action));

        buttonState = reading;
        if (buttonState == HIGH) {
            handler();
        }

        lastButtonLockTime = now;
    }
}

void checkButtons()
{
  for (auto &relay : relays) {
      checkButtonsForRelay(relay);
  }
}

bool isIdle()
{
  return true;
}

bool getWifiConfigFromSD()
{
  LOG_PRINTLN("Wifi SSID: ");
  LOG_PRINTLN(wifiSsid);
  LOG_PRINTLN("Wifi Password: ");
  LOG_PRINTLN(wifiPassword);

  String fileContents;
  if (!sdManager.readFileContents(wifiConfigFile, fileContents))
  {
    LOG_PRINTLN("Failed to read from SD card.");
      
    return false;
  }

  if (fileContents.length() == 0)
  {
    return false;
  }

  return scanStringAndSetConfig(fileContents.c_str());
}

bool scanStringAndSetConfig(const char *content)
{
  char *modifiable_str = strdup(content);
  MatchState ms;
  const char ssidRegex[] = "ssid=([A-Za-z0-9_@.\\/#&+-]*)";
  const char passwordRegex[] = "password=([A-Za-z0-9_@.\\/#&+-]*)";

  ms.Target(modifiable_str);

  if (ms.Match(ssidRegex) == REGEXP_NOMATCH)
  {
    LOG_PRINTLN("No match for ssid");
    LOG_PRINTLN(content);

    free(modifiable_str);
    return false;
  }

  strncpy(wifiSsid, ms.GetCapture(modifiable_str, 0), sizeof(wifiSsid));
  wifiSsid[sizeof(wifiSsid) - 1] = '\0';

  if (ms.Match(passwordRegex) == REGEXP_NOMATCH)
  {
    LOG_PRINTLN("No match for password");
    LOG_PRINTLN(content);

    free(modifiable_str);
    return false;
  }

  strncpy(wifiPassword, ms.GetCapture(modifiable_str, 0), sizeof(wifiPassword));
  wifiPassword[sizeof(wifiPassword) - 1] = '\0';

  free(modifiable_str);
  return true;
}

void loop()
{
  if (!wifiManager.isConnected()) {
    if (wifiCheckLock == 0) {
      wifiCheckLock = millis();
    } else if (millis() - wifiCheckLock > wifiCheckInterval) {
      wifiCheckLock = 0;
      LOG_PRINTLN("Wifi not connected, reconnecting...");
      wifiManager.connect(wifiSsid, wifiPassword, wifiConnectionTimeout);

      delay(10000);
    }
  }

  checkButtons();

  if (stateChanged) {
    LOG_PRINTLN("State changed");

    checkRelays();
    stateChanged = false;

    #if LOG == 1
      for (auto &relay : relays) {
        Serial.print(relay.name);
        Serial.print(": ");
        Serial.println(relay.state == HIGH ? "OFF" : "ON");
      }
    #endif
  }

  tasks.performTasks();
}
