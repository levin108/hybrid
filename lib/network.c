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

#include <glib.h>
#include "util.h"
#include "network.h"

static GHashTable *host_hash = NULL;

gint
resolve_host(const gchar *hostname, gchar *ip)
{
    g_return_val_if_fail(hostname != NULL, HYBRID_ERROR);

    struct addrinfo    *result;
    struct addrinfo    *rp;
    struct sockaddr_in *addr;
    gchar              *hash_value;
    gchar               buf[32];

    hybrid_debug_info("dns", "resolve host \'%s\'", hostname);

    if (host_hash && (hash_value = g_hash_table_lookup(host_hash, hostname))) {
        strcpy(ip, (gchar*)hash_value);
        hybrid_debug_info("dns", "ip of \'%s\' is \'%s\'[cached]", hostname, ip);
        return HYBRID_OK;
    }

    if (getaddrinfo(hostname, NULL, NULL, &result) != 0) {
        hybrid_debug_error("resolve_host", "resolve host \'%s\' failed",
                hostname);
        return HYBRID_ERROR;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        addr = (struct sockaddr_in*)rp->ai_addr;

        memset(buf, 0, sizeof(buf));

        if (!inet_ntop(AF_INET, (void*)&addr->sin_addr, buf, 16)) {

            hybrid_debug_error("dns", "reslove host \'%s\' failed when"
                               " transforming binary ip address to doted ip address",
                               hostname);

            continue;
        }

        if (g_strcmp0(buf, "0.0.0.0") == 0    ||
            g_strcmp0(buf, "127.0.0.1")          == 0) {
            continue;
        }

        strncpy(ip, buf, strlen(buf));
        goto addr_success;
    }

    return HYBRID_ERROR;

 addr_success:

    if (!host_hash) {
        host_hash = g_hash_table_new(g_str_hash, g_str_equal);
    }

    hash_value = g_strdup(ip);

    g_hash_table_insert(host_hash, (gchar*)hostname, hash_value);

    hybrid_debug_info("dns", "ip of \'%s\' is \'%s\'[new]", hostname, ip);

    return HYBRID_OK;
}
