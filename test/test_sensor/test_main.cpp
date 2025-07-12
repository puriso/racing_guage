#include <Arduino.h>
#include <unity.h>
#include "modules/sensor.h"
#include "../../src/modules/sensor.cpp" // センサー変換関数をリンクするために追加
#include <math.h>

// 温度から電圧を求めるヘルパー
// センサー変換定数は sensor.cpp と同じ値をテスト側で定義している
// config から読み込む処理は行っていない
static float voltageFromTemperature(float temp)
{
    constexpr float SUPPLY_VOLTAGE = 5.0f;
    constexpr float THERMISTOR_R25 = 10000.0f;
    constexpr float THERMISTOR_B_CONSTANT = 3380.0f;
    constexpr float SERIES_REFERENCE_RES = 10000.0f;
    constexpr float ABSOLUTE_TEMPERATURE_25 = 298.16f; // 273.16 + 25

    float kelvin = temp + 273.16f;
    float resistance = THERMISTOR_R25 * exp(THERMISTOR_B_CONSTANT * (1.0f/kelvin - 1.0f/ABSOLUTE_TEMPERATURE_25));
    return SUPPLY_VOLTAGE * resistance / (resistance + SERIES_REFERENCE_RES);
}

// ─── 圧力テスト ────────────────────────────────────────────────
static void test_pressure_0(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, convertVoltageToOilPressure(0.5f));
}

static void test_pressure_3(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.0f, convertVoltageToOilPressure(1.7f));
}

static void test_pressure_5(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, convertVoltageToOilPressure(2.5f));
}

static void test_pressure_7(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 7.0f, convertVoltageToOilPressure(3.3f));
}

static void test_pressure_10(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, convertVoltageToOilPressure(4.5f));
}

// ─── 温度テスト ────────────────────────────────────────────────
static void test_temp_100(void) {
    float v = voltageFromTemperature(100.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 100.0f, convertVoltageToTemp(v));
}

static void test_temp_90(void) {
    float v = voltageFromTemperature(90.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 90.0f, convertVoltageToTemp(v));
}

static void test_temp_80(void) {
    float v = voltageFromTemperature(80.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 80.0f, convertVoltageToTemp(v));
}

static void test_temp_70(void) {
    float v = voltageFromTemperature(70.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 70.0f, convertVoltageToTemp(v));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_pressure_0);
    RUN_TEST(test_pressure_3);
    RUN_TEST(test_pressure_5);
    RUN_TEST(test_pressure_7);
    RUN_TEST(test_pressure_10);
    RUN_TEST(test_temp_100);
    RUN_TEST(test_temp_90);
    RUN_TEST(test_temp_80);
    RUN_TEST(test_temp_70);
    UNITY_END();
}

void loop() {
    // ループは使用しない
}

