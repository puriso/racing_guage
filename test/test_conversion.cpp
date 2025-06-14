#include <unity.h>

#include "conversion.h"

// conversion.h のテスト

// 各テストの前処理
void setUp(void) {}
// 各テストの後処理
void tearDown(void) {}

void test_convertAdcToVoltage()
{
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, convertAdcToVoltage(0));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0705f, convertAdcToVoltage(1023));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 6.144f, convertAdcToVoltage(2047));
}

void test_convertVoltageToOilPressure()
{
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, convertVoltageToOilPressure(0.4f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, convertVoltageToOilPressure(0.5f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.5f, convertVoltageToOilPressure(1.5f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, convertVoltageToOilPressure(4.5f));
}

void test_convertVoltageToTemp()
{
  TEST_ASSERT_FLOAT_WITHIN(1.0f, -23.4f, convertVoltageToTemp(0.5f));
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 25.0f, convertVoltageToTemp(2.5f));
  TEST_ASSERT_FLOAT_WITHIN(1.0f, 96.7f, convertVoltageToTemp(4.5f));
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();
  RUN_TEST(test_convertAdcToVoltage);
  RUN_TEST(test_convertVoltageToOilPressure);
  RUN_TEST(test_convertVoltageToTemp);
  return UNITY_END();
}
