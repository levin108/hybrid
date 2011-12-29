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
 * Get the absolute path of the certificate directory,
 * such as /home/levin/.config/hybrid/certificates, if not
 * exists, create it.
 */
gchar *hybrid_config_get_cert_path(void);

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
