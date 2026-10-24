/**
 * Layout per sensor:
 *   Name
 *   Value C   (устаревшее: NN* C)
 * Краткий статус BT/OBD — сверху слева (см. ui_status.h).
 */

#pragma once

#include <Arduino.h>
#include "config.h"

struct DisplayStatus {
  float coolantC;
  float oilC;
  bool coolantStale;
  bool oilStale;
  const char *status;  // например UI_STATUS_ONLINE
};

void displayBegin();
void displayShow(const DisplayStatus &status);
/** Green live-dot, top-right. Pass true=on, false=off (blink). */
void displayHeartbeat(bool on);
