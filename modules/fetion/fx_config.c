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

#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"

#include "fx_config.h"

gchar*
fetion_get_config_dir(fetion_account *account)
{
    gchar *hybrid_dir;
    gchar *fetion_dir;
    gchar *account_dir;
    gint   e;

    g_return_val_if_fail(account != NULL, NULL);

    if (!(hybrid_dir = hybrid_config_get_path())) {
        return NULL;
    }

    fetion_dir = g_strdup_printf("%s/%s",
            hybrid_dir, account->account->proto->info->name);

    e = mkdir(fetion_dir, S_IRWXU|S_IRWXO|S_IRWXG);

    if (e && access(fetion_dir, R_OK|W_OK)) {
        hybrid_debug_error("fetion", "%s,cannot create, read or write",
                fetion_dir);
        g_free(fetion_dir);

        return NULL;
    }

    account_dir = g_strdup_printf("%s/%s", fetion_dir, account->sid);
    g_free(fetion_dir);

    e = mkdir(account_dir, S_IRWXU|S_IRWXO|S_IRWXG);

    if (e && access(account_dir, R_OK|W_OK)) {
        hybrid_debug_error("fetion", "%s,cannot create, read or write",
                account_dir);
        g_free(account_dir);

        return NULL;
    }

    return account_dir;
}

void
fetion_config_save_account(fetion_account *account)
{
    const gchar *body;
    gchar       *account_dir;
    gchar       *account_name;
    gchar       *port;
    xmlnode     *root;
    xmlnode     *node;
    xmlnode     *new;

    g_return_if_fail(account != NULL);

    if (!(account_dir = fetion_get_config_dir(account))) {
        hybrid_debug_error("fetion", "get config dir error.");
        return;
    }

    account_name = g_strdup_printf("%s/account.xml", account_dir);

    body = "<root></root>";

    root = xmlnode_root(body, strlen(body));

    node = xmlnode_new_child(root, "account");

    xmlnode_new_prop(node, "personal-version", account->personal_version);
    xmlnode_new_prop(node, "contact-version", account->contact_list_version);
    xmlnode_new_prop(node, "custom-version", account->custom_config_version);

    node = xmlnode_new_child(root, "custom-config");
    xmlnode_set_content(node, account->custom_config);

    node = xmlnode_new_child(root, "config");
    xmlnode_new_prop(node, "servers-version", account->cfg_server_version);
    xmlnode_new_prop(node, "parameters-version", account->cfg_param_version);
    xmlnode_new_prop(node, "hints-version", account->cfg_hint_version);
    xmlnode_new_prop(node, "client-version", account->cfg_client_version);

    port = g_strdup_printf("%d", account->sipc_proxy_port);
    new = xmlnode_new_child(node, "sipc-server");
    xmlnode_new_prop(new, "host", account->sipc_proxy_ip);
    xmlnode_new_prop(new, "port", port);
    g_free(port);

    new = xmlnode_new_child(node, "portrait-server");
    xmlnode_new_prop(new, "name", account->portrait_host_name);
    xmlnode_new_prop(new, "path", account->portrait_host_path);

    xmlnode_save_file(root, account_name);
    xmlnode_free(root);
    g_free(account_name);
}

