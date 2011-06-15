#ifndef IM_MODULE_H
#define IM_MODULE_H

#include <glib.h>
#include <gmodule.h>

typedef struct _IMModule IMModule;
typedef struct _IMModuleInfo IMModuleInfo;

#include "account.h"

struct _IMModuleInfo {
	gchar *name;
	gchar *author;
	gchar *summary;
	gchar *description;
	gchar *homepage;
	gchar *major_version;
	gchar *minor_version;

	gboolean (*login)(IMAccount *ac);
};

struct _IMModule {
	/* full path of the library file */
	gchar *path;
	gboolean loaded;
	/* plugin info */
	IMModuleInfo *info;
};

#ifdef __cplusplus
extern "C" {
#endif

#define IM_MODULE_INIT(func, moduleinfo) \
	G_MODULE_EXPORT gboolean proto_module_init(IMModule *module); \
	G_MODULE_EXPORT gboolean proto_module_init(IMModule *module) { \
		module->info = (moduleinfo); \
		func(module); \
		im_module_register(module); \
		return TRUE; \
	}


/**
 * Initialize the module function, load all the 
 * protocol modules in MODULE_DIR directory.
 *
 * @return IM_OK if success, orelse IM_ERROR.
 */
gint im_module_init();

/**
 * Create a new protocol plugin.
 *
 * @param path Full path to the module library file *.so,*.dll.
 *
 * @return IM Module created, need to be destroyed after use.
 */
IMModule *im_module_create(const gchar *path);

/**
 * Destroy a module, free the memory allocated.
 * 
 * @param module Module to destroy.
 */
void im_module_destroy(IMModule *module);

/**
 * Load the module from the module file, run the exported sympol 
 * function im_plugin_init() inside the module.
 *
 * @param module Module to load.
 *
 * @return IM_OK if success, orelse IM_ERROR.
 */
gint im_module_load(IMModule *module);

/**
 * Register the plugin to the plugin chain.
 *
 * @param module Module to register.
 */
void im_module_register(IMModule *module);

/**
 * Find a protocol module by name.
 *
 * @param name Name of the module.
 *
 * @return IM Module if found, orelse NULL.
 */
IMModule *im_module_find(const gchar *name);

#ifdef __cplusplus
}
#endif

#endif /* IM_MODULE_H */
