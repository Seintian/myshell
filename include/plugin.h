#ifndef PLUGIN_H
#define PLUGIN_H

// Dynamic command ABI

typedef struct plugin plugin_t;

typedef struct {
    const char *name;
    const char *version;
    const char *description;
    int (*init)(void);
    int (*execute)(int argc, char **argv);
    void (*cleanup)(void);
} plugin_info_t;

// Plugin functions
int plugin_load(const char *path);
int plugin_unload(const char *name);
plugin_t *plugin_find(const char *name);
int plugin_execute(const char *name, int argc, char **argv);
void plugin_list(void);
void plugin_cleanup_all(void);

#endif // PLUGIN_H
