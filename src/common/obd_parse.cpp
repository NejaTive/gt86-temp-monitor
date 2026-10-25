#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "obd_parse.h"

static size_t normalizeHexish(const char *src, char *dst, size_t dstSize) {
  size_t n = 0;
  if (dstSize == 0) {
    return 0;
  }
  for (const char *p = src; *p != '\0' && n + 1 < dstSize; ++p) {
    const unsigned char c = (unsigned char)*p;
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '>') {
      continue;
    }
    dst[n++] = (char)toupper(c);
  }
  dst[n] = '\0';
  return n;
}

static bool looksLikeNoData(const char *norm) {
  return strstr(norm, "NODATA") != nullptr;
}

static bool hexByteAt(const char *s, int *out) {
  if (!isxdigit((unsigned char)s[0]) || !isxdigit((unsigned char)s[1])) {
    return false;
  }
  char tmp[3] = {s[0], s[1], '\0'};
  *out = (int)strtol(tmp, nullptr, 16);
  return true;
}

static ObdParseStatus parseAfterPattern(const char *norm, const char *pattern, int *outC) {
  if (looksLikeNoData(norm)) {
    return OBD_PARSE_NO_DATA;
  }

  const char *hit = strstr(norm, pattern);
  if (hit == nullptr) {
    return OBD_PARSE_UNRELATED;
  }

  const char *data = hit + strlen(pattern);
  int valueByte = 0;
  if (!hexByteAt(data, &valueByte)) {
    return OBD_PARSE_ERROR;
  }

  if (outC != nullptr) {
    *outC = valueByte - 40;
  }
  return OBD_PARSE_OK;
}

ObdParseStatus obdParseCoolantC(const char *text, int *outC) {
  if (text == nullptr) {
    return OBD_PARSE_ERROR;
  }
  char norm[192];
  normalizeHexish(text, norm, sizeof(norm));
  if (norm[0] == '\0') {
    return OBD_PARSE_UNRELATED;
  }

  return parseAfterPattern(norm, "4105", outC);
}

ObdParseStatus obdParseOilC(const char *text, int *outC) {
  if (text == nullptr) {
    return OBD_PARSE_ERROR;
  }
  char norm[192];
  normalizeHexish(text, norm, sizeof(norm));
  if (norm[0] == '\0') {
    return OBD_PARSE_UNRELATED;
  }

  return parseAfterPattern(norm, "6210E7", outC);
}
