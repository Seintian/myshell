#include "unity.h"
#include "env.h"
#include <stdlib.h>
#include <string.h>

// Test functions only - setUp/tearDown and main are in test_runner.c

void test_env_get_existing(void) {
    // Test getting an environment variable that should exist
    char *path = env_get("PATH");
    TEST_ASSERT_NOT_NULL(path);
}

void test_env_get_nonexistent(void) {
    char *result = env_get("NONEXISTENT_VAR_12345");
    TEST_ASSERT_NULL(result);
}

void test_env_set_and_get(void) {
    int result = env_set("TEST_VAR", "test_value");
    TEST_ASSERT_EQUAL(0, result);
    
    char *value = env_get("TEST_VAR");
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL_STRING("test_value", value);
}

void test_env_unset(void) {
    // First set a variable
    env_set("TEST_UNSET", "temp_value");
    char *value = env_get("TEST_UNSET");
    TEST_ASSERT_NOT_NULL(value);
    
    // Now unset it
    int result = env_unset("TEST_UNSET");
    TEST_ASSERT_EQUAL(0, result);
    
    // Should be NULL now
    value = env_get("TEST_UNSET");
    TEST_ASSERT_NULL(value);
}

void test_expand_variables_simple(void) {
    env_set("TEST_EXPAND", "hello");
    
    char *result = expand_variables("$TEST_EXPAND world");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("hello world", result);
    
    free(result);
}

void test_expand_variables_no_expansion(void) {
    char *result = expand_variables("no variables here");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("no variables here", result);
    
    free(result);
}

// End of environment tests
