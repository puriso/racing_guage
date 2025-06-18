#include <unity.h>
#include "utils.h"

void test_convertAdcToVoltage() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, convertAdcToVoltage(0));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 6.144f, convertAdcToVoltage(2047));
}

void test_convertVoltageToOilPressure() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, convertVoltageToOilPressure(0.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, convertVoltageToOilPressure(4.5f));
}

void test_convertVoltageToTemp() {
    TEST_ASSERT_FLOAT_WITHIN(0.5f, -7.5f, convertVoltageToTemp(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 66.5f, convertVoltageToTemp(4.0f));
}

void test_calculateAverage() {
    float arr[3] = {1.0f, 2.0f, 3.0f};
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, calculateAverage(arr));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_convertAdcToVoltage);
    RUN_TEST(test_convertVoltageToOilPressure);
    RUN_TEST(test_convertVoltageToTemp);
    RUN_TEST(test_calculateAverage);
    return UNITY_END();
}
