/**
 * @file term.h
 * @brief Terminal control helpers (modes, size, drawing, signals).
 */
#ifndef TERM_H
#define TERM_H
/** \defgroup group_term Terminal
 *  @brief Terminal control and signal handling.
 *  @{ */

// Terminal control
/** Put the terminal into a raw mode appropriate for key-by-key input. */
int term_raw_mode(void);
/** Restore the terminal to the original saved (cooked) mode. */
int term_cooked_mode(void);
/** Query terminal size in rows/cols; returns 0 on success. */
int term_get_size(int *rows, int *cols);
/** Clear the screen and move the cursor to home (1,1). */
void term_clear_screen(void);
/** Position the cursor at 1-based row/col; flushes stdout. */
void term_move_cursor(int row, int col);

// Signal handling
/** Install simple SIGINT/SIGTSTP handlers (prints tidy newlines). */
void term_setup_signals(void);
/** Restore default signal handlers. */
void term_restore_signals(void);

/** @} */

#endif // TERM_H
