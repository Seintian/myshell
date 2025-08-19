#include "env.h"
#include "shell.h"
#include "unity.h"
#include <fcntl.h>
#include <stdio.h>
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

    shell_init();
    int rc = shell_main(0, NULL);
    shell_cleanup();

    // Restore global and stdin
    shell_running = saved_running;
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
    return rc;
}

static int run_shell_with_input(const char *input) {
    return run_shell_with_input_ex(input, 1);
}

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
    TEST_ASSERT_NOT_EQUAL(0, rc);
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

// ---- New tests for non-interactive script mode and flags ----

static char *write_temp_script(const char *contents) {
    char templ[] = "/tmp/myshell_script_XXXXXX";
    int fd = mkstemp(templ);
    TEST_ASSERT_NOT_EQUAL(-1, fd);
    size_t len = strlen(contents);
    ssize_t w = write(fd, contents, len);
    (void)w;
    close(fd);
    return strdup(templ);
}

// (helper removed)

void test_shell_run_file_status_propagates(void) {
    char *path = write_temp_script("exit 42\n");
    char *argvv[] = {"myshell", path, NULL};
    int rc = shell_main(2, argvv);
    TEST_ASSERT_EQUAL(42, rc);
    unlink(path);
    free(path);
}

void test_shell_run_file_shebang_skipped(void) {
    char *path = write_temp_script("#!/usr/bin/env myshell\nexit 7\n");
    char *argvv[] = {"myshell", path, NULL};
    int rc = shell_main(2, argvv);
    TEST_ASSERT_EQUAL(7, rc);
    unlink(path);
    free(path);
}

void test_shell_run_file_errexit_stops_on_error(void) {
    // cd to nonexistent path should fail; with -e script stops and returns non-zero
    char *path = write_temp_script("cd /definitely/not/exists\necho after\nexit 0\n");
    char *argvv[] = {"myshell", "-e", path, NULL};
    int rc = shell_main(3, argvv);
    TEST_ASSERT_NOT_EQUAL(0, rc);
    unlink(path);
    free(path);
}

void test_shell_run_file_errexit_off_allows_continue(void) {
    // Without -e, last status should be from final exit 0
    char *path = write_temp_script("cd /definitely/not/exists\nexit 0\n");
    char *argvv[] = {"myshell", path, NULL};
    int rc = shell_main(2, argvv);
    TEST_ASSERT_EQUAL(0, rc);
    unlink(path);
    free(path);
}

void test_shell_run_file_xtrace_does_not_change_status(void) {
    char *path = write_temp_script("exit 3\n");
    char *argvv[] = {"myshell", "-x", path, NULL};
    int rc = shell_main(3, argvv);
    TEST_ASSERT_EQUAL(3, rc);
    unlink(path);
    free(path);
}

void test_shell_source_semantics_env_persists(void) {
    // Script exports a variable; it should persist in the current process
    char *path = write_temp_script("export FOO=bar\n");
    char *argvv[] = {"myshell", path, NULL};
    int rc = shell_main(2, argvv);
    TEST_ASSERT_EQUAL(0, rc);
    const char *val = env_get("FOO");
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_TRUE(strcmp(val, "bar") == 0);
    unlink(path);
    free(path);
}

void test_shell_set_builtin_toggles_flags(void) {
    // Ensure clean env for side-effect checks
    (void)env_unset("T1");
    (void)env_unset("T2");

    // 1) errexit on: failure should stop before later commands (export T1)
    char *path1 = write_temp_script(
        "set -e\n"
        "cd /definitely/not/exists\n"
        "export T1=ok\n"
        "exit 0\n");
    char *argvv1[] = {"myshell", path1, NULL};
    int rc = shell_main(2, argvv1);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, rc, "invalid cd > rc == 0");
    const char *t1 = env_get("T1");
    TEST_ASSERT_TRUE(t1 == NULL || t1[0] == '\0');
    unlink(path1);
    free(path1);

    // 2) toggle -e off before a failure: subsequent commands continue and set T2
    char *path2 = write_temp_script(
        "set -e\n"
        "set +e\n"
        "set\n"
        "cd /definitely/not/exists\n"
        "export T2=ok\n"
        "echo $T2\n"
        "exit 0\n");
    char *argvv2[] = {"myshell", path2, NULL};
    rc = shell_main(2, argvv2);
    TEST_ASSERT_EQUAL(0, rc);
    const char *t2 = env_get("T2");
    TEST_ASSERT_NOT_NULL(t2);
    TEST_ASSERT_TRUE(strcmp(t2, "ok") == 0);
    unlink(path2);
    free(path2);

    // 3) xtrace on/off should not affect status
    char *path3 = write_temp_script("set -x\nexit 0\n");
    char *argvv3[] = {"myshell", path3, NULL};
    rc = shell_main(2, argvv3);
    TEST_ASSERT_EQUAL(0, rc);
    unlink(path3);
    free(path3);

    char *path4 = write_temp_script("set -x\nset +x\nexit 0\n");
    char *argvv4[] = {"myshell", path4, NULL};
    rc = shell_main(2, argvv4);
    TEST_ASSERT_EQUAL(0, rc);
    unlink(path4);
    free(path4);

    // 4) invalid option yields non-zero (builtin returns error)
    char *path5 = write_temp_script("set -q\n");
    char *argvv5[] = {"myshell", path5, NULL};
    rc = shell_main(2, argvv5);
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0, rc, "set -q > rc == 0");
    unlink(path5);
    free(path5);

    // Cleanup env side effects
    (void)env_unset("T1");
    (void)env_unset("T2");
}
