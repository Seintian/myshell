/**
 * @file env.h
 * @brief Environment variable helpers and expansion.
 */
#ifndef ENV_H
#define ENV_H
/** \defgroup group_env Environment
 *  @brief Environment variables and expansion.
 *  @{ */

// Environment functions
/** Get the value of an environment variable or NULL if unset. */
char *env_get(const char *name);
/** Set an environment variable (overwrites existing). Returns 0 on success. */
int env_set(const char *name, const char *value);
/** Unset an environment variable. Returns 0 on success. */
int env_unset(const char *name);
/** Snapshot environment as a NULL-terminated array for iteration. */
char **env_get_all(void);
/** Print all environment variables to stdout in NAME=VALUE form. */
void env_print(void);

// Variable expansion
/** Expand $VAR occurrences in a string; returns a newly allocated string. */
char *expand_variables(const char *str);

/** @} */

#endif // ENV_H
