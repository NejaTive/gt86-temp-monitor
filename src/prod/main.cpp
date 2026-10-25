#include <Arduino.h>
#include "config.h"
#include "ui_display.h"
#include "obd_client.h"

static uint32_t lastUiMs = 0;
static uint32_t lastBlinkMs = 0;
static bool aliveOn = true;

void setup() {
  obdClientBegin();
  displayBegin();
}

void loop() {
  const uint32_t now = millis();

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
}
