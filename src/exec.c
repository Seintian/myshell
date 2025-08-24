/**
 * @file exec.c
 * @brief Execution of AST nodes (builtins, plugins, and external commands).
 */
#include "exec.h"
#include "builtin.h"
#include "env.h" // for expand_variables
#include "plugin.h"
#include "jobs.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "pipeline.h"

/** Simple AST node structures (normally defined in ast.c). */
struct ast_node {
    ast_node_type_t type;
    union {
        struct {
            char **argv;
            struct {
                int fd;
                int type; // 0=input,1=output,2=append
                char *filename;
            } redirs[8];
            int n_redirs;
        } command;
        struct {
            ast_node_t *left;
            ast_node_t *right;
        } pipeline;
        struct {
            ast_node_t *left;
            ast_node_t *right;
        } sequence;
        struct {
            ast_node_t *child;
        } background;
        struct { // and / or
            ast_node_t *left;
            ast_node_t *right;
        } boollist;
        struct { // subshell
            ast_node_t *child;
        } subshell;
    } data;
};

// Helper: build a short label string from an AST (best-effort, <= ~80 chars)
static char *ast_to_label_rec(ast_node_t *node, int depth) {
    if (!node || depth > 8) return strdup_safe("...");
    switch (node->type) {
    case AST_COMMAND: {
        char *res = strdup_safe("");
        for (int i = 0; node->data.command.argv && node->data.command.argv[i]; ++i) {
            const char *part = node->data.command.argv[i];
            size_t need = strlen(res) + (res[0] ? 1 : 0) + strlen(part) + 1;
            if (need > 80) break;
            char *tmp = malloc_safe(need);
            tmp[0] = '\0';
            if (res[0]) { strcpy(tmp, res); strcat(tmp, " "); }
            else { strcpy(tmp, res); }
            strcat(tmp, part);
            free(res);
            res = tmp;
        }
        return res;
    }
    case AST_PIPELINE: {
        char *l = ast_to_label_rec(node->data.pipeline.left, depth + 1);
        char *r = ast_to_label_rec(node->data.pipeline.right, depth + 1);
        size_t need = strlen(l) + 3 + strlen(r) + 1;
        char *res = malloc_safe(need);
        snprintf(res, need, "%s | %s", l, r);
        free(l); free(r);
        return res;
    }
    case AST_SEQUENCE: {
        char *l = ast_to_label_rec(node->data.sequence.left, depth + 1);
        char *r = ast_to_label_rec(node->data.sequence.right, depth + 1);
        size_t need = strlen(l) + 3 + strlen(r) + 1;
        char *res = malloc_safe(need);
        snprintf(res, need, "%s ; %s", l, r);
        free(l); free(r);
        return res;
    }
    case AST_BACKGROUND:
        return strdup_safe("(bg)");
    case AST_AND: {
        char *l = ast_to_label_rec(node->data.boollist.left, depth + 1);
        char *r = ast_to_label_rec(node->data.boollist.right, depth + 1);
        size_t need = strlen(l) + 4 + strlen(r) + 1;
        char *res = malloc_safe(need);
        snprintf(res, need, "%s && %s", l, r);
        free(l); free(r);
        return res;
    }
    case AST_OR: {
        char *l = ast_to_label_rec(node->data.boollist.left, depth + 1);
        char *r = ast_to_label_rec(node->data.boollist.right, depth + 1);
        size_t need = strlen(l) + 4 + strlen(r) + 1;
        char *res = malloc_safe(need);
        snprintf(res, need, "%s || %s", l, r);
        free(l); free(r);
        return res;
    }
    case AST_SUBSHELL: {
        char *c = ast_to_label_rec(node->data.subshell.child, depth + 1);
        size_t need = strlen(c) + 3;
        char *res = malloc_safe(need);
        snprintf(res, need, "(%s)", c);
        free(c);
        return res;
    }
    }
    return strdup_safe("job");
}

