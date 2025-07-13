#include <unity.h>

// CI向けのダミーテスト
void test_ci_dummy(void) {
    TEST_ASSERT_TRUE(true);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_ci_dummy);
    UNITY_END();
}

void loop() {
    // テスト用なので空
}
