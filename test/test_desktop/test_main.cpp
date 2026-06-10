#include <unity.h>
// Include the header for the code you want to test here!
// #include "my_logic.h" 

void setUp(void) {
    // This runs before every single test
}

void tearDown(void) {
    // This runs after every single test
}

void test_simple_addition(void) {
    TEST_ASSERT_EQUAL(4, 2 + 2);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_simple_addition);
    
    UNITY_END();
    return 0;
}