#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"
#include "config.h"

static gint hybird_blist_cache_init(HybirdConfig *config);

HybirdConfig *global_config;

gchar*
hybird_config_get_path(void)
{
	gchar *home;
	gchar *config_path;
	gchar *hybird_path;
	gint e;

	if (!(home = getenv("HOME"))) {
		hybird_debug_error("config", "No environment variable named HOME\n");
		return NULL;
	}

	config_path = g_strdup_printf("%s/.config", home);

	e = mkdir(config_path, S_IRWXU|S_IRWXO|S_IRWXG);

	if (e && access(config_path, R_OK|W_OK)) {
		hybird_debug_error("config", "%s,cannot create, read or write",
				config_path);
		g_free(config_path);

		return NULL;
	}

	hybird_path = g_strdup_printf("%s/hybird", config_path);

	e = mkdir(hybird_path, S_IRWXU|S_IRWXO|S_IRWXG);

	if (e && access(hybird_path, R_OK|W_OK)) {
		hybird_debug_error("config", "%s,cannot create, read or write",
				hybird_path);
		g_free(config_path);
		g_free(hybird_path);

		return NULL;
	}

	g_free(config_path);

	return hybird_path;
}

HybirdConfig*
hybird_config_create()
{
	HybirdConfig *config;

	config = g_new0(HybirdConfig, 1);

	config->config_path = hybird_config_get_path();

	return config;
}

void
hybird_config_destroy(HybirdConfig *config)
{
	if (config) {
		g_free(config->config_path);
		g_free(config);
	}
}

gint
hybird_config_init(void)
{
	global_config = hybird_config_create();

	if (hybird_blist_cache_init(global_config) != HYBIRD_OK ) {
		return HYBIRD_ERROR;
	}

	return HYBIRD_OK;
}

static gint
hybird_blist_cache_init(HybirdConfig *config)
{
	gchar *cache_file_name;
	HybirdBlistCache *cache;
	xmlnode *root;

	g_return_val_if_fail(config != NULL, HYBIRD_ERROR);

	cache_file_name = g_strdup_printf("%s/blist.xml", config->config_path);


	hybird_debug_info("config", "init the blist cache from %s",
			cache_file_name);

	cache = g_new0(HybirdBlistCache, 1);
	cache->cache_file_name = cache_file_name;

	config->blist_cache = cache;

	if (!(root = xmlnode_root_from_file(cache_file_name))) {
		const gchar *root_string = "<blist></blist>";
		root = xmlnode_root(root_string, strlen(root_string));
		cache->root = root;

		goto blist_cache_init_null;
	}

	if (!root) {
		hybird_debug_error("config", "FATAL, init blist cache");
		return HYBIRD_ERROR;
	}

	cache->root = root;

	/* Load the cached buddy list */
	goto blist_cache_init_fin;

blist_cache_init_null:
	/* initialize the xml context since we don't have local cache */
	xmlnode_new_child(root, "accounts");

	xmlnode_save_file(root, cache->cache_file_name);

blist_cache_init_fin:
	return HYBIRD_OK;
}

void
hybird_blist_cache_flush()
{
	HybirdBlistCache *cache;

	cache = global_config->blist_cache;

	xmlnode_save_file(cache->root, cache->cache_file_name);
}
