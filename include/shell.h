#ifndef SHELL_H
#define SHELL_H

// Public entry points for the shell

// Global shell state
extern int shell_running;

int shell_main(int argc, char **argv);
void shell_init(void);
void shell_cleanup(void);

#endif // SHELL_H
