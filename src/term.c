/**
 * @file term.c
 * @brief Terminal helpers: raw/cooked modes, cursor ops, and signals.
 */
#include "term.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/** Saved terminal attributes to restore cooked mode. */
static struct termios original_termios;
/** Whether original_termios has been captured. */
static int termios_saved = 0;

/** SIGINT handler printing a newline to keep prompt tidy. */
static void sigint_handler(int sig __attribute__((unused))) {
    // Handle Ctrl+C
    printf("\n");
}

/** SIGTSTP handler printing a note about stop. */
static void sigtstp_handler(int sig __attribute__((unused))) {
    // Handle Ctrl+Z
    printf("\n[Stopped]\n");
}

void term_setup_signals(void) {
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
}

void term_restore_signals(void) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
}

int term_raw_mode(void) {
    if (!termios_saved) {
        if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
            perror("tcgetattr");
            return -1;
        }
        termios_saved = 1;
    }

    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= CS8;
    raw.c_oflag &= ~OPOST;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

int term_cooked_mode(void) {
    if (!termios_saved) {
        return -1;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) {
        perror("tcsetattr");
        return -1;
    }

    return 0;
}

int term_get_size(int *rows, int *cols) {
    if (!rows || !cols) {
        return -1;
    }

    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return -1;
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}

void term_clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void term_move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}
