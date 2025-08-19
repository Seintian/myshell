#include "env.h"
#include "util.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *expand_variables(const char *str) {
    // Simple variable expansion implementation
    // This would need to be expanded for full shell variable expansion
    if (!str) return NULL;

    size_t len = strlen(str);
    size_t cap = len + 1;           // initial capacity
    char *result = malloc_safe(cap);
    size_t result_pos = 0;

    // Helper to ensure capacity for appending n more chars (plus NUL)
    #define ENSURE_CAP(NEED) \
        do { \
            size_t _need = (NEED); \
            if (result_pos + _need + 1 > cap) { \
                while (result_pos + _need + 1 > cap) { \
                    cap *= 2; \
                } \
                result = realloc_safe(result, cap); \
            } \
        } while (0)

    for (size_t i = 0; i < len; i++) {
        if (str[i] == '$' && i + 1 < len) {
            // Simple variable name extraction
            size_t var_start = i + 1;
            size_t var_end = var_start;

            while (var_end < len && (isalnum((unsigned char)str[var_end]) || str[var_end] == '_'))
                var_end++;

            if (var_end > var_start) {
                size_t name_len = var_end - var_start;
                char *var_name = malloc_safe(name_len + 1);
                memcpy(var_name, &str[var_start], name_len);
                var_name[name_len] = '\0';

                char *var_value = env_get(var_name);
                if (var_value) {
                    size_t value_len = strlen(var_value);
                    ENSURE_CAP(value_len);
                    memcpy(&result[result_pos], var_value, value_len);
                    result_pos += value_len;
                }

                free(var_name);
                i = var_end - 1;
                continue;
            }
        }

    // Default: copy the character as-is
    ENSURE_CAP(1);
        result[result_pos++] = str[i];
    }

    result[result_pos] = '\0';
    return result;

    #undef ENSURE_CAP
}

char *env_get(const char *name) {
    return getenv(name);
}

int env_set(const char *name, const char *value) {
    return setenv(name, value, 1);
}

int env_unset(const char *name) {
    return unsetenv(name);
}

char **env_get_all(void) {
    extern char **environ;
    return environ;
}

void env_print(void) {
    char **env = env_get_all();
    for (int i = 0; env[i]; i++) {
        printf("%s\n", env[i]);
    }
}
