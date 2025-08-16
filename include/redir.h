#ifndef REDIR_H
#define REDIR_H

// Redirection helpers

typedef enum {
    REDIR_INPUT,
    REDIR_OUTPUT,
    REDIR_APPEND
} redir_type_t;

typedef struct {
    redir_type_t type;
    int fd;
    char *filename;
} redir_t;

// Redirection functions
int redir_setup(redir_t *redir);
void redir_cleanup(redir_t *redir);
redir_t *redir_create(redir_type_t type, int fd, const char *filename);
void redir_free(redir_t *redir);

#endif // REDIR_H
