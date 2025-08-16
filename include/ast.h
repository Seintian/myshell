#ifndef AST_H
#define AST_H

// AST nodes (typedefs + opaque handles)

typedef struct ast_node ast_node_t;
typedef struct ast_command ast_command_t;
typedef struct ast_pipeline ast_pipeline_t;

// AST node types
typedef enum {
    AST_COMMAND,
    AST_PIPELINE,
    AST_SEQUENCE,
    AST_BACKGROUND
} ast_node_type_t;

// Public API for AST manipulation
ast_node_t *ast_create_command(char **argv);
ast_node_t *ast_create_pipeline(ast_node_t *left, ast_node_t *right);
void ast_free(ast_node_t *node);

#endif // AST_H
