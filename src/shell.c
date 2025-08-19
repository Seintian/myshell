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
    if (!line_in || *line_in == '\0')
        return 0;
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
        if (read > 0 && line[read - 1] == '\n')
            line[read - 1] = '\0';
        if (line[0] == '\0')
            continue;
        last_status = execute_line(line);
        if (shell_flag_errexit && last_status != 0) {
            break;
        }
    }
    free(line);
    fclose(f);
    return last_status;
}

void shell_set_errexit(int on) {
    shell_flag_errexit = on ? 1 : 0;
}
void shell_set_xtrace(int on) {
    shell_flag_xtrace = on ? 1 : 0;
}
int shell_get_errexit(void) {
    return shell_flag_errexit;
}
int shell_get_xtrace(void) {
    return shell_flag_xtrace;
}

int shell_main(int argc, char **argv) {
    // Reset flags per invocation (separate shells shouldn't inherit state)
    shell_set_errexit(0);
    shell_set_xtrace(0);
    shell_interactive = 1;
    // Ensure stdin stream flags are clear (tests may have hit EOF earlier)
    clearerr(stdin);
    // Basic CLI: myshell [-e] [-x] [script [args...]]
    int argi = 1;
    while (argi < argc && argv[argi][0] == '-' && argv[argi][1] != '\0') {
        if (strcmp(argv[argi], "-e") == 0)
            shell_set_errexit(1);
        else if (strcmp(argv[argi], "-x") == 0)
            shell_set_xtrace(1);
        else
            break; // unknown, let script handle
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

    char *multiline_buffer = NULL;
    size_t multiline_capacity = 0;
    size_t multiline_length = 0;

    while (shell_running) {
        // Be defensive each iteration in case callers swap stdin between loops
        clearerr(stdin);
        if (shell_interactive) {
            // Show continuation prompt if we're in multiline mode
            if (multiline_length > 0) {
                printf("      >> "); // Continuation prompt
            } else {
                printf("myshell> "); // Normal prompt
            }
            fflush(stdout);
        }

        if ((read = getline(&line, &len, stdin)) == -1) {
            if (feof(stdin)) {
                // EOF reached. If we have a pending multiline command, execute it once.
                if (multiline_length > 0 && multiline_buffer && multiline_buffer[0] != '\0') {
                    exit_code = execute_line(multiline_buffer);
                }
                break; // then exit loop
            } else {
                perror("getline");
                break;
            }
        }

        // Remove newline
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
            read--;
        }

        // Skip empty lines when not in multiline mode
        if (read == 0 && multiline_length == 0) {
            continue;
        }

        // Check for line continuation (backslash at end)
        int has_continuation = 0;
        if (read > 0 && line[read - 1] == '\\') {
            has_continuation = 1;
            line[read - 1] = '\0'; // Remove the backslash
            read--;
        }

        // If we're in continuation mode, use the multiline buffer; otherwise execute single line directly
        if (has_continuation || multiline_length > 0) {
            // Accumulate the line in multiline buffer
            size_t needed = multiline_length + read + 2; // +1 for space, +1 for null terminator
            if (needed > multiline_capacity) {
                multiline_capacity = needed * 2; // Double the capacity
                multiline_buffer = realloc(multiline_buffer, multiline_capacity);
                if (!multiline_buffer) {
                    perror("realloc");
                    exit_code = 1;
                    break;
                }
                if (multiline_length == 0) {
                    multiline_buffer[0] = '\0';
                }
            }

            // Append current line to multiline buffer
            if (multiline_length > 0 && read > 0) {
                strcat(multiline_buffer, " "); // Add space between lines
                multiline_length++;
            }
            if (read > 0) {
                strcat(multiline_buffer, line);
                multiline_length += read;
            }

            // If no continuation now, execute the accumulated buffer
            if (!has_continuation && multiline_length > 0) {
                exit_code = execute_line(multiline_buffer);

                // Reset multiline buffer
                multiline_length = 0;
                if (multiline_buffer) {
                    multiline_buffer[0] = '\0';
                }

                if (shell_flag_errexit && exit_code != 0) {
                    break;
                }
            }
        } else {
            // No continuation and nothing buffered: execute this line immediately
            exit_code = execute_line(line);
            if (shell_flag_errexit && exit_code != 0) {
                break;
            }
        }
    }

    // Clean up multiline buffer
    if (multiline_buffer) {
        free(multiline_buffer);
    }

    if (line) {
        free(line);
    }
    return exit_code;
}
