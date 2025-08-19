/**
 * @file redir.h
 * @brief File descriptor redirection helpers.
 */
#ifndef REDIR_H
#define REDIR_H
/** \defgroup group_redir redirection
 *  @brief File descriptor redirection helpers.
 *  @{ */

/** Types of redirection supported. */
typedef enum { REDIR_INPUT,   /**< Redirect input from file. */
               REDIR_OUTPUT,  /**< Redirect output to file. */
               REDIR_APPEND } /**< Append output to file. */
redir_type_t;

/** A redirection specification. The filename is owned by the redir_t. */
typedef struct {
    redir_type_t type; /**< Kind of redirection. */
    int fd;            /**< Target file descriptor (e.g., STDIN_FILENO). */
    char *filename;    /**< Path to the file. */
} redir_t;

int redir_setup(redir_t *redir);
/** Placeholder that would restore descriptors in a fuller implementation. */
void redir_cleanup(redir_t *redir);
/** Allocate a redir object; copies filename; free with redir_free(). */
redir_t *redir_create(redir_type_t type, int fd, const char *filename);
/** Free a redir object and its owned filename. Safe on NULL. */
void redir_free(redir_t *redir);

/** @} */

#endif // REDIR_H
