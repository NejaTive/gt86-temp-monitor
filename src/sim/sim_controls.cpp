#include "sim_controls.h"
#include "temp_sim.h"

void simControlsBegin() {
  // ADC pot на ESP32 не требует pinMode
}

void simControlsPoll() {
  auto rawToC = [](int raw) -> float {
    return (float)TEMP_SIM_MIN_C +
           (raw / 4095.0f) * (float)(TEMP_SIM_MAX_C - TEMP_SIM_MIN_C);
  };

  if (PIN_POT_COOLANT >= 0) {
    tempSimSetCoolantC(rawToC(analogRead(PIN_POT_COOLANT)));
  }
  if (PIN_POT_OIL >= 0) {
    tempSimSetOilC(rawToC(analogRead(PIN_POT_OIL)));
  }

  // Serial без pot: "c 95" = coolant, "o 110" = oil (и Enter)
  static char line[24];
  static uint8_t len = 0;
  while (Serial.available() > 0) {
    const char ch = (char)Serial.read();
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      line[len] = '\0';
      if (len > 0) {
        if ((line[0] == 'c' || line[0] == 'C') && line[1] == ' ') {
          tempSimSetCoolantC(atof(line + 2));
        } else if ((line[0] == 'o' || line[0] == 'O') && line[1] == ' ') {
          tempSimSetOilC(atof(line + 2));
        } else {
          // одно число → оба (удобно для быстрой проверки)
          const float v = atof(line);
          tempSimSetCoolantC(v);
          tempSimSetOilC(v);
        }
      }
      len = 0;
    } else if (len + 1 < sizeof(line)) {
      line[len++] = ch;
    }
  }
}
