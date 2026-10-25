#include "temp_sim.h"

static int16_t coolantC = 0;
static int16_t oilC = 0;

static int16_t clampi(int16_t v, int16_t lo, int16_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void tempSimBegin() {
  coolantC = 0;
  oilC = 0;
}

void tempSimSetCoolantC(int16_t celsius) {
  coolantC = clampi(celsius, (int16_t)TEMP_SIM_MIN_C, (int16_t)TEMP_SIM_MAX_C);
}

void tempSimSetOilC(int16_t celsius) {
  oilC = clampi(celsius, (int16_t)TEMP_SIM_MIN_C, (int16_t)TEMP_SIM_MAX_C);
}

TempReading tempSimReading() { return TempReading{coolantC, oilC}; }
