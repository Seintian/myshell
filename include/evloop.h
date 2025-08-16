#ifndef EVLOOP_H
#define EVLOOP_H

// select()/epoll() abstraction

typedef struct evloop evloop_t;
typedef void (*evloop_callback_t)(int fd, void *data);

typedef enum {
    EVLOOP_READ = 1,
    EVLOOP_WRITE = 2,
    EVLOOP_ERROR = 4
} evloop_events_t;

// Event loop functions
evloop_t *evloop_create(void);
int evloop_add_fd(evloop_t *loop, int fd, evloop_events_t events, evloop_callback_t callback, void *data);
int evloop_remove_fd(evloop_t *loop, int fd);
int evloop_run(evloop_t *loop, int timeout_ms);
void evloop_stop(evloop_t *loop);
void evloop_free(evloop_t *loop);

#endif // EVLOOP_H
