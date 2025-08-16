#ifndef LEXER_H
#define LEXER_H

// Token stream API

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,
    TOKEN_BACKGROUND,
    TOKEN_SEMICOLON,
    TOKEN_EOF
} token_type_t;

typedef struct {
    token_type_t type;
    char *value;
} token_t;

typedef struct lexer lexer_t;

// Lexer functions
lexer_t *lexer_create(const char *input);
token_t *lexer_next_token(lexer_t *lexer);
void lexer_free(lexer_t *lexer);
void token_free(token_t *token);

#endif // LEXER_H
