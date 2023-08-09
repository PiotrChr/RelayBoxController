#pragma once
#include <Arduino.h>

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

Relay relays[RELAY_COUNT] = {
    {"Circuit 1", 16, HIGH, 36, 33, HIGH, HIGH},
    {"Circuit 2", 17, HIGH, 34, 39, HIGH, HIGH},
    {"Circuit 3", 2, HIGH, 32, 35, HIGH, HIGH},
    {"Circuit 4", 15, HIGH, 13, 4, HIGH, HIGH}
};
