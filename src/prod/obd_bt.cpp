#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <NimBLEDevice.h>
#include "obd_bt.h"

static const size_t kRxCap = 512;

static bool stackUp = false;
static bool linkUp = false;
static NimBLEClient *client = nullptr;
static NimBLERemoteCharacteristic *chrWrite = nullptr;
static NimBLERemoteCharacteristic *chrNotify = nullptr;

static uint8_t rxBuf[kRxCap];
static size_t rxHead = 0;
static size_t rxTail = 0;
static portMUX_TYPE rxMux = portMUX_INITIALIZER_UNLOCKED;

struct UuidTriplet {
  const char *service;
  const char *notifyUuid;
  const char *writeUuid;
};

static const UuidTriplet kUuidSets[] = {
    {"FFF0", "FFF1", "FFF2"},
    {"FFE0", "FFE1", "FFE1"},
    {"18F0", "2AF0", "2AF1"},
    {"0000fff0-0000-1000-8000-00805f9b34fb",
     "0000fff1-0000-1000-8000-00805f9b34fb",
     "0000fff2-0000-1000-8000-00805f9b34fb"},
};

static void blog(const char *msg) {
  Serial.printf("[obd-ble] %s\n", msg);
}

static std::string macConfigured() {
  uint8_t raw[] = OBD_BT_ADDR;
  char buf[18];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", raw[0], raw[1],
           raw[2], raw[3], raw[4], raw[5]);
  return std::string(buf);
}

static bool macEqual(const std::string &a, const std::string &b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a.size(); i++) {
    if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) {
      return false;
    }
  }
  return true;
}

class ObdClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient * /*c*/) override { blog("GATT connected"); }

  void onDisconnect(NimBLEClient * /*c*/, int reason) override {
    Serial.printf("[obd-ble] disconnect reason=%d\n", reason);
    linkUp = false;
    chrWrite = nullptr;
    chrNotify = nullptr;
  }

  void onPassKeyEntry(NimBLEConnInfo &connInfo) override {
    blog("passkey entry -> 1234");
    NimBLEDevice::injectPassKey(connInfo, 1234);
  }

  uint32_t onPassKeyDisplay(NimBLEConnInfo & /*connInfo*/) override {
    blog("passkey display -> 1234");
    return 1234;
  }

  void onConfirmPasskey(NimBLEConnInfo &connInfo, uint32_t pin) override {
    Serial.printf("[obd-ble] confirm passkey %u\n", pin);
    NimBLEDevice::injectConfirmPasskey(connInfo, true);
  }

  void onAuthenticationComplete(NimBLEConnInfo &connInfo) override {
    Serial.printf("[obd-ble] auth complete encrypted=%d\n",
                  connInfo.isEncrypted());
  }
};

static ObdClientCallbacks clientCb;

static void rxPushByte(uint8_t b) {
  portENTER_CRITICAL(&rxMux);
  const size_t next = (rxHead + 1) % kRxCap;
  if (next != rxTail) {
    rxBuf[rxHead] = b;
    rxHead = next;
  }
  portEXIT_CRITICAL(&rxMux);
}

static void rxPush(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    rxPushByte(data[i]);
  }
}

static void rxClear() {
  portENTER_CRITICAL(&rxMux);
  rxHead = 0;
  rxTail = 0;
  portEXIT_CRITICAL(&rxMux);
}

static void onNotify(NimBLERemoteCharacteristic * /*c*/, uint8_t *data,
                     size_t length, bool /*isNotify*/) {
  if (data != nullptr && length > 0) {
    rxPush(data, length);
  }
}

static void clearChars() {
  chrWrite = nullptr;
  chrNotify = nullptr;
}

static void logServices(NimBLEClient *c) {
  const std::vector<NimBLERemoteService *> &services = c->getServices(true);
  Serial.printf("[obd-ble] services=%u\n", (unsigned)services.size());
  for (NimBLERemoteService *svc : services) {
    if (svc == nullptr) {
      continue;
    }
    Serial.printf("[obd-ble]  svc %s\n", svc->getUUID().toString().c_str());
    const std::vector<NimBLERemoteCharacteristic *> &chars =
        svc->getCharacteristics(true);
    for (NimBLERemoteCharacteristic *ch : chars) {
      if (ch == nullptr) {
        continue;
      }
      Serial.printf(
          "[obd-ble]    chr %s wr=%d wn=%d ntf=%d ind=%d\n",
          ch->getUUID().toString().c_str(), ch->canWrite(),
          ch->canWriteNoResponse(), ch->canNotify(), ch->canIndicate());
    }
  }
}

