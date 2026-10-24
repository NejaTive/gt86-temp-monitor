/**
 * SIM: симуляция BT-подключения кнопкой (без реального Bluetooth).
 * Нажал → пауза как на железе (Connecting → Starting → Online), ещё раз → Offline.
 */

#pragma once

#include <Arduino.h>
#include "config.h"

void btSimBegin();
void btSimPoll();

/** true только когда «линк готов» (можно показывать °C с pot) */
bool btSimConnected();

/** Статус для UI — строки из ui_status.h */
const char *btSimStatus();
