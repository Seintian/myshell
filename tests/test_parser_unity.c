#include "unity.h"
#include "parser.h"
#include "lexer.h"
#include "exec.h"
#include <stdlib.h>
#include <string.h>

// Test functions only - setUp/tearDown and main are in test_runner.c

void test_parser_create_and_free(void) {
    lexer_t *lexer = lexer_create("ls");
    parser_t *parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser);
    
    parser_free(parser);
    lexer_free(lexer);
}

void test_parser_simple_command(void) {
    lexer_t *lexer = lexer_create("ls -la");
    parser_t *parser = parser_create(lexer);
    
    ast_node_t *ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast);
    // TODO: Add helper function to check AST type without exposing structure
    // TEST_ASSERT_EQUAL(AST_COMMAND, ast->type);
    
    ast_free(ast);
    parser_free(parser);
    lexer_free(lexer);
}

void test_parser_empty_input(void) {
    lexer_t *lexer = lexer_create("");
    parser_t *parser = parser_create(lexer);
    
    ast_node_t *ast = parser_parse(parser);
    TEST_ASSERT_NULL(ast);
    
    parser_free(parser);
    lexer_free(lexer);
}

void test_parser_pipeline(void) {
    lexer_t *lexer = lexer_create("ls | grep test");
    parser_t *parser = parser_create(lexer);
    
    ast_node_t *ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast);
    // TODO: Add helper function to check AST type without exposing structure
    // TEST_ASSERT_EQUAL(AST_PIPELINE, ast->type);
    
    ast_free(ast);
    parser_free(parser);
    lexer_free(lexer);
}

// End of parser tests
