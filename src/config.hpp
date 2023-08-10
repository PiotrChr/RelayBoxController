#pragma once

#define LOG 1

#include <Arduino.h>
#if LOG == 1
  #define LOG_PRINT(x) Serial.print(x)
  #define LOG_PRINTLN(x) Serial.println(x)
#else
  #define LOG_PRINT(x) 
  #define LOG_PRINTLN(x) 
#endif

struct Relay
{
  char name[50];
  int pin;
  int state;
  int btnOnPin;
  int btnOffPin;
  int btnOnState;
  int btnOffState;
};

#define RELAY_COUNT 4

extern Relay relays[RELAY_COUNT];
