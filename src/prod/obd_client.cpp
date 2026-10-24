#include "obd_client.h"

#include "obd_bt.h"
#include "obd_parse.h"
#include "ui_status.h"

#include <string.h>

// ---------------------------------------------------------------------------
// Состояния автомата (без delay — только millis)
// ---------------------------------------------------------------------------
enum ObdState : uint8_t {
  ST_DISCONNECTED = 0,
  ST_CONNECTING,
  ST_ELM_ATZ,
  ST_ELM_ATI,
  ST_ELM_SETUP,  // ATE0 / ATH1 / ATL0 / ATS0 / ATSH7E0 по шагам
  ST_POLL_COOLANT,
  ST_WAIT_COOLANT,
  ST_POLL_OIL,
  ST_WAIT_OIL,
  ST_WAIT_RECONNECT,
};

static ObdState state = ST_DISCONNECTED;
static unsigned long stateSinceMs = 0;
static unsigned long lastPollDoneMs = 0;

static char rxBuf[192];
static size_t rxLen = 0;

static uint8_t setupStep = 0;

static float coolantC = -999.0f;
static float oilC = -999.0f;
static unsigned long coolantAtMs = 0;
static unsigned long oilAtMs = 0;
static bool coolantEver = false;
static bool oilEver = false;

static const char *statusText = UI_STATUS_CONNECTING;

static void setState(ObdState next) {
  state = next;
  stateSinceMs = millis();
  rxLen = 0;
  rxBuf[0] = '\0';
}

static void setStatus(const char *s) { statusText = s; }

static void clearRx() {
  rxLen = 0;
  rxBuf[0] = '\0';
}

/** Копим байты до приглашения '>' или переполнения */
static bool accumulateRx() {
  while (obdBtAvailable() > 0) {
    const int b = obdBtRead();
    if (b < 0) {
      break;
    }
    const char ch = (char)b;
    if (rxLen + 1 < sizeof(rxBuf)) {
      rxBuf[rxLen++] = ch;
      rxBuf[rxLen] = '\0';
    }
    if (ch == '>') {
      return true;
    }
  }
  return false;
}

static bool timedOut(unsigned long timeoutMs) {
  return (millis() - stateSinceMs) >= timeoutMs;
}

static void applyCoolant(int c) {
  coolantC = (float)c;
  coolantAtMs = millis();
  coolantEver = true;
}

static void applyOil(int c) {
  oilC = (float)c;
  oilAtMs = millis();
  oilEver = true;
}

static void startReconnect() {
  obdBtDisconnect();
  setStatus(UI_STATUS_OFFLINE);
  setState(ST_WAIT_RECONNECT);
}

// Шаги init ELM после ATI
static const char *kSetupCmds[] = {
    "ATE0",    // без эха
    "ATL0",    // без лишних LF
    "ATS0",    // без пробелов в ответе (парсер и с пробелами умеет)
    "ATH1",    // заголовки CAN (7E8 …) — как в примерах ТЗ
    "ATSP0",   // автопротокол
    "ATSH7E0", // запросы к ЭБУ двигателя (масло 22 10 E7)
};
static const uint8_t kSetupCount =
    (uint8_t)(sizeof(kSetupCmds) / sizeof(kSetupCmds[0]));

void obdClientBegin() {
  coolantC = -999.0f;
  oilC = -999.0f;
  coolantEver = false;
  oilEver = false;
  setStatus(UI_STATUS_CONNECTING);
  if (!obdBtBegin()) {
    setStatus(UI_STATUS_OFFLINE);
    setState(ST_WAIT_RECONNECT);
    return;
  }
  setState(ST_CONNECTING);
}

