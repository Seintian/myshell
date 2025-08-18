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
/** Non-zero when interactive mode is enabled. */
int shell_interactive = 1;
/** -e: exit on error */
int shell_flag_errexit = 0;
/** -x: trace commands */
int shell_flag_xtrace = 0;

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

static int execute_line(const char *line_in) {
    int rc = 0;
    if (!line_in || *line_in == '\0') return 0;
    if (shell_flag_xtrace) {
        fprintf(stderr, "+ %s\n", line_in);
    }
    lexer_t *lexer = lexer_create(line_in);
    parser_t *parser = parser_create(lexer);
    ast_node_t *ast = parser_parse(parser);
    if (ast) {
        rc = exec_ast(ast);
        ast_free(ast);
    }
    parser_free(parser);
    lexer_free(lexer);
    return rc;
}

int shell_run_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror(path);
        return 127;
    }
    shell_interactive = 0; // disable prompts/job-control tweaks

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int last_status = 0;
    int lineno = 0;
    while ((read = getline(&line, &len, f)) != -1) {
        lineno++;
        if (lineno == 1 && read >= 2 && line[0] == '#' && line[1] == '!') {
            // skip shebang
            continue;
        }
        if (read > 0 && line[read - 1] == '\n') line[read - 1] = '\0';
        if (line[0] == '\0') continue;
        last_status = execute_line(line);
        if (shell_flag_errexit && last_status != 0) {
            break;
        }
    }
    free(line);
    fclose(f);
    return last_status;
}

void shell_set_errexit(int on) { shell_flag_errexit = on ? 1 : 0; }
void shell_set_xtrace(int on) { shell_flag_xtrace = on ? 1 : 0; }
int shell_get_errexit(void) { return shell_flag_errexit; }
int shell_get_xtrace(void) { return shell_flag_xtrace; }

int shell_main(int argc, char **argv) {
    // Basic CLI: myshell [-e] [-x] [script [args...]]
    int argi = 1;
    while (argi < argc && argv[argi][0] == '-' && argv[argi][1] != '\0') {
        if (strcmp(argv[argi], "-e") == 0) shell_set_errexit(1);
        else if (strcmp(argv[argi], "-x") == 0) shell_set_xtrace(1);
        else break; // unknown, let script handle
        argi++;
    }
    if (argi < argc) {
        // Non-interactive: run file
        return shell_run_file(argv[argi]);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int exit_code = 0;

    while (shell_running) {
        if (shell_interactive) {
            printf("myshell> ");
            fflush(stdout);
        }

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
        exit_code = execute_line(line);
        if (shell_flag_errexit && exit_code != 0) {
            break;
        }
    }

    if (line) {
        free(line);
    }
    return exit_code;
}
