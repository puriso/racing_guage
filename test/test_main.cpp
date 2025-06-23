#include <unity.h>
#include "config.h"  // 色変換テスト用の設定
#include "sensor.h"  // 平均計算のテンプレート

// rgb565 関数のテスト
void test_rgb565() {
    TEST_ASSERT_EQUAL_UINT16(0xF800, rgb565(255, 0, 0));
    TEST_ASSERT_EQUAL_UINT16(0x07E0, rgb565(0, 255, 0));
    TEST_ASSERT_EQUAL_UINT16(0x001F, rgb565(0, 0, 255));
}

// calculateAverage テンプレートのテスト
void test_calculate_average() {
    float values[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 3.0f, calculateAverage(values));
}

// Unity テストランナー
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_rgb565);
    RUN_TEST(test_calculate_average);
    return UNITY_END();
}
