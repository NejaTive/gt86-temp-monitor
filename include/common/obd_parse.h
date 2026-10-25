#pragma once

#include <stddef.h>

enum ObdParseStatus {
  OBD_PARSE_OK = 0,
  OBD_PARSE_NO_DATA,
  OBD_PARSE_UNRELATED,
  OBD_PARSE_ERROR,
};

ObdParseStatus obdParseCoolantC(const char *text, int *outC);

ObdParseStatus obdParseOilC(const char *text, int *outC);
