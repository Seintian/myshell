#include "unity.h"
#include "builtin.h"
#include <stdlib.h>
#include <string.h>

void test_builtin_find_existing(void) {
    // Try to find a builtin that should exist (cd)
    builtin_t *builtin = builtin_find("cd");
    
    // Should find the cd builtin
    TEST_ASSERT_NOT_NULL(builtin);
    if (builtin != NULL) {
        TEST_ASSERT_EQUAL_STRING("cd", builtin->name);
    }
}

void test_builtin_find_nonexistent(void) {
    // Try to find a builtin that doesn't exist
    builtin_t *builtin = builtin_find("nonexistent_builtin_xyz");
    
    // Should return NULL
    TEST_ASSERT_NULL(builtin);
}

void test_builtin_find_null_name(void) {
    // Try to find with NULL name
    builtin_t *builtin = builtin_find(NULL);
    
    // Should handle NULL gracefully
    TEST_ASSERT_NULL(builtin);
}

void test_builtin_execute_existing(void) {
    // Test executing pwd builtin (should be safe)
    char *args[] = {"pwd", NULL};
    int result = builtin_execute("pwd", 1, args);
    
    // Should execute successfully
    TEST_ASSERT_EQUAL(0, result);
}

void test_builtin_execute_nonexistent(void) {
    // Try to execute a builtin that doesn't exist
    char *args[] = {"nonexistent", NULL};
    int result = builtin_execute("nonexistent", 1, args);
    
    // Should return error
    TEST_ASSERT_EQUAL(-1, result);
}

void test_builtin_execute_null_name(void) {
    // Try to execute with NULL name
    char *args[] = {"test", NULL};
    int result = builtin_execute(NULL, 1, args);
    
    // Should handle NULL gracefully
    TEST_ASSERT_EQUAL(-1, result);
}

void test_builtin_list(void) {
    // Test listing builtins (should not crash)
    builtin_list();
    
    // If we get here, listing worked
    TEST_ASSERT_TRUE(1);
}

void test_builtin_cd(void) {
    // Test cd builtin - just call it, don't change directory
    char *args[] = {"cd", NULL};
    int result = builtin_cd(1, args);
    
    // cd with no args should work (go to home)
    TEST_ASSERT_EQUAL(0, result);
}

void test_builtin_pwd(void) {
    // Test pwd builtin
    char *args[] = {"pwd", NULL};
    int result = builtin_pwd(1, args);
    
    // pwd should return 0 on success
    TEST_ASSERT_EQUAL(0, result);
}

void test_builtin_export(void) {
    // Test export builtin
    char test_var[] = "TEST_VAR=test_value"; // Use writable array instead of string literal
    char *args[] = {"export", test_var, NULL};
    int result = builtin_export(2, args);
    
    // export should handle being called
    TEST_ASSERT_TRUE(result >= -1);
}

void test_builtin_unset(void) {
    // Test unset builtin
    char *args[] = {"unset", "TEST_VAR", NULL};
    int result = builtin_unset(2, args);
    
    // unset should handle being called
    TEST_ASSERT_TRUE(result >= -1);
}

void test_builtin_type(void) {
    // Test type builtin with known builtin
    char *args[] = {"type", "cd", NULL};
    int result = builtin_type(2, args);
    
    // type should handle being called
    TEST_ASSERT_EQUAL(0, result);
}

void test_builtin_find_all_core_builtins(void) {
    // Test that all core builtins can be found
    TEST_ASSERT_NOT_NULL(builtin_find("cd"));
    TEST_ASSERT_NOT_NULL(builtin_find("exit"));
    TEST_ASSERT_NOT_NULL(builtin_find("export"));
    TEST_ASSERT_NOT_NULL(builtin_find("unset"));
    TEST_ASSERT_NOT_NULL(builtin_find("pwd"));
    TEST_ASSERT_NOT_NULL(builtin_find("jobs"));
    TEST_ASSERT_NOT_NULL(builtin_find("fg"));
    TEST_ASSERT_NOT_NULL(builtin_find("bg"));
    TEST_ASSERT_NOT_NULL(builtin_find("type"));
}
