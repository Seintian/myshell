/**
 * @file builtin_core.c
 * @brief Core builtin implementations (cd, exit, env, jobs, type, etc.).
 */
#include "builtin.h"
#include "env.h"
#include "jobs.h"
#include "shell.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Builtin command implementations

int builtin_cd(int argc, char **argv) {
    const char *path;

    if (argc < 2) {
        path = env_get("HOME");
        if (!path) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    } else {
        path = argv[1];
    }

    if (chdir(path) != 0) {
        perror("cd");
        return 1;
    }

    return 0;
}

int builtin_exit(int argc, char **argv) {
    // Store exit code for later use if needed
    int exit_code = 0;

    if (argc > 1) {
        exit_code = atoi(argv[1]);
    }

    // Set shell_running to 0 to trigger graceful exit
    shell_running = 0;
    return exit_code;
}

int builtin_export(int argc, char **argv) {
    if (argc < 2) {
        env_print();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        char *eq = strchr(argv[i], '=');
        if (eq) {
            *eq = '\0';
            if (env_set(argv[i], eq + 1) != 0) {
                perror("export");
                return 1;
            }
            *eq = '='; // Restore original string
        } else {
            // Export existing variable
            char *value = env_get(argv[i]);
            if (value) {
                if (env_set(argv[i], value) != 0) {
                    perror("export");
                    return 1;
                }
            }
        }
    }

    return 0;
}

int builtin_unset(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "unset: missing variable name\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (env_unset(argv[i]) != 0) {
            perror("unset");
            return 1;
        }
    }

    return 0;
}

int builtin_pwd(int argc __attribute__((unused)),
                char **argv __attribute__((unused))) {
    char *cwd = getcwd(NULL, 0);
    if (!cwd) {
        perror("pwd");
        return 1;
    }

    printf("%s\n", cwd);
    free(cwd);
    return 0;
}

int builtin_jobs(int argc __attribute__((unused)),
                 char **argv __attribute__((unused))) {
    job_list();
    return 0;
}

int builtin_fg(int argc, char **argv) {
    int job_id = 1; // Default to job 1

    if (argc > 1) {
        job_id = atoi(argv[1]);
    }

    job_t *job = job_find(job_id);
    if (!job) {
        fprintf(stderr, "fg: job %d not found\n", job_id);
        return 1;
    }

    job_fg(job);
    return 0;
}

int builtin_bg(int argc, char **argv) {
    int job_id = 1; // Default to job 1

    if (argc > 1) {
        job_id = atoi(argv[1]);
    }

    job_t *job = job_find(job_id);
    if (!job) {
        fprintf(stderr, "bg: job %d not found\n", job_id);
        return 1;
    }

    job_bg(job);
    return 0;
}

int builtin_type(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "type: missing argument\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (builtin_find(argv[i])) {
            printf("%s is a shell builtin\n", argv[i]);
        } else {
            // Check if it's an external command
            char *path = resolve_path(argv[i]);
            if (path && is_executable(path)) {
                printf("%s is %s\n", argv[i], path);
                free(path);
            } else {
                printf("%s: not found\n", argv[i]);
            }
        }
    }

    return 0;
}

static int builtin_set(int argc, char **argv) {
    // Support: set -e | +e | -x | +x
    if (argc < 2) {
        printf("errexit=%d xtrace=%d\n", shell_get_errexit(), shell_get_xtrace());
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) shell_set_errexit(1);
        else if (strcmp(argv[i], "+e") == 0) shell_set_errexit(0);
        else if (strcmp(argv[i], "-x") == 0) shell_set_xtrace(1);
        else if (strcmp(argv[i], "+x") == 0) shell_set_xtrace(0);
        else {
            fprintf(stderr, "set: unsupported option '%s'\n", argv[i]);
            return 2;
        }
    }
    return 0;
}

// Builtin registry
static builtin_t builtins[] = {
    {"cd", builtin_cd, "Change directory"},
    {"exit", builtin_exit, "Exit the shell"},
    {"export", builtin_export, "Set environment variables"},
    {"unset", builtin_unset, "Unset environment variables"},
    {"pwd", builtin_pwd, "Print working directory"},
    {"jobs", builtin_jobs, "List active jobs"},
    {"fg", builtin_fg, "Bring job to foreground"},
    {"bg", builtin_bg, "Put job in background"},
    {"type", builtin_type, "Display command type"},
    {"set", builtin_set, "Set shell options: -e/+e, -x/+x"},
    {NULL, NULL, NULL}};

builtin_t *builtin_find(const char *name) {
    if (!name) {
        return NULL;
    }

    for (int i = 0; builtins[i].name; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            return &builtins[i];
        }
    }
    return NULL;
}

int builtin_execute(const char *name, int argc, char **argv) {
    builtin_t *builtin = builtin_find(name);
    if (builtin) {
        return builtin->func(argc, argv);
    }
    return -1; // Not a builtin
}

void builtin_list(void) {
    for (int i = 0; builtins[i].name; i++) {
        printf("%-10s %s\n", builtins[i].name, builtins[i].description);
    }
}
