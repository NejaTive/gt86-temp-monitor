#include "bt_sim.h"

#if ENABLE_BT_SIM

#include "ui_status.h"

enum BtSimState : uint8_t {
  ST_DOWN = 0,  // Offline
  ST_BT,        // Connecting
  ST_ELM,       // Starting
  ST_UP,        // Online
};

static BtSimState state = ST_DOWN;
static unsigned long stateSinceMs = 0;
static int lastRaw = HIGH;
static unsigned long lastEdgeMs = 0;

static void setState(BtSimState next, const __FlashStringHelper *logMsg) {
  state = next;
  stateSinceMs = millis();
  Serial.println(logMsg);
}

void btSimBegin() {
  if (PIN_BTN_BT_SIM < 0) {
    return;
  }
  pinMode(PIN_BTN_BT_SIM, INPUT_PULLUP);  // кнопка на GND, нажатие = LOW
  lastRaw = digitalRead(PIN_BTN_BT_SIM);
  state = ST_DOWN;
  stateSinceMs = millis();
  Serial.printf("[BT-SIM] button GPIO%d: press = connect/disconnect\r\n",
                PIN_BTN_BT_SIM);
  Serial.printf("[BT-SIM] connect delay: Connecting %u ms, Starting %u ms\r\n",
                (unsigned)BT_SIM_CONNECT_MS, (unsigned)BT_SIM_ELM_MS);
  Serial.println(F("[BT-SIM] start: Offline"));
}

void btSimPoll() {
  if (PIN_BTN_BT_SIM < 0) {
    return;
  }

  const unsigned long now = millis();
  const int raw = digitalRead(PIN_BTN_BT_SIM);

  // Антидребезг + фронт нажатия (HIGH → LOW)
  if (raw != lastRaw && (now - lastEdgeMs) >= 40) {
    lastEdgeMs = now;
    if (raw == LOW && lastRaw == HIGH) {
      if (state == ST_DOWN) {
        setState(ST_BT, F("[BT-SIM] Connecting"));
      } else {
        setState(ST_DOWN, F("[BT-SIM] Offline"));
      }
    }
    lastRaw = raw;
  }

  if (state == ST_BT && (now - stateSinceMs) >= BT_SIM_CONNECT_MS) {
    setState(ST_ELM, F("[BT-SIM] Starting (ELM)"));
  } else if (state == ST_ELM && (now - stateSinceMs) >= BT_SIM_ELM_MS) {
    setState(ST_UP, F("[BT-SIM] Online"));
  }
}

bool btSimConnected() { return state == ST_UP; }

const char *btSimStatus() {
  switch (state) {
    case ST_BT:
      return UI_STATUS_CONNECTING;
    case ST_ELM:
      return UI_STATUS_STARTING;
    case ST_UP:
      return UI_STATUS_ONLINE;
    case ST_DOWN:
    default:
      return UI_STATUS_OFFLINE;
  }
}

#endif  // ENABLE_BT_SIM
