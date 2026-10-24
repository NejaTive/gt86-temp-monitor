/**
 * Реальное устройство: ESP32 + ST7789V + ScanX (OBD)
 * Подключается из config.h при HW_PROFILE == 0
 *
 * Меняй здесь: имя BT адаптера, PIN, интервалы опроса.
 * Пины TFT — общие в config.h (совпадают с Wokwi).
 */

#pragma once

#define ENABLE_TEMP_SIM 0
#define ENABLE_OBD 1
#define ENABLE_BT_SIM 0

#define TFT_WIDTH 240
#define TFT_HEIGHT 240  // 1.54" IPS ST7789V
#define PIN_POT_COOLANT -1 // на машине потенциометров нет
#define PIN_POT_OIL -1
#define PIN_BTN_BT_SIM -1  // кнопка BT-SIM только на DEV

// ----- Bluetooth OBD (Classic SPP, ELM327-совместимый ScanX) -----
// Имя — как в списке Bluetooth телефона
#define OBD_BT_NAME "ScanX"
#define OBD_BT_PIN "1234"
#define OBD_BT_LOCAL_NAME "GT86Temp"  // имя нашего ESP32 как SPP master

#define OBD_POLL_INTERVAL_MS 1000     // ~1 Гц на параметр
#define OBD_RESPONSE_TIMEOUT_MS 2000  // таймаут ответа ELM
#define OBD_RECONNECT_DELAY_MS 3000   // пауза перед повторным connect
#define OBD_STALE_MS 3000             // старше → "NN* C" на экране
