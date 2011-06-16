#ifndef Hybird_MODULE_H
#define Hybird_MODULE_H

#include <glib.h>
#include <gmodule.h>

typedef struct _HybirdModule HybirdModule;
typedef struct _HybirdModuleInfo HybirdModuleInfo;

#include "account.h"

struct _HybirdModuleInfo {
	gchar *name;
	gchar *author;
	gchar *summary;
	gchar *description;
	gchar *homepage;
	gchar *major_version;
	gchar *minor_version;

	gboolean (*login)(HybirdAccount *ac);
};

struct _HybirdModule {
	/* full path of the library file */
	gchar *path;
	gboolean loaded;
	/* plugin info */
	HybirdModuleInfo *info;
};

#ifdef __cplusplus
extern "C" {
#endif

#define Hybird_MODULE_INIT(func, moduleinfo) \
	G_MODULE_EXPORT gboolean proto_module_init(HybirdModule *module); \
	G_MODULE_EXPORT gboolean proto_module_init(HybirdModule *module) { \
		module->info = (moduleinfo); \
		func(module); \
		hybird_module_register(module); \
		return TRUE; \
	}


/**
 * Initialize the module function, load all the 
 * protocol modules in MODULE_DIR directory.
 *
 * @return Hybird_OK if success, orelse Hybird_ERROR.
 */
gint hybird_module_init();

/**
 * Create a new protocol plugin.
 *
 * @param path Full path to the module library file *.so,*.dll.
 *
 * @return Hybird Module created, need to be destroyed after use.
 */
HybirdModule *hybird_module_create(const gchar *path);

/**
 * Destroy a module, free the memory allocated.
 * 
 * @param module Module to destroy.
 */
void hybird_module_destroy(HybirdModule *module);

/**
 * Load the module from the module file, run the exported sympol 
 * function hybird_plugin_init() inside the module.
 *
 * @param module Module to load.
 *
 * @return Hybird_OK if success, orelse Hybird_ERROR.
 */
gint hybird_module_load(HybirdModule *module);

/**
 * Register the plugin to the plugin chain.
 *
 * @param module Module to register.
 */
void hybird_module_register(HybirdModule *module);

/**
 * Find a protocol module by name.
 *
 * @param name Name of the module.
 *
 * @return Hybird Module if found, orelse NULL.
 */
HybirdModule *hybird_module_find(const gchar *name);

#ifdef __cplusplus
}
#endif

#endif /* Hybird_MODULE_H */
