#include "unity.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>

// Test functions only - setUp/tearDown and main are in test_runner.c

void test_lexer_create_and_free(void) {
    lexer_t *lexer = lexer_create("test input");
    TEST_ASSERT_NOT_NULL(lexer);
    lexer_free(lexer);
}

void test_lexer_empty_input(void) {
    lexer_t *lexer = lexer_create("");
    TEST_ASSERT_NOT_NULL(lexer);
    
    token_t *token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token);
    TEST_ASSERT_EQUAL(TOKEN_EOF, token->type);
    
    token_free(token);
    lexer_free(lexer);
}

void test_lexer_simple_word(void) {
    lexer_t *lexer = lexer_create("hello");
    TEST_ASSERT_NOT_NULL(lexer);
    
    token_t *token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token);
    TEST_ASSERT_EQUAL(TOKEN_WORD, token->type);
    TEST_ASSERT_EQUAL_STRING("hello", token->value);
    
    token_free(token);
    
    // Next token should be EOF
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_EOF, token->type);
    
    token_free(token);
    lexer_free(lexer);
}

void test_lexer_multiple_words(void) {
    lexer_t *lexer = lexer_create("hello world");
    
    // First word
    token_t *token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_WORD, token->type);
    TEST_ASSERT_EQUAL_STRING("hello", token->value);
    token_free(token);
    
    // Second word
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_WORD, token->type);
    TEST_ASSERT_EQUAL_STRING("world", token->value);
    token_free(token);
    
    // EOF
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_EOF, token->type);
    token_free(token);
    
    lexer_free(lexer);
}

void test_lexer_pipe_token(void) {
    lexer_t *lexer = lexer_create("ls | grep test");
    
    // "ls"
    token_t *token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_WORD, token->type);
    TEST_ASSERT_EQUAL_STRING("ls", token->value);
    token_free(token);
    
    // "|"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_PIPE, token->type);
    TEST_ASSERT_EQUAL_STRING("|", token->value);
    token_free(token);
    
    // "grep"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_WORD, token->type);
    TEST_ASSERT_EQUAL_STRING("grep", token->value);
    token_free(token);
    
    // "test"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_WORD, token->type);
    TEST_ASSERT_EQUAL_STRING("test", token->value);
    token_free(token);
    
    lexer_free(lexer);
}

void test_lexer_redirection_tokens(void) {
    lexer_t *lexer = lexer_create("cat < input.txt > output.txt >> append.txt");
    
    // Skip "cat" and test redirections
    token_t *token = lexer_next_token(lexer);
    token_free(token);
    
    // "<"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_REDIRECT_IN, token->type);
    token_free(token);
    
    // Skip "input.txt"
    token = lexer_next_token(lexer);
    token_free(token);
    
    // ">"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_REDIRECT_OUT, token->type);
    token_free(token);
    
    // Skip "output.txt"
    token = lexer_next_token(lexer);
    token_free(token);
    
    // ">>"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_REDIRECT_APPEND, token->type);
    TEST_ASSERT_EQUAL_STRING(">>", token->value);
    token_free(token);
    
    lexer_free(lexer);
}

void test_lexer_special_characters(void) {
    lexer_t *lexer = lexer_create("cmd & ; background");
    
    // Skip "cmd"
    token_t *token = lexer_next_token(lexer);
    token_free(token);
    
    // "&"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_BACKGROUND, token->type);
    token_free(token);
    
    // ";"
    token = lexer_next_token(lexer);
    TEST_ASSERT_EQUAL(TOKEN_SEMICOLON, token->type);
    token_free(token);
    
    lexer_free(lexer);
}

// End of lexer tests
