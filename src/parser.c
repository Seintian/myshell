#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "util.h"

struct parser {
    lexer_t *lexer;
    token_t *current_token;
};

parser_t *parser_create(lexer_t *lexer) {
    parser_t *parser = malloc_safe(sizeof(parser_t));
    parser->lexer = lexer;
    parser->current_token = lexer_next_token(lexer);
    return parser;
}

static void advance_token(parser_t *parser) {
    token_free(parser->current_token);
    parser->current_token = lexer_next_token(parser->lexer);
}

static ast_node_t *parse_command(parser_t *parser) {
    // Simple command parsing - collect words until pipe, redirect, or end
    char **argv = NULL;
    int argc = 0;
    int capacity = 8;
    
    argv = malloc_safe(capacity * sizeof(char*));
    
    while (parser->current_token->type == TOKEN_WORD) {
        if (argc >= capacity - 1) {
            capacity *= 2;
            argv = realloc_safe(argv, capacity * sizeof(char*));
        }
        
        argv[argc++] = strdup_safe(parser->current_token->value);
        advance_token(parser);
    }
    
    if (argc == 0) {
        free(argv);
        return NULL;
    }
    
    argv[argc] = NULL;
    return ast_create_command(argv);
}

ast_node_t *parser_parse(parser_t *parser) {
    if (parser->current_token->type == TOKEN_EOF) {
        return NULL;
    }
    
    ast_node_t *left = parse_command(parser);
    if (!left) {
        return NULL;
    }
    
    // Handle pipes
    while (parser->current_token->type == TOKEN_PIPE) {
        advance_token(parser);
        ast_node_t *right = parse_command(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_create_pipeline(left, right);
    }
    
    return left;
}

void parser_free(parser_t *parser) {
    if (parser) {
        token_free(parser->current_token);
        free(parser);
    }
}
