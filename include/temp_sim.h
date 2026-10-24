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

#if ENABLE_TEMP_SIM

void tempSimBegin();
void tempSimSetCoolantC(float celsius);
void tempSimSetOilC(float celsius);
TempReading tempSimReading();

#else

inline void tempSimBegin() {}
inline void tempSimSetCoolantC(float) {}
inline void tempSimSetOilC(float) {}
inline TempReading tempSimReading() { return TempReading{-999.0f, -999.0f}; }

#endif
