/**
 * GT86 Temperature Monitor — SIM (Wokwi)
 *
 * 2 pot (ОЖ / масло); кнопка BT = connect/disconnect (симуляция).
 * UI: Coolant / Oil, "NN C" / "NN* C" / "-- C", статус, зелёная точка.
 */

#include <Arduino.h>
#include "config.h"
#include "bt_sim.h"
#include "display_ui.h"
#include "sim_controls.h"
#include "temp_sim.h"

static unsigned long lastUiMs = 0;
static unsigned long lastBlinkMs = 0;
static bool aliveOn = true;

void setup() {
  Serial.begin(SERIAL_BAUD);  // для опционального ввода "c 95" / "o 110"
  delay(500);

  if (PIN_STATUS_LED >= 0) {
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
  }

  tempSimBegin();
  simControlsBegin();
  btSimBegin();
  displayBegin();
  displayHeartbeat(true);
}

void loop() {
  const unsigned long now = millis();

  simControlsPoll();
  btSimPoll();

  if (now - lastBlinkMs >= 500) {
    lastBlinkMs = now;
    aliveOn = !aliveOn;
  }

  if (now - lastUiMs >= 100) {
    lastUiMs = now;
    DisplayStatus st{};
    st.status = btSimStatus();
    if (btSimConnected()) {
      const TempReading reading = tempSimReading();
      st.coolantC = reading.coolantC;
      st.oilC = reading.oilC;
      st.coolantStale = false;
      st.oilStale = false;
    } else {
      // Offline / Connecting / Starting — температур ещё нет
      st.coolantC = -999.0f;
      st.oilC = -999.0f;
      st.coolantStale = false;
      st.oilStale = false;
    }
    displayShow(st);
    displayHeartbeat(aliveOn);
  }
}
