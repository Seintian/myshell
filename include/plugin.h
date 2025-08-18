/**
 * @file plugin.h
 * @brief Plugin API for dynamically loaded commands.
 */
#ifndef PLUGIN_H
#define PLUGIN_H
/** \defgroup group_plugin Plugins
 *  @brief Dynamically loaded commands.
 *  @{ */

/** Opaque handle to a loaded plugin. */
typedef struct plugin plugin_t;

/** Plugin metadata and callbacks provided by shared objects. */
typedef struct {
    const char *name;        /**< Plugin name. */
    const char *version;     /**< Semantic version string. */
    const char *description; /**< Short description. */
    int (*init)(void);       /**< Optional init callback (0 on success). */
    int (*execute)(int argc, char **argv); /**< Command entry point. */
    void (*cleanup)(void);   /**< Optional cleanup callback. */
} plugin_info_t;

/**
 * Load a plugin from a shared object path.
 * Convention: the shared object must expose get_plugin_info() returning
 * a valid ::plugin_info_t with non-NULL required fields.
 */
int plugin_load(const char *path);
/** Unload a previously loaded plugin by name; invokes its cleanup hook. */
int plugin_unload(const char *name);
/** Find a loaded plugin by name. */
plugin_t *plugin_find(const char *name);
/** Execute a plugin by name, passing argc/argv. */
int plugin_execute(const char *name, int argc, char **argv);
/** List loaded plugins to stdout (name, version, description). */
void plugin_list(void);
/** Unload all plugins and free resources. */
void plugin_cleanup_all(void);

/** @} */

#endif // PLUGIN_H
