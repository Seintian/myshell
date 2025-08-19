/**
 * @file evloop.h
 * @brief Event loop abstraction over select()/epoll().
 */
#ifndef EVLOOP_H
#define EVLOOP_H
/** \defgroup group_evloop EventLoop
 *  @brief select/epoll event loop abstraction.
 *  @{ */

typedef struct evloop evloop_t;                        /**< Opaque event loop. */
typedef void (*evloop_callback_t)(int fd, void *data); /**< FD callback. */

/** Bitmask of interest events. */
typedef enum {
    EVLOOP_READ = 1,  /**< Read readiness. */
    EVLOOP_WRITE = 2, /**< Write readiness. */
    EVLOOP_ERROR = 4  /**< Error condition. */
} evloop_events_t;

// Event loop functions
/** Create a new event loop instance. */
evloop_t *evloop_create(void);
/** Register a file descriptor with interest events and a callback. */
int evloop_add_fd(evloop_t *loop, int fd, evloop_events_t events,
                  evloop_callback_t callback, void *data);
/** Unregister a file descriptor from the loop. */
int evloop_remove_fd(evloop_t *loop, int fd);
/** Run the loop, optionally with a timeout in milliseconds (-1 = infinite). */
int evloop_run(evloop_t *loop, int timeout_ms);
/** Request that the loop stop on the next iteration. */
void evloop_stop(evloop_t *loop);
/** Free the event loop and its internal registrations. */
void evloop_free(evloop_t *loop);

/** @} */

#endif // EVLOOP_H
