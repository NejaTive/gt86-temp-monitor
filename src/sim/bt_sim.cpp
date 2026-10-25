#include "bt_sim.h"
#include "ui_status.h"

enum BtSimState : uint8_t {
  ST_DOWN = 0,
  ST_BT,
  ST_ELM,
  ST_UP,
};

static BtSimState state = ST_DOWN;
static uint32_t stateSinceMs = 0;
static int lastRaw = HIGH;
static uint32_t lastEdgeMs = 0;

static void setState(BtSimState next) {
  state = next;
  stateSinceMs = millis();
}

void btSimBegin() {
  if (PIN_BTN_BT_SIM < 0) {
    return;
  }
  pinMode(PIN_BTN_BT_SIM, INPUT_PULLUP);
  lastRaw = digitalRead(PIN_BTN_BT_SIM);
  state = ST_DOWN;
  stateSinceMs = millis();
}

void btSimPoll() {
  if (PIN_BTN_BT_SIM < 0) {
    return;
  }

  const uint32_t now = millis();
  const int raw = digitalRead(PIN_BTN_BT_SIM);

  if (raw != lastRaw && (now - lastEdgeMs) >= 40) {
    lastEdgeMs = now;
    if (raw == LOW && lastRaw == HIGH) {
      if (state == ST_DOWN) {
        setState(ST_BT);
      } else {
        setState(ST_DOWN);
      }
    }
    lastRaw = raw;
  }

  if (state == ST_BT && (now - stateSinceMs) >= BT_SIM_CONNECT_MS) {
    setState(ST_ELM);
  } else if (state == ST_ELM && (now - stateSinceMs) >= BT_SIM_ELM_MS) {
    setState(ST_UP);
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
