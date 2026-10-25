#pragma once

#include <Arduino.h>
#include "config.h"

struct TempReading {
  int16_t coolantC;
  int16_t oilC;
};

void tempSimBegin();
void tempSimSetCoolantC(int16_t celsius);
void tempSimSetOilC(int16_t celsius);
TempReading tempSimReading();
