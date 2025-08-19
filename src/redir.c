/**
 * @file redir.c
 * @brief File descriptor redirection helpers.
 */
#include "redir.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

redir_t *redir_create(redir_type_t type, int fd, const char *filename) {
    redir_t *redir = malloc_safe(sizeof(redir_t));
    redir->type = type;
    redir->fd = fd;
    redir->filename = strdup_safe(filename);
    return redir;
}

static int open_and_dup(const char *path, int oflags, mode_t mode, int target_fd) {
    int file_fd = (oflags & O_CREAT) ? open(path, oflags, mode) : open(path, oflags);
    if (file_fd == -1) {
        perror("open");
        return -1;
    }
    if (dup2(file_fd, target_fd) == -1) {
        perror("dup2");
        close(file_fd);
        return -1;
    }
    close(file_fd);
    return 0;
}

int redir_setup(redir_t *redir) {
    if (!redir || !redir->filename) {
        errno = EINVAL;
        return -1;
    }

    switch (redir->type) {
    case REDIR_INPUT:
        return open_and_dup(redir->filename, O_RDONLY, 0, redir->fd);
    case REDIR_OUTPUT:
        return open_and_dup(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644, redir->fd);
    case REDIR_APPEND:
        return open_and_dup(redir->filename, O_WRONLY | O_CREAT | O_APPEND, 0644, redir->fd);
    default:
        errno = EINVAL;
        return -1;
    }
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
