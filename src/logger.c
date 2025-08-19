/**
 * @file logger.c
 * @brief Minimal async logger implementation.
 */
#include "logger.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_QUEUE_CAP 1024
#define LOG_MSG_MAX 1024

typedef struct {
    log_level_t level;
    char msg[LOG_MSG_MAX];
} log_item_t;

static pthread_t log_thread;
static pthread_mutex_t log_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t log_cv = PTHREAD_COND_INITIALIZER;
static log_item_t queue[LOG_QUEUE_CAP];
static size_t q_head = 0, q_size = 0;
static int running = 0;
static log_level_t current_level = LOG_LEVEL_OFF;
static int logger_enabled = 1;

static const char *lvl_name(log_level_t lvl) {
    switch (lvl) {
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_WARN:
        return "WARN";
    case LOG_LEVEL_INFO:
        return "INFO";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_TRACE:
        return "TRACE";
    default:
        return "OFF";
    }
}

static void *log_consumer(void *arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&log_mu);
        while (running && q_size == 0)
            pthread_cond_wait(&log_cv, &log_mu);

        if (!running && q_size == 0) {
            pthread_mutex_unlock(&log_mu);
            break;
        }
        log_item_t item = queue[q_head];
        q_head = (q_head + 1) % LOG_QUEUE_CAP;
        q_size--;
        pthread_mutex_unlock(&log_mu);

        // Timestamp and print to stderr
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        struct tm tm;
        localtime_r(&ts.tv_sec, &tm);
        char tbuf[32];
        strftime(tbuf, sizeof tbuf, "%H:%M:%S", &tm);
        fprintf(stderr,
                "%s.%03ld [%s] %s\n",
                tbuf,
                ts.tv_nsec / 1000000,
                lvl_name(item.level),
                item.msg);
    }
    return NULL;
}

int logger_init(void) {
    const char *q = getenv("MYSHELL_DISABLE_LOGGER");
    if (q && q[0] == '1') {
        logger_enabled = 0;
        return 0;
    }
    pthread_mutex_lock(&log_mu);
    if (running) {
        pthread_mutex_unlock(&log_mu);
        return 0;
    }
    running = 1;
    pthread_mutex_unlock(&log_mu);
    if (pthread_create(&log_thread, NULL, log_consumer, NULL) != 0) {
        pthread_mutex_lock(&log_mu);
        running = 0;
        pthread_mutex_unlock(&log_mu);
        return -1;
    }
    return 0;
}

void logger_shutdown(void) {
    if (!logger_enabled)
        return;
    pthread_mutex_lock(&log_mu);
    if (!running) {
        pthread_mutex_unlock(&log_mu);
        return;
    }
    running = 0;
    pthread_cond_broadcast(&log_cv);
    pthread_mutex_unlock(&log_mu);
    pthread_join(log_thread, NULL);
}

void logger_set_level(log_level_t level) {
    current_level = level;
}

log_level_t logger_get_level(void) {
    return current_level;
}

static void enqueue(log_level_t level, const char *buf) {
    pthread_mutex_lock(&log_mu);
    if (q_size == LOG_QUEUE_CAP) {
        // Drop oldest (bounded queue)
        q_head = (q_head + 1) % LOG_QUEUE_CAP;
        q_size--;
    }
    size_t idx = (q_head + q_size) % LOG_QUEUE_CAP;
    queue[idx].level = level;
    strncpy(queue[idx].msg, buf, LOG_MSG_MAX - 1);
    queue[idx].msg[LOG_MSG_MAX - 1] = '\0';
    q_size++;
    pthread_cond_signal(&log_cv);
    pthread_mutex_unlock(&log_mu);
}

void logger_vlog(log_level_t level, const char *fmt, va_list ap) {
    if (!logger_enabled)
        return;
    if (level > current_level || level == LOG_LEVEL_OFF)
        return;
    char buf[LOG_MSG_MAX];
    vsnprintf(buf, sizeof buf, fmt, ap);
    enqueue(level, buf);
}

void logger_log(log_level_t level, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logger_vlog(level, fmt, ap);
    va_end(ap);
}
