/**
 * GT86 Temperature Monitor
 *
 * DEV (Wokwi): 2 pot (ОЖ / масло); кнопка BT = connect/disconnect (симуляция).
 * PROD: Bluetooth Classic SPP → ScanX (ELM) → OBD.
 * UI: Coolant / Oil, "NN C" / "NN* C" / "-- C", статус, зелёная точка.
 */

#include <Arduino.h>
#include "config.h"
#include "bt_sim.h"
#include "display_ui.h"
#include "obd_client.h"
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
  Serial.println(F("Mode: SIM (2 pots: coolant GPIO34, oil GPIO35)"));
  Serial.println(F("Serial: \"c 95\" / \"o 110\" or one number for both"));
#if ENABLE_BT_SIM
  Serial.println(F("BT-SIM: button = Connecting→Starting→Online / Offline"));
#endif
  tempSimBegin();
  simControlsBegin();
  btSimBegin();
#elif ENABLE_OBD
  Serial.println(F("Mode: OBD (Bluetooth Classic SPP + ELM)"));
  obdClientBegin();
#else
  Serial.println(F("Mode: none (enable SIM or OBD in config)"));
#endif

  displayBegin();
  displayHeartbeat(true);
}

void loop() {
  const unsigned long now = millis();

#if ENABLE_TEMP_SIM
  simControlsPoll();
  btSimPoll();
#elif ENABLE_OBD
  obdClientPoll();
#endif

  if (now - lastBlinkMs >= 500) {
    lastBlinkMs = now;
    aliveOn = !aliveOn;
  }

  if (now - lastUiMs >= 100) {
    lastUiMs = now;
    DisplayStatus st{};

#if ENABLE_TEMP_SIM
#if !ENABLE_BT_SIM
#error "DEV SIM requires ENABLE_BT_SIM 1 (кнопка BT)"
#endif
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
#elif ENABLE_OBD
    const ObdReading reading = obdClientReading();
    st.coolantC = reading.coolantC;
    st.oilC = reading.oilC;
    st.coolantStale = reading.coolantStale;
    st.oilStale = reading.oilStale;
    st.status = reading.status;
#else
#error "Enable ENABLE_TEMP_SIM (DEV) or ENABLE_OBD (PROD) in config"
#endif

    displayShow(st);
    displayHeartbeat(aliveOn);
  }

  if (now - lastLogMs >= 1000) {
    lastLogMs = now;
#if ENABLE_TEMP_SIM
    Serial.printf("[SIM] bt=%s  Coolant=%d C  Oil=%d C\r\n",
                  btSimConnected() ? "up" : "down",
                  btSimConnected() ? (int)lroundf(tempSimReading().coolantC)
                                   : -999,
                  btSimConnected() ? (int)lroundf(tempSimReading().oilC)
                                   : -999);
#elif ENABLE_OBD
    const ObdReading reading = obdClientReading();
    Serial.printf("[UI] Coolant=%d%s  Oil=%d%s  status=%s\r\n",
                  (int)lroundf(reading.coolantC),
                  reading.coolantStale ? "*" : "",
                  (int)lroundf(reading.oilC), reading.oilStale ? "*" : "",
                  reading.status ? reading.status : "");
#endif
  }
}
