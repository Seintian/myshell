/**
 * @file pipeline.c
 * @brief Implementation of N-stage pipeline execution using fork/pipe/dup2.
 */
#include "pipeline.h"
#include "exec.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int pipeline_execute(ast_node_t **commands, int count) {
    if (count <= 0 || !commands)
        return -1;
    if (count == 1)
        return exec_ast(commands[0]);

    const int n_pipes = count - 1;
    int (*pipes)[2] = NULL;
    pid_t *pids = NULL;
    int created = 0;
    int spawned = 0;

    pipes = n_pipes > 0 ? malloc(sizeof(int[2]) * (size_t)n_pipes) : NULL;
    pids = malloc(sizeof(pid_t) * (size_t)count);
    if ((n_pipes > 0 && !pipes) || !pids) {
        perror("malloc");
        free(pipes);
        free(pids);
        return -1;
    }

    // Create all pipes
    for (int i = 0; i < n_pipes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            // Close any previously created pipes
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            free(pipes);
            free(pids);
            return -1;
        }
        created++;
    }

    // Execute each command in the pipeline
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            // Set up input
            if (i > 0) {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    _exit(127);
                }
            }

            // Set up output
            if (i < count - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    _exit(127);
                }
            }

            // Close all pipe file descriptors
            for (int j = 0; j < n_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute the command and exit with its status
            int st = exec_ast(commands[i]);
            _exit(st & 0xFF);
        } else if (pid < 0) {
            perror("fork");
            // On fork failure, stop spawning more; break to cleanup
            break;
        } else {
            pids[i] = pid;
            spawned++;
        }
    }

    // Close all pipe file descriptors in parent
    for (int i = 0; i < created; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all spawned children
    int last_status = 0;
    for (int i = 0; i < spawned; i++) {
        int status = 0;
        if (waitpid(pids[i], &status, 0) == -1) {
            // If a child is already handled or error, keep going
            continue;
        }
        if (i == count - 1) {
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                last_status = 128 + WTERMSIG(status);
            } else {
                last_status = 1; // Fallback
            }
        }
    }

    free(pipes);
    free(pids);
    // If we failed to spawn all, surface an error unless at least one ran
    if (spawned < count) {
        // Prefer returning the last known status of the last spawned
        // but if none spawned, return error
        return spawned > 0 ? last_status : -1;
    }
    return last_status;
}
