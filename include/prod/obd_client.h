/**
 * OBD-клиент: ELM327 по SPP, последовательный опрос coolant / oil.
 * Только чтение. Без DTC, записи и смены конфига ЭБУ.
 */

#pragma once

#include <Arduino.h>
#include "config.h"

struct ObdReading {
  float coolantC;     // < -100 → нет данных (-- C)
  float oilC;
  bool coolantStale;  // true → показать NN* C
  bool oilStale;
  const char *status;  // короткий статус для UI (статическая строка)
};

void obdClientBegin();
/** Неблокирующий тик: вызывать из loop() часто */
void obdClientPoll();
ObdReading obdClientReading();
