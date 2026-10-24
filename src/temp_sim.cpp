#include "temp_sim.h"

#if ENABLE_TEMP_SIM

static float valueC = 0.0f;

static float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void tempSimBegin() { valueC = 0.0f; }

void tempSimSetC(float celsius) {
  valueC = clampf(celsius, (float)TEMP_SIM_MIN_C, (float)TEMP_SIM_MAX_C);
}

TempReading tempSimReading() { return TempReading{valueC, valueC}; }

#endif
