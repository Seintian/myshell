/**
 * @file plugin.c
 * @brief Runtime loading and dispatch of shared-object plugins.
 */
#include "plugin.h"
#include "util.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct plugin {
    char *name;
    void *handle;
    plugin_info_t *info;
    struct plugin *next;
};

/** Head of the singly-linked list of loaded plugins. */
static plugin_t *_plugin_list = NULL;

int plugin_load(const char *path) {
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Cannot load plugin %s: %s\n", path, dlerror());
        return -1;
    }

    plugin_info_t *(*get_plugin_info)(void) = dlsym(handle, "get_plugin_info");
    if (!get_plugin_info) {
        fprintf(stderr, "Plugin %s missing get_plugin_info function\n", path);
        dlclose(handle);
        return -1;
    }

    plugin_info_t *info = get_plugin_info();
    if (!info) {
        fprintf(stderr, "Plugin %s returned NULL info\n", path);
        dlclose(handle);
        return -1;
    }

    // Initialize the plugin
    if (info->init && info->init() != 0) {
        fprintf(stderr, "Plugin %s initialization failed\n", path);
        dlclose(handle);
        return -1;
    }

    // Add to plugin list
    plugin_t *plugin = malloc_safe(sizeof(plugin_t));
    plugin->name = strdup_safe(info->name);
    plugin->handle = handle;
    plugin->info = info;
    plugin->next = _plugin_list;
    _plugin_list = plugin;

    printf("Loaded plugin: %s v%s\n", info->name, info->version);
    return 0;
}

int plugin_unload(const char *name) {
    plugin_t *current = _plugin_list;
    plugin_t *prev = NULL;

    while (current) {
        if (strcmp(current->name, name) == 0) {
            // Call cleanup function
            if (current->info->cleanup) {
                current->info->cleanup();
            }

            // Remove from list
            if (prev) {
                prev->next = current->next;
            } else {
                _plugin_list = current->next;
            }

            // Unload and free
            dlclose(current->handle);
            free(current->name);
            free(current);

            printf("Unloaded plugin: %s\n", name);
            return 0;
        }
        prev = current;
        current = current->next;
    }

    fprintf(stderr, "Plugin %s not found\n", name);
    return -1;
}

plugin_t *plugin_find(const char *name) {
    plugin_t *current = _plugin_list;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int plugin_execute(const char *name, int argc, char **argv) {
    plugin_t *plugin = plugin_find(name);
    if (plugin && plugin->info->execute) {
        return plugin->info->execute(argc, argv);
    }
    return -1; // Plugin not found or no execute function
}

void plugin_list(void) {
    plugin_t *current = _plugin_list;
    while (current) {
        printf("%-15s v%-8s %s\n", current->info->name, current->info->version,
               current->info->description);
        current = current->next;
    }
}

void plugin_cleanup_all(void) {
    while (_plugin_list) {
        plugin_unload(_plugin_list->name);
    }
}
