#include <stdio.h>
#include <stdlib.h>
#include "../../include/plugin.h"

static int hello_init(void) {
    printf("Hello plugin initialized\n");
    return 0;
}

static int hello_execute(int argc, char **argv) {
    printf("Hello, World!");
    
    if (argc > 1) {
        printf(" Arguments:");
        for (int i = 1; i < argc; i++) {
            printf(" %s", argv[i]);
        }
    }
    
    printf("\n");
    return 0;
}

static void hello_cleanup(void) {
    printf("Hello plugin cleaned up\n");
}

static plugin_info_t plugin_info = {
    .name = "hello",
    .version = "1.0.0",
    .description = "Simple hello world plugin",
    .init = hello_init,
    .execute = hello_execute,
    .cleanup = hello_cleanup
};

plugin_info_t *get_plugin_info(void) {
    return &plugin_info;
}
