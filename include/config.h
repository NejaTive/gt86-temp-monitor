/**
 * Hardware + UI configuration for GT86 Temperature Monitor
 *
 * IMPORTANT:
 * - Do NOT invent pin numbers for the real car display.
 * - HW_PROFILE 0 = real ESP32 DevKit (TFT pins TBD, display code off).
 * - HW_PROFILE 1 = Wokwi / sim wiring (NOT for your physical module).
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
// Temperature simulation (manual pot only — NOT OBD)
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

#if HW_PROFILE == 1
#define PIN_STATUS_LED -1  // no LED on Wokwi diagram
#define PIN_TFT_SCLK 18
#define PIN_TFT_MOSI 23
#define PIN_TFT_CS 15
#define PIN_TFT_DC 2
#define PIN_TFT_RST -1  // unused on Wokwi ILI9341 part
#define PIN_TFT_BL -1
#define ENABLE_TFT 1
// wokwi-ili9341 panel is 240x320 (not the future 240x240 ST7789)
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define PIN_POT_TEMP 34  // manual temperature knob

#else
#ifndef PIN_STATUS_LED
#define PIN_STATUS_LED 2
#endif
#ifndef PIN_TFT_SCLK
#define PIN_TFT_SCLK -1
#endif
#ifndef PIN_TFT_MOSI
#define PIN_TFT_MOSI -1
#endif
#ifndef PIN_TFT_CS
#define PIN_TFT_CS -1
#endif
#ifndef PIN_TFT_DC
#define PIN_TFT_DC -1
#endif
#ifndef PIN_TFT_RST
#define PIN_TFT_RST -1
#endif
#ifndef PIN_TFT_BL
#define PIN_TFT_BL -1
#endif
#ifndef ENABLE_TFT
#define ENABLE_TFT 0
#endif
#ifndef TFT_WIDTH
#define TFT_WIDTH 240
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 240
#endif
// No pot on real board yet — Serial: type a number 0…120 + Enter
#ifndef PIN_POT_TEMP
#define PIN_POT_TEMP -1
#endif
#endif  // HW_PROFILE
