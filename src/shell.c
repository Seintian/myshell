/**
 * @file shell.c
 * @brief Interactive shell main loop and lifecycle management.
 */
#define _GNU_SOURCE
#include "shell.h"
#include "builtin.h"
#include "env.h"
#include "exec.h"
#include "lexer.h"
#include "parser.h"
#include "plugin.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** Global flag for main loop run-state; set to 0 to stop. */
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

int shell_main(int argc __attribute__((unused)),
               char **argv __attribute__((unused))) {
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
