#include "unity.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

// Test functions only - setUp/tearDown and main are in test_runner.c

void test_strdup_safe(void) {
    char *original = "test string";
    char *copy = strdup_safe(original);

    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_STRING(original, copy);
    TEST_ASSERT_NOT_EQUAL(original, copy); // Different memory addresses

    free(copy);
}

void test_strdup_safe_null(void) {
    char *copy = strdup_safe(NULL);
    TEST_ASSERT_NULL(copy);
}

void test_split_string(void) {
    char **result = split_string("one,two,three", ",");

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("one", result[0]);
    TEST_ASSERT_EQUAL_STRING("two", result[1]);
    TEST_ASSERT_EQUAL_STRING("three", result[2]);
    TEST_ASSERT_NULL(result[3]);

    free_string_array(result);
}

void test_split_string_single(void) {
    char **result = split_string("single", ",");

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("single", result[0]);
    TEST_ASSERT_NULL(result[1]);

    free_string_array(result);
}

void test_string_array_length(void) {
    char *test_array[] = {"one", "two", "three", NULL};
    size_t length = string_array_length(test_array);

    TEST_ASSERT_EQUAL(3, length);
}

void test_string_array_length_empty(void) {
    char *test_array[] = {NULL};
    size_t length = string_array_length(test_array);

    TEST_ASSERT_EQUAL(0, length);
}

void test_string_array_length_null(void) {
    size_t length = string_array_length(NULL);
    TEST_ASSERT_EQUAL(0, length);
}

// End of utility tests
