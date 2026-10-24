#include "obd_bt.h"

#if ENABLE_OBD

#include "BluetoothSerial.h"

#include <string.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth Classic не включён в SDK. Нужен ESP32 с Bluedroid.
#endif
#if !defined(CONFIG_BT_SPP_ENABLED)
#error SPP не включён. BluetoothSerial / Classic SPP недоступен.
#endif

static BluetoothSerial serialBt;
static bool begun = false;

bool obdBtBegin() {
  if (begun) {
    return true;
  }
  // Фиксированный PIN (как у многих ELM/ScanX), не SSP-passkey
  serialBt.disableSSP();
  // true = master: сами подключаемся к адаптеру
  if (!serialBt.begin(OBD_BT_LOCAL_NAME, true)) {
    Serial.println(F("[BT] begin failed"));
    return false;
  }
  const uint8_t pinLen = (uint8_t)strlen(OBD_BT_PIN);
  if (!serialBt.setPin(OBD_BT_PIN, pinLen)) {
    Serial.println(F("[BT] setPin failed"));
  }
  begun = true;
  Serial.printf("[BT] master \"%s\", PIN %s\r\n", OBD_BT_LOCAL_NAME,
                OBD_BT_PIN);
  return true;
}

bool obdBtConnect() {
  if (!begun && !obdBtBegin()) {
    return false;
  }
  // Внимание: connect по имени может занять до ~30 с (поиск BT).
  Serial.printf("[BT] connecting to \"%s\"...\r\n", OBD_BT_NAME);
  const bool ok = serialBt.connect(String(OBD_BT_NAME));
  Serial.println(ok ? F("[BT] connected") : F("[BT] connect failed"));
  return ok;
}

bool obdBtIsConnected() { return begun && serialBt.connected(); }

void obdBtDisconnect() {
  if (begun) {
    serialBt.disconnect();
  }
}

void obdBtSendLine(const char *line) {
  if (line == nullptr || !obdBtIsConnected()) {
    return;
  }
  serialBt.print(line);
  serialBt.print('\r');
  Serial.printf("[BT>>] %s\r\n", line);
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

#endif  // ENABLE_OBD
