#ifndef PIPELINE_H
#define PIPELINE_H

#include "ast.h"

// Pipeline execution API

int pipeline_execute(ast_node_t **commands, int count);

#endif // PIPELINE_H
