/**
 * @file parser.h
 * @brief Parser that builds ASTs from a token stream.
 */
#ifndef PARSER_H
#define PARSER_H
/** \defgroup group_parser parser
 *  @brief Parsing tokens into ASTs.
 *  @{ */

#include "ast.h"
#include "lexer.h"

/**
 * Opaque parser instance bound to a lexer. Consumes tokens from the lexer
 * and produces ASTs according to a minimal grammar:
 *   command := WORD { WORD }
 *   pipeline := command { '|' command }
 */
typedef struct parser parser_t;

/** Create a parser for the given lexer. Ownership remains with caller. */
parser_t *parser_create(lexer_t *lexer);
/**
 * Parse the next command/pipeline; returns NULL on EOF or parse error.
 * The returned AST must be freed with ast_free().
 */
ast_node_t *parser_parse(parser_t *parser);
/** Destroy the parser and free internal resources. Safe on NULL. */
void parser_free(parser_t *parser);

/** @} */

#endif // PARSER_H
