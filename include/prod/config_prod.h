#pragma once

#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define PIN_POT_COOLANT -1
#define PIN_POT_OIL -1
#define PIN_BTN_BT_SIM -1

#define OBD_BT_NAME "Rokodil ScanX"
#define OBD_BT_PIN "1234"
#define OBD_BT_ADDR \
  { 0x22, 0xC0, 0xC0, 0x06, 0x61, 0x0F }
#define OBD_BT_LOCAL_NAME "GT86Temp"

#define OBD_POLL_INTERVAL_MS 1000
#define OBD_RESPONSE_TIMEOUT_MS 2000
#define OBD_RECONNECT_DELAY_MS 3000
#define OBD_STALE_MS 5000
