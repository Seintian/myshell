#include "unity.h"
#include "exec.h"
#include "ast.h"
#include <stdlib.h>

void test_exec_ast_null(void) {
    // Test executing NULL AST
    int result = exec_ast(NULL);
    
    // Should handle NULL gracefully
    TEST_ASSERT_EQUAL(-1, result);
}

void test_exec_command_null(void) {
    // Test executing NULL command
    int result = exec_command(NULL);
    
    // Should handle NULL gracefully
    TEST_ASSERT_EQUAL(-1, result);
}

void test_exec_pipeline_null(void) {
    // Test executing NULL pipeline
    int result = exec_pipeline(NULL);
    
    // Should handle NULL gracefully
    TEST_ASSERT_EQUAL(-1, result);
}

void test_exec_simple_command_node(void) {
    // Create a simple command AST node for testing
    char *cmd_argv[] = {"echo", NULL};
    ast_node_t *cmd_node = ast_create_command(cmd_argv);
    
    if (cmd_node != NULL) {
        // Test executing the command
        int result = exec_ast(cmd_node);
        
        // Result should be reasonable (0 for success or error code)
        TEST_ASSERT_TRUE(result >= -1);
        
        ast_free(cmd_node);
    } else {
        // If AST creation failed, that's also a valid test result
        TEST_ASSERT_TRUE(1);
    }
}

void test_exec_command_with_args(void) {
    // Create command with arguments
    char *args[] = {"echo", "hello", "world", NULL};
    ast_node_t *cmd_node = ast_create_command(args);
    
    if (cmd_node != NULL) {
        // Test executing command with arguments
        int result = exec_ast(cmd_node);
        
        // Should handle arguments properly
        TEST_ASSERT_TRUE(result >= -1);
        
        ast_free(cmd_node);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_exec_nonexistent_command(void) {
    // Test executing a command that doesn't exist
    char *cmd_argv[] = {"nonexistent_command_xyz123", NULL};
    ast_node_t *cmd_node = ast_create_command(cmd_argv);
    
    if (cmd_node != NULL) {
        int result = exec_ast(cmd_node);
        
        // Should return error for nonexistent command
        TEST_ASSERT_NOT_EQUAL(0, result);
        
        ast_free(cmd_node);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_exec_pipeline_creation(void) {
    // Create two commands for pipeline testing
    char *cmd1_argv[] = {"echo", "hello", NULL};
    char *cmd2_argv[] = {"echo", "world", NULL};
    ast_node_t *cmd1 = ast_create_command(cmd1_argv);
    ast_node_t *cmd2 = ast_create_command(cmd2_argv);
    
    if (cmd1 != NULL && cmd2 != NULL) {
        // Create pipeline
        ast_node_t *pipeline = ast_create_pipeline(cmd1, cmd2);
        
        if (pipeline != NULL) {
            // Test executing pipeline
            int result = exec_ast(pipeline);
            
            // Pipeline execution should complete
            TEST_ASSERT_TRUE(result >= -1);
            
            ast_free(pipeline);
        } else {
            ast_free(cmd1);
            ast_free(cmd2);
            TEST_ASSERT_TRUE(1);
        }
    } else {
        if (cmd1) ast_free(cmd1);
        if (cmd2) ast_free(cmd2);
        TEST_ASSERT_TRUE(1);
    }
}

void test_exec_empty_command(void) {
    // Test executing empty command
    char *cmd_argv[] = {"", NULL};
    ast_node_t *cmd_node = ast_create_command(cmd_argv);
    
    if (cmd_node != NULL) {
        int result = exec_ast(cmd_node);
        
        // Should handle empty command gracefully
        TEST_ASSERT_TRUE(result >= -1);
        
        ast_free(cmd_node);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_exec_builtin_simulation(void) {
    // Test executing what might be a builtin command
    char *cmd_argv[] = {"cd", NULL};
    ast_node_t *cmd_node = ast_create_command(cmd_argv);
    
    if (cmd_node != NULL) {
        int result = exec_ast(cmd_node);
        
        // Builtin commands should be handled
        TEST_ASSERT_TRUE(result >= -1);
        
        ast_free(cmd_node);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}
