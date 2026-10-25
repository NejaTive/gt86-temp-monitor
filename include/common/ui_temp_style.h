/**
 * Temperature color zones (°C only).
 */

#pragma once

#include <Arduino.h>
#include "config.h"

enum TempZone : uint8_t {
  TEMP_ZONE_COLD = 0,  // yellow
  TEMP_ZONE_OK = 1,    // green
  TEMP_ZONE_HOT = 2,   // red
};

inline TempZone tempZoneFromC(float celsius) {
  if (celsius >= (float)TEMP_RED_FROM_C) {
    return TEMP_ZONE_HOT;
  }
  if (celsius >= (float)TEMP_GREEN_FROM_C) {
    return TEMP_ZONE_OK;
  }
  return TEMP_ZONE_COLD;
}
