#include <unity.h>
#include "sensor_utils.h"

// センサー計算関数のテスト

void test_convertAdcToVoltage() {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, convertAdcToVoltage(0));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 6.144f, convertAdcToVoltage(2047));
}

void test_convertVoltageToOilPressure() {
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, convertVoltageToOilPressure(0.4f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.5f, convertVoltageToOilPressure(1.5f));
}

void test_convertVoltageToTemp() {
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 25.0f, convertVoltageToTemp(2.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 200.0f, convertVoltageToTemp(0.0f));
}

void test_calculateAverage() {
    float arr[3] = {1.0f, 2.0f, 3.0f};
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, calculateAverage(arr));
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_convertAdcToVoltage);
    RUN_TEST(test_convertVoltageToOilPressure);
    RUN_TEST(test_convertVoltageToTemp);
    RUN_TEST(test_calculateAverage);
    return UNITY_END();
}
