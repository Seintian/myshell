/**
 * @file shell.h
 * @brief Public entry points and global state for the interactive shell.
 *
 * @details The shell provides an interactive read-eval-print loop (REPL).
 * On each iteration it:
 *  1. prints a prompt ("myshell> "),
 *  2. reads a line from stdin (getline),
 *  3. lexes and parses the input into an AST,
 *  4. executes the AST through the execution engine,
 *  5. stores the exit status of the last executed command.
 *
 * The loop continues while ::shell_running is non-zero and until EOF or
 * the "exit" builtin sets ::shell_running to 0. The return value of
 * shell_main() is the status of the last executed command (0 if nothing
 * was executed, or if the last command succeeded).
 *
 * Non-interactive mode: If a script file is provided, the shell reads and
 * executes lines from that file without prompts. The script is executed in
 * the current shell (so builtins affect the shell state). Options:
 *  -e: exit immediately on command error (non-zero status)
 *  -x: print commands as they are executed (trace)
 */
#ifndef SHELL_H
#define SHELL_H

/** \defgroup group_shell shell
 *  @brief Interactive shell API and lifecycle.
 *  @{ */

/**
 * Global flag controlling the main loop; non-zero while the shell runs.
 *
 * Side effects: The "exit" builtin sets this to 0 to terminate the loop.
 * Thread-safety: Not thread-safe; intended for single-threaded use.
 */
extern int shell_running;

/** Whether the shell is running interactively (prints prompts, job control). */
extern int shell_interactive;

/** If non-zero, exit immediately when a command returns non-zero (set -e). */
extern int shell_flag_errexit;

/** If non-zero, print commands as they are executed (set -x). */
extern int shell_flag_xtrace;

/**
 * @brief Shell entry point invoked by main(). Starts the REPL.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Status of the last executed command, or 0 when nothing ran.
 */
int shell_main(int argc, char **argv);

/**
 * @brief Execute commands from a file path in non-interactive mode.
 *
 * Shebang (#!) on the first line is ignored. Commands run in the current
 * shell context (builtins affect variables). Honors shell_flag_errexit and
 * shell_flag_xtrace.
 *
 * @param path Path to the script file to execute.
 * @return Last command status, or first failing status when -e is set;
 *         127 if the file cannot be opened.
 */
int shell_run_file(const char *path);

/** Toggle shell options (used by the 'set' builtin and CLI flags). */
void shell_set_errexit(int on);
void shell_set_xtrace(int on);
int shell_get_errexit(void);
int shell_get_xtrace(void);

/**
 * @brief Initialize shell subsystems (signals, plugins, builtins).
 *
 * Idempotency: Safe to call multiple times; subsequent calls are no-ops.
 * Side effects: Installs signal handlers, prepares plugin/builtin systems.
 */
void shell_init(void);

/**
 * @brief Cleanup shell subsystems and restore signal handlers.
 *
 * Idempotency: Safe to call without prior shell_init(); multiple calls ok.
 * Side effects: Unloads plugins, restores default signal handlers.
 */
void shell_cleanup(void);

/** @} */

#endif // SHELL_H
