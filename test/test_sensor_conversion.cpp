#include <unity.h>
#include "sensor_conversion.h"

void test_oil_pressure() {
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, adc_to_oil_press(409));   // 0バー付近
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 5.0f, adc_to_oil_press(2048));  // 5バー付近
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 9.9f, adc_to_oil_press(3686));  // 上限付近
}

void test_water_and_oil_temp() {
    TEST_ASSERT_FLOAT_WITHIN(0.5f, -40.0f, adc_to_water_temp(409));  // 下限付近
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 100.0f, adc_to_water_temp(2457)); // 目安温度
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 150.0f, adc_to_water_temp(3686)); // 上限付近
}

void test_invalid() {
    TEST_ASSERT_TRUE(isnan(adc_to_oil_press(-1)));    // 負値
    TEST_ASSERT_TRUE(isnan(adc_to_oil_press(5000)));  // 範囲外
    TEST_ASSERT_TRUE(isnan(adc_to_water_temp(-10)));  // 負値
    TEST_ASSERT_TRUE(isnan(adc_to_water_temp(5000))); // 範囲外
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_oil_pressure);
    RUN_TEST(test_water_and_oil_temp);
    RUN_TEST(test_invalid);
    return UNITY_END();
}
