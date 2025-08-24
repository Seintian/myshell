/**
 * @file util.c
 * @brief Utility helpers for memory, strings, paths, and debugging.
 */
#include "util.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

char *strdup_safe(const char *str) {
    if (!str)
        return NULL;

    size_t len = strlen(str);
    char *copy = malloc_safe(len + 1);
    strcpy(copy, str);
    return copy;
}

char **split_string(const char *str, const char *delim) {
    if (!str || !delim)
        return NULL;

    char *str_copy = strdup_safe(str);
    char **result = NULL;
    int count = 0;
    int capacity = 8;

    result = malloc_safe(capacity * sizeof(char *));

    char *token = strtok(str_copy, delim);
    while (token) {
        if (count >= capacity - 1) {
            capacity *= 2;
            result = realloc_safe(result, capacity * sizeof(char *));
        }

        result[count++] = strdup_safe(token);
        token = strtok(NULL, delim);
    }

    result[count] = NULL;
    free(str_copy);
    return result;
}

void free_string_array(char **array) {
    if (!array)
        return;

    for (int i = 0; array[i]; i++) {
        free(array[i]);
    }
    free(array);
}

size_t string_array_length(char **array) {
    if (!array)
        return 0;

    size_t count = 0;
    while (array[count]) {
        count++;
    }
    return count;
}

void *malloc_safe(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    return ptr;
}

void *realloc_safe(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        fprintf(stderr, "realloc failed\n");
        exit(1);
    }
    return new_ptr;
}

char *resolve_path(const char *path) {
    if (!path)
        return NULL;

    // If path contains '/', it's a path, return as-is
    if (strchr(path, '/')) {
        return strdup_safe(path);
    }

    // Search in PATH
    char *path_env = getenv("PATH");
    if (!path_env)
        return NULL;

    char **paths = split_string(path_env, ":");
    if (!paths)
        return NULL;

    for (int i = 0; paths[i]; i++) {
        size_t len = strlen(paths[i]) + strlen(path) + 2;
        char *full_path = malloc_safe(len);
        snprintf(full_path, len, "%s/%s", paths[i], path);

        if (is_executable(full_path)) {
            free_string_array(paths);
            return full_path;
        }

        free(full_path);
    }

    free_string_array(paths);
    return NULL;
}

int is_executable(const char *path) {
    if (!path)
        return 0;

    return access(path, X_OK) == 0 ? 1 : 0;
}

void debug_print(const char *format, ...) {
    if (getenv("SHELL_DEBUG")) {
        va_list args;
        va_start(args, format);
        fprintf(stderr, "[DEBUG] ");
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
}

void sig_safe_write(int fd, const void *buf, size_t len) {
    ssize_t r;
    do { r = write(fd, buf, len); } while (r < 0 && errno == EINTR);
    (void)r; // best-effort
}
