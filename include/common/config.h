/**
 * Общий конфиг + выбор профиля.
 *
 * HW_PROFILE только из platformio.ini (-DHW_PROFILE=…):
 *   0 → config_prod.h  (машина, ST7789V, OBD)
 *   1 → config_sim.h   (Wokwi, ILI9341 + pot)
 *
 * Код везде включает только этот файл: #include "config.h"
 * Пути include/common + include/prod|sim задаются в platformio.ini.
 */

#pragma once
#define TEMP_GREEN_FROM_C 80
#define TEMP_RED_FROM_C 115

#define PIN_TFT_SCLK 18
#define PIN_TFT_MOSI 23
#define PIN_TFT_CS 5
#define PIN_TFT_DC 2
#define PIN_TFT_RST 4
#define PIN_TFT_BL -1
#define ENABLE_TFT 1

#if HW_PROFILE == 1
#include "config_sim.h"
#else
#include "config_prod.h"
#endif
