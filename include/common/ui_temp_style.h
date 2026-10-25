#pragma once

#include <Arduino.h>
#include "config.h"

enum TempZone : uint8_t {
  TEMP_ZONE_COLD = 0,
  TEMP_ZONE_OK = 1,
  TEMP_ZONE_HOT = 2,
};

inline TempZone tempZoneFromC(int16_t celsius) {
  if (celsius >= TEMP_RED_FROM_C) {
    return TEMP_ZONE_HOT;
  }
  if (celsius >= TEMP_GREEN_FROM_C) {
    return TEMP_ZONE_OK;
  }
  return TEMP_ZONE_COLD;
}
