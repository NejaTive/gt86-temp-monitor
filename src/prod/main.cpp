/**
 * GT86 Temperature Monitor — PROD (реальная машина)
 *
 * Bluetooth Classic SPP → ScanX (ELM) → OBD.
 * UI: Coolant / Oil, "NN C" / "NN* C" / "-- C", статус, зелёная точка.
 */

#include <Arduino.h>
#include "config.h"
#include "display_ui.h"
#include "obd_client.h"

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
  Serial.println(F("Mode: OBD (Bluetooth Classic SPP + ELM)"));

  obdClientBegin();
  displayBegin();
  displayHeartbeat(true);
}

void loop() {
  const unsigned long now = millis();

  obdClientPoll();

  if (now - lastBlinkMs >= 500) {
    lastBlinkMs = now;
    aliveOn = !aliveOn;
  }

  if (now - lastUiMs >= 100) {
    lastUiMs = now;
    DisplayStatus st{};
    const ObdReading reading = obdClientReading();
    st.coolantC = reading.coolantC;
    st.oilC = reading.oilC;
    st.coolantStale = reading.coolantStale;
    st.oilStale = reading.oilStale;
    st.status = reading.status;
    displayShow(st);
    displayHeartbeat(aliveOn);
  }

  if (now - lastLogMs >= 1000) {
    lastLogMs = now;
    const ObdReading reading = obdClientReading();
    Serial.printf("[UI] Coolant=%d%s  Oil=%d%s  status=%s\r\n",
                  (int)lroundf(reading.coolantC),
                  reading.coolantStale ? "*" : "",
                  (int)lroundf(reading.oilC), reading.oilStale ? "*" : "",
                  reading.status ? reading.status : "");
  }
}
