#ifndef BUILTIN_H
#define BUILTIN_H

// Builtin registry

typedef int (*builtin_func_t)(int argc, char **argv);

typedef struct {
    const char *name;
    builtin_func_t func;
    const char *description;
} builtin_t;

// Builtin functions
int builtin_register(const char *name, builtin_func_t func, const char *description);
builtin_t *builtin_find(const char *name);
int builtin_execute(const char *name, int argc, char **argv);
void builtin_list(void);

// Core builtin implementations
int builtin_cd(int argc, char **argv);
int builtin_exit(int argc, char **argv);
int builtin_export(int argc, char **argv);
int builtin_unset(int argc, char **argv);
int builtin_pwd(int argc, char **argv);
int builtin_jobs(int argc, char **argv);
int builtin_fg(int argc, char **argv);
int builtin_bg(int argc, char **argv);
int builtin_type(int argc, char **argv);

#endif // BUILTIN_H
