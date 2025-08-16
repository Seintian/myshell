#ifndef TERM_H
#define TERM_H

// Terminal control

// Terminal control functions
int term_raw_mode(void);
int term_cooked_mode(void);
int term_get_size(int *rows, int *cols);
void term_clear_screen(void);
void term_move_cursor(int row, int col);

// Signal handling
void term_setup_signals(void);
void term_restore_signals(void);

#endif // TERM_H
