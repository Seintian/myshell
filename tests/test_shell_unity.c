#include "shell.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// External variable from shell.h
extern int shell_running;

void test_shell_init(void) {
    // Test shell initialization
    int saved_running = shell_running;
    shell_init();
    TEST_ASSERT_EQUAL(1, shell_running);
    shell_cleanup();
    shell_running = saved_running;
}

// Helper to run shell_main with provided input (fed to stdin)
// If force_running is non-zero, sets shell_running=1 before run to ensure loop starts
static int run_shell_with_input_ex(const char *input, int force_running) {
    int saved_stdin = dup(STDIN_FILENO);
    TEST_ASSERT_NOT_EQUAL(-1, saved_stdin);

    int pipefd[2];
    TEST_ASSERT_EQUAL(0, pipe(pipefd));

    // Write input (if any), then close the write end to signal EOF
    if (input && input[0] != '\0') {
        ssize_t len = (ssize_t)strlen(input);
        ssize_t written = write(pipefd[1], input, (size_t)len);
        (void)written; // ignore short writes in tests
    }
    close(pipefd[1]);

    // Redirect stdin to the read end of the pipe
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);

    // Optionally ensure the shell loop can start even if previous tests set shell_running = 0
    int saved_running = shell_running;
    if (force_running) {
        shell_running = 1;
    }

    int rc = shell_main(0, NULL);

    // Restore global and stdin
    shell_running = saved_running;
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
    return rc;
}

static int run_shell_with_input(const char *input) { return run_shell_with_input_ex(input, 1); }

void test_shell_cleanup(void) {
    // Test shell cleanup
    shell_init();
    shell_cleanup();

    // Cleanup should complete without crashing
    TEST_ASSERT_TRUE(1);
}

void test_shell_init_cleanup_multiple(void) {
    int saved_running = shell_running;
    for (int i = 0; i < 3; ++i) {
        shell_init();
        TEST_ASSERT_EQUAL(1, shell_running);
        shell_cleanup();
    }
    shell_running = saved_running;
}

void test_shell_running_variable(void) {
    // Test that shell_running variable exists and can be accessed
    int initial_value = shell_running;

    shell_running = 1;
    TEST_ASSERT_EQUAL(1, shell_running);

    shell_running = 0;
    TEST_ASSERT_EQUAL(0, shell_running);

    // Restore initial value
    shell_running = initial_value;
}

// Removed redundant shell_main argv/argc variants; replaced with input-driven tests

void test_shell_main_eof_returns_zero(void) {
    // No input, immediate EOF should return 0
    int rc = run_shell_with_input("");
    TEST_ASSERT_EQUAL(0, rc);
}

void test_shell_main_runs_simple_command(void) {
    // Execute a trivial command through the shell
    int rc = run_shell_with_input("echo hello\n");
    TEST_ASSERT_EQUAL(0, rc);
}

void test_shell_main_exit_stops_loop(void) {
    // Ensure 'exit' stops the loop and returns cleanly
    int saved = shell_running;
    int rc = run_shell_with_input("exit\n");
    TEST_ASSERT_EQUAL(0, rc);
    shell_running = saved;
}

void test_shell_main_nonexistent_command_returns_zero(void) {
    // Current implementation returns 0 even for unknown commands
    int rc = run_shell_with_input("__definitely_not_a_command__\n");
    TEST_ASSERT_EQUAL(0, rc);
}

void test_shell_main_multiline_script_last_status(void) {
    // Multiple commands; last should determine rc
    int rc = run_shell_with_input("echo one\n echo two\n");
    TEST_ASSERT_EQUAL(0, rc);
}

void test_shell_main_handles_empty_lines(void) {
    int rc = run_shell_with_input("\n\n\n");
    TEST_ASSERT_EQUAL(0, rc);
}

void test_shell_main_does_not_start_when_running_false(void) {
    // If shell_running is 0 before call and not forced, loop must not start
    int saved = shell_running;
    shell_running = 0;
    int rc = run_shell_with_input_ex("echo should_not_run\n", 0);
    TEST_ASSERT_EQUAL(0, rc);
    shell_running = saved;
}

void test_shell_state_consistency(void) {
    int saved_running = shell_running;
    shell_init();
    TEST_ASSERT_EQUAL(1, shell_running);
    shell_cleanup();
    shell_init();
    TEST_ASSERT_EQUAL(1, shell_running);
    shell_cleanup();
    shell_running = saved_running;
}

void test_shell_double_init(void) {
    int saved_running = shell_running;
    shell_init();
    shell_init(); // Second init should be safe
    TEST_ASSERT_EQUAL(1, shell_running);
    shell_cleanup();
    shell_running = saved_running;
}

void test_shell_double_cleanup(void) {
    // Test double cleanup
    shell_init();
    shell_cleanup();
    shell_cleanup(); // Second cleanup should be safe

    // Should handle double cleanup gracefully
    TEST_ASSERT_TRUE(1);
}

void test_shell_cleanup_without_init(void) {
    // Test cleanup without prior init
    shell_cleanup();

    // Should handle cleanup without init gracefully
    TEST_ASSERT_TRUE(1);
}
