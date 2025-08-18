/**
 * @file ast.h
 * @brief Abstract Syntax Tree (AST) node types and constructors.
 */
#ifndef AST_H
#define AST_H
/** \defgroup group_ast AST
 *  @brief Abstract Syntax Tree nodes and constructors.
 *  @{ */

/** Opaque base node type. */
typedef struct ast_node ast_node_t;
/** Opaque command node type. */
typedef struct ast_command ast_command_t;
/** Opaque pipeline node type. */
typedef struct ast_pipeline ast_pipeline_t;

/** Kinds of AST nodes understood by the executor. */
typedef enum {
    AST_COMMAND,   /**< Simple command with argv. */
    AST_PIPELINE,  /**< Two nodes connected with a pipe. */
    AST_SEQUENCE,  /**< Sequence (reserved). */
    AST_BACKGROUND /**< Background (reserved). */
} ast_node_type_t;

/**
 * @brief Create a command node from an argv array (NULL-terminated).
 *
 * Ownership: The function deep-copies argv; caller retains ownership of
 * the original array and strings and must free them separately if allocated.
 *
 * @param argv Argument vector ending with NULL; argv[0] is the program.
 * @return Newly allocated AST node; must be freed with ast_free().
 */
ast_node_t *ast_create_command(char **argv);

/**
 * @brief Create a pipeline node that connects left | right.
 *
 * Ownership: Takes ownership of left and right nodes; the returned node
 * must be freed with ast_free() which will free the entire subtree.
 */
ast_node_t *ast_create_pipeline(ast_node_t *left, ast_node_t *right);

/**
 * @brief Recursively free an AST subtree.
 *
 * Safe to call with NULL. Frees both structure and any copied argv arrays.
 */
void ast_free(ast_node_t *node);

/** @} */

#endif // AST_H
