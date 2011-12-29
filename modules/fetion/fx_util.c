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

#include "util.h"
#include "xmlnode.h"

#include "fx_util.h"

gchar*
get_province_name(const gchar *province)
{
    xmlnode *root;
    xmlnode *node;
    gchar   *name;
    gchar   *value;

    g_return_val_if_fail(province != NULL, NULL);

    if (!(root = xmlnode_root_from_file(FETION_RES_DIR"province.xml"))) {
        return NULL;
    }

    if (!(node = xmlnode_child(root)) || g_strcmp0(node->name, "Province")) {
        hybrid_debug_error("fetion",
                "get full province name");
        return NULL;
    }

    for (; node; node = xmlnode_next(node)) {

        if (!xmlnode_has_prop(node, "id")) {
            continue;
        }

        value = xmlnode_prop(node, "id");

        if (g_strcmp0(value, province) == 0) {
            name = xmlnode_content(node);

            /* found, do cleanup. */
            g_free(value);
            xmlnode_free(root);

            return name;
        }

        g_free(value);
    }

    xmlnode_free(root);

    return NULL;
}

gchar*
get_city_name(const gchar *province, const gchar *city)
{
    xmlnode *root;
    xmlnode *node;
    gchar   *name;
    gchar   *value;

    if (!(root = xmlnode_root_from_file(FETION_RES_DIR"city.xml"))) {
        return NULL;
    }

    if (!(node = xmlnode_child(root)) || g_strcmp0(node->name, "Province")) {
        hybrid_debug_error("fetion",
                "get full city name");
        return NULL;
    }

    for (; node; node = xmlnode_next(node)) {

        if (!xmlnode_has_prop(node, "id")) {
            continue;
        }

        value = xmlnode_prop(node, "id");

        if (g_strcmp0(value, province) == 0) {
            /* found, do cleanup. */
            g_free(value);

            goto province_found;
        }

        g_free(value);

    }

    xmlnode_free(root);

    return NULL;

province_found:

    if (!(node = xmlnode_child(node)) || g_strcmp0(node->name, "City")) {
        hybrid_debug_error("fetion",
                "get full city name");
        xmlnode_free(root);

        return NULL;
    }

    for (; node; node = xmlnode_next(node)) {

        if (!xmlnode_has_prop(node, "id")) {
            continue;
        }

        value = xmlnode_prop(node, "id");

        if (g_strcmp0(value, city) == 0) {

            name= xmlnode_content(node);

            /* found, do cleanup. */
            g_free(value);
            xmlnode_free(root);

            return name;
        }

        g_free(value);

    }

    xmlnode_free(root);

    return NULL;
}
