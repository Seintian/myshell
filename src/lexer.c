/**
 * @file lexer.c
 * @brief Tokenizer implementation for shell input strings.
 */
#include "lexer.h"
#include "util.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct lexer {
    const char *input;
    size_t pos;
    size_t length;
};

lexer_t *lexer_create(const char *input) {
    lexer_t *lexer = malloc_safe(sizeof(lexer_t));
    lexer->input = input;
    lexer->pos = 0;
    lexer->length = strlen(input);
    return lexer;
}

static void skip_whitespace(lexer_t *lexer) {
    while (lexer->pos < lexer->length && isspace(lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}

static char *read_word(lexer_t *lexer) {
    size_t start = lexer->pos;
    while (lexer->pos < lexer->length && !isspace(lexer->input[lexer->pos]) &&
           lexer->input[lexer->pos] != '|' && lexer->input[lexer->pos] != '<' &&
           lexer->input[lexer->pos] != '>' && lexer->input[lexer->pos] != '&' &&
           lexer->input[lexer->pos] != ';') {
        lexer->pos++;
    }

    size_t len = lexer->pos - start;
    char *word = malloc_safe(len + 1);
    strncpy(word, &lexer->input[start], len);
    word[len] = '\0';
    return word;
}

token_t *lexer_next_token(lexer_t *lexer) {
    skip_whitespace(lexer);

    if (lexer->pos >= lexer->length) {
        token_t *token = malloc_safe(sizeof(token_t));
        token->type = TOKEN_EOF;
        token->value = NULL;
        return token;
    }

    token_t *token = malloc_safe(sizeof(token_t));
    char ch = lexer->input[lexer->pos];

    switch (ch) {
    case '|':
        token->type = TOKEN_PIPE;
        token->value = strdup_safe("|");
        lexer->pos++;
        break;
    case '<':
        token->type = TOKEN_REDIRECT_IN;
        token->value = strdup_safe("<");
        lexer->pos++;
        break;
    case '>':
        if (lexer->pos + 1 < lexer->length && lexer->input[lexer->pos + 1] == '>') {
            token->type = TOKEN_REDIRECT_APPEND;
            token->value = strdup_safe(">>");
            lexer->pos += 2;
        } else {
            token->type = TOKEN_REDIRECT_OUT;
            token->value = strdup_safe(">");
            lexer->pos++;
        }
        break;
    case '&':
        token->type = TOKEN_BACKGROUND;
        token->value = strdup_safe("&");
        lexer->pos++;
        break;
    case ';':
        token->type = TOKEN_SEMICOLON;
        token->value = strdup_safe(";");
        lexer->pos++;
        break;
    default:
        token->type = TOKEN_WORD;
        token->value = read_word(lexer);
        break;
    }

    return token;
}

void lexer_free(lexer_t *lexer) {
    if (lexer) {
        free(lexer);
    }
}

void token_free(token_t *token) {
    if (token) {
        free(token->value);
        free(token);
    }
}
