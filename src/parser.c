/**
 * @file parser.c
 * @brief Simple recursive-descent parser for commands, pipelines, background, and sequences.
 */
#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

/** Parser state holding input lexer and current token. */
struct parser {
    lexer_t *lexer;         /**< Source token stream. */
    token_t *current_token; /**< Lookahead token. */
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
    // Collect WORD tokens as argv elements and track simple redirections
    int capacity = 8;
    int argc = 0;
    char **argv = malloc_safe(capacity * sizeof(char *));

    // Temporary redirection collection
    struct { int fd; int type; char *file; } redirs[8];
    int rcount = 0;

    // Helper to test if string is all digits
    auto int is_all_digits(const char *s) {
        if (!s || !*s) return 0;
        for (const char *p = s; *p; ++p) if (*p < '0' || *p > '9') return 0;
        return 1;
    }

    while (parser->current_token->type == TOKEN_WORD ||
           parser->current_token->type == TOKEN_REDIRECT_IN ||
           parser->current_token->type == TOKEN_REDIRECT_OUT ||
           parser->current_token->type == TOKEN_REDIRECT_APPEND ||
           parser->current_token->type == TOKEN_HEREDOC ||
           parser->current_token->type == TOKEN_REDIRECT_AND_OUT) {
        if (parser->current_token->type == TOKEN_WORD) {
            if (argc >= capacity - 1) {
                capacity *= 2;
                argv = realloc_safe(argv, capacity * sizeof(char *));
            }
            argv[argc++] = strdup_safe(parser->current_token->value);
            advance_token(parser);
            continue;
        }

        // Determine redirection type and default fd
    int type = -1; // ast_redir_type_t: input/output/append/heredoc
        int dfd = 1;
        int is_and_out = 0;
        if (parser->current_token->type == TOKEN_REDIRECT_IN) {
            type = REDIR_INPUT; dfd = 0;
        } else if (parser->current_token->type == TOKEN_REDIRECT_OUT) {
            type = REDIR_OUTPUT; dfd = 1;
        } else if (parser->current_token->type == TOKEN_REDIRECT_APPEND) {
            type = REDIR_APPEND; dfd = 1;
        } else if (parser->current_token->type == TOKEN_HEREDOC) {
            type = REDIR_HEREDOC; dfd = 0;
        } else if (parser->current_token->type == TOKEN_REDIRECT_AND_OUT) {
            type = REDIR_OUTPUT; dfd = 1; is_and_out = 1;
        }
        advance_token(parser);
        if (parser->current_token->type != TOKEN_WORD) {
            // Missing filename; abort
            free_string_array(argv);
            for (int i = 0; i < rcount; ++i) free(redirs[i].file);
            return NULL;
        }
        char *filename = strdup_safe(parser->current_token->value);
        advance_token(parser);

        int fd = dfd;
        if (!is_and_out && argc > 0 && is_all_digits(argv[argc - 1])) {
            fd = atoi(argv[argc - 1]);
            free(argv[argc - 1]);
            argv[--argc] = NULL;
        }

        if (rcount < 8) {
            redirs[rcount].fd = fd;
            redirs[rcount].type = type;
            redirs[rcount].file = filename;
            rcount++;
            if (is_and_out && rcount < 8) {
                redirs[rcount].fd = 2;
                redirs[rcount].type = type;
                redirs[rcount].file = strdup_safe(filename);
                rcount++;
            }
        } else {
            free(filename);
        }
    }

    if (argc == 0) {
        free(argv);
        return NULL;
    }

    argv[argc] = NULL;
    ast_node_t *node = ast_create_command(argv);
    // Attach collected redirections
    for (int i = 0; i < rcount; ++i) {
        ast_command_add_redirection(node, redirs[i].fd, redirs[i].type, redirs[i].file);
        free(redirs[i].file);
    }
    free_string_array(argv);
    return node;
}

// primary := ( list ) | command
// Forward declarations for recursive descent
static ast_node_t *parse_list(parser_t *parser);
static ast_node_t *parse_primary(parser_t *parser);

static ast_node_t *parse_primary(parser_t *parser) {
    if (parser->current_token->type == TOKEN_LPAREN) {
        advance_token(parser);
        ast_node_t *inside = NULL;
    // Parse a full list inside parentheses
    inside = parse_list(parser);
        if (parser->current_token->type == TOKEN_RPAREN) {
            advance_token(parser);
        } else {
            ast_free(inside);
            return NULL;
        }
        return ast_create_subshell(inside);
    }
    return parse_command(parser);
}

static ast_node_t *parse_pipeline(parser_t *parser) {
    ast_node_t *left = parse_primary(parser);
    if (!left) return NULL;
    while (parser->current_token->type == TOKEN_PIPE) {
        advance_token(parser);
        ast_node_t *right = parse_primary(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_create_pipeline(left, right);
    }
    return left;
}

static ast_node_t *parse_and_or(parser_t *parser) {
    ast_node_t *left = parse_pipeline(parser);
    if (!left) return NULL;
    while (parser->current_token->type == TOKEN_AND_IF || parser->current_token->type == TOKEN_OR_IF) {
        token_type_t t = parser->current_token->type;
        advance_token(parser);
        ast_node_t *right = parse_pipeline(parser);
        if (!right) { ast_free(left); return NULL; }
        if (t == TOKEN_AND_IF) left = ast_create_and(left, right);
        else left = ast_create_or(left, right);
    }
    return left;
}

// list := and_or [ ; list ] [ & ]
static ast_node_t *parse_list(parser_t *parser) {
    ast_node_t *left = parse_and_or(parser);
    if (!left) return NULL;

    // Optional background on this and_or
    if (parser->current_token->type == TOKEN_BACKGROUND) {
        advance_token(parser);
        left = ast_create_background(left);
    }

    if (parser->current_token->type == TOKEN_SEMICOLON) {
        advance_token(parser);
        if (parser->current_token->type != TOKEN_EOF && parser->current_token->type != TOKEN_RPAREN) {
            ast_node_t *right = parse_list(parser);
            if (right) left = ast_create_sequence(left, right);
        }
    }
    return left;
}

ast_node_t *parser_parse(parser_t *parser) {
    if (parser->current_token->type == TOKEN_EOF) {
        return NULL;
    }
    return parse_list(parser);
}

void parser_free(parser_t *parser) {
    if (parser) {
        token_free(parser->current_token);
        free(parser);
    }
}