gint
fetion_config_load_account(fetion_account *account)
{
    gchar   *account_dir;
    gchar   *account_name;
    gchar   *value;
    xmlnode *root;
    xmlnode *node;

    g_return_val_if_fail(account != NULL, HYBRID_ERROR);

    if (!(account_dir = fetion_get_config_dir(account))) {
        hybrid_debug_error("fetion", "get config dir error.");
        return HYBRID_ERROR;
    }

    account_name = g_strdup_printf("%s/account.xml", account_dir);

    if (!(root = xmlnode_root_from_file(account_name))) {
        g_free(account_name);
        return HYBRID_ERROR;
    }

    g_free(account_name);

    if (!(node = xmlnode_find(root, "account"))) {
        goto load_account_err;
    }

    g_free(account->personal_version);

    if (xmlnode_has_prop(node, "personal-version")) {
        account->personal_version = xmlnode_prop(node, "personal-version");

    }

    g_free(account->contact_list_version);
    if (xmlnode_has_prop(node, "contact-version")) {
        account->contact_list_version = xmlnode_prop(node, "contact-version");
    }

    g_free(account->custom_config_version);
    if (xmlnode_has_prop(node, "custom-version")) {
        account->custom_config_version = xmlnode_prop(node, "custom-version");
    }

    if ((node = xmlnode_find(root, "custom-config"))) {
        g_free(account->custom_config);
        account->custom_config = xmlnode_content(node);
    }

    /* load config versions */
    if (!(node = xmlnode_find(root, "config"))) {
        goto load_account_err;
    }

    g_free(account->cfg_server_version);
    if (xmlnode_has_prop(node, "servers-version")) {
        account->cfg_server_version = xmlnode_prop(node, "servers-version");
    }

    g_free(account->cfg_param_version);
    if (xmlnode_has_prop(node, "parameters-version")) {
        account->cfg_param_version = xmlnode_prop(node, "parameters-version");
    }

    g_free(account->cfg_hint_version);
    if (xmlnode_has_prop(node, "hints-version")) {
        account->cfg_hint_version = xmlnode_prop(node, "hints-version");
    }

    g_free(account->cfg_client_version);
    if (xmlnode_has_prop(node, "client-version")) {
        account->cfg_client_version = xmlnode_prop(node, "client-version");
    }

    /* load configs */
    if ((node = xmlnode_find(root, "sipc-server"))) {
        if (xmlnode_has_prop(node, "host")) {
            g_free(account->sipc_proxy_ip);
            account->sipc_proxy_ip = xmlnode_prop(node, "host");
        }

        if (xmlnode_has_prop(node, "port")) {
            value = xmlnode_prop(node, "port");
            account->sipc_proxy_port = atoi(value);
            g_free(value);
        }
    }

    if ((node = xmlnode_find(root, "portrait-server"))) {
        if (xmlnode_has_prop(node, "name")) {
            g_free(account->portrait_host_name);
            account->portrait_host_name = xmlnode_prop(node, "name");
        }

        if (xmlnode_has_prop(node, "path")) {
            g_free(account->portrait_host_path);
            account->portrait_host_path = xmlnode_prop(node, "path");
        }
    }

    xmlnode_free(root);

    return HYBRID_OK;

load_account_err:
    hybrid_debug_error("fetion", "invalid fetion account cache file.");
    xmlnode_free(root);

    return HYBRID_ERROR;
}

void
fetion_config_save_personal(fetion_account *account, xmlnode *node)
{
    gchar       *fetion_dir;
    gchar       *buddies_dir;
    const gchar *body;
    xmlnode     *root;

    g_return_if_fail(account != NULL);
    g_return_if_fail(node != NULL);

    fetion_dir  = fetion_get_config_dir(account);
    buddies_dir = g_strdup_printf("%s/personal.xml", fetion_dir);
    g_free(fetion_dir);

    body = "<root></root>";
    root = xmlnode_root(body, strlen(body));

    xmlnode_add_child(root, node);

    xmlnode_save_file(root, buddies_dir);
    xmlnode_free(root);
    g_free(buddies_dir);
}

xmlnode*
fetion_config_load_personal(fetion_account *account)
{
    gchar   *fetion_dir;
    gchar   *personal_dir;
    xmlnode *root;

    g_return_val_if_fail(account != NULL, NULL);

    fetion_dir = fetion_get_config_dir(account);
    personal_dir = g_strdup_printf("%s/personal.xml", fetion_dir);
    g_free(fetion_dir);

    root = xmlnode_root_from_file(personal_dir);
    g_free(personal_dir);

    return root;
}

void
fetion_config_save_buddies(fetion_account * account, xmlnode *node)
{
    gchar       *fetion_dir;
    gchar       *buddies_dir;
    const gchar *body;
    xmlnode     *root;

    g_return_if_fail(account != NULL);
    g_return_if_fail(node != NULL);

    fetion_dir = fetion_get_config_dir(account);
    buddies_dir = g_strdup_printf("%s/buddies.xml", fetion_dir);
    g_free(fetion_dir);

    body = "<root></root>";
    root = xmlnode_root(body, strlen(body));

    xmlnode_add_child(root, node);

    xmlnode_save_file(root, buddies_dir);
    xmlnode_free(root);
    g_free(buddies_dir);
}

xmlnode*
fetion_config_load_buddies(fetion_account *account)
{
    gchar   *fetion_dir;
    gchar   *buddies_dir;
    xmlnode *root;

    g_return_val_if_fail(account != NULL, NULL);

    fetion_dir = fetion_get_config_dir(account);
    buddies_dir = g_strdup_printf("%s/buddies.xml", fetion_dir);
    g_free(fetion_dir);

    root = xmlnode_root_from_file(buddies_dir);
    g_free(buddies_dir);

    return root;
}
