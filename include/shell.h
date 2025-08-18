/**
 * @file shell.h
 * @brief Public entry points and global state for the interactive shell.
 *
 * @details The shell provides an interactive read–eval–print loop (REPL).
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
 */
#ifndef SHELL_H
#define SHELL_H

/** \defgroup group_shell Shell
 *  @brief Interactive shell API and lifecycle.
 *  @{ */

/**
 * Global flag controlling the main loop; non-zero while the shell runs.
 *
 * Side effects: The "exit" builtin sets this to 0 to terminate the loop.
 * Thread-safety: Not thread-safe; intended for single-threaded use.
 */
extern int shell_running;

/**
 * @brief Shell entry point invoked by main(). Starts the REPL.
 *
 * @param argc Argument count (currently unused).
 * @param argv Argument vector (currently unused).
 * @retval 0 if no command was executed or last command succeeded.
 * @retval non-zero if the last command failed (its exit status), or if
 *         an internal error occurred while spawning an external command.
 *
 * Ownership: Does not take ownership of argv.
 * I/O: Reads from stdin, writes prompt and command outputs to stdout/stderr.
 * Signals: Installs simple handlers via term_setup_signals().
 *
 * @code{.c}
 * int main(int argc, char **argv) {
 *     shell_init();
 *     int rc = shell_main(argc, argv);
 *     shell_cleanup();
 *     return rc;
 * }
 * @endcode
 */
int shell_main(int argc, char **argv);

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