static bool bindKnownUuids(NimBLEClient *c) {
  for (size_t i = 0; i < sizeof(kUuidSets) / sizeof(kUuidSets[0]); i++) {
    const UuidTriplet &t = kUuidSets[i];
    NimBLERemoteService *svc = c->getService(t.service);
    if (svc == nullptr) {
      continue;
    }
    NimBLERemoteCharacteristic *n = svc->getCharacteristic(t.notifyUuid);
    NimBLERemoteCharacteristic *w = svc->getCharacteristic(t.writeUuid);
    if (n == nullptr || w == nullptr) {
      continue;
    }
    if (!w->canWrite() && !w->canWriteNoResponse()) {
      continue;
    }
    if (!n->canNotify() && !n->canIndicate()) {
      continue;
    }
    chrNotify = n;
    chrWrite = w;
    Serial.printf("[obd-ble] bound known %s / %s / %s\n", t.service,
                  t.notifyUuid, t.writeUuid);
    return true;
  }
  return false;
}

static bool bindDiscoveredUuids(NimBLEClient *c) {
  const std::vector<NimBLERemoteService *> &services = c->getServices(true);
  for (NimBLERemoteService *svc : services) {
    if (svc == nullptr) {
      continue;
    }
    const std::vector<NimBLERemoteCharacteristic *> &chars =
        svc->getCharacteristics(true);
    NimBLERemoteCharacteristic *n = nullptr;
    NimBLERemoteCharacteristic *w = nullptr;
    for (NimBLERemoteCharacteristic *ch : chars) {
      if (ch == nullptr) {
        continue;
      }
      if (n == nullptr && (ch->canNotify() || ch->canIndicate())) {
        n = ch;
      }
      if (w == nullptr && (ch->canWrite() || ch->canWriteNoResponse())) {
        w = ch;
      }
    }
    if (n != nullptr && w != nullptr) {
      chrNotify = n;
      chrWrite = w;
      Serial.printf("[obd-ble] bound discovered notify=%s write=%s\n",
                    n->getUUID().toString().c_str(),
                    w->getUUID().toString().c_str());
      return true;
    }
  }
  return false;
}

static bool subscribeNotify() {
  if (chrNotify == nullptr) {
    return false;
  }
  if (chrNotify->canNotify()) {
    return chrNotify->subscribe(true, onNotify);
  }
  if (chrNotify->canIndicate()) {
    return chrNotify->subscribe(false, onNotify);
  }
  return false;
}

static bool connectAddress(const NimBLEAddress &addr) {
  Serial.printf("[obd-ble] connect %s type=%u\n", addr.toString().c_str(),
                addr.getType());

  if (client == nullptr) {
    client = NimBLEDevice::createClient();
    client->setClientCallbacks(&clientCb, false);
  }

  if (client->isConnected()) {
    client->disconnect();
    delay(200);
  }

  client->setConnectTimeout(15 * 1000);
  if (!client->connect(addr, true, false, true)) {
    blog("connect() failed");
    return false;
  }

  if (!client->secureConnection()) {
    blog("secureConnection skipped/failed (may be ok)");
  }

  clearChars();
  logServices(client);
  if (!bindKnownUuids(client) && !bindDiscoveredUuids(client)) {
    blog("no usable TX/RX characteristics");
    client->disconnect();
    return false;
  }
  if (!subscribeNotify()) {
    blog("subscribe failed");
    client->disconnect();
    clearChars();
    return false;
  }

  rxClear();
  linkUp = true;
  blog("link ready");
  return true;
}

static bool nameLooksLikeObd(const std::string &name) {
  if (name.empty()) {
    return false;
  }
  if (name == OBD_BT_NAME) {
    return true;
  }
  std::string lower = name;
  for (char &c : lower) {
    c = (char)tolower((unsigned char)c);
  }
  return lower.find("scanx") != std::string::npos ||
         lower.find("obd") != std::string::npos ||
         lower.find("rokodil") != std::string::npos ||
         lower.find("elm") != std::string::npos;
}

