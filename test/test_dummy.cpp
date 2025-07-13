#include <Arduino.h>
#include <unity.h>

// 単純な真偽テスト
void test_true(void) {
    TEST_ASSERT_TRUE(true);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_true);
    UNITY_END();
}

void loop() {
    // ループは空のまま
}

