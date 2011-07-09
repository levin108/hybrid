#ifndef HYBRID_MODULE_H
#define HYBRID_MODULE_H

#include <glib.h>
#include <gmodule.h>

typedef struct _HybridModule HybridModule;
typedef struct _HybridModuleInfo HybridModuleInfo;

#include "account.h"
#include "tooltip.h"
#include "blist.h"

struct _HybridModuleInfo {
	gchar *name;
	gchar *author;
	gchar *summary;
	gchar *description;
	gchar *homepage;
	gchar *major_version;
	gchar *minor_version;
	gchar *icon; /**< The name of the protocol icon, ie,
				   "msn" for "msn.png", the protocol icon must
				   be in "png" format, and stored in 
				   HYBRID_INSTALL_DIR/share/hybrid/protocols
				   */

	gboolean (*login)(HybridAccount *);
	void     (*get_info)(HybridAccount *, HybridBuddy *);
	gboolean (*change_state)(HybridAccount *, gint);
	gboolean (*keep_alive)(HybridAccount *);
	gboolean (*account_tooltip)(HybridAccount *, HybridTooltipData *data);
	gboolean (*buddy_tooltip)(HybridAccount *, HybridBuddy *buddy, HybridTooltipData *data);
	gboolean (*buddy_move)(HybridAccount *, HybridBuddy *,HybridGroup *);
	gboolean (*buddy_remove)(HybridAccount *, HybridBuddy *);
	gboolean (*buddy_rename)(HybridAccount *, HybridBuddy *, const gchar *);
	gboolean (*buddy_add)(HybridAccount *, HybridGroup *, const gchar *,
						const gchar*, const gchar *);
	gboolean (*group_rename)(HybridAccount *, HybridGroup *, const gchar *);
	gboolean (*group_remove)(HybridAccount *, HybridGroup *);

	/*
	 * Group-add hook function, note that it won't add the new group to the
	 * buddy list automaticly, you should add it manually in the hook function.
	 */
	void     (*group_add)(HybridAccount *, const gchar *);
	gboolean (*chat_start)(HybridAccount *, HybridBuddy *);
	void     (*chat_send)(HybridAccount *, HybridBuddy *, const gchar *);
	void     (*close)(HybridAccount *);

	/* To tell the plugin which menus this account have. */
	GSList*  (*actions)(HybridAccount *account);
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
