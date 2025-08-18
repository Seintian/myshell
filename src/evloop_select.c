/**
 * @file evloop_select.c
 * @brief select(2)-based implementation of the event loop abstraction.
 */
#include "evloop.h"
#include "util.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

struct evloop_fd {
    int fd;
    evloop_events_t events;
    evloop_callback_t callback;
    void *data;
    struct evloop_fd *next;
};

struct evloop {
    struct evloop_fd *fds;
    int running;
    int max_fd;
};

evloop_t *evloop_create(void) {
    evloop_t *loop = malloc_safe(sizeof(evloop_t));
    loop->fds = NULL;
    loop->running = 0;
    loop->max_fd = -1;
    return loop;
}

int evloop_add_fd(evloop_t *loop, int fd, evloop_events_t events,
                  evloop_callback_t callback, void *data) {
    if (!loop || fd < 0 || !callback) {
        return -1;
    }

    struct evloop_fd *evfd = malloc_safe(sizeof(struct evloop_fd));
    evfd->fd = fd;
    evfd->events = events;
    evfd->callback = callback;
    evfd->data = data;
    evfd->next = loop->fds;
    loop->fds = evfd;

    if (fd > loop->max_fd) {
        loop->max_fd = fd;
    }

    return 0;
}

int evloop_remove_fd(evloop_t *loop, int fd) {
    if (!loop)
        return -1;

    struct evloop_fd *current = loop->fds;
    struct evloop_fd *prev = NULL;

    while (current) {
        if (current->fd == fd) {
            if (prev) {
                prev->next = current->next;
            } else {
                loop->fds = current->next;
            }
            free(current);

            // Recalculate max_fd
            loop->max_fd = -1;
            current = loop->fds;
            while (current) {
                if (current->fd > loop->max_fd) {
                    loop->max_fd = current->fd;
                }
                current = current->next;
            }

            return 0;
        }
        prev = current;
        current = current->next;
    }

    return -1;
}

int evloop_run(evloop_t *loop, int timeout_ms) {
    if (!loop)
        return -1;

    loop->running = 1;

    while (loop->running && loop->fds) {
        fd_set readfds, writefds, errorfds;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&errorfds);

        struct evloop_fd *current = loop->fds;
        while (current) {
            if (current->events & EVLOOP_READ) {
                FD_SET(current->fd, &readfds);
            }
            if (current->events & EVLOOP_WRITE) {
                FD_SET(current->fd, &writefds);
            }
            if (current->events & EVLOOP_ERROR) {
                FD_SET(current->fd, &errorfds);
            }
            current = current->next;
        }

        struct timeval timeout;
        struct timeval *timeout_ptr = NULL;
        if (timeout_ms >= 0) {
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;
            timeout_ptr = &timeout;
        }

        int result =
            select(loop->max_fd + 1, &readfds, &writefds, &errorfds, timeout_ptr);

        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            return -1;
        }

        if (result == 0) {
            // Timeout
            if (timeout_ms >= 0) {
                break;
            }
            continue;
        }

        // Process ready file descriptors
        current = loop->fds;
        while (current && loop->running) {
            int fd_ready = 0;

            if (FD_ISSET(current->fd, &readfds) && (current->events & EVLOOP_READ)) {
                fd_ready = 1;
            }
            if (FD_ISSET(current->fd, &writefds) &&
                (current->events & EVLOOP_WRITE)) {
                fd_ready = 1;
            }
            if (FD_ISSET(current->fd, &errorfds) &&
                (current->events & EVLOOP_ERROR)) {
                fd_ready = 1;
            }

            if (fd_ready) {
                current->callback(current->fd, current->data);
            }

            current = current->next;
        }
    }

    return 0;
}

void evloop_stop(evloop_t *loop) {
    if (loop) {
        loop->running = 0;
    }
}

void evloop_free(evloop_t *loop) {
    if (!loop)
        return;

    while (loop->fds) {
        struct evloop_fd *next = loop->fds->next;
        free(loop->fds);
        loop->fds = next;
    }

    free(loop);
}
