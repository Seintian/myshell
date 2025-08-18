#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    shell_init();
    int result = shell_main(argc, argv);
    shell_cleanup();
    return result;
}
