/**
 * Unit-тесты парсера OBD на примерах из ТЗ.
 * Запуск: pio test -e native
 */

#include <unity.h>
#include "obd_parse.h"

void test_oil_tz_70c(void) {
  int c = -999;
  // 7E8 04 62 10 E7 6E → 0x6E − 40 = 70
  TEST_ASSERT_EQUAL(OBD_PARSE_OK, obdParseOilC("7E8046210E76E", &c));
  TEST_ASSERT_EQUAL(70, c);
}

void test_oil_tz_67c(void) {
  int c = -999;
  TEST_ASSERT_EQUAL(OBD_PARSE_OK, obdParseOilC("7E8046210E76B", &c));
  TEST_ASSERT_EQUAL(67, c);
}

void test_oil_with_spaces(void) {
  int c = -999;
  TEST_ASSERT_EQUAL(OBD_PARSE_OK,
                    obdParseOilC("7E8 04 62 10 E7 6E\r\n>", &c));
  TEST_ASSERT_EQUAL(70, c);
}

void test_coolant_tz_69c(void) {
  int c = -999;
  // 7E8 03 41 05 6D → 0x6D − 40 = 69
  TEST_ASSERT_EQUAL(OBD_PARSE_OK, obdParseCoolantC("7E80341056D", &c));
  TEST_ASSERT_EQUAL(69, c);
}

void test_coolant_with_spaces(void) {
  int c = -999;
  TEST_ASSERT_EQUAL(OBD_PARSE_OK,
                    obdParseCoolantC("7E8 03 41 05 6D\r", &c));
  TEST_ASSERT_EQUAL(69, c);
}

void test_no_data(void) {
  int c = 0;
  TEST_ASSERT_EQUAL(OBD_PARSE_NO_DATA, obdParseOilC("NO DATA", &c));
  TEST_ASSERT_EQUAL(OBD_PARSE_NO_DATA, obdParseCoolantC("NODATA\r\n>", &c));
}

void test_unrelated_wrong_pid(void) {
  int c = 0;
  // Ответ coolant не должен парситься как oil
  TEST_ASSERT_EQUAL(OBD_PARSE_UNRELATED, obdParseOilC("7E80341056D", &c));
  TEST_ASSERT_EQUAL(OBD_PARSE_UNRELATED, obdParseCoolantC("7E8046210E76E", &c));
}

void test_oil_not_015c(void) {
  // Убеждаемся: парсер масла ищет только 62 10 E7, не 01 5C
  int c = 0;
  TEST_ASSERT_EQUAL(OBD_PARSE_UNRELATED, obdParseOilC("41 5C 6E", &c));
}

void setUp(void) {}
void tearDown(void) {}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_oil_tz_70c);
  RUN_TEST(test_oil_tz_67c);
  RUN_TEST(test_oil_with_spaces);
  RUN_TEST(test_coolant_tz_69c);
  RUN_TEST(test_coolant_with_spaces);
  RUN_TEST(test_no_data);
  RUN_TEST(test_unrelated_wrong_pid);
  RUN_TEST(test_oil_not_015c);
  return UNITY_END();
}

#if defined(ARDUINO)
#include <Arduino.h>
void setup() {
  delay(2000);
  runUnityTests();
}
void loop() {}
#else
int main(void) { return runUnityTests(); }
#endif
