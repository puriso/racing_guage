// 以下のテストは一時的に無効化しています
// #include <unity.h>
// #include "modules/sensor.h"

// ────────────────────── convertVoltageToOilPressure のテスト ──────────────────────
// void test_convertVoltageToOilPressure() {
//     TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, convertVoltageToOilPressure(0.0f));
//     TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, convertVoltageToOilPressure(0.5f));
//     TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, convertVoltageToOilPressure(0.7f));
//     TEST_ASSERT_FLOAT_WITHIN(0.001f, 8.75f, convertVoltageToOilPressure(4.0f));
// }

// ────────────────────── convertVoltageToTemp のテスト ──────────────────────
// void test_convertVoltageToTemp() {
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 96.68f, convertVoltageToTemp(0.5f));
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 66.54f, convertVoltageToTemp(1.0f));
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.0f, convertVoltageToTemp(2.5f));
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 14.70f, convertVoltageToTemp(3.0f));
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, -23.41f, convertVoltageToTemp(4.5f));
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 200.0f, convertVoltageToTemp(0.0f));
// }

// ────────────────────── calculateAverage のテスト ──────────────────────
// void test_calculateAverage() {
//     float a1[] = {1.0f, 2.0f, 3.0f};
//     TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, calculateAverage(a1));

//     float a2[] = {0.0f, 0.0f, 0.0f, 0.0f};
//     TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, calculateAverage(a2));

//     float a3[] = {2.0f, 4.0f, 6.0f, 8.0f};
//     TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, calculateAverage(a3));
// }

// void setup() {
//     UNITY_BEGIN();
//     RUN_TEST(test_convertVoltageToOilPressure);
//     RUN_TEST(test_convertVoltageToTemp);
//     RUN_TEST(test_calculateAverage);
//     UNITY_END();
// }

// void loop() {
//     // テストループは空
// }
