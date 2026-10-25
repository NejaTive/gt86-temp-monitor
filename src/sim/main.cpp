#include <Arduino.h>
#include "config.h"
#include "bt_sim.h"
#include "ui_display.h"
#include "sim_controls.h"
#include "temp_sim.h"

static uint32_t lastUiMs = 0;
static uint32_t lastBlinkMs = 0;
static bool aliveOn = true;

void setup() {
  tempSimBegin();
  simControlsBegin();
  btSimBegin();
  displayBegin();
}

void loop() {
  const uint32_t now = millis();

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
      st.coolantC = -999;
      st.oilC = -999;
      st.coolantStale = false;
      st.oilStale = false;
    }
    displayShow(st);
    displayHeartbeat(aliveOn);
  }
}
