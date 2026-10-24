/**
 * Auto-centering text helpers for Adafruit_GFX displays.
 * Positions depend on screen width/height — change TFT_WIDTH/HEIGHT in config,
 * labels stay centered.
 */

#pragma once

#include <Adafruit_GFX.h>
#include <math.h>
#include <stdio.h>

struct UiBounds {
  int16_t w;
  int16_t h;
};

inline UiBounds uiMeasure(Adafruit_GFX &gfx, const char *text, uint8_t size) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx.setTextSize(size);
  gfx.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return UiBounds{(int16_t)w, (int16_t)h};
}

inline int16_t uiCenterX(int16_t screenW, int16_t textW) {
  if (textW >= screenW) {
    return 0;
  }
  return (screenW - textW) / 2;
}

inline void uiDrawCentered(Adafruit_GFX &gfx, int16_t screenW, int16_t y,
                           const char *text, uint8_t size, uint16_t color) {
  const UiBounds b = uiMeasure(gfx, text, size);
  gfx.setTextColor(color);
  gfx.setTextSize(size);
  gfx.setCursor(uiCenterX(screenW, b.w), y);
  gfx.print(text);
}

/**
 * Vertical slot for one sensor inside the usable area (above warning bar).
 * index 0..count-1, evenly stacked and centered as a block.
 */
inline int16_t uiSensorBlockTop(int16_t screenH, int16_t warningH,
                                int count, int index) {
  const int16_t usable = screenH - warningH;
  const int16_t block = usable / count;
  return block * index;
}

/** Свежее: "95 C"; устаревшее: "95* C"; нет данных: "-- C" */
inline void uiFormatTempC(char *buf, size_t buflen, float celsius,
                          bool stale = false) {
  if (celsius < -100.0f) {
    snprintf(buf, buflen, "-- C");
  } else if (stale) {
    snprintf(buf, buflen, "%d* C", (int)lroundf(celsius));
  } else {
    snprintf(buf, buflen, "%d C", (int)lroundf(celsius));
  }
}
