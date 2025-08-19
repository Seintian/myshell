#include "unity.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG 0

// Include all test modules (declare their test functions)
// Lexer tests
void test_lexer_create_and_free(void);
void test_lexer_empty_input(void);
void test_lexer_simple_word(void);
void test_lexer_multiple_words(void);
void test_lexer_pipe_token(void);
void test_lexer_redirection_tokens(void);
void test_lexer_special_characters(void);

// Parser tests
void test_parser_create_and_free(void);
void test_parser_simple_command(void);
void test_parser_empty_input(void);
void test_parser_pipeline(void);

// AST tests
void test_ast_free_null(void);
void test_ast_create_command_basic(void);
void test_ast_create_pipeline_basic(void);
void test_ast_create_command_null_argv(void);
void test_ast_create_command_deep_copy_safe(void);
void test_ast_free_nested_pipeline(void);
void test_ast_create_command_empty_argv_array(void);

// Environment tests
void test_env_get_existing(void);
void test_env_get_nonexistent(void);
void test_env_set_and_get(void);
void test_env_unset(void);
void test_expand_variables_simple(void);
void test_expand_variables_no_expansion(void);

// Utility tests
void test_strdup_safe(void);
void test_strdup_safe_null(void);
void test_split_string(void);
void test_split_string_single(void);
void test_string_array_length(void);
void test_string_array_length_empty(void);
void test_string_array_length_null(void);

// Jobs tests
void test_job_create(void);
void test_job_create_null_command(void);
void test_job_set_status(void);
void test_job_find_nonexistent(void);
void test_job_find_after_create(void);
void test_job_fg_null(void);
void test_job_bg_null(void);
void test_job_list_empty(void);
void test_job_cleanup_empty(void);
void test_job_multiple_create(void);

// Execution tests
void test_exec_ast_null(void);
void test_exec_command_null(void);
void test_exec_pipeline_null(void);
void test_exec_simple_command_node(void);
void test_exec_command_with_args(void);
void test_exec_nonexistent_command(void);
void test_exec_pipeline_creation(void);
void test_exec_empty_command(void);
void test_exec_builtin_simulation(void);

// Builtin tests
void test_builtin_find_existing(void);
void test_builtin_find_nonexistent(void);
void test_builtin_find_null_name(void);
void test_builtin_execute_existing(void);
void test_builtin_execute_nonexistent(void);
void test_builtin_execute_null_name(void);
void test_builtin_list(void);
void test_builtin_cd(void);
void test_builtin_pwd(void);
void test_builtin_export(void);
void test_builtin_unset(void);
void test_builtin_type(void);
void test_builtin_find_all_core_builtins(void);

// Plugin tests
void test_plugin_load_null_path(void);
void test_plugin_load_nonexistent_file(void);
void test_plugin_load_invalid_file(void);
void test_plugin_find_nonexistent(void);
void test_plugin_find_null_name(void);
void test_plugin_execute_nonexistent(void);
void test_plugin_execute_null_name(void);
void test_plugin_unload_nonexistent(void);
void test_plugin_unload_null_name(void);
void test_plugin_list_empty(void);
void test_plugin_cleanup_all_empty(void);
void test_plugin_load_existing_hello(void);
void test_plugin_load_relative_path(void);
void test_plugin_multiple_operations(void);

// Redirection tests
void test_redir_create_input(void);
void test_redir_create_output(void);
void test_redir_create_append(void);
void test_redir_create_null_filename(void);
void test_redir_free_null(void);
void test_redir_setup_null(void);
void test_redir_cleanup_null(void);
void test_redir_setup_input_dev_null(void);
void test_redir_setup_output_dev_null(void);
void test_redir_setup_nonexistent_input(void);
void test_redir_invalid_fd(void);
void test_redir_large_fd(void);
void test_redir_all_types(void);

// Terminal tests
void test_term_get_size(void);
void test_term_get_size_null_pointers(void);
void test_term_raw_mode(void);
void test_term_cooked_mode(void);
void test_term_mode_transitions(void);
void test_term_clear_screen(void);
void test_term_move_cursor(void);
void test_term_move_cursor_negative(void);
void test_term_move_cursor_large(void);
void test_term_setup_signals(void);
void test_term_restore_signals(void);
void test_term_signal_transitions(void);
void test_term_combined_operations(void);

