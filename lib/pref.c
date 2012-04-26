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

#include "pref.h"
#include "config.h"

static HybridPref *hybrid_pref = NULL;

#define PREF_INIT_BODY "<pref></pref>"

HybridPref*
hybrid_pref_new(const gchar *name)
{
    HybridPref *pref;
    const gchar *config_path;

    if (!(config_path = hybrid_config_get_path())) {
        hybrid_debug_error("pref", "get config path error.");
        return NULL;
    }

    pref = g_new0(HybridPref, 1);

    pref->filename = g_strdup_printf("%s/%s", config_path,
                                     name ? name : "pref.xml");

    if (!(pref->root = xmlnode_root_from_file(pref->filename))) {
        if (!(pref->root = xmlnode_root(PREF_INIT_BODY,
                                        sizeof(PREF_INIT_BODY) - 1))) {
            hybrid_pref_destroy(pref);
            return NULL;
        }
        hybrid_pref_save(pref);
    }

    return pref;
}

void
hybrid_pref_destroy(HybridPref *pref)
{
    if (!pref)
        return;
    if (pref->filename)
        g_free(pref->filename);
    if (pref->root)
        xmlnode_free(pref->root);

    g_free(pref);
}

gint
hybrid_pref_init(void)
{
    hybrid_pref = hybrid_pref_new(NULL);
    return hybrid_pref ? HYBRID_OK : HYBRID_ERROR;
}

void
hybrid_pref_set_boolean(HybridPref *pref, const gchar *name,
                        const gboolean value)
{
    xmlnode *node;

    pref = pref ? pref : hybrid_pref;
    g_return_if_fail(pref->root != NULL);

    if (!(node = xmlnode_find(pref->root, name))) {
        node = xmlnode_new_child(pref->root, name);
    }

    if (xmlnode_has_prop(node, "type")) {
        xmlnode_set_prop(node, "type", "bool");
    } else {
        xmlnode_new_prop(node, "type", "bool");
    }

    if (value) {
        xmlnode_set_content(node, "1");
    } else {
        xmlnode_set_content(node, "0");
    }
}

gboolean
hybrid_pref_get_boolean(HybridPref *pref, const gchar *name)
{
    xmlnode *node;
    gchar   *type;
    gchar   *value;

    pref = pref ? pref : hybrid_pref;
    g_return_val_if_fail(pref->root != NULL, FALSE);

    if (!(node = xmlnode_find(pref->root, name))) {
        return FALSE;
    }

    if (!xmlnode_has_prop(node, "type")) {
        hybrid_debug_info("pref", "invalid pref node.");
        return FALSE;
    }

    type = xmlnode_prop(node, "type");

    if (g_strcmp0(type, "bool") != 0) {
        hybrid_debug_error("pref",
                           "bool pref node with a type which is not bool.");
        return FALSE;
    }

    value = xmlnode_content(node);

    if (g_strcmp0(value, "0") == 0) {
        g_free(value);
        return FALSE;
    } else {
        g_free(value);
        return TRUE;
    }
}

void
hybrid_pref_set_string(HybridPref *pref, const gchar *name, const gchar *value)
{
    xmlnode *node;

    pref = pref ? pref : hybrid_pref;
    g_return_if_fail(pref->root != NULL);

    if (!(node = xmlnode_find(pref->root, name))) {
        node = xmlnode_new_child(pref->root, name);
    }

    if (xmlnode_has_prop(node, "type")) {
        xmlnode_set_prop(node, "type", "string");
    } else {
        xmlnode_new_prop(node, "type", "string");
    }

    xmlnode_set_content(node, value);
}

gchar*
hybrid_pref_get_string(HybridPref *pref, const gchar *name)
{
    xmlnode *node;
    gchar   *type;
    gchar   *value;

    pref = pref ? pref : hybrid_pref;
    g_return_val_if_fail(pref->root != NULL, FALSE);

    if (!(node = xmlnode_find(pref->root, name))) {
        return NULL;
    }

    if (!xmlnode_has_prop(node, "type")) {
        hybrid_debug_info("pref", "invalid pref node.");
        return NULL;
    }

    type = xmlnode_prop(node, "type");

    if (g_strcmp0(type, "string") != 0) {
        hybrid_debug_error("pref",
                           "string pref node with a type which is not string.");
        return NULL;
    }

    value = xmlnode_content(node);

    return value;
}

void
hybrid_pref_set_int(HybridPref *pref, const gchar *name, gint value)
{
    xmlnode *node;
    gchar   *value_string;

    pref = pref ? pref : hybrid_pref;
    g_return_if_fail(pref->root != NULL);

    if (!(node = xmlnode_find(pref->root, name))) {
        node = xmlnode_new_child(pref->root, name);
    }

    if (xmlnode_has_prop(node, "type")) {
        xmlnode_set_prop(node, "type", "int");
    } else {
        xmlnode_new_prop(node, "type", "int");
    }

    value_string = g_strdup_printf("%d", value);

    xmlnode_set_content(node, value_string);

    g_free(value_string);
}

gint
hybrid_pref_get_int(HybridPref *pref, const gchar *name)
{
    xmlnode *node;
    gchar   *type;
    gchar   *value;
    gint     value_int;

    pref = pref ? pref : hybrid_pref;
    g_return_val_if_fail(pref->root != NULL, FALSE);

    if (!(node = xmlnode_find(pref->root, name))) {
        return -1;
    }

    if (!xmlnode_has_prop(node, "type")) {
        hybrid_debug_info("pref", "invalid pref node.");
        return -1;
    }

    type = xmlnode_prop(node, "type");

    if (g_strcmp0(type, "int") != 0) {
        hybrid_debug_error("pref",
                           "integer pref node with a type which is not int.");
        return -1;
    }

    value = xmlnode_content(node);
    value_int = atoi(value);
    g_free(value);

    return value_int;
}

void
hybrid_pref_save(HybridPref *pref)
{
    pref = pref ? pref : hybrid_pref;
    g_return_if_fail(pref->root != NULL);

    xmlnode_save_file(pref->root, pref->filename);
}
