#ifndef ENV_H
#define ENV_H

// Environment variables API

// Environment functions
char *env_get(const char *name);
int env_set(const char *name, const char *value);
int env_unset(const char *name);
char **env_get_all(void);
void env_print(void);

// Variable expansion
char *expand_variables(const char *str);

#endif // ENV_H
