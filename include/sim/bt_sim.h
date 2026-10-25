#pragma once

#include <Arduino.h>
#include "config.h"

void btSimBegin();
void btSimPoll();

bool btSimConnected();

const char *btSimStatus();