void obdClientPoll() {
  const unsigned long now = millis();

  // Обрыв линка в любом состоянии после connect
  if (state >= ST_ELM_ATZ && state <= ST_WAIT_OIL && !obdBtIsConnected()) {
    startReconnect();
    return;
  }

  switch (state) {
    case ST_DISCONNECTED:
      setState(ST_CONNECTING);
      break;

    case ST_CONNECTING:
      setStatus(UI_STATUS_CONNECTING);
      if (obdBtConnect()) {
        setStatus(UI_STATUS_STARTING);
        setupStep = 0;
        setState(ST_ELM_ATZ);
        obdBtSendLine("ATZ");
      } else {
        setStatus(UI_STATUS_OFFLINE);
        setState(ST_WAIT_RECONNECT);
      }
      break;

    case ST_ELM_ATZ:
      if (accumulateRx()) {
        // После ATZ ждём что-то похожее на ELM / prompt
        setState(ST_ELM_ATI);
        obdBtSendLine("ATI");
      } else if (timedOut(OBD_RESPONSE_TIMEOUT_MS)) {
        startReconnect();
      }
      break;

    case ST_ELM_ATI:
      if (accumulateRx()) {
        setupStep = 0;
        setState(ST_ELM_SETUP);
        clearRx();
        obdBtSendLine(kSetupCmds[0]);
      } else if (timedOut(OBD_RESPONSE_TIMEOUT_MS)) {
        startReconnect();
      }
      break;

    case ST_ELM_SETUP:
      if (accumulateRx()) {
        setupStep++;
        if (setupStep >= kSetupCount) {
          setStatus(UI_STATUS_ONLINE);
          setState(ST_POLL_COOLANT);
        } else {
          clearRx();
          setState(ST_ELM_SETUP);
          obdBtSendLine(kSetupCmds[setupStep]);
        }
      } else if (timedOut(OBD_RESPONSE_TIMEOUT_MS)) {
        startReconnect();
      }
      break;

    case ST_POLL_COOLANT:
      // Держим ~1 Гц на параметр: ждём интервал после прошлого полного цикла
      if (lastPollDoneMs != 0 &&
          (now - lastPollDoneMs) < OBD_POLL_INTERVAL_MS) {
        break;
      }
      setStatus(UI_STATUS_ONLINE);
      clearRx();
      obdBtSendLine("0105");
      setState(ST_WAIT_COOLANT);
      break;

    case ST_WAIT_COOLANT:
      if (accumulateRx()) {
        int c = 0;
        const ObdParseStatus st = obdParseCoolantC(rxBuf, &c);
        if (st == OBD_PARSE_OK) {
          applyCoolant(c);
        } else if (st == OBD_PARSE_NO_DATA) {
          setStatus(UI_STATUS_NO_DATA);
        }
        setState(ST_POLL_OIL);
      } else if (timedOut(OBD_RESPONSE_TIMEOUT_MS)) {
        setStatus(UI_STATUS_TIMEOUT);
        setState(ST_POLL_OIL);  // пробуем масло в том же цикле
      }
      break;

    case ST_POLL_OIL:
      clearRx();
      // Заголовок 7E0 уже в init (ATSH7E0); команда масла из ТЗ
      obdBtSendLine("2210E7");
      setState(ST_WAIT_OIL);
      break;

    case ST_WAIT_OIL:
      if (accumulateRx()) {
        int c = 0;
        const ObdParseStatus st = obdParseOilC(rxBuf, &c);
        if (st == OBD_PARSE_OK) {
          applyOil(c);
          setStatus(UI_STATUS_ONLINE);
        } else if (st == OBD_PARSE_NO_DATA) {
          setStatus(UI_STATUS_NO_DATA);
        }
        lastPollDoneMs = millis();
        setState(ST_POLL_COOLANT);
      } else if (timedOut(OBD_RESPONSE_TIMEOUT_MS)) {
        setStatus(UI_STATUS_TIMEOUT);
        lastPollDoneMs = millis();
        setState(ST_POLL_COOLANT);
      }
      break;

    case ST_WAIT_RECONNECT:
      if (timedOut(OBD_RECONNECT_DELAY_MS)) {
        setStatus(UI_STATUS_CONNECTING);
        setState(ST_CONNECTING);
      }
      break;
  }
}

ObdReading obdClientReading() {
  ObdReading r;
  r.status = statusText;

  const unsigned long now = millis();

  if (coolantEver) {
    r.coolantC = coolantC;
    r.coolantStale = (now - coolantAtMs) > OBD_STALE_MS;
  } else {
    r.coolantC = -999.0f;
    r.coolantStale = false;
  }

  if (oilEver) {
    r.oilC = oilC;
    r.oilStale = (now - oilAtMs) > OBD_STALE_MS;
  } else {
    r.oilC = -999.0f;
    r.oilStale = false;
  }

  return r;
}
