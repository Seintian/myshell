#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

// Parse API

typedef struct parser parser_t;

// Parser functions
parser_t *parser_create(lexer_t *lexer);
ast_node_t *parser_parse(parser_t *parser);
void parser_free(parser_t *parser);

#endif // PARSER_H
