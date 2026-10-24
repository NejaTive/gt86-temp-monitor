/**
 * Bluetooth Classic SPP к OBD-адаптеру (клиент / master).
 * Не BLE. Только чтение потока байт — без OBD-логики.
 */

#pragma once

#include <Arduino.h>
#include "config.h"

#if ENABLE_OBD

bool obdBtBegin();
bool obdBtConnect();
bool obdBtIsConnected();
void obdBtDisconnect();

/** Неблокирующая запись строки + CR (как ждёт ELM) */
void obdBtSendLine(const char *line);

/** Сколько байт доступно на приём */
int obdBtAvailable();

int obdBtRead();

#endif  // ENABLE_OBD