static bool deviceMatch(const NimBLEAdvertisedDevice *dev,
                        const std::string &wantMac) {
  if (dev == nullptr) {
    return false;
  }
  if (macEqual(dev->getAddress().toString(), wantMac)) {
    return true;
  }
  if (dev->haveName() && nameLooksLikeObd(dev->getName())) {
    return true;
  }
  if (dev->isAdvertisingService(NimBLEUUID("FFF0")) ||
      dev->isAdvertisingService(NimBLEUUID("FFE0")) ||
      dev->isAdvertisingService(NimBLEUUID("18F0"))) {
    return true;
  }
  return false;
}

static bool connectByScan() {
  NimBLEScan *scan = NimBLEDevice::getScan();
  if (scan == nullptr) {
    blog("no scanner");
    return false;
  }

  const std::string wantMac = macConfigured();
  Serial.printf("[obd-ble] scanning 12s for %s / '%s'\n", wantMac.c_str(),
                OBD_BT_NAME);

  scan->setActiveScan(true);
  scan->setInterval(45);
  scan->setWindow(30);
  scan->setMaxResults(50);
  scan->clearResults();

  const NimBLEScanResults results = scan->getResults(12 * 1000, false);
  Serial.printf("[obd-ble] scan found %d device(s)\n", results.getCount());

  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice *dev = results.getDevice(i);
    if (dev == nullptr) {
      continue;
    }
    const std::string nm = dev->haveName() ? dev->getName() : "";
    Serial.printf("[obd-ble]  [%d] %s name='%s' rssi=%d\n", i,
                  dev->getAddress().toString().c_str(), nm.c_str(),
                  dev->getRSSI());
  }

  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice *dev = results.getDevice(i);
    if (!deviceMatch(dev, wantMac)) {
      continue;
    }
    Serial.printf("[obd-ble] candidate %s\n",
                  dev->getAddress().toString().c_str());
    scan->stop();
    if (connectAddress(dev->getAddress())) {
      return true;
    }
  }

  blog("no matching BLE device / connect failed");
  return false;
}

bool obdBtBegin() {
  if (stackUp) {
    return true;
  }

  NimBLEDevice::init(OBD_BT_LOCAL_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(true, true, false);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_DISPLAY);
  NimBLEDevice::setSecurityPasskey(1234);
  stackUp = true;
  blog("stack up");
  return true;
}

bool obdBtConnect() {
  if (!stackUp) {
    return false;
  }
  return connectByScan();
}

bool obdBtIsConnected() {
  return linkUp && client != nullptr && client->isConnected() &&
         chrWrite != nullptr;
}

void obdBtDisconnect() {
  linkUp = false;
  clearChars();
  if (client != nullptr && client->isConnected()) {
    client->disconnect();
  }
  rxClear();
}

void obdBtEnd() {
  obdBtDisconnect();
  if (client != nullptr) {
    NimBLEDevice::deleteClient(client);
    client = nullptr;
  }
  if (stackUp) {
    NimBLEDevice::deinit(true);
    stackUp = false;
  }
  delay(100);
}

void obdBtSendLine(const char *line) {
  if (line == nullptr || !obdBtIsConnected() || chrWrite == nullptr) {
    return;
  }

  char out[96];
  const size_t n = strlen(line);
  if (n + 1 >= sizeof(out)) {
    return;
  }
  memcpy(out, line, n);
  out[n] = '\r';
  out[n + 1] = '\0';

  if (chrWrite->canWrite()) {
    chrWrite->writeValue((uint8_t *)out, n + 1, true);
  } else {
    chrWrite->writeValue((uint8_t *)out, n + 1, false);
  }
}

int obdBtAvailable() {
  portENTER_CRITICAL(&rxMux);
  int n = 0;
  if (rxHead >= rxTail) {
    n = (int)(rxHead - rxTail);
  } else {
    n = (int)(kRxCap - rxTail + rxHead);
  }
  portEXIT_CRITICAL(&rxMux);
  return n;
}

int obdBtRead() {
  int b = -1;
  portENTER_CRITICAL(&rxMux);
  if (rxTail != rxHead) {
    b = rxBuf[rxTail];
    rxTail = (rxTail + 1) % kRxCap;
  }
  portEXIT_CRITICAL(&rxMux);
  return b;
}
