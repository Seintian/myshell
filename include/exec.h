/**
 * @file exec.h
 * @brief Execution engine for AST nodes (commands and pipelines).
 *
 * @details The executor handles three categories of commands:
 *  - Builtins: Dispatched directly in-process via builtin_execute().
 *  - Plugins: Dispatched to dynamically loaded modules via plugin_execute().
 *  - External commands: Fork/exec with argv search through PATH.
 *
 * Pipelines are executed from left to right, and the return code is the
 * status of the last command in the pipeline (conventional shell behavior).
 */
#ifndef EXEC_H
#define EXEC_H
/** \defgroup group_exec Execution
 *  @brief Execution of ASTs, commands, and pipelines.
 *  @{ */

#include "ast.h"

/** Opaque execution context (reserved for future use). */
typedef struct exec_context exec_context_t;

/**
 * @brief Execute an AST tree and return its exit status.
 *
 * @param ast AST node to execute; may be a command or pipeline.
 * @retval -1 if ast is NULL or an unsupported node is encountered.
 * @retval >=0 status code for successful dispatch/execution.
 */
int exec_ast(ast_node_t *ast);

/**
 * @brief Execute a single command node and return its exit status.
 *
 * Resolution order: builtin -> plugin -> external (fork/exec).
 * On external exec failure (e.g., command not found), returns non-zero.
 */
int exec_command(ast_command_t *cmd);

/**
 * @brief Execute a pipeline node and return the last command's status.
 *
 * Implementation detail: current simple form executes left then right
 * sequentially; the dedicated pipeline_execute() supports N-stage pipes.
 */
int exec_pipeline(ast_pipeline_t *pipeline);

/** @} */

#endif // EXEC_H
