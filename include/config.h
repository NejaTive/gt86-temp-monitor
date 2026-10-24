/**
 * Hardware + UI configuration for GT86 Temperature Monitor
 *
 * HW_PROFILE 0 = real ESP32 DevKit + ST7789V 1.54" (agreed wiring below)
 * HW_PROFILE 1 = Wokwi simulator (ILI9341 stand-in + pot; same TFT pins as car)
 */

#pragma once

#ifndef HW_PROFILE
#define HW_PROFILE 0
#endif

#ifndef SERIAL_BAUD
#define SERIAL_BAUD 115200
#endif

// ---------------------------------------------------------------------------
// Temperature color thresholds (°C) — edit HERE only
//   below TEMP_GREEN_FROM_C → yellow
//   TEMP_GREEN_FROM_C … TEMP_RED_FROM_C-1 → green
//   TEMP_RED_FROM_C and above → red
// ---------------------------------------------------------------------------
#ifndef TEMP_GREEN_FROM_C
#define TEMP_GREEN_FROM_C 80
#endif

#ifndef TEMP_RED_FROM_C
#define TEMP_RED_FROM_C 115
#endif

// ---------------------------------------------------------------------------
// Temperature simulation (potentiometer) — DEV / Wokwi only
// On real car (HW_PROFILE 0) pot is disabled; data will come from OBD later.
// ---------------------------------------------------------------------------
#ifndef ENABLE_TEMP_SIM
#define ENABLE_TEMP_SIM 1
#endif

#ifndef TEMP_SIM_MIN_C
#define TEMP_SIM_MIN_C 0
#endif

#ifndef TEMP_SIM_MAX_C
#define TEMP_SIM_MAX_C 120
#endif

// ---------------------------------------------------------------------------
// TFT SPI pinout (same on car and Wokwi stand)
//   SCL→D18  SDA→D23  CS→D5  DC→D2  RES→D4  BLK→3V3
// Onboard LED shares GPIO2 with DC — do not use as status LED
// ---------------------------------------------------------------------------
#define PIN_STATUS_LED -1
#define PIN_TFT_SCLK 18
#define PIN_TFT_MOSI 23
#define PIN_TFT_CS 5
#define PIN_TFT_DC 2
#define PIN_TFT_RST 4
#define PIN_TFT_BL -1
#define ENABLE_TFT 1

#if HW_PROFILE == 1
// ----- Wokwi (ILI9341 stand-in + pot on GPIO34) -----
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define PIN_POT_TEMP 34

#else
// ----- Real hardware: 1.54" IPS 240x240 ST7789V -----
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define PIN_POT_TEMP -1  // no pot on car build

#endif  // HW_PROFILE
