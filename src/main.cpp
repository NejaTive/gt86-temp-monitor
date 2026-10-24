/**
 * GT86 Temperature Monitor
 *
 * SIM: temperature only from manual control (pot / Serial), °C only.
 * Layout: sensor name, then "NN C". Live green dot top-right.
 */

#include <Arduino.h>
#include "config.h"
#include "display_ui.h"
#include "sim_controls.h"
#include "temp_sim.h"

static unsigned long lastUiMs = 0;
static unsigned long lastLogMs = 0;
static unsigned long lastBlinkMs = 0;
static bool aliveOn = true;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  if (PIN_STATUS_LED >= 0) {
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
  }

  Serial.println();
  Serial.println(F("GT86 Temperature Monitor"));
  Serial.printf("Green from %d C, red from %d C\r\n", TEMP_GREEN_FROM_C,
                TEMP_RED_FROM_C);
#if ENABLE_TEMP_SIM
  Serial.println(F("Manual SIM: turn pot (or type 0..120 + Enter)"));
  tempSimBegin();
  simControlsBegin();
#endif

  displayBegin();
  displayHeartbeat(true);
}

void loop() {
  const unsigned long now = millis();

#if ENABLE_TEMP_SIM
  simControlsPoll();

  if (now - lastBlinkMs >= 500) {
    lastBlinkMs = now;
    aliveOn = !aliveOn;
  }

  if (now - lastUiMs >= 100) {
    lastUiMs = now;
    const TempReading reading = tempSimReading();
    DisplayStatus st{reading.coolantC, reading.oilC};
    displayShow(st);
    displayHeartbeat(aliveOn);  // redraw after full UI clear
  }

  if (now - lastLogMs >= 1000) {
    lastLogMs = now;
    const TempReading reading = tempSimReading();
    Serial.printf("[SIM] Coolant=%d C  Oil=%d C\r\n",
                  (int)lroundf(reading.coolantC),
                  (int)lroundf(reading.oilC));
  }
#endif
}
