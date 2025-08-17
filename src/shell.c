#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "lexer.h"
#include "parser.h"
#include "exec.h"
#include "builtin.h"
#include "plugin.h"
#include "env.h"
#include "term.h"

int shell_running = 1;

void shell_init(void) {
    // Initialize terminal
    term_setup_signals();
    
    // Initialize builtins
    // Register core builtins here
    
    // Initialize plugins
    // Load plugins from default directory
}

void shell_cleanup(void) {
    plugin_cleanup_all();
    term_restore_signals();
}

int shell_main(int argc __attribute__((unused)), char **argv __attribute__((unused))) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int exit_code = 0;
    
    while (shell_running) {
        printf("myshell> ");
        fflush(stdout);
        
        if ((read = getline(&line, &len, stdin)) == -1) {
            break;
        }
        
        // Remove newline
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        
        if (strlen(line) == 0) {
            continue;
        }
        
        // Create lexer and parser
        lexer_t *lexer = lexer_create(line);
        parser_t *parser = parser_create(lexer);
        
        // Parse the command
        ast_node_t *ast = parser_parse(parser);
        if (ast) {
            exit_code = exec_ast(ast);
            ast_free(ast);
        }
        
        parser_free(parser);
        lexer_free(lexer);
    }
    
    if (line) {
        free(line);
    }
    return exit_code;
}
