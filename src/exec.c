#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "exec.h"
#include "builtin.h"
#include "plugin.h"
#include "util.h"

// Simple AST node structures (would be in ast.c normally)
struct ast_node {
    ast_node_type_t type;
    union {
        struct {
            char **argv;
        } command;
        struct {
            ast_node_t *left;
            ast_node_t *right;
        } pipeline;
    } data;
};

ast_node_t *ast_create_command(char **argv) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_COMMAND;
    
    // Duplicate the argv array so we own the memory
    if (argv) {
        int argc = string_array_length(argv);
        node->data.command.argv = malloc_safe((argc + 1) * sizeof(char*));
        for (int i = 0; i < argc; i++) {
            node->data.command.argv[i] = strdup_safe(argv[i]);
        }
        node->data.command.argv[argc] = NULL;
    } else {
        node->data.command.argv = NULL;
    }
    
    return node;
}

ast_node_t *ast_create_pipeline(ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_PIPELINE;
    node->data.pipeline.left = left;
    node->data.pipeline.right = right;
    return node;
}

void ast_free(ast_node_t *node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_COMMAND:
            free_string_array(node->data.command.argv);
            break;
        case AST_PIPELINE:
            ast_free(node->data.pipeline.left);
            ast_free(node->data.pipeline.right);
            break;
        case AST_SEQUENCE:
        case AST_BACKGROUND:
            // TODO: Implement these node types
            break;
    }
    free(node);
}

int exec_command(ast_command_t *cmd) {
    // Cast for simplicity
    ast_node_t *node = (ast_node_t*)cmd;
    
    if (!node) {
        return -1;
    }
    
    char **argv = node->data.command.argv;
    
    if (!argv || !argv[0]) {
        return -1;
    }
    
    // Check for builtin commands
    int builtin_result = builtin_execute(argv[0], string_array_length(argv), argv);
    if (builtin_result != -1) {
        return builtin_result;
    }
    
    // Check for plugin commands
    if (plugin_execute(argv[0], string_array_length(argv), argv) == 0) {
        return 0;
    }
    
    // Execute external command
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return -1;
    }
}

int exec_pipeline(ast_pipeline_t *pipeline) {
    // Simple pipeline execution
    // This would need proper pipe setup in a real implementation
    ast_node_t *node = (ast_node_t*)pipeline;
    
    if (!node) {
        return -1;
    }
    
    // For now, just execute left then right
    int result = exec_ast(node->data.pipeline.left);
    if (result == 0) {
        result = exec_ast(node->data.pipeline.right);
    }
    return result;
}

int exec_ast(ast_node_t *ast) {
    if (!ast) return -1;
    
    switch (ast->type) {
        case AST_COMMAND:
            return exec_command((ast_command_t*)ast);
        case AST_PIPELINE:
            return exec_pipeline((ast_pipeline_t*)ast);
        default:
            return -1;
    }
}
