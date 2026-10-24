/**
 * SIM-стенд: Wokwi
 * Подключается из config.h при HW_PROFILE == 1
 *
 * Меняй здесь: экран симулятора, два pot (ОЖ / масло), диапазон SIM.
 * Пины TFT — общие в config.h (совпадают с машиной).
 */

#pragma once

#define TFT_WIDTH 240
#define TFT_HEIGHT 320     // ILI9341 stand-in в Wokwi
#define PIN_POT_COOLANT 34 // ОЖ
#define PIN_POT_OIL 35     // масло
#define PIN_BTN_BT_SIM 27  // Wokwi pushbutton → симуляция BT

/** Пауза Connecting при connect (на железе поиск по имени может быть дольше) */
#define BT_SIM_CONNECT_MS 3000
/** Пауза Starting после Connecting (ATZ / ATI / setup), потом Online */
#define BT_SIM_ELM_MS 2000

#define TEMP_SIM_MIN_C 0
#define TEMP_SIM_MAX_C 120
