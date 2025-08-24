/**
 * @file util.h
 * @brief Miscellaneous helpers: strings, memory, paths, and logging.
 */
#ifndef UTIL_H
#define UTIL_H
/** \defgroup group_util utilities
 *  @brief String, memory, path, and debug helpers.
 *  @{ */

#include <stddef.h>

// String utilities
/** strdup() with exit-on-OOM semantics. Returns newly allocated copy. */
char *strdup_safe(const char *str);
/** Split a string by delimiters; returns NULL-terminated vector (free with free_string_array). */
char **split_string(const char *str, const char *delim);
/** Free a NULL-terminated array of strings produced by split_string. */
void free_string_array(char **array);
/** Count elements in a NULL-terminated array. */
size_t string_array_length(char **array);

// Memory utilities
/** malloc() with exit-on-OOM semantics. */
void *malloc_safe(size_t size);
/** realloc() with exit-on-OOM semantics. */
void *realloc_safe(void *ptr, size_t size);

// Path utilities
/** Resolve an executable by searching PATH when no '/' is present. */
char *resolve_path(const char *path);
/** Return non-zero if the file exists and is user-executable. */
int is_executable(const char *path);

// Debug utilities
/** Print a formatted debug line to stderr if SHELL_DEBUG is set. */
void debug_print(const char *format, ...);

// Async-signal-safe write (best-effort, retries once on EINTR)
void sig_safe_write(int fd, const void *buf, size_t len);

/** @} */

#endif // UTIL_H
