/**
 * DEV: симуляция BT-подключения кнопкой (без реального Bluetooth).
 * Нажал → пауза как на железе (Connecting → Starting → Online), ещё раз → Offline.
 */

#pragma once

#include <Arduino.h>
#include "config.h"

#if ENABLE_BT_SIM

void btSimBegin();
void btSimPoll();

/** true только когда «линк готов» (можно показывать °C с pot) */
bool btSimConnected();

/** Статус для UI — строки из ui_status.h */
const char *btSimStatus();

#else

inline void btSimBegin() {}
inline void btSimPoll() {}
inline bool btSimConnected() { return false; }
inline const char *btSimStatus() { return "Offline"; }  // как UI_STATUS_OFFLINE

#endif
