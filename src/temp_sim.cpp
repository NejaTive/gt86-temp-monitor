#include "temp_sim.h"

#if ENABLE_TEMP_SIM

static float coolantC = 0.0f;
static float oilC = 0.0f;

static float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void tempSimBegin() {
  coolantC = 0.0f;
  oilC = 0.0f;
}

void tempSimSetCoolantC(float celsius) {
  coolantC = clampf(celsius, (float)TEMP_SIM_MIN_C, (float)TEMP_SIM_MAX_C);
}

void tempSimSetOilC(float celsius) {
  oilC = clampf(celsius, (float)TEMP_SIM_MIN_C, (float)TEMP_SIM_MAX_C);
}

TempReading tempSimReading() { return TempReading{coolantC, oilC}; }

#endif
