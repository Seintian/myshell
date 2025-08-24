/**
 * @file ast.h
 * @brief Abstract Syntax Tree (AST) node types and constructors.
 */
#ifndef AST_H
#define AST_H
/** \defgroup group_ast ast
 *  @brief Abstract Syntax Tree nodes and constructors.
 *  @{ */

/** Opaque base node type. */
typedef struct ast_node ast_node_t;
/** Opaque command node type. */
typedef struct ast_command ast_command_t;
/** Opaque pipeline node type. */
typedef struct ast_pipeline ast_pipeline_t;
/** Opaque sequence node type. */
typedef struct ast_sequence ast_sequence_t;
/** Opaque background node type. */
typedef struct ast_background ast_background_t;
/** Opaque boolean-list node type. */
typedef struct ast_boollist ast_boollist_t;
/** Opaque subshell node type. */
typedef struct ast_subshell ast_subshell_t;

/** Kinds of AST nodes understood by the executor. */
typedef enum {
    AST_COMMAND,   /**< Simple command with argv. */
    AST_PIPELINE,  /**< Two nodes connected with a pipe. */
    AST_SEQUENCE,  /**< Sequence of two nodes: left ; right. */
    AST_BACKGROUND,/**< Background execution of a child node. */
    AST_AND,       /**< Logical AND: execute right only if left succeeded. */
    AST_OR,        /**< Logical OR: execute right only if left failed. */
    AST_SUBSHELL   /**< Execute child in a subshell environment. */
} ast_node_type_t;

/**
 * @brief Redirection types for command I/O.
 */
typedef enum {
    REDIR_INPUT = 0,   /**< '<'  read from file into fd (default fd 0). */
    REDIR_OUTPUT = 1,  /**< '>'  write to file (truncate). */
    REDIR_APPEND = 2,  /**< '>>' append to file. */
    REDIR_HEREDOC = 3  /**< '<<' here-doc (stdin from inline data until delimiter). */
} ast_redir_type_t;

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
 * @brief Create a sequence node that runs left; then right.
 *
 * Ownership: Takes ownership of left and right nodes; free with ast_free().
 */
ast_node_t *ast_create_sequence(ast_node_t *left, ast_node_t *right);

/**
 * @brief Wrap a node to execute in the background.
 *
 * Ownership: Takes ownership of child; free with ast_free().
 */
ast_node_t *ast_create_background(ast_node_t *child);

/** Create a logical AND node: left && right. Takes ownership of children. */
ast_node_t *ast_create_and(ast_node_t *left, ast_node_t *right);
/** Create a logical OR node: left || right. Takes ownership of children. */
ast_node_t *ast_create_or(ast_node_t *left, ast_node_t *right);
/** Create a subshell node: ( child ). Takes ownership of child. */
ast_node_t *ast_create_subshell(ast_node_t *child);

/**
 * @brief Add an I/O redirection to a command node.
 *
 * The redirection applies to the given target file descriptor (fd). Type is
 * one of ast_redir_type_t. The filename is copied; for REDIR_HEREDOC, it
 * carries the delimiter string.
 */
void ast_command_add_redirection(ast_node_t *cmd, int fd, int type, const char *filename);

/**
 * @brief Recursively free an AST subtree.
 *
 * Safe to call with NULL. Frees both structure and any copied argv arrays.
 */
void ast_free(ast_node_t *node);

/** @} */

#endif // AST_H
