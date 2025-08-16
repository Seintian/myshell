#ifndef EXEC_H
#define EXEC_H

#include "ast.h"

// Executor API

typedef struct exec_context exec_context_t;

// Execution functions
int exec_ast(ast_node_t *ast);
int exec_command(ast_command_t *cmd);
int exec_pipeline(ast_pipeline_t *pipeline);

#endif // EXEC_H
