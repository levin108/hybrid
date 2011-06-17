#ifndef HYBIRD_CONFIG_H
#define HYBIRD_CONFIG_H
#include <gtk/gtk.h>
#include "xmlnode.h"

typedef struct _HybirdBlistCache HybirdBlistCache;
typedef struct _HybirdConfig HybirdConfig;

struct _HybirdBlistCache {
	xmlnode *root;
	gchar *cache_file_name;
};

struct _HybirdConfig {
	gchar *config_path;
	gchar *icon_path;
	HybirdBlistCache *blist_cache;
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
gchar *hybird_config_get_path(void);

/**
 * Create a config context.
 *
 * @return The config structure created.
 */
HybirdConfig *hybird_config_create();

/**
 * Initialize the config context. Things to do:
 * 1. Initialize the buddy list cache.
 * 2. ...
 *
 * @return HYBIRD_OK if success, HYBIRD_ERROR if there was an error.
 */
gint hybird_config_init(void);

/**
 * Destroy the config context.
 *
 * @param config The config context to destroy.
 */
void hybird_config_destroy(HybirdConfig *config);

/**
 * Flush the blist cache, synchronize the cache in memory
 * with the local xml file.
 * 
 * @param blist_cache The cache to flush.
 */
void hybird_blist_cache_flush();

#ifdef __cplusplus
}
#endif

#endif /* HYBIRD_CONFIG_H */
