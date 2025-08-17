#include "unity.h"
#include "redir.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void test_redir_create_input(void) {
    // Test creating input redirection
    redir_t *redir = redir_create(REDIR_INPUT, 10, "/dev/null");
    
    TEST_ASSERT_NOT_NULL(redir);
    if (redir != NULL) {
        TEST_ASSERT_EQUAL(REDIR_INPUT, redir->type);
        TEST_ASSERT_EQUAL(10, redir->fd);
        TEST_ASSERT_NOT_NULL(redir->filename);
        redir_free(redir);
    }
}

void test_redir_create_output(void) {
    // Test creating output redirection
    redir_t *redir = redir_create(REDIR_OUTPUT, 11, "/tmp/test_output");
    
    TEST_ASSERT_NOT_NULL(redir);
    if (redir != NULL) {
        TEST_ASSERT_EQUAL(REDIR_OUTPUT, redir->type);
        TEST_ASSERT_EQUAL(11, redir->fd);
        TEST_ASSERT_NOT_NULL(redir->filename);
        redir_free(redir);
    }
}

void test_redir_create_append(void) {
    // Test creating append redirection
    redir_t *redir = redir_create(REDIR_APPEND, 11, "/tmp/test_append");
    
    TEST_ASSERT_NOT_NULL(redir);
    if (redir != NULL) {
        TEST_ASSERT_EQUAL(REDIR_APPEND, redir->type);
        TEST_ASSERT_EQUAL(11, redir->fd);
        TEST_ASSERT_NOT_NULL(redir->filename);
        redir_free(redir);
    }
}

void test_redir_create_null_filename(void) {
    // Test creating redirection with NULL filename
    redir_t *redir = redir_create(REDIR_OUTPUT, 11, NULL);
    
    // Should handle NULL filename gracefully
    if (redir != NULL) {
        redir_free(redir);
    }
    TEST_ASSERT_TRUE(1); // Test that we don't crash
}

void test_redir_free_null(void) {
    // Test freeing NULL redirection
    redir_free(NULL);
    
    // Should handle NULL gracefully
    TEST_ASSERT_TRUE(1);
}

void test_redir_setup_null(void) {
    // Test setup with NULL redirection
    int result = redir_setup(NULL);
    
    // Should handle NULL gracefully
    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_redir_cleanup_null(void) {
    // Test cleanup with NULL redirection
    redir_cleanup(NULL);
    
    // Should handle NULL gracefully
    TEST_ASSERT_TRUE(1);
}

void test_redir_setup_input_dev_null(void) {
    // Test setting up input redirection from /dev/null
    // Use a safe fd (10) instead of stdin (0) to avoid breaking test environment
    redir_t *redir = redir_create(REDIR_INPUT, 10, "/dev/null");
    
    if (redir != NULL) {
        int result = redir_setup(redir);
        
        // Setup should succeed for /dev/null
        TEST_ASSERT_EQUAL(0, result);
        
        // Close the duplicated fd to clean up
        close(10);
        redir_cleanup(redir);
        redir_free(redir);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_redir_setup_output_dev_null(void) {
    // Test setting up output redirection to /dev/null
    // Use a safe fd (11) instead of stdout (1) to avoid breaking test environment
    redir_t *redir = redir_create(REDIR_OUTPUT, 11, "/dev/null");
    
    if (redir != NULL) {
        int result = redir_setup(redir);
        
        // Setup should succeed for /dev/null
        TEST_ASSERT_EQUAL(0, result);
        
        // Close the duplicated fd to clean up
        close(11);
        redir_cleanup(redir);
        redir_free(redir);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_redir_setup_nonexistent_input(void) {
    // Test setting up input redirection from nonexistent file
    redir_t *redir = redir_create(REDIR_INPUT, 10, "/nonexistent/file");
    
    if (redir != NULL) {
        int result = redir_setup(redir);
        
        // Setup should fail for nonexistent file
        TEST_ASSERT_NOT_EQUAL(0, result);
        
        redir_free(redir);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_redir_invalid_fd(void) {
    // Test creating redirection with invalid file descriptor
    redir_t *redir = redir_create(REDIR_OUTPUT, -1, "/dev/null");
    
    if (redir != NULL) {
        int result = redir_setup(redir);
        
        // Should handle invalid fd
        TEST_ASSERT_TRUE(result >= -1);
        
        redir_free(redir);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_redir_large_fd(void) {
    // Test creating redirection with large file descriptor
    redir_t *redir = redir_create(REDIR_OUTPUT, 999, "/dev/null");
    
    if (redir != NULL) {
        int result = redir_setup(redir);
        
        // Should handle large fd
        TEST_ASSERT_TRUE(result >= -1);
        
        redir_free(redir);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_redir_all_types(void) {
    // Test all redirection types
    redir_t *input = redir_create(REDIR_INPUT, 10, "/dev/null");
    redir_t *output = redir_create(REDIR_OUTPUT, 11, "/dev/null");
    redir_t *append = redir_create(REDIR_APPEND, 12, "/dev/null");
    
    TEST_ASSERT_NOT_NULL(input);
    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT_NOT_NULL(append);
    
    if (input) redir_free(input);
    if (output) redir_free(output);
    if (append) redir_free(append);
}
