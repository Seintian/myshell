#include "ast.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

void test_ast_free_null(void) {
    ast_free(NULL);
    TEST_ASSERT_TRUE(1);
}

void test_ast_create_command_basic(void) {
    char *argv[] = {"echo", "hello", NULL};
    ast_node_t *cmd = ast_create_command(argv);
    TEST_ASSERT_NOT_NULL(cmd);
    // Freeing should be safe
    ast_free(cmd);
}

void test_ast_create_pipeline_basic(void) {
    char *argv1[] = {"echo", "x", NULL};
    char *argv2[] = {"cat", NULL};
    ast_node_t *left = ast_create_command(argv1);
    ast_node_t *right = ast_create_command(argv2);
    TEST_ASSERT_NOT_NULL(left);
    TEST_ASSERT_NOT_NULL(right);

    ast_node_t *pipe = ast_create_pipeline(left, right);
    TEST_ASSERT_NOT_NULL(pipe);

    // Should free the whole subtree without leaks/crash
    ast_free(pipe);
}

void test_ast_create_command_null_argv(void) {
    ast_node_t *cmd = ast_create_command(NULL);
    TEST_ASSERT_NOT_NULL(cmd);
    ast_free(cmd);
}

void test_ast_create_command_deep_copy_safe(void) {
    // Use dynamically allocated strings to ensure deep-copy is independent
    char *a0 = strdup("echo");
    char *a1 = strdup("deepcopy");
    char *argv[] = {a0, a1, NULL};
    ast_node_t *cmd = ast_create_command(argv);
    // Free original strings to simulate caller releasing ownership
    free(a0);
    free(a1);
    TEST_ASSERT_NOT_NULL(cmd);
    // Free AST; should not double free or crash
    ast_free(cmd);
}

void test_ast_free_nested_pipeline(void) {
    char *a1[] = {"echo", "x", NULL};
    char *a2[] = {"cat", NULL};
    char *a3[] = {"cat", NULL};
    ast_node_t *p1 = ast_create_pipeline(ast_create_command(a1), ast_create_command(a2));
    ast_node_t *root = ast_create_pipeline(p1, ast_create_command(a3));
    TEST_ASSERT_NOT_NULL(root);
    ast_free(root);
}

void test_ast_create_command_empty_argv_array(void) {
    char *argv[] = {NULL};
    ast_node_t *cmd = ast_create_command(argv);
    TEST_ASSERT_NOT_NULL(cmd);
    ast_free(cmd);
}
