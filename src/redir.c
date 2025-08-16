#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "redir.h"
#include "util.h"

redir_t *redir_create(redir_type_t type, int fd, const char *filename) {
    redir_t *redir = malloc_safe(sizeof(redir_t));
    redir->type = type;
    redir->fd = fd;
    redir->filename = strdup_safe(filename);
    return redir;
}

int redir_setup(redir_t *redir) {
    if (!redir || !redir->filename) {
        return -1;
    }
    
    int file_fd;
    
    switch (redir->type) {
        case REDIR_INPUT:
            file_fd = open(redir->filename, O_RDONLY);
            if (file_fd == -1) {
                perror("open");
                return -1;
            }
            if (dup2(file_fd, redir->fd) == -1) {
                perror("dup2");
                close(file_fd);
                return -1;
            }
            close(file_fd);
            break;
            
        case REDIR_OUTPUT:
            file_fd = open(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file_fd == -1) {
                perror("open");
                return -1;
            }
            if (dup2(file_fd, redir->fd) == -1) {
                perror("dup2");
                close(file_fd);
                return -1;
            }
            close(file_fd);
            break;
            
        case REDIR_APPEND:
            file_fd = open(redir->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (file_fd == -1) {
                perror("open");
                return -1;
            }
            if (dup2(file_fd, redir->fd) == -1) {
                perror("dup2");
                close(file_fd);
                return -1;
            }
            close(file_fd);
            break;
            
        default:
            return -1;
    }
    
    return 0;
}

void redir_cleanup(redir_t *redir __attribute__((unused))) {
    // In a real implementation, this would restore original file descriptors
    // For now, it's a placeholder
}

void redir_free(redir_t *redir) {
    if (redir) {
        free(redir->filename);
        free(redir);
    }
}
