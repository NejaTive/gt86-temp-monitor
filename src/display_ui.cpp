#include "display_ui.h"
#include "temp_style.h"

#if ENABLE_TFT

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <math.h>
#include <string.h>
#include "ui_layout.h"

#if HW_PROFILE == 1
#include <Adafruit_ILI9341.h>
static Adafruit_ILI9341 tft =
    Adafruit_ILI9341(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
#define COLOR_BLACK ILI9341_BLACK
#define COLOR_WHITE ILI9341_WHITE
#define COLOR_RED ILI9341_RED
#define COLOR_GREEN ILI9341_GREEN
#define COLOR_YELLOW ILI9341_YELLOW
#else
#include <Adafruit_ST7789.h>
static Adafruit_ST7789 tft =
    Adafruit_ST7789(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
#define COLOR_BLACK ST77XX_BLACK
#define COLOR_WHITE ST77XX_WHITE
#define COLOR_RED ST77XX_RED
#define COLOR_GREEN ST77XX_GREEN
#define COLOR_YELLOW ST77XX_YELLOW
#endif

static const int16_t kWarningH = 32;
static const int16_t kTopMargin = 12;
static const int16_t kNameValueGap = 6;
static const int16_t kSensorGap = 14;
static const uint8_t kNameSize = 2;
static const uint8_t kValueSize = 3;
static const uint8_t kStatusSize = 1;
static const unsigned long kOverheatBlinkMs = 400;  // период мигания OVERHEAT

static int16_t yCoolantName = 0;
static int16_t yCoolantValue = 0;
static int16_t yOilName = 0;
static int16_t yOilValue = 0;
static int16_t valueAreaH = 0;

static float lastCoolant = -9999.0f;
static float lastOil = -9999.0f;
static bool lastCoolantStale = false;
static bool lastOilStale = false;
static int8_t lastHot = -1;
static int8_t lastOverheatOn = -1;  // фаза мигания: 1=полоска видна, 0=гасим
static char lastStatus[20] = "";

static uint16_t colorForZone(TempZone zone) {
  switch (zone) {
    case TEMP_ZONE_HOT:
      return COLOR_RED;
    case TEMP_ZONE_OK:
      return COLOR_GREEN;
    case TEMP_ZONE_COLD:
    default:
      return COLOR_YELLOW;
  }
}

static void drawStatus(const char *status) {
  const char *s = (status != nullptr) ? status : "";
  // Полоска под статус (не задевает зелёную точку справа)
  tft.fillRect(0, 0, TFT_WIDTH - 28, 12, COLOR_BLACK);
  tft.setTextSize(kStatusSize);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(2, 2);
  tft.print(s);
}

static void drawValueAt(int16_t y, float celsius, bool stale) {
  char valueBuf[16];
  uiFormatTempC(valueBuf, sizeof(valueBuf), celsius, stale);
  const UiBounds valB = uiMeasure(tft, valueBuf, kValueSize);
  // Clear only the value strip, not the whole screen
  tft.fillRect(0, y, TFT_WIDTH, valueAreaH > 0 ? valueAreaH : valB.h,
               COLOR_BLACK);
  uiDrawCentered(tft, TFT_WIDTH, y, valueBuf, kValueSize,
                 colorForZone(tempZoneFromC(celsius)));
}

/** visible=true — красная полоска + OVERHEAT; false — погасить */
static void drawWarning(bool visible) {
  const int16_t warnY = TFT_HEIGHT - kWarningH;
  tft.fillRect(0, warnY, TFT_WIDTH, kWarningH,
               visible ? COLOR_RED : COLOR_BLACK);
  if (visible) {
    uiDrawCentered(tft, TFT_WIDTH, warnY + 8, "OVERHEAT", 2, COLOR_WHITE);
  }
}

void displayBegin() {
  SPI.begin(PIN_TFT_SCLK, -1, PIN_TFT_MOSI, PIN_TFT_CS);

#if HW_PROFILE == 1
  tft.begin();
  tft.setRotation(0);
#else
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(0);
#endif

  if (PIN_TFT_BL >= 0) {
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);
  }

  tft.fillScreen(COLOR_BLACK);
  tft.setTextWrap(false);

  // Static labels once — avoids flicker on every update
  const UiBounds nameSample = uiMeasure(tft, "Coolant", kNameSize);
  const UiBounds valSample = uiMeasure(tft, "888* C", kValueSize);
  valueAreaH = valSample.h + 2;

  // Чуть ниже из‑за строки статуса сверху
  yCoolantName = kTopMargin + 4;
  yCoolantValue = yCoolantName + nameSample.h + kNameValueGap;
  yOilName = yCoolantValue + valueAreaH + kSensorGap;
  yOilValue = yOilName + nameSample.h + kNameValueGap;

  uiDrawCentered(tft, TFT_WIDTH, yCoolantName, "Coolant", kNameSize,
                 COLOR_WHITE);
  uiDrawCentered(tft, TFT_WIDTH, yOilName, "Oil", kNameSize, COLOR_WHITE);

  lastCoolant = -9999.0f;
  lastOil = -9999.0f;
  lastCoolantStale = false;
  lastOilStale = false;
  lastHot = -1;
  lastOverheatOn = -1;
  lastStatus[0] = '\0';
}

void displayShow(const DisplayStatus &status) {
  const bool hot = tempZoneFromC(status.coolantC) == TEMP_ZONE_HOT ||
                   tempZoneFromC(status.oilC) == TEMP_ZONE_HOT;

  const char *st = (status.status != nullptr) ? status.status : "";
  if (strncmp(st, lastStatus, sizeof(lastStatus)) != 0) {
    drawStatus(st);
    strncpy(lastStatus, st, sizeof(lastStatus) - 1);
    lastStatus[sizeof(lastStatus) - 1] = '\0';
  }

  // Перерисовка при смене числа или флага устаревания
  if ((int)lroundf(status.coolantC) != (int)lroundf(lastCoolant) ||
      status.coolantStale != lastCoolantStale) {
    drawValueAt(yCoolantValue, status.coolantC, status.coolantStale);
    lastCoolant = status.coolantC;
    lastCoolantStale = status.coolantStale;
  }
  if ((int)lroundf(status.oilC) != (int)lroundf(lastOil) ||
      status.oilStale != lastOilStale) {
    drawValueAt(yOilValue, status.oilC, status.oilStale);
    lastOil = status.oilC;
    lastOilStale = status.oilStale;
  }
  // OVERHEAT: пока зона HOT — мигаем; иначе гасим один раз
  if (!hot) {
    if (lastHot != 0) {
      drawWarning(false);
      lastHot = 0;
      lastOverheatOn = -1;
    }
  } else {
    lastHot = 1;
    const int8_t on =
        (int8_t)(((millis() / kOverheatBlinkMs) % 2) == 0 ? 1 : 0);
    if (on != lastOverheatOn) {
      drawWarning(on != 0);
      lastOverheatOn = on;
    }
  }
}

void displayHeartbeat(bool on) {
  const int16_t r = 6;
  const int16_t cx = TFT_WIDTH - 14;
  const int16_t cy = 14;
  tft.fillCircle(cx, cy, r, on ? COLOR_GREEN : COLOR_BLACK);
}

#else

void displayBegin() {}
void displayShow(const DisplayStatus &) {}
void displayHeartbeat(bool) {}

#endif
