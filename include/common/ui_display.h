#pragma once

#include <Arduino.h>
#include "config.h"

struct DisplayStatus {
  int16_t coolantC;
  int16_t oilC;
  bool coolantStale;
  bool oilStale;
  const char *status;
};

void displayBegin();
void displayShow(const DisplayStatus &status);
void displayHeartbeat(bool on);
