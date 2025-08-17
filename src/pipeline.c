#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "pipeline.h"
#include "exec.h"

int pipeline_execute(ast_node_t **commands, int count) {
    if (count <= 0 || !commands) return -1;
    if (count == 1) return exec_ast(commands[0]);
    
    int pipes[count - 1][2];
    pid_t pids[count];
    
    // Create all pipes
    for (int i = 0; i < count - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }
    
    // Execute each command in the pipeline
    for (int i = 0; i < count; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            // Child process
            
            // Set up input
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Set up output
            if (i < count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute the command
            exec_ast(commands[i]);
            exit(1);
        } else if (pids[i] < 0) {
            perror("fork");
            return -1;
        }
    }
    
    // Close all pipe file descriptors in parent
    for (int i = 0; i < count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children
    int status, last_status = 0;
    for (int i = 0; i < count; i++) {
        waitpid(pids[i], &status, 0);
        if (i == count - 1) {
            last_status = WEXITSTATUS(status);
        }
    }
    
    return last_status;
}
