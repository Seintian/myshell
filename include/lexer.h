/**
 * @file lexer.h
 * @brief Tokenizer for shell input strings.
 */
#ifndef LEXER_H
#define LEXER_H
/** \defgroup group_lexer Lexer
 *  @brief Tokenization of input strings.
 *  @{ */

/** Types of lexical tokens recognized by the lexer. */
typedef enum {
    TOKEN_WORD,            /**< Plain word (command or arg). */
    TOKEN_PIPE,            /**< '|' character. */
    TOKEN_REDIRECT_IN,     /**< '<' redirection. */
    TOKEN_REDIRECT_OUT,    /**< '>' redirection. */
    TOKEN_REDIRECT_APPEND, /**< '>>' redirection. */
    TOKEN_BACKGROUND,      /**< '&' background. */
    TOKEN_SEMICOLON,       /**< ';' sequence separator. */
    TOKEN_EOF              /**< End of input. */
} token_type_t;

/** A single token with type and optional string value. */
typedef struct {
    token_type_t type; /**< Token kind. */
    char *value;       /**< Token text (for words), else NULL. */
} token_t;

/**
 * Opaque lexer state. One lexer instance scans one immutable input string.
 * Not thread-safe. The input string must outlive the lexer.
 */
typedef struct lexer lexer_t;

/**
 * @brief Create a lexer over the given input string.
 * @param input NUL-terminated string to scan; not copied.
 * @return New lexer instance; free with lexer_free().
 */
lexer_t *lexer_create(const char *input);
/**
 * @brief Retrieve the next token from the input stream.
 *
 * Repeatedly calling returns a sequence of tokens and finally TOKEN_EOF.
 * Ownership: Caller owns the returned token and must free via token_free().
 */
token_t *lexer_next_token(lexer_t *lexer);
/** Free the lexer. Does not free tokens produced earlier. */
void lexer_free(lexer_t *lexer);
/** Free a token object and its value (if any). Safe on NULL. */
void token_free(token_t *token);

/** @} */

#endif // LEXER_H