static char **expand_argv(char **argv) {
    int argc = string_array_length(argv);
    char **expanded = malloc_safe((argc + 1) * sizeof(char *));
    for (int i = 0; i < argc; i++) {
        expanded[i] = expand_variables(argv[i]);
        if (!expanded[i]) {
            expanded[i] = strdup_safe(argv[i]);
        }
    }
    expanded[argc] = NULL;
    return expanded;
}

static int exec_external(char **argv) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child: become a process group leader for job control consistency
        // Ignore errors if already in a group or permissions block
        (void)setpgid(0, 0);
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    } else if (pid > 0) {
        // Parent: ensure child is in its own process group (race-safe double call)
        (void)setpgid(pid, pid);
        int status;
        void (*oldint)(int) = signal(SIGINT, SIG_IGN);
        waitpid(pid, &status, 0);
        signal(SIGINT, oldint);
        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        if (WIFSIGNALED(status))
            return 128 + WTERMSIG(status);
        return 1;
    } else {
        perror("fork");
        return -1;
    }
}

ast_node_t *ast_create_command(char **argv) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_COMMAND;

    // Copy the argv array so we own the memory
    if (argv) {
        int argc = string_array_length(argv);
        node->data.command.argv = malloc_safe((argc + 1) * sizeof(char *));
        for (int i = 0; i < argc; i++) {
            node->data.command.argv[i] = strdup_safe(argv[i]);
        }
        node->data.command.argv[argc] = NULL;
    } else {
        node->data.command.argv = NULL;
    }

    node->data.command.n_redirs = 0;

    return node;
}

ast_node_t *ast_create_pipeline(ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_PIPELINE;
    node->data.pipeline.left = left;
    node->data.pipeline.right = right;
    return node;
}

ast_node_t *ast_create_sequence(ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_SEQUENCE;
    node->data.sequence.left = left;
    node->data.sequence.right = right;
    return node;
}

ast_node_t *ast_create_background(ast_node_t *child) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_BACKGROUND;
    node->data.background.child = child;
    return node;
}

ast_node_t *ast_create_and(ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_AND;
    node->data.boollist.left = left;
    node->data.boollist.right = right;
    return node;
}

ast_node_t *ast_create_or(ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_OR;
    node->data.boollist.left = left;
    node->data.boollist.right = right;
    return node;
}

ast_node_t *ast_create_subshell(ast_node_t *child) {
    ast_node_t *node = malloc_safe(sizeof(ast_node_t));
    node->type = AST_SUBSHELL;
    node->data.subshell.child = child;
    return node;
}

void ast_command_add_redirection(ast_node_t *cmd, int fd, int type, const char *filename) {
    if (!cmd || cmd->type != AST_COMMAND || !filename)
        return;
    if (cmd->data.command.n_redirs >= 8)
        return;
    int n = cmd->data.command.n_redirs++;
    cmd->data.command.redirs[n].fd = fd;
    cmd->data.command.redirs[n].type = type;
    cmd->data.command.redirs[n].filename = strdup_safe(filename);
}

void ast_free(ast_node_t *node) {
    if (!node)
        return;

    switch (node->type) {
    case AST_COMMAND:
        free_string_array(node->data.command.argv);
        for (int i = 0; i < node->data.command.n_redirs; ++i) {
            free(node->data.command.redirs[i].filename);
        }
        break;
    case AST_PIPELINE:
        ast_free(node->data.pipeline.left);
        ast_free(node->data.pipeline.right);
        break;
    case AST_SEQUENCE:
        ast_free(node->data.sequence.left);
        ast_free(node->data.sequence.right);
        break;
    case AST_BACKGROUND:
        ast_free(node->data.background.child);
        break;
    case AST_AND:
    case AST_OR:
        ast_free(node->data.boollist.left);
        ast_free(node->data.boollist.right);
        break;
    case AST_SUBSHELL:
        ast_free(node->data.subshell.child);
        break;
    }
    free(node);
}

