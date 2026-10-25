#pragma once

#include <Arduino.h>
#include "config.h"

struct ObdReading {
  int16_t coolantC;
  int16_t oilC;
  bool coolantStale;
  bool oilStale;
  const char *status;
};

void obdClientBegin();
void obdClientPoll();
ObdReading obdClientReading();
