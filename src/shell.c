/**
 * @file shell.c
 * @brief Interactive shell main loop and lifecycle management.
 */
#include "shell.h"
#include "builtin.h"
#include "env.h"
#include "exec.h"
#include "jobs.h"
#include "lexer.h"
#include "logger.h"
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

    // Initialize logger (optional, default off)
    logger_init();
    logger_set_level(LOG_LEVEL_OFF);

    // Initialize builtins
    // Register core builtins here

    // Initialize plugins
    // Load plugins from default directory
}

void shell_cleanup(void) {
    plugin_cleanup_all();
    term_restore_signals();
    logger_shutdown();
}

// --- internal helpers -------------------------------------------------------

// Print prompt if interactive; continuation prompt when multiline is active.
static inline void shell_print_prompt(size_t multiline_length) {
    if (!shell_interactive)
        return;

    if (multiline_length > 0)
        printf("      >> ");
    else
        printf("myshell> ");

    fflush(stdout);
}

// Ensure the multiline buffer has capacity for at least needed bytes
// (including the trailing NUL once appended). Returns 0 on success, non-zero on OOM.
static int ensure_capacity(char **buffer, size_t *capacity, size_t needed) {
    if (needed <= *capacity)
        return 0;
    size_t new_cap = (*capacity == 0) ? (needed * 2) : *capacity;
    while (new_cap < needed) {
        new_cap *= 2;
    }
    char *new_buf = realloc(*buffer, new_cap);
    if (!new_buf)
        return -1;
    *buffer = new_buf;
    *capacity = new_cap;
    return 0;
}

// Append a line into the multiline buffer, inserting a separating space when
// there is already content and the new line is non-empty.
static int append_line(char **buffer, size_t *capacity, size_t *length,
                       const char *line, size_t line_len) {
    size_t extra = line_len;
    int add_space = (*length > 0 && line_len > 0) ? 1 : 0;
    size_t needed = *length + (size_t)add_space + extra + 1; // +1 for NUL
    if (ensure_capacity(buffer, capacity, needed) != 0) {
        return -1;
    }
    if (*length == 0) {
        (*buffer)[0] = '\0';
    }
    if (add_space) {
        (*buffer)[*length] = ' ';
        *length += 1;
        (*buffer)[*length] = '\0';
    }
    if (line_len > 0) {
        memcpy((*buffer) + *length, line, line_len);
        *length += line_len;
        (*buffer)[*length] = '\0';
    }
    return 0;
}

static int execute_line(const char *line_in) {
    int rc = 0;
    if (!line_in || *line_in == '\0')
        return 0;

    if (shell_flag_xtrace)
        fprintf(stderr, "+ %s\n", line_in);

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
    shell_interactive = isatty(STDIN_FILENO) ? 1 : 0;
    shell_running = 1; // ensure a fresh loop for each invocation
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
        // Drain any background children if interactive
        if (shell_interactive) {
            jobs_reap_background();
        }
        shell_print_prompt(multiline_length);

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
        if (read == 0 && multiline_length == 0)
            continue;

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
            if (append_line(&multiline_buffer, &multiline_capacity, &multiline_length,
                            line, (size_t)read) != 0) {
                perror("realloc");
                exit_code = 1;
                break;
            }

            // If no continuation now, execute the accumulated buffer
            if (!has_continuation && multiline_length > 0) {
                exit_code = execute_line(multiline_buffer);

                // Reset multiline buffer
                multiline_length = 0;
                if (multiline_buffer) {
                    multiline_buffer[0] = '\0';
                }

                if (shell_flag_errexit && exit_code != 0)
                    break;
            }
        } else {
            // No continuation and nothing buffered: execute this line immediately
            exit_code = execute_line(line);
            if (shell_flag_errexit && exit_code != 0)
                break;
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
