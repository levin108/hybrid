/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "util.h"
#include "config.h"

static gint hybrid_blist_cache_init(HybridConfig *config);

HybridConfig *global_config;

/* The return value is owned by the function, don't free or modify. */
/* Add lock if it might be access concurrently. */

gchar*
hybrid_config_get_path(void)
{
    gchar        *home;
    static gchar *config_path = NULL;
    static gchar *hybrid_path = NULL;
    gint         e;

    if (hybrid_path)
        goto check_hybrid;
    if (config_path)
        goto check_conf;
    if (!(config_path = getenv("XDG_CONFIG_HOME"))) {
        if (!(home = getenv("HOME"))) {
            hybrid_debug_error("config", "No environment variable "
                               "named HOME\n");
            return NULL;
        }
        config_path = g_strdup_printf("%s/.config", home);
    } else {
        config_path = g_strdup_printf("%s", config_path);
    }

check_conf:
    e = mkdir(config_path, S_IRWXU|S_IRWXO|S_IRWXG);
    if (e && access(config_path, R_OK|W_OK)) {
        hybrid_debug_error("config", "cannot create, read from or write to %s",
                           config_path);
        g_free(config_path);
        config_path = NULL;
        return NULL;
    }

    hybrid_path = g_strdup_printf("%s/hybrid", config_path);

check_hybrid:
    e = mkdir(hybrid_path, S_IRWXU|S_IRWXO|S_IRWXG);
    if (e && access(hybrid_path, R_OK|W_OK)) {
        hybrid_debug_error("config", "cannot create, read from or write to %s",
                           hybrid_path);
        g_free(hybrid_path);
        hybrid_path = NULL;
        return NULL;
    }

    return hybrid_path;
}

gchar*
hybrid_config_get_cert_path(void)
{
    gchar *config_path;
    gchar *cert_path;
    gint   e;

    config_path = hybrid_config_get_path();
    cert_path   = g_strdup_printf("%s/certificates", config_path);

    e = mkdir(cert_path, S_IRWXU|S_IRWXO|S_IRWXG);

    if (e && access(cert_path, R_OK|W_OK)) {
        hybrid_debug_error("config", "%s,cannot create, read or write",
                           cert_path);
        g_free(cert_path);
        return NULL;
    }

    return cert_path;
}


HybridConfig*
hybrid_config_create()
{
    HybridConfig *config;

    config = g_new0(HybridConfig, 1);

    config->config_path = g_strdup(hybrid_config_get_path());

    return config;
}

void
hybrid_config_destroy(HybridConfig *config)
{
    if (config) {
        g_free(config->config_path);
        g_free(config);
    }
}

gint
hybrid_config_init(void)
{
    global_config = hybrid_config_create();

    if (hybrid_blist_cache_init(global_config) != HYBRID_OK ) {
        return HYBRID_ERROR;
    }

    return HYBRID_OK;
}

static gint
hybrid_blist_cache_init(HybridConfig *config)
{
    gchar            *cache_file_name;
    HybridBlistCache *cache;
    xmlnode          *root;
    gint              err;

    g_return_val_if_fail(config != NULL, HYBRID_ERROR);

    cache_file_name = g_strdup_printf("%s/blist.xml", config->config_path);

    hybrid_debug_info("config", "init the blist cache from %s",
            cache_file_name);

    cache                  = g_new0(HybridBlistCache, 1);
    cache->cache_file_name = cache_file_name;
    config->blist_cache    = cache;

    if (!(root = xmlnode_root_from_file(cache_file_name))) {
        const gchar *root_string = "<blist></blist>";
        root = xmlnode_root(root_string, strlen(root_string));
        cache->root = root;

        goto blist_cache_init_null;
    }

    if (!root) {
        hybrid_debug_error("config", "FATAL, init blist cache");
        return HYBRID_ERROR;
    }

    cache->root = root;

    /* Load the cached buddy list */
    goto blist_cache_init_fin;

blist_cache_init_null:
    /* initialize the xml context since we don't have local cache */
    xmlnode_new_child(root, "accounts");

    xmlnode_save_file(root, cache->cache_file_name);

blist_cache_init_fin:
    /* initialize the icon path. */
    config->icon_path = g_strdup_printf("%s/icons", config->config_path);

    err = mkdir(config->icon_path, S_IRWXU|S_IRWXO|S_IRWXG);

    if (err && access(config->icon_path, R_OK|W_OK)) {
        hybrid_debug_error("config", "%s,cannot create, read or write",
                config->icon_path);
        g_free(config->icon_path);

        return HYBRID_ERROR;
    }

    return HYBRID_OK;
}

void
hybrid_blist_cache_flush()
{
    HybridBlistCache *cache;

    cache = global_config->blist_cache;

    xmlnode_save_file(cache->root, cache->cache_file_name);
}
