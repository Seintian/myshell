#include "evloop.h"
#include "unity.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static int cb_count = 0;
static void cb_stop_on_event(int fd, void *data) {
    (void)fd;
    evloop_t *loop = (evloop_t *)data;
    cb_count++;
    evloop_stop(loop);
}

static int cb_data_count = 0;
static void cb_count_only(int fd, void *data) {
    (void)fd;
    (void)data;
    cb_data_count++;
}

typedef struct {
    evloop_t *loop;
    int *counter;
    int target;
} multi_cb_ctx_t;

static void cb_multi_and_maybe_stop(int fd, void *data) {
    (void)fd;
    multi_cb_ctx_t *ctx = (multi_cb_ctx_t *)data;
    (*(ctx->counter))++;
    if (*(ctx->counter) >= ctx->target)
        evloop_stop(ctx->loop);
}

void test_evloop_create_and_free(void) {
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);
    evloop_free(loop);
}

void test_evloop_add_invalid_params(void) {
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);
    int rc;
    rc = evloop_add_fd(NULL, 0, EVLOOP_READ, cb_count_only, NULL);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    rc = evloop_add_fd(loop, -1, EVLOOP_READ, cb_count_only, NULL);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    rc = evloop_add_fd(loop, 0, EVLOOP_READ, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    evloop_free(loop);
}

void test_evloop_run_timeout_no_events(void) {
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);
    // No fds registered; run with finite timeout should return quickly
    int rc = evloop_run(loop, 10);
    TEST_ASSERT_EQUAL_INT(0, rc);
    evloop_free(loop);
}

void test_evloop_callback_trigger_and_stop(void) {
    int fds[2];
    TEST_ASSERT_EQUAL_INT(0, pipe(fds));
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);

    cb_count = 0;
    int rc = evloop_add_fd(loop, fds[0], EVLOOP_READ, cb_stop_on_event, loop);
    TEST_ASSERT_EQUAL_INT(0, rc);

    // Trigger readiness
    const char *msg = "x";
    ssize_t w = write(fds[1], msg, strlen(msg));
    TEST_ASSERT_TRUE(w > 0);

    rc = evloop_run(loop, -1);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_INT(1, cb_count);

    // Cleanup
    close(fds[0]);
    close(fds[1]);
    evloop_free(loop);
}

void test_evloop_remove_fd_and_rerun(void) {
    int fds[2];
    TEST_ASSERT_EQUAL_INT(0, pipe(fds));
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);

    cb_data_count = 0;
    TEST_ASSERT_EQUAL_INT(0, evloop_add_fd(loop, fds[0], EVLOOP_READ, cb_count_only, NULL));
    // Remove it and ensure run with timeout doesn't invoke callback
    TEST_ASSERT_EQUAL_INT(0, evloop_remove_fd(loop, fds[0]));
    int rc = evloop_run(loop, 5);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_INT(0, cb_data_count);
    close(fds[0]);
    close(fds[1]);
    evloop_free(loop);
}

void test_evloop_add_write_event_and_trigger(void) {
    // Register WRITE interest and stop the loop once it's reported ready.
    int fds[2];
    TEST_ASSERT_EQUAL_INT(0, pipe(fds));
    // Set non-blocking (benign here) and expect write end usually writable.
    fcntl(fds[1], F_SETFL, O_NONBLOCK);
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);
    cb_count = 0;
    TEST_ASSERT_EQUAL_INT(0, evloop_add_fd(loop, fds[1], EVLOOP_WRITE, cb_stop_on_event, loop));
    int rc = evloop_run(loop, -1); // will stop via callback on first readiness
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_INT(1, cb_count);
    close(fds[0]);
    close(fds[1]);
    evloop_free(loop);
}

void test_evloop_remove_fd_not_found(void) {
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);
    int rc = evloop_remove_fd(loop, 12345); // not registered
    TEST_ASSERT_EQUAL_INT(-1, rc);
    // Also invalid loop
    rc = evloop_remove_fd(NULL, 3);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    evloop_free(loop);
}

void test_evloop_run_null_loop_returns_error(void) {
    int rc = evloop_run(NULL, 0);
    TEST_ASSERT_EQUAL_INT(-1, rc);
}

void test_evloop_two_fds_callbacks_and_stop(void) {
    int p1[2], p2[2];
    TEST_ASSERT_EQUAL_INT(0, pipe(p1));
    TEST_ASSERT_EQUAL_INT(0, pipe(p2));
    evloop_t *loop = evloop_create();
    TEST_ASSERT_NOT_NULL(loop);

    int ready_count = 0;
    multi_cb_ctx_t c1 = {.loop = loop, .counter = &ready_count, .target = 2};
    multi_cb_ctx_t c2 = {.loop = loop, .counter = &ready_count, .target = 2};
    TEST_ASSERT_EQUAL_INT(0, evloop_add_fd(loop, p1[0], EVLOOP_READ, cb_multi_and_maybe_stop, &c1));
    TEST_ASSERT_EQUAL_INT(0, evloop_add_fd(loop, p2[0], EVLOOP_READ, cb_multi_and_maybe_stop, &c2));

    // Trigger both
    const char *x = "x";
    TEST_ASSERT_TRUE(write(p1[1], x, 1) == 1);
    TEST_ASSERT_TRUE(write(p2[1], x, 1) == 1);

    int rc = evloop_run(loop, -1); // will stop after both callbacks fire
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_INT(2, ready_count);

    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    evloop_free(loop);
}
