#include <string.h>
#include "obd_bt.h"
#include "obd_parse.h"
#include "ui_status.h"
#include "obd_client.h"

enum ObdState : uint8_t {
  ST_DISCONNECTED,
  ST_CONNECTING,
  ST_ELM_SETUP,
  ST_POLL_GATE,
  ST_WAIT_PID,
  ST_WAIT_RECONNECT,
};

enum AwaitResult : uint8_t {
  AWAIT_PENDING,
  AWAIT_READY,
  AWAIT_TIMEOUT,
};

using ObdParseFn = ObdParseStatus (*)(const char *, int *);
using ObdApplyFn = void (*)(int);

struct PidStep {
  const char *cmd;
  ObdParseFn parse;
  ObdApplyFn apply;
};

static ObdState state = ST_DISCONNECTED;
static uint32_t stateSinceMs = 0;
static uint32_t lastPollDoneMs = 0;
static bool connectUiArmed = false;

static char rxBuf[192];
static size_t rxLen = 0;

static uint8_t setupStep = 0;
static uint8_t pidIndex = 0;

static int16_t coolantC = -999;
static int16_t oilC = -999;
static uint32_t coolantAtMs = 0;
static uint32_t oilAtMs = 0;

static const char *statusText = UI_STATUS_CONNECTING;

static void clearRx() {
  rxLen = 0;
  rxBuf[0] = '\0';
}

static void drainBt() {
  while (obdBtAvailable() > 0) {
    if (obdBtRead() < 0) {
      break;
    }
  }
  clearRx();
}

static void setState(ObdState next) {
  state = next;
  stateSinceMs = millis();
  clearRx();
}

static void restartWait() {
  stateSinceMs = millis();
  clearRx();
}

static void setStatus(const char *s) {
  statusText = s;
}

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

static bool timedOut(uint32_t timeoutMs) {
  return (millis() - stateSinceMs) >= timeoutMs;
}

static AwaitResult awaitPrompt(uint32_t timeoutMs) {
  if (accumulateRx()) {
    return AWAIT_READY;
  }
  if (timedOut(timeoutMs)) {
    return AWAIT_TIMEOUT;
  }
  return AWAIT_PENDING;
}

static void applyCoolant(int c) {
  coolantC = (int16_t)c;
  coolantAtMs = millis();
}

static void applyOil(int c) {
  oilC = (int16_t)c;
  oilAtMs = millis();
}

static void startReconnect() {
  obdBtEnd();
  connectUiArmed = false;
  setStatus(UI_STATUS_OFFLINE);
  setState(ST_WAIT_RECONNECT);
}

static bool needsBtLink(ObdState s) {
  switch (s) {
    case ST_ELM_SETUP:
    case ST_POLL_GATE:
    case ST_WAIT_PID:
      return true;
    default:
      return false;
  }
}

static const char *kSetupCmds[] = {
    "ATZ",
    "ATI",
    "ATE0",
    "ATL0",
    "ATS0",
    "ATH1",
    "ATSP6",
    "ATSH7E0",
};

static const uint8_t kSetupCount =
    (uint8_t)(sizeof(kSetupCmds) / sizeof(kSetupCmds[0]));

static const PidStep kPids[] = {
    {"0105", obdParseCoolantC, applyCoolant},
    {"2210E7", obdParseOilC, applyOil},
};

static const uint8_t kPidCount =
    (uint8_t)(sizeof(kPids) / sizeof(kPids[0]));

static void sendSetupCmd(uint8_t step) {
  drainBt();
  restartWait();
  obdBtSendLine(kSetupCmds[step]);
}

static void sendPidCmd(uint8_t index) {
  drainBt();
  setState(ST_WAIT_PID);
  obdBtSendLine(kPids[index].cmd);
}

static void handlePidParse(ObdParseStatus st, ObdApplyFn apply, int valueC) {
  if (st == OBD_PARSE_OK) {
    apply(valueC);
    setStatus(UI_STATUS_ONLINE);
  } else if (st == OBD_PARSE_NO_DATA) {
    setStatus(UI_STATUS_NO_DATA);
  }
}

static void advancePidOrIdle() {
  pidIndex++;
  if (pidIndex >= kPidCount) {
    lastPollDoneMs = millis();
    pidIndex = 0;
    setState(ST_POLL_GATE);
    return;
  }
  sendPidCmd(pidIndex);
}

void obdClientBegin() {
  setState(ST_CONNECTING);
}

void obdClientPoll() {
  const uint32_t now = millis();

  if (needsBtLink(state) && !obdBtIsConnected()) {
    startReconnect();
    return;
  }

  switch (state) {
    case ST_DISCONNECTED:
      setState(ST_CONNECTING);
      break;

    case ST_CONNECTING:
      setStatus(UI_STATUS_CONNECTING);
      if (!connectUiArmed) {
        connectUiArmed = true;
        break;
      }
      connectUiArmed = false;
      if (!obdBtBegin()) {
        startReconnect();
        break;
      }
      if (obdBtConnect()) {
        setStatus(UI_STATUS_STARTING);
        setupStep = 0;
        setState(ST_ELM_SETUP);
        drainBt();
        obdBtSendLine(kSetupCmds[0]);
      } else {
        startReconnect();
      }
      break;

    case ST_ELM_SETUP: {
      const AwaitResult ar = awaitPrompt(OBD_RESPONSE_TIMEOUT_MS);
      if (ar == AWAIT_PENDING) {
        break;
      }
      if (ar == AWAIT_TIMEOUT) {
        startReconnect();
        break;
      }
      setupStep++;
      if (setupStep >= kSetupCount) {
        setStatus(UI_STATUS_ONLINE);
        pidIndex = 0;
        setState(ST_POLL_GATE);
      } else {
        sendSetupCmd(setupStep);
      }
      break;
    }

    case ST_POLL_GATE:
      if (lastPollDoneMs != 0 &&
          (now - lastPollDoneMs) < OBD_POLL_INTERVAL_MS) {
        break;
      }
      pidIndex = 0;
      sendPidCmd(0);
      break;

    case ST_WAIT_PID: {
      const AwaitResult ar = awaitPrompt(OBD_RESPONSE_TIMEOUT_MS);
      if (ar == AWAIT_PENDING) {
        break;
      }
      if (ar == AWAIT_READY) {
        int valueC = 0;
        const PidStep &pid = kPids[pidIndex];
        handlePidParse(pid.parse(rxBuf, &valueC), pid.apply, valueC);
      } else {
        setStatus(UI_STATUS_TIMEOUT);
        drainBt();
      }
      advancePidOrIdle();
      break;
    }

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
  r.coolantC = coolantC;
  r.oilC = oilC;

  const uint32_t now = millis();
  r.coolantStale = (coolantC >= -100) && ((now - coolantAtMs) > OBD_STALE_MS);
  r.oilStale = (oilC >= -100) && ((now - oilAtMs) > OBD_STALE_MS);

  return r;
}
