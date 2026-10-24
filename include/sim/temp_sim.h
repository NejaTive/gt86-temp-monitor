/**
 * Ручные температуры (два pot / Serial). Не OBD.
 */

#pragma once

#include <Arduino.h>
#include "config.h"

struct TempReading {
  float coolantC;
  float oilC;
};

void tempSimBegin();
void tempSimSetCoolantC(float celsius);
void tempSimSetOilC(float celsius);
TempReading tempSimReading();
