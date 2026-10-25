#include "temp_sim.h"
#include "sim_controls.h"

void simControlsBegin() {}

void simControlsPoll() {
  auto rawToC = [](int raw) -> int16_t {
    const int span = TEMP_SIM_MAX_C - TEMP_SIM_MIN_C;
    return (int16_t)(TEMP_SIM_MIN_C + (raw * span) / 4095);
  };

  if (PIN_POT_COOLANT >= 0) {
    tempSimSetCoolantC(rawToC(analogRead(PIN_POT_COOLANT)));
  }
  if (PIN_POT_OIL >= 0) {
    tempSimSetOilC(rawToC(analogRead(PIN_POT_OIL)));
  }
}
