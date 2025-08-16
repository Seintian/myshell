#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include "evloop.h"
#include "util.h"

#define MAX_EVENTS 64

struct evloop_fd {
    int fd;
    evloop_events_t events;
    evloop_callback_t callback;
    void *data;
    struct evloop_fd *next;
};

struct evloop {
    int epoll_fd;
    struct evloop_fd *fds;
    int running;
};

evloop_t *evloop_create(void) {
    evloop_t *loop = malloc_safe(sizeof(evloop_t));
    loop->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (loop->epoll_fd == -1) {
        perror("epoll_create1");
        free(loop);
        return NULL;
    }
    loop->fds = NULL;
    loop->running = 0;
    return loop;
}

int evloop_add_fd(evloop_t *loop, int fd, evloop_events_t events, evloop_callback_t callback, void *data) {
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
    
    struct epoll_event ev;
    ev.events = 0;
    if (events & EVLOOP_READ) ev.events |= EPOLLIN;
    if (events & EVLOOP_WRITE) ev.events |= EPOLLOUT;
    if (events & EVLOOP_ERROR) ev.events |= EPOLLERR;
    ev.data.ptr = evfd;
    
    if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl");
        free(evfd);
        return -1;
    }
    
    return 0;
}

int evloop_remove_fd(evloop_t *loop, int fd) {
    if (!loop) return -1;
    
    struct evloop_fd *current = loop->fds;
    struct evloop_fd *prev = NULL;
    
    while (current) {
        if (current->fd == fd) {
            if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
                perror("epoll_ctl");
            }
            
            if (prev) {
                prev->next = current->next;
            } else {
                loop->fds = current->next;
            }
            free(current);
            return 0;
        }
        prev = current;
        current = current->next;
    }
    
    return -1;
}

int evloop_run(evloop_t *loop, int timeout_ms) {
    if (!loop) return -1;
    
    loop->running = 1;
    
    struct epoll_event events[MAX_EVENTS];
    
    while (loop->running && loop->fds) {
        int nfds = epoll_wait(loop->epoll_fd, events, MAX_EVENTS, timeout_ms);
        
        if (nfds < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait");
            return -1;
        }
        
        if (nfds == 0) {
            // Timeout
            if (timeout_ms >= 0) {
                break;
            }
            continue;
        }
        
        for (int i = 0; i < nfds && loop->running; i++) {
            struct evloop_fd *evfd = (struct evloop_fd *)events[i].data.ptr;
            if (evfd && evfd->callback) {
                evfd->callback(evfd->fd, evfd->data);
            }
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
    if (!loop) return;
    
    while (loop->fds) {
        struct evloop_fd *next = loop->fds->next;
        epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, loop->fds->fd, NULL);
        free(loop->fds);
        loop->fds = next;
    }
    
    close(loop->epoll_fd);
    free(loop);
}

#endif // __linux__
