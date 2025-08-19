#include "logger.h"
#include "unity.h"
#include <stdarg.h>
#include <stdlib.h>

void test_logger_init_shutdown_idempotent(void) {
    unsetenv("MYSHELL_DISABLE_LOGGER");
    TEST_ASSERT_EQUAL_INT(0, logger_init());
    // Second init should be ok
    TEST_ASSERT_EQUAL_INT(0, logger_init());
    logger_shutdown();
    // Second shutdown should be safe
    logger_shutdown();
}

void test_logger_set_get_level(void) {
    unsetenv("MYSHELL_DISABLE_LOGGER");
    TEST_ASSERT_EQUAL_INT(0, logger_init());
    logger_set_level(LOG_LEVEL_DEBUG);
    TEST_ASSERT_EQUAL(LOG_LEVEL_DEBUG, logger_get_level());
    logger_set_level(LOG_LEVEL_OFF);
    TEST_ASSERT_EQUAL(LOG_LEVEL_OFF, logger_get_level());
    logger_shutdown();
}

void test_logger_log_calls_do_not_crash(void) {
    unsetenv("MYSHELL_DISABLE_LOGGER");
    TEST_ASSERT_EQUAL_INT(0, logger_init());
    logger_set_level(LOG_LEVEL_INFO);
    LOG_DEBUG("this should be dropped");
    LOG_INFO("hello %d", 42);
    LOG_WARN("warn");
    LOG_ERROR("error");
    logger_shutdown();
}

void test_logger_disabled_env(void) {
    setenv("MYSHELL_DISABLE_LOGGER", "1", 1);
    TEST_ASSERT_EQUAL_INT(0, logger_init());
    logger_set_level(LOG_LEVEL_TRACE);
    LOG_TRACE("trace even if disabled");
    logger_shutdown();
    unsetenv("MYSHELL_DISABLE_LOGGER");
}

static void vlog_helper(log_level_t lvl, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    logger_vlog(lvl, fmt, ap);
    va_end(ap);
}

void test_logger_vlog_variadic_path(void) {
    unsetenv("MYSHELL_DISABLE_LOGGER");
    TEST_ASSERT_EQUAL_INT(0, logger_init());
    logger_set_level(LOG_LEVEL_TRACE);
    vlog_helper(LOG_LEVEL_TRACE, "trace %s %d", "x", 1);
    vlog_helper(LOG_LEVEL_INFO, "info");
    logger_shutdown();
}

void test_logger_level_gating(void) {
    unsetenv("MYSHELL_DISABLE_LOGGER");
    TEST_ASSERT_EQUAL_INT(0, logger_init());
    logger_set_level(LOG_LEVEL_ERROR);
    // Only ERROR should pass through; others should be dropped silently
    LOG_TRACE("t");
    LOG_DEBUG("d");
    LOG_INFO("i");
    LOG_WARN("w");
    LOG_ERROR("e");
    logger_shutdown();
}
