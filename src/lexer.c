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
    while (lexer->pos < lexer->length && isspace((unsigned char)lexer->input[lexer->pos]))
        lexer->pos++;
}

static char *read_word(lexer_t *lexer) {
    int in_single = 0, in_double = 0;
    // Accumulate into dynamic buffer to handle quotes/escapes
    size_t cap = 32;
    char *buf = malloc_safe(cap);
    size_t out = 0;
    while (lexer->pos < lexer->length) {
        char c = lexer->input[lexer->pos];
        if (!in_single && !in_double) {
            if (isspace((unsigned char)c) || c == '|' || c == '<' || c == '>' || c == '&' || c == ';')
                break;
            if (c == '\'' ) { in_single = 1; lexer->pos++; continue; }
            if (c == '"' ) { in_double = 1; lexer->pos++; continue; }
        } else if (in_single) {
            if (c == '\'') { lexer->pos++; in_single = 0; continue; }
        } else if (in_double) {
            if (c == '"') { lexer->pos++; in_double = 0; continue; }
            if (c == '\\' && lexer->pos + 1 < lexer->length) {
                // support simple escapes inside double quotes
                lexer->pos++;
                c = lexer->input[lexer->pos];
            }
        }
        if (out + 2 > cap) { cap *= 2; buf = realloc_safe(buf, cap); }
        buf[out++] = c;
        lexer->pos++;
    }
    buf[out] = '\0';
    return buf;
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

    // Support numeric FD prefix for redirections (e.g., 2>)
    if (isdigit((unsigned char)ch)) {
        size_t save = lexer->pos;
        while (lexer->pos < lexer->length && isdigit((unsigned char)lexer->input[lexer->pos]))
            lexer->pos++;
        if (lexer->pos < lexer->length && (lexer->input[lexer->pos] == '>' || lexer->input[lexer->pos] == '<')) {
            // roll back and treat as WORD; parser can interpret numeric fd before redir
            lexer->pos = save;
        } else {
            lexer->pos = save;
        }
    }

    switch (ch) {
    case '|':
        if (lexer->pos + 1 < lexer->length && lexer->input[lexer->pos + 1] == '|') {
            token->type = TOKEN_OR_IF;
            token->value = strdup_safe("||");
            lexer->pos += 2;
        } else {
            token->type = TOKEN_PIPE;
            token->value = strdup_safe("|");
            lexer->pos++;
        }
        break;
    case '<':
        if (lexer->pos + 1 < lexer->length && lexer->input[lexer->pos + 1] == '<') {
            token->type = TOKEN_HEREDOC;
            token->value = strdup_safe("<<");
            lexer->pos += 2;
        } else {
            token->type = TOKEN_REDIRECT_IN;
            token->value = strdup_safe("<");
            lexer->pos++;
        }
        break;
    case '>':
        if (lexer->pos + 1 < lexer->length && lexer->input[lexer->pos + 1] == '>') {
            token->type = TOKEN_REDIRECT_APPEND;
            token->value = strdup_safe(">>");
            lexer->pos += 2;
        } else if (lexer->pos + 1 < lexer->length && lexer->input[lexer->pos + 1] == '&') {
            token->type = TOKEN_REDIRECT_AND_OUT;
            token->value = strdup_safe("&>");
            lexer->pos += 2;
        } else {
            token->type = TOKEN_REDIRECT_OUT;
            token->value = strdup_safe(">");
            lexer->pos++;
        }
        break;
    case '&':
        if (lexer->pos + 1 < lexer->length && lexer->input[lexer->pos + 1] == '&') {
            token->type = TOKEN_AND_IF;
            token->value = strdup_safe("&&");
            lexer->pos += 2;
        } else {
            token->type = TOKEN_BACKGROUND;
            token->value = strdup_safe("&");
            lexer->pos++;
        }
        break;
    case ';':
        token->type = TOKEN_SEMICOLON;
        token->value = strdup_safe(";");
        lexer->pos++;
        break;
    case '(':
        token->type = TOKEN_LPAREN;
        token->value = strdup_safe("(");
        lexer->pos++;
        break;
    case ')':
        token->type = TOKEN_RPAREN;
        token->value = strdup_safe(")");
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
