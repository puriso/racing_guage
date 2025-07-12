#include <unity.h>
#include "modules/sensor.h"
// センサ変換関数の単体テスト

void test_convertAdcToVoltage_nominal() {
    // 正常範囲の ADC 値
    float result = convertAdcToVoltage(1024);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.07f, result);
}

void test_convertAdcToVoltage_negative() {
    // 負の ADC 値でも計算可能か確認
    float result = convertAdcToVoltage(-1024);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -3.07f, result);
}

void test_convertVoltageToTemp_normal() {
    // 2.5V は約 25℃
    float temp = convertVoltageToTemp(2.5f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 25.0f, temp);
}

void test_convertVoltageToTemp_invalid_low() {
    // 0V 以下は異常値として 200℃ を返す
    TEST_ASSERT_EQUAL_FLOAT(200.0f, convertVoltageToTemp(0.0f));
}

void test_convertVoltageToTemp_invalid_high() {
    // 5V 以上も異常値
    TEST_ASSERT_EQUAL_FLOAT(200.0f, convertVoltageToTemp(5.0f));
}

void setup() {
    // テスト初期化
    UNITY_BEGIN();
    RUN_TEST(test_convertAdcToVoltage_nominal);
    RUN_TEST(test_convertAdcToVoltage_negative);
    RUN_TEST(test_convertVoltageToTemp_normal);
    RUN_TEST(test_convertVoltageToTemp_invalid_low);
    RUN_TEST(test_convertVoltageToTemp_invalid_high);
    UNITY_END();
}

void loop() {
    // ループ処理は使用しない
}
