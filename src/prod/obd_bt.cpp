#include <string.h>
#include "BluetoothSerial.h"
#include "obd_bt.h"

static BluetoothSerial serialBt;
static bool stackUp = false;

bool obdBtBegin() {
  if (stackUp) {
    return true;
  }

  serialBt.disableSSP();

  if (!serialBt.begin(OBD_BT_LOCAL_NAME, true)) {
    return false;
  }
  stackUp = true;

  const uint8_t pinLen = (uint8_t)strlen(OBD_BT_PIN);
  return serialBt.setPin(OBD_BT_PIN, pinLen);
}

bool obdBtConnect() {
  return serialBt.connect(String(OBD_BT_NAME));
}

bool obdBtIsConnected() {
  return serialBt.connected();
}

void obdBtDisconnect() {
  serialBt.disconnect();
}

void obdBtSendLine(const char *line) {
  if (line == nullptr || !obdBtIsConnected()) {
    return;
  }
  serialBt.print(line);
  serialBt.print('\r');
}

int obdBtAvailable() {
  if (!obdBtIsConnected()) {
    return 0;
  }
  return serialBt.available();
}

int obdBtRead() {
  if (!obdBtIsConnected()) {
    return -1;
  }
  return serialBt.read();
}
