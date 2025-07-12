#include <unity.h>

#include <cmath>

#include "modules/sensor.h"

// 温度から電圧を求めるヘルパ関数
static float tempToVoltage(float temp_c)
{
  constexpr float SUPPLY_VOLTAGE = 5.0f;
  constexpr float THERMISTOR_R25 = 10000.0f;
  constexpr float THERMISTOR_B_CONSTANT = 3380.0f;
  constexpr float ABSOLUTE_TEMPERATURE_25 = 298.16f;  // 273.16 + 25
  constexpr float SERIES_REFERENCE_RES = 10000.0f;

  float kelvin = temp_c + 273.16f;
  float resistance = THERMISTOR_R25 * std::exp(THERMISTOR_B_CONSTANT * (1.0f / kelvin - 1.0f / ABSOLUTE_TEMPERATURE_25));
  return SUPPLY_VOLTAGE * resistance / (resistance + SERIES_REFERENCE_RES);
}

// ── 油圧テスト ──
void test_oil_pressure_0()
{
  float voltage = 0.5f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, convertVoltageToOilPressure(voltage));
}

void test_oil_pressure_3()
{
  float voltage = 0.5f + 3.0f / 2.5f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.0f, convertVoltageToOilPressure(voltage));
}

void test_oil_pressure_5()
{
  float voltage = 0.5f + 5.0f / 2.5f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, convertVoltageToOilPressure(voltage));
}

void test_oil_pressure_7()
{
  float voltage = 0.5f + 7.0f / 2.5f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 7.0f, convertVoltageToOilPressure(voltage));
}

void test_oil_pressure_10()
{
  float voltage = 0.5f + 10.0f / 2.5f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, convertVoltageToOilPressure(voltage));
}

// ── 温度テスト ──
void test_temperature_100()
{
  float voltage = tempToVoltage(100.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 100.0f, convertVoltageToTemp(voltage));
}

void test_temperature_90()
{
  float voltage = tempToVoltage(90.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 90.0f, convertVoltageToTemp(voltage));
}

void test_temperature_80()
{
  float voltage = tempToVoltage(80.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 80.0f, convertVoltageToTemp(voltage));
}

void test_temperature_70()
{
  float voltage = tempToVoltage(70.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 70.0f, convertVoltageToTemp(voltage));
}

int main()
{
  UNITY_BEGIN();
  RUN_TEST(test_oil_pressure_0);
  RUN_TEST(test_oil_pressure_3);
  RUN_TEST(test_oil_pressure_5);
  RUN_TEST(test_oil_pressure_7);
  RUN_TEST(test_oil_pressure_10);

  RUN_TEST(test_temperature_100);
  RUN_TEST(test_temperature_90);
  RUN_TEST(test_temperature_80);
  RUN_TEST(test_temperature_70);

  return UNITY_END();
}
