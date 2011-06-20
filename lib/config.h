#ifndef HYBRID_CONFIG_H
#define HYBRID_CONFIG_H
#include <gtk/gtk.h>
#include "xmlnode.h"

typedef struct _HybridBlistCache HybridBlistCache;
typedef struct _HybridConfig HybridConfig;

struct _HybridBlistCache {
	xmlnode *root;
	gchar *cache_file_name;
};

struct _HybridConfig {
	gchar *config_path;
	gchar *icon_path;
	HybridBlistCache *blist_cache;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the absolute path of the config directory,
 * such as /home/levin/.config/hyhird
 *
 * @return The path name, needs to be freed when no longer used.
 */
gchar *hybrid_config_get_path(void);

/**
 * Create a config context.
 *
 * @return The config structure created.
 */
HybridConfig *hybrid_config_create();

/**
 * Initialize the config context. Things to do:
 * 1. Initialize the buddy list cache.
 * 2. ...
 *
 * @return HYBRID_OK if success, HYBRID_ERROR if there was an error.
 */
gint hybrid_config_init(void);

/**
 * Destroy the config context.
 *
 * @param config The config context to destroy.
 */
void hybrid_config_destroy(HybridConfig *config);

/**
 * Flush the blist cache, synchronize the cache in memory
 * with the local xml file.
 * 
 * @param blist_cache The cache to flush.
 */
void hybrid_blist_cache_flush();

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_CONFIG_H */
