/**
 * @file logger.h
 * @brief Minimal async logger with single consumer thread and bounded queue.
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

typedef enum {
    LOG_LEVEL_OFF = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
} log_level_t;

int logger_init(void);      // idempotent; returns 0 on success
void logger_shutdown(void); // joins background thread

void logger_set_level(log_level_t level);
log_level_t logger_get_level(void);

void logger_log(log_level_t level, const char *path, const char *fmt, ...);
void logger_vlog(log_level_t level, const char *path, const char *fmt, va_list ap);

#define LOG_ERROR(...) logger_log(LOG_LEVEL_ERROR, __FILE__, __VA_ARGS__)
#define LOG_WARN(...) logger_log(LOG_LEVEL_WARN, __FILE__, __VA_ARGS__)
#define LOG_INFO(...) logger_log(LOG_LEVEL_INFO, __FILE__, __VA_ARGS__)
#define LOG_DEBUG(...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __VA_ARGS__)
#define LOG_TRACE(...) logger_log(LOG_LEVEL_TRACE, __FILE__, __VA_ARGS__)

#endif // LOGGER_H
