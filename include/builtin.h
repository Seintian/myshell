/**
 * @file builtin.h
 * @brief Built-in command registry and implementations.
 */
#ifndef BUILTIN_H
#define BUILTIN_H
/** \defgroup group_builtin builtins
 *  @brief Built-in command registry and implementations.
 *  @{ */

/** Function signature for builtin commands. */
typedef int (*builtin_func_t)(int argc, char **argv);

/** Descriptor of a builtin command. */
typedef struct {
    const char *name;        /**< Command name. */
    builtin_func_t func;     /**< Implementation function. */
    const char *description; /**< Short help text. */
} builtin_t;

// Builtin functions
/** Register a builtin at runtime (optional registry extension). */
int builtin_register(const char *name, builtin_func_t func,
                     const char *description);
/** Find a builtin by name (exact match), or NULL if not found. */
builtin_t *builtin_find(const char *name);
/** Execute a builtin if found, else return -1 (not a builtin). */
int builtin_execute(const char *name, int argc, char **argv);
/** Print builtin names and descriptions to stdout. */
void builtin_list(void);

// Core builtin implementations
/** Change directory to path or $HOME if no argument. */
int builtin_cd(int argc, char **argv);
/** Exit the shell with optional status; sets ::shell_running = 0. */
int builtin_exit(int argc, char **argv);
/** Set or print environment variables. */
int builtin_export(int argc, char **argv);
/** Unset environment variables. */
int builtin_unset(int argc, char **argv);
/** Print current working directory. */
int builtin_pwd(int argc, char **argv);
/** List known jobs. */
int builtin_jobs(int argc, char **argv);
/** Bring a job to the foreground. */
int builtin_fg(int argc, char **argv);
/** Continue a job in the background. */
int builtin_bg(int argc, char **argv);
/** Report how a command name would be resolved. */
int builtin_type(int argc, char **argv);

/** @} */

#endif // BUILTIN_H
