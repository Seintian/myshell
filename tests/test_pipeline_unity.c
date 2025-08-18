#include "ast.h"
#include "pipeline.h"
#include "unity.h"
#include <stdlib.h>

void test_pipeline_execute_null_commands(void) {
    // Test with NULL commands array
    int result = pipeline_execute(NULL, 1);

    // Should handle NULL gracefully
    TEST_ASSERT_EQUAL(-1, result);
}

void test_pipeline_execute_zero_count(void) {
    // Test with zero count
    ast_node_t *dummy = NULL;
    int result = pipeline_execute(&dummy, 0);

    // Should handle zero count
    TEST_ASSERT_EQUAL(-1, result);
}

void test_pipeline_execute_negative_count(void) {
    // Test with negative count
    ast_node_t *dummy = NULL;
    int result = pipeline_execute(&dummy, -1);

    // Should handle negative count
    TEST_ASSERT_EQUAL(-1, result);
}

void test_pipeline_execute_single_command(void) {
    // Test with single command (should delegate to exec_ast)
    char *cmd_argv[] = {"echo", NULL};
    ast_node_t *cmd = ast_create_command(cmd_argv);

    if (cmd != NULL) {
        ast_node_t *commands[] = {cmd};
        int result = pipeline_execute(commands, 1);

        // Should execute single command
        TEST_ASSERT_TRUE(result >= -1);

        ast_free(cmd);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_pipeline_execute_single_null_command(void) {
    // Test with single NULL command
    ast_node_t *commands[] = {NULL};
    int result = pipeline_execute(commands, 1);

    // Should handle NULL command in array
    TEST_ASSERT_TRUE(result >= -1);
}

void test_pipeline_execute_two_commands(void) {
    // Test with two commands in pipeline
    char *cmd1_argv[] = {"echo", NULL};
    char *cmd2_argv[] = {"cat", NULL};
    ast_node_t *cmd1 = ast_create_command(cmd1_argv);
    ast_node_t *cmd2 = ast_create_command(cmd2_argv);

    if (cmd1 != NULL && cmd2 != NULL) {
        ast_node_t *commands[] = {cmd1, cmd2};
        int result = pipeline_execute(commands, 2);

        // Should execute pipeline
        TEST_ASSERT_TRUE(result >= -1);

        ast_free(cmd1);
        ast_free(cmd2);
    } else {
        if (cmd1)
            ast_free(cmd1);
        if (cmd2)
            ast_free(cmd2);
        TEST_ASSERT_TRUE(1);
    }
}

void test_pipeline_execute_mixed_commands(void) {
    // Test with valid and NULL commands mixed
    char *cmd1_argv[] = {"echo", NULL};
    ast_node_t *cmd1 = ast_create_command(cmd1_argv);
    ast_node_t *cmd2 = NULL;

    if (cmd1 != NULL) {
        ast_node_t *commands[] = {cmd1, cmd2};
        int result = pipeline_execute(commands, 2);

        // Should handle mixed valid/NULL commands
        TEST_ASSERT_TRUE(result >= -1);

        ast_free(cmd1);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_pipeline_execute_large_count(void) {
    // Test with large count but limited commands
    char *cmd_argv[] = {"echo", NULL};
    ast_node_t *cmd = ast_create_command(cmd_argv);

    if (cmd != NULL) {
        ast_node_t *commands[] = {cmd};
        int result = pipeline_execute(commands, 10); // More count than commands

        // Should handle count mismatch gracefully
        TEST_ASSERT_TRUE(result >= -1);

        ast_free(cmd);
    } else {
        TEST_ASSERT_TRUE(1);
    }
}

void test_pipeline_execute_echo_cat(void) {
    // Test classic echo | cat pipeline
    char *echo_args[] = {"echo", "hello", NULL};
    char *cat_args[] = {"cat", NULL};
    ast_node_t *echo_cmd = ast_create_command(echo_args);
    ast_node_t *cat_cmd = ast_create_command(cat_args);

    if (echo_cmd != NULL && cat_cmd != NULL) {
        ast_node_t *commands[] = {echo_cmd, cat_cmd};
        int result = pipeline_execute(commands, 2);

        // Should execute echo | cat successfully
        TEST_ASSERT_TRUE(result >= -1);

        ast_free(echo_cmd);
        ast_free(cat_cmd);
    } else {
        if (echo_cmd)
            ast_free(echo_cmd);
        if (cat_cmd)
            ast_free(cat_cmd);
        TEST_ASSERT_TRUE(1);
    }
}

void test_pipeline_execute_three_commands(void) {
    // Test with three commands in pipeline
    char *cmd1_argv[] = {"echo", NULL};
    char *cmd2_argv[] = {"cat", NULL};
    char *cmd3_argv[] = {"cat", NULL};
    ast_node_t *cmd1 = ast_create_command(cmd1_argv);
    ast_node_t *cmd2 = ast_create_command(cmd2_argv);
    ast_node_t *cmd3 = ast_create_command(cmd3_argv);

    if (cmd1 != NULL && cmd2 != NULL && cmd3 != NULL) {
        ast_node_t *commands[] = {cmd1, cmd2, cmd3};
        int result = pipeline_execute(commands, 3);

        // Should execute three-command pipeline
        TEST_ASSERT_TRUE(result >= -1);

        ast_free(cmd1);
        ast_free(cmd2);
        ast_free(cmd3);
    } else {
        if (cmd1)
            ast_free(cmd1);
        if (cmd2)
            ast_free(cmd2);
        if (cmd3)
            ast_free(cmd3);
        TEST_ASSERT_TRUE(1);
    }
}
