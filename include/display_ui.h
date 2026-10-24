/**
 * Layout per sensor:
 *   Name
 *   Value C
 */

#pragma once

#include <Arduino.h>
#include "config.h"

struct DisplayStatus {
  float coolantC;
  float oilC;
};

void displayBegin();
void displayShow(const DisplayStatus &status);
/** Green live-dot, top-right. Pass true=on, false=off (blink). */
void displayHeartbeat(bool on);
