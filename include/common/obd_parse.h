/**
 * Разбор ответов OBD/ELM в °C.
 * Без Bluetooth — удобно тестировать на строках из ТЗ.
 *
 * Coolant: PID 01 05 → ответ … 41 05 <byte>, °C = byte − 40
 * Oil (GT86): 22 10 E7 → … 62 10 E7 <byte>, °C = byte − 40
 * Не использовать 01 5C для масла.
 */

#pragma once

#include <stddef.h>

enum ObdParseStatus {
  OBD_PARSE_OK = 0,       // температура извлечена
  OBD_PARSE_NO_DATA,      // ELM вернул NO DATA
  OBD_PARSE_UNRELATED,    // строка не про этот PID (ещё копим ответ)
  OBD_PARSE_ERROR,        // мусор / обрезано
};

/**
 * Ищет в text ответ coolant (41 05 …).
 * Устойчив к CAN ID, длине, пробелам, CR/LF, разному регистру hex.
 * outC — только при OBD_PARSE_OK.
 */
ObdParseStatus obdParseCoolantC(const char *text, int *outC);

/**
 * Ищет в text ответ oil DID 10E7 (62 10 E7 …).
 * outC — только при OBD_PARSE_OK.
 */
ObdParseStatus obdParseOilC(const char *text, int *outC);
