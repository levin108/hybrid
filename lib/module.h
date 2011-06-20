#ifndef HYBRID_MODULE_H
#define HYBRID_MODULE_H

#include <glib.h>
#include <gmodule.h>

typedef struct _HybridModule HybridModule;
typedef struct _HybridModuleInfo HybridModuleInfo;

#include "account.h"
#include "blist.h"

struct _HybridModuleInfo {
	gchar *name;
	gchar *author;
	gchar *summary;
	gchar *description;
	gchar *homepage;
	gchar *major_version;
	gchar *minor_version;

	gboolean (*login)(HybridAccount *ac);
	void (*get_info)(HybridAccount *ac, HybridBuddy *buddy);
};

struct _HybridModule {
	/* full path of the library file */
	gchar *path;
	gboolean loaded;
	/* plugin info */
	HybridModuleInfo *info;
};

#ifdef __cplusplus
extern "C" {
#endif

#define HYBRID_MODULE_INIT(func, moduleinfo) \
	G_MODULE_EXPORT gboolean proto_module_init(HybridModule *module); \
	G_MODULE_EXPORT gboolean proto_module_init(HybridModule *module) { \
		module->info = (moduleinfo); \
		func(module); \
		hybrid_module_register(module); \
		return TRUE; \
	}


/**
 * Initialize the module function, load all the 
 * protocol modules in MODULE_DIR directory.
 *
 * @return HYBRID_OK if success, orelse HYBRID_ERROR.
 */
gint hybrid_module_init();

/**
 * Create a new protocol plugin.
 *
 * @param path Full path to the module library file *.so,*.dll.
 *
 * @return Hybrid Module created, need to be destroyed after use.
 */
HybridModule *hybrid_module_create(const gchar *path);

/**
 * Destroy a module, free the memory allocated.
 * 
 * @param module Module to destroy.
 */
void hybrid_module_destroy(HybridModule *module);

/**
 * Load the module from the module file, run the exported sympol 
 * function hybrid_plugin_init() inside the module.
 *
 * @param module Module to load.
 *
 * @return HYBRID_OK if success, orelse HYBRID_ERROR.
 */
gint hybrid_module_load(HybridModule *module);

/**
 * Register the plugin to the plugin chain.
 *
 * @param module Module to register.
 */
void hybrid_module_register(HybridModule *module);

/**
 * Find a protocol module by name.
 *
 * @param name Name of the module.
 *
 * @return Hybrid Module if found, orelse NULL.
 */
HybridModule *hybrid_module_find(const gchar *name);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_MODULE_H */
