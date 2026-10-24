/**
 * Manual temperature only (pot / Serial). Not OBD.
 */

#pragma once

#include <Arduino.h>
#include "config.h"

struct TempReading {
  float coolantC;
  float oilC;
};

#if ENABLE_TEMP_SIM

void tempSimBegin();
void tempSimSetC(float celsius);
TempReading tempSimReading();

#else

inline void tempSimBegin() {}
inline void tempSimSetC(float) {}
inline TempReading tempSimReading() { return TempReading{-999.0f, -999.0f}; }

#endif
