#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

// Utility functions

// String utilities
char *strdup_safe(const char *str);
char **split_string(const char *str, const char *delim);
void free_string_array(char **array);
size_t string_array_length(char **array);

// Memory utilities
void *malloc_safe(size_t size);
void *realloc_safe(void *ptr, size_t size);

// Path utilities
char *resolve_path(const char *path);
int is_executable(const char *path);

// Debug utilities
void debug_print(const char *format, ...);

#endif // UTIL_H