int exec_command(ast_command_t *cmd) {
    // Cast for simplicity
    ast_node_t *node = (ast_node_t *)cmd;

    if (!node) {
        return -1;
    }

    char **argv = node->data.command.argv;

    if (!argv || !argv[0]) {
        return -1;
    }

    // Expand variables in all arguments
    char **expanded_argv = expand_argv(argv);
    if (!expanded_argv || !expanded_argv[0]) {
        free_string_array(expanded_argv);
        return -1;
    }
    int argc = string_array_length(expanded_argv);

    // Check for builtin commands
    int builtin_result = builtin_execute(expanded_argv[0], argc, expanded_argv);
    if (builtin_result != -1) {
        // If this command has redirections, execute builtin in child to apply them
        ast_node_t *n = (ast_node_t *)cmd;
        if (n->data.command.n_redirs > 0) {
            pid_t c = fork();
            if (c == 0) {
                for (int i = 0; i < n->data.command.n_redirs; ++i) {
                    int fd = n->data.command.redirs[i].fd;
                    int t = n->data.command.redirs[i].type;
                    const char *fn = n->data.command.redirs[i].filename;
                    int f = -1;
                    if (t == REDIR_INPUT) f = open(fn, O_RDONLY | O_CLOEXEC);
                    else if (t == REDIR_OUTPUT) f = open(fn, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
                    else if (t == REDIR_APPEND) f = open(fn, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
                    else if (t == REDIR_HEREDOC) {
                        // Create a pipe, feed stdin until delimiter, and hook read end to fd
                        int p[2];
                        if (pipe(p) == 0) {
                            size_t cap = 0; char *line = NULL; ssize_t nread;
                            // Read from current stdin until a line equal to delimiter
                            while ((nread = getline(&line, &cap, stdin)) != -1) {
                                // Strip trailing newline for comparison
                                if (nread > 0 && line[nread - 1] == '\n') {
                                    line[nread - 1] = '\0';
                                    nread--;
                                }
                                if (strcmp(line, fn) == 0) {
                                    break;
                                }
                                // Write the line and a newline back
                                if (nread > 0) (void)write(p[1], line, (size_t)nread);
                                (void)write(p[1], "\n", 1);
                            }
                            free(line);
                            close(p[1]);
                            f = p[0];
                        }
                    }
                    if (f >= 0) { dup2(f, fd); close(f); }
                }
                int rc = builtin_execute(expanded_argv[0], argc, expanded_argv);
                _exit(rc & 0xFF);
            } else if (c > 0) {
                int st; void (*oldint)(int) = signal(SIGINT, SIG_IGN);
                waitpid(c, &st, 0);
                signal(SIGINT, oldint);
                free_string_array(expanded_argv);
                if (WIFEXITED(st)) return WEXITSTATUS(st);
                if (WIFSIGNALED(st)) return 128 + WTERMSIG(st);
                return 1;
            } else {
                perror("fork");
                free_string_array(expanded_argv);
                return -1;
            }
        }
        free_string_array(expanded_argv);
        return builtin_result;
    }

    // Check for plugin commands
    if (plugin_execute(expanded_argv[0], argc, expanded_argv) == 0) {
        free_string_array(expanded_argv);
        return 0;
    }

    // Execute external command (apply redirs if present)
    ast_node_t *n = (ast_node_t *)cmd;
    int rc;
    if (n->data.command.n_redirs == 0) {
        rc = exec_external(expanded_argv);
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            for (int i = 0; i < n->data.command.n_redirs; ++i) {
                int fd = n->data.command.redirs[i].fd;
                int t = n->data.command.redirs[i].type;
                const char *fn = n->data.command.redirs[i].filename;
                int f = -1;
                if (t == REDIR_INPUT) f = open(fn, O_RDONLY | O_CLOEXEC);
                else if (t == REDIR_OUTPUT) f = open(fn, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
                else if (t == REDIR_APPEND) f = open(fn, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
                else if (t == REDIR_HEREDOC) {
                    // Create a pipe, feed stdin until delimiter, and hook read end to fd
                    int p[2];
                    if (pipe(p) == 0) {
                        size_t cap = 0; char *line = NULL; ssize_t nread;
                        while ((nread = getline(&line, &cap, stdin)) != -1) {
                            if (nread > 0 && line[nread - 1] == '\n') {
                                line[nread - 1] = '\0';
                                nread--;
                            }
                            if (strcmp(line, fn) == 0) break;
                            if (nread > 0) (void)write(p[1], line, (size_t)nread);
                            (void)write(p[1], "\n", 1);
                        }
                        free(line);
                        close(p[1]);
                        f = p[0];
                    }
                }
                if (f >= 0) { dup2(f, fd); close(f); }
            }
            execvp(expanded_argv[0], expanded_argv);
            perror("execvp");
            _exit(127);
        } else if (pid > 0) {
            int st; void (*oldint)(int) = signal(SIGINT, SIG_IGN);
            waitpid(pid, &st, 0);
            signal(SIGINT, oldint);
            if (WIFEXITED(st)) rc = WEXITSTATUS(st);
            else if (WIFSIGNALED(st)) rc = 128 + WTERMSIG(st);
            else rc = 1;
        } else {
            perror("fork");
            rc = -1;
        }
    }
    free_string_array(expanded_argv);
    return rc;
}

int exec_pipeline(ast_pipeline_t *pipeline) {
    ast_node_t *node = (ast_node_t *)pipeline;
    if (!node) return -1;
    // Flatten simple binary tree into array
    ast_node_t *arr[16];
    int n = 0;
    ast_node_t *cur = node;
    while (cur && n < 16) {
        if (cur->type == AST_PIPELINE) {
            arr[n++] = cur->data.pipeline.left;
            cur = cur->data.pipeline.right;
        } else {
            arr[n++] = cur;
            break;
        }
    }
    return pipeline_execute(arr, n);
}

int exec_ast(ast_node_t *ast) {
    if (!ast)
        return -1;

    switch (ast->type) {
    case AST_COMMAND:
        return exec_command((ast_command_t *)ast);
    case AST_PIPELINE:
        return exec_pipeline((ast_pipeline_t *)ast);
    case AST_SEQUENCE: {
        (void)exec_ast(ast->data.sequence.left);
        return exec_ast(ast->data.sequence.right);
    }
    case AST_BACKGROUND: {
        pid_t pid = fork();
        if (pid == 0) {
            (void)setpgid(0, 0);
            int rc = exec_ast(ast->data.background.child);
            _exit(rc & 0xFF);
        } else if (pid > 0) {
            (void)setpgid(pid, pid);
            // Build a small label from the AST
            char *label = ast_to_label_rec(ast->data.background.child, 0);
            if (!label || !label[0]) { free(label); label = strdup_safe("job"); }
            job_create(pid, label);
            free(label);
            return 0;
        } else {
            perror("fork");
            return -1;
        }
    }
    case AST_AND: {
        int lrc = exec_ast(ast->data.boollist.left);
        if (lrc == 0) return exec_ast(ast->data.boollist.right);
        return lrc;
    }
    case AST_OR: {
        int lrc = exec_ast(ast->data.boollist.left);
        if (lrc != 0) return exec_ast(ast->data.boollist.right);
        return lrc;
    }
    case AST_SUBSHELL: {
        pid_t pid = fork();
        if (pid == 0) {
            (void)setpgid(0, 0);
            int rc = exec_ast(ast->data.subshell.child);
            _exit(rc & 0xFF);
        } else if (pid > 0) {
            (void)setpgid(pid, pid);
            int st; void (*oldint)(int) = signal(SIGINT, SIG_IGN);
            waitpid(pid, &st, 0);
            signal(SIGINT, oldint);
            if (WIFEXITED(st)) return WEXITSTATUS(st);
            if (WIFSIGNALED(st)) return 128 + WTERMSIG(st);
            return 1;
        } else {
            perror("fork");
            return -1;
        }
    }
    default:
        return -1;
    }
}