// Event loop tests
void test_evloop_create_and_free(void);
void test_evloop_add_invalid_params(void);
void test_evloop_run_timeout_no_events(void);
void test_evloop_callback_trigger_and_stop(void);
void test_evloop_remove_fd_and_rerun(void);
void test_evloop_add_write_event_and_trigger(void);
void test_evloop_remove_fd_not_found(void);
void test_evloop_run_null_loop_returns_error(void);
void test_evloop_two_fds_callbacks_and_stop(void);

// Logger tests
void test_logger_init_shutdown_idempotent(void);
void test_logger_set_get_level(void);
void test_logger_log_calls_do_not_crash(void);
void test_logger_disabled_env(void);
void test_logger_vlog_variadic_path(void);
void test_logger_level_gating(void);

// Shell tests
void test_shell_init(void);
void test_shell_cleanup(void);
void test_shell_init_cleanup_multiple(void);
void test_shell_running_variable(void);
void test_shell_main_eof_returns_zero(void);
void test_shell_main_runs_simple_command(void);
void test_shell_main_exit_stops_loop(void);
void test_shell_main_nonexistent_command_returns_zero(void);
void test_shell_main_multiline_script_last_status(void);
void test_shell_main_handles_empty_lines(void);
void test_shell_main_does_not_start_when_running_false(void);
void test_shell_state_consistency(void);
void test_shell_double_init(void);
void test_shell_double_cleanup(void);
void test_shell_cleanup_without_init(void);
// New non-interactive/script and flags tests
void test_shell_run_file_status_propagates(void);
void test_shell_run_file_shebang_skipped(void);
void test_shell_run_file_errexit_stops_on_error(void);
void test_shell_run_file_errexit_off_allows_continue(void);
void test_shell_run_file_xtrace_does_not_change_status(void);
void test_shell_source_semantics_env_persists(void);
void test_shell_set_builtin_toggles_flags(void);

// Pipeline tests
void test_pipeline_execute_null_commands(void);
void test_pipeline_execute_zero_count(void);
void test_pipeline_execute_negative_count(void);
void test_pipeline_execute_single_command(void);
void test_pipeline_execute_single_null_command(void);
void test_pipeline_execute_two_commands(void);
void test_pipeline_execute_mixed_commands(void);
void test_pipeline_execute_large_count(void);
void test_pipeline_execute_echo_cat(void);
void test_pipeline_execute_three_commands(void);

// Global variables to store original file descriptors
static int original_stdout = -1;
static int original_stderr = -1;
static int devnull_fd = -1;

void setUp(void) {
#if DEBUG != 1
    // Redirect both stdout and stderr before each test to suppress API output
    fflush(stdout);
    fflush(stderr);
    original_stdout = dup(STDOUT_FILENO);
    original_stderr = dup(STDERR_FILENO);
    devnull_fd = open("/dev/null", O_RDWR);

    dup2(devnull_fd, STDOUT_FILENO);
    dup2(devnull_fd, STDERR_FILENO);
    // Disable buffering to avoid delayed writes leaking after tearDown
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif // DEBUG
}

