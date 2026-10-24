#include "sim_controls.h"
#include "temp_sim.h"

void simControlsBegin() {
  // ADC pot needs no pinMode on ESP32
}

void simControlsPoll() {
#if !ENABLE_TEMP_SIM
  return;
#else

  if (PIN_POT_TEMP >= 0) {
    const int raw = analogRead(PIN_POT_TEMP);
    const float c =
        (float)TEMP_SIM_MIN_C +
        (raw / 4095.0f) * (float)(TEMP_SIM_MAX_C - TEMP_SIM_MIN_C);
    tempSimSetC(c);
  }

  // Without a pot: type e.g. 95 and press Enter in Serial Monitor
  static char line[16];
  static uint8_t len = 0;
  while (Serial.available() > 0) {
    const char ch = (char)Serial.read();
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      line[len] = '\0';
      if (len > 0) {
        tempSimSetC(atof(line));
      }
      len = 0;
    } else if (len + 1 < sizeof(line)) {
      line[len++] = ch;
    }
  }

#endif
}