void tearDown(void) {
    // Restore stdout and stderr after each test
    fflush(stdout);
    fflush(stderr);
    if (original_stdout != -1) {
        dup2(original_stdout, STDOUT_FILENO);
        close(original_stdout);
        original_stdout = -1;
    }
    if (original_stderr != -1) {
        dup2(original_stderr, STDERR_FILENO);
        close(original_stderr);
        original_stderr = -1;
    }
    if (devnull_fd != -1) {
        close(devnull_fd);
        devnull_fd = -1;
    }
    // Restore default buffering (line-buffered for stdout if tty, full for stderr)
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

int main(void) {
    UNITY_BEGIN();

    // Lexer tests
    printf("=== Running Lexer Tests ===\n");
    RUN_TEST(test_lexer_create_and_free);
    RUN_TEST(test_lexer_empty_input);
    RUN_TEST(test_lexer_simple_word);
    RUN_TEST(test_lexer_multiple_words);
    RUN_TEST(test_lexer_pipe_token);
    RUN_TEST(test_lexer_redirection_tokens);
    RUN_TEST(test_lexer_special_characters);

    // Parser tests
    printf("=== Running Parser Tests ===\n");
    RUN_TEST(test_parser_create_and_free);
    RUN_TEST(test_parser_simple_command);
    RUN_TEST(test_parser_empty_input);
    RUN_TEST(test_parser_pipeline);

    // AST tests
    printf("=== Running AST Tests ===\n");
    RUN_TEST(test_ast_free_null);
    RUN_TEST(test_ast_create_command_basic);
    RUN_TEST(test_ast_create_pipeline_basic);
    RUN_TEST(test_ast_create_command_null_argv);
    RUN_TEST(test_ast_create_command_deep_copy_safe);
    RUN_TEST(test_ast_free_nested_pipeline);
    RUN_TEST(test_ast_create_command_empty_argv_array);

    // Environment tests
    printf("=== Running Environment Tests ===\n");
    RUN_TEST(test_env_get_existing);
    RUN_TEST(test_env_get_nonexistent);
    RUN_TEST(test_env_set_and_get);
    RUN_TEST(test_env_unset);
    RUN_TEST(test_expand_variables_simple);
    RUN_TEST(test_expand_variables_no_expansion);

    // Utility tests
    printf("=== Running Utility Tests ===\n");
    RUN_TEST(test_strdup_safe);
    RUN_TEST(test_strdup_safe_null);
    RUN_TEST(test_split_string);
    RUN_TEST(test_split_string_single);
    RUN_TEST(test_string_array_length);
    RUN_TEST(test_string_array_length_empty);
    RUN_TEST(test_string_array_length_null);

    // Jobs tests
    printf("=== Running Jobs Tests ===\n");
    RUN_TEST(test_job_create);
    RUN_TEST(test_job_create_null_command);
    RUN_TEST(test_job_set_status);
    RUN_TEST(test_job_find_nonexistent);
    RUN_TEST(test_job_find_after_create);
    RUN_TEST(test_job_fg_null);
    RUN_TEST(test_job_bg_null);
    RUN_TEST(test_job_list_empty);
    RUN_TEST(test_job_cleanup_empty);
    RUN_TEST(test_job_multiple_create);

    // Execution tests
    printf("=== Running Execution Tests ===\n");
    RUN_TEST(test_exec_ast_null);
    RUN_TEST(test_exec_command_null);
    RUN_TEST(test_exec_pipeline_null);
    RUN_TEST(test_exec_simple_command_node);
    RUN_TEST(test_exec_command_with_args);
    RUN_TEST(test_exec_nonexistent_command);
    RUN_TEST(test_exec_pipeline_creation);
    RUN_TEST(test_exec_empty_command);
    RUN_TEST(test_exec_builtin_simulation);

    // Builtin tests
    printf("=== Running Builtin Tests ===\n");
    RUN_TEST(test_builtin_find_existing);
    RUN_TEST(test_builtin_find_nonexistent);
    RUN_TEST(test_builtin_find_null_name);
    RUN_TEST(test_builtin_execute_existing);
    RUN_TEST(test_builtin_execute_nonexistent);
    RUN_TEST(test_builtin_execute_null_name);
    RUN_TEST(test_builtin_list);
    RUN_TEST(test_builtin_cd);
    RUN_TEST(test_builtin_pwd);
    RUN_TEST(test_builtin_export);
    RUN_TEST(test_builtin_unset);
    RUN_TEST(test_builtin_type);
    RUN_TEST(test_builtin_find_all_core_builtins);

    // Plugin tests
    printf("=== Running Plugin Tests ===\n");
    RUN_TEST(test_plugin_load_null_path);
    RUN_TEST(test_plugin_load_nonexistent_file);
    RUN_TEST(test_plugin_load_invalid_file);
    RUN_TEST(test_plugin_find_nonexistent);
    RUN_TEST(test_plugin_find_null_name);
    RUN_TEST(test_plugin_execute_nonexistent);
    RUN_TEST(test_plugin_execute_null_name);
    RUN_TEST(test_plugin_unload_nonexistent);
    RUN_TEST(test_plugin_unload_null_name);
    RUN_TEST(test_plugin_list_empty);
    RUN_TEST(test_plugin_cleanup_all_empty);
    RUN_TEST(test_plugin_load_existing_hello);
    RUN_TEST(test_plugin_load_relative_path);
    RUN_TEST(test_plugin_multiple_operations);

    // Redirection tests
    printf("=== Running Redirection Tests ===\n");
    RUN_TEST(test_redir_create_input);
    RUN_TEST(test_redir_create_output);
    RUN_TEST(test_redir_create_append);
    RUN_TEST(test_redir_create_null_filename);
    RUN_TEST(test_redir_free_null);
    RUN_TEST(test_redir_setup_null);
    RUN_TEST(test_redir_cleanup_null);
    RUN_TEST(test_redir_setup_input_dev_null);
    RUN_TEST(test_redir_setup_output_dev_null);
    RUN_TEST(test_redir_setup_nonexistent_input);
    RUN_TEST(test_redir_invalid_fd);
    RUN_TEST(test_redir_large_fd);
    RUN_TEST(test_redir_all_types);

    // Terminal tests
    printf("=== Running Terminal Tests ===\n");
    RUN_TEST(test_term_get_size);
    RUN_TEST(test_term_get_size_null_pointers);
    RUN_TEST(test_term_raw_mode);
    RUN_TEST(test_term_cooked_mode);
    RUN_TEST(test_term_mode_transitions);
    RUN_TEST(test_term_clear_screen);
    RUN_TEST(test_term_move_cursor);
    RUN_TEST(test_term_move_cursor_negative);
    RUN_TEST(test_term_move_cursor_large);
    RUN_TEST(test_term_setup_signals);
    RUN_TEST(test_term_restore_signals);
    RUN_TEST(test_term_signal_transitions);
    RUN_TEST(test_term_combined_operations);

    // Event loop tests
    printf("=== Running Event Loop Tests ===\n");
    RUN_TEST(test_evloop_create_and_free);
    RUN_TEST(test_evloop_add_invalid_params);
    RUN_TEST(test_evloop_run_timeout_no_events);
    RUN_TEST(test_evloop_callback_trigger_and_stop);
    RUN_TEST(test_evloop_remove_fd_and_rerun);
    RUN_TEST(test_evloop_add_write_event_and_trigger);
    RUN_TEST(test_evloop_remove_fd_not_found);
    RUN_TEST(test_evloop_run_null_loop_returns_error);
    RUN_TEST(test_evloop_two_fds_callbacks_and_stop);

    // Logger tests
    printf("=== Running Logger Tests ===\n");
    RUN_TEST(test_logger_init_shutdown_idempotent);
    RUN_TEST(test_logger_set_get_level);
    RUN_TEST(test_logger_log_calls_do_not_crash);
    RUN_TEST(test_logger_disabled_env);
    RUN_TEST(test_logger_vlog_variadic_path);
    RUN_TEST(test_logger_level_gating);

    // Shell tests
    printf("=== Running Shell Tests ===\n");
    RUN_TEST(test_shell_init);
    RUN_TEST(test_shell_cleanup);
    RUN_TEST(test_shell_init_cleanup_multiple);
    RUN_TEST(test_shell_running_variable);
    RUN_TEST(test_shell_main_eof_returns_zero);
    RUN_TEST(test_shell_main_runs_simple_command);
    RUN_TEST(test_shell_main_exit_stops_loop);
    RUN_TEST(test_shell_main_nonexistent_command_returns_zero);
    RUN_TEST(test_shell_main_multiline_script_last_status);
    RUN_TEST(test_shell_main_handles_empty_lines);
    RUN_TEST(test_shell_main_does_not_start_when_running_false);
    RUN_TEST(test_shell_state_consistency);
    RUN_TEST(test_shell_double_init);
    RUN_TEST(test_shell_double_cleanup);
    RUN_TEST(test_shell_cleanup_without_init);
    RUN_TEST(test_shell_run_file_status_propagates);
    RUN_TEST(test_shell_run_file_shebang_skipped);
    RUN_TEST(test_shell_run_file_errexit_stops_on_error);
    RUN_TEST(test_shell_run_file_errexit_off_allows_continue);
    RUN_TEST(test_shell_run_file_xtrace_does_not_change_status);
    RUN_TEST(test_shell_source_semantics_env_persists);
    RUN_TEST(test_shell_set_builtin_toggles_flags);

    // Pipeline tests
    printf("=== Running Pipeline Tests ===\n");
    RUN_TEST(test_pipeline_execute_null_commands);
    RUN_TEST(test_pipeline_execute_zero_count);
    RUN_TEST(test_pipeline_execute_negative_count);
    RUN_TEST(test_pipeline_execute_single_command);
    RUN_TEST(test_pipeline_execute_single_null_command);
    RUN_TEST(test_pipeline_execute_two_commands);
    RUN_TEST(test_pipeline_execute_mixed_commands);
    RUN_TEST(test_pipeline_execute_large_count);
    RUN_TEST(test_pipeline_execute_echo_cat);
    RUN_TEST(test_pipeline_execute_three_commands);

    return UNITY_END();
}
