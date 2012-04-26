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

#include "logs.h"
#include "config.h"

gint
hybrid_logs_init(void)
{
    const gchar *config_path;
    gchar *log_path;
    gint e;

    config_path = hybrid_config_get_path();

    log_path = g_strdup_printf("%s/logs/", config_path);

    e = mkdir(log_path, S_IRWXU|S_IRWXO|S_IRWXG);

    if (e && access(log_path, R_OK|W_OK)) {
        hybrid_debug_error("logs", "%s,cannot create, read or write",
                            log_path);
        g_free(log_path);

        return HYBRID_ERROR;
    }

    g_free(log_path);

    return HYBRID_OK;
}

gchar*
hybrid_logs_get_path(HybridAccount *account, const gchar *id)
{
    gchar *path;
	const gchar *config_path;

    config_path = hybrid_config_get_path();
    path = g_strdup_printf("%s/logs/%s/%s/%s",
            config_path, account->proto->info->name,
            account->username, id);

    return path;
}

HybridLogs*
hybrid_logs_create(HybridAccount *account, const gchar *id)
{
    HybridLogs *log;
    const gchar *config_path;
    gchar *log_path;
    gchar *account_path;
    gchar *final_path;
    gchar *proto_path;
    struct tm *local_time;
    time_t now;
    gint e;

    g_return_val_if_fail(account != NULL, NULL);
    g_return_val_if_fail(id != NULL, NULL);

    now = time(NULL);

    log = g_new0(HybridLogs, 1);

    log->id = g_strdup(id);
    log->time = now;

    config_path = hybrid_config_get_path();
    log_path = g_strdup_printf("%s/logs", config_path);

    proto_path = g_strdup_printf("%s/%s", log_path,
                    account->proto->info->name);
    g_free(log_path);

    e = mkdir(proto_path, S_IRWXU|S_IRWXO|S_IRWXG);

    if (e && access(proto_path, R_OK|W_OK)) {
        hybrid_debug_error("logs", "%s,cannot create, read or write",
                            proto_path);
        g_free(proto_path);

        return NULL;
    }

    /* create log directory for a specified account. */
    account_path = g_strdup_printf("%s/%s", proto_path, account->username);
    g_free(proto_path);

    e = mkdir(account_path, S_IRWXU|S_IRWXO|S_IRWXG);

    if (e && access(account_path, R_OK|W_OK)) {
        hybrid_debug_error("logs", "%s,cannot create, read or write",
                            account_path);
        g_free(account_path);

        return NULL;
    }


    /* create log directory for a specified account. */
    final_path = g_strdup_printf("%s/%s", account_path, id);
    g_free(account_path);

    e = mkdir(final_path, S_IRWXU|S_IRWXO|S_IRWXG);

    if (e && access(final_path, R_OK|W_OK)) {

        hybrid_debug_error("logs", "%s,cannot create, read or write",
                            final_path);
        g_free(final_path);

        return NULL;
    }

    /* create/get the log file for a specified chat id, with the name
     * in form of id_date.xml */
    local_time = localtime(&now);

    log->log_path = g_strdup_printf("%s/%d_%d_%d.xml",
                    final_path, local_time->tm_year + 1900,
                    local_time->tm_mon, local_time->tm_mday);

    g_free(final_path);

    log->root = xmlnode_root_from_file(log->log_path);

    return log;
}

gint
hybrid_logs_write(HybridLogs *log, const gchar *name, const gchar *msg,
                    gboolean sendout)
{
    xmlnode *time_node;
    xmlnode *name_node;
    xmlnode *cont_node;
    xmlnode *node;
    time_t now;
    const gchar *body;
    struct tm *local_time;
    gchar time_str[128];

    g_return_val_if_fail(log != NULL, HYBRID_ERROR);
    g_return_val_if_fail(name != NULL, HYBRID_ERROR);

    now = time(NULL);

    local_time = localtime(&now);

    /* log file doesn't exist, we create one. */
    if (!log->root) {
        body = "<root></root>";
        log->root = xmlnode_root(body, strlen(body));
    }

    node = xmlnode_new_child(log->root, "m");
    if (sendout) {
        xmlnode_new_prop(node, "type", "o");

    } else {
        xmlnode_new_prop(node, "type", "i");
    }

    time_node = xmlnode_new_child(node, "t");

    memset(time_str, 0, sizeof(time_str));
    strftime(time_str, sizeof(time_str) - 1, "%H:%M:%S", local_time);

    xmlnode_set_content(time_node, time_str);

    name_node = xmlnode_new_child(node, "n");
    xmlnode_set_content(name_node, name);

    cont_node = xmlnode_new_child(node, "c");
    xmlnode_set_content(cont_node, msg);

    xmlnode_save_file(log->root, log->log_path);

    return HYBRID_OK;
}

GSList*
hybrid_logs_read(HybridAccount *account, const gchar *id, const gchar *logname)
{
    gchar *log_path = NULL;
    gchar *log_name = NULL;
    gchar *tmp;
    GSList *list = NULL;
    xmlnode *root = NULL, *node, *child;
    HybridLogEntry *entry;

    log_path = hybrid_logs_get_path(account, id);
    log_name = g_strdup_printf("%s/%s", log_path, logname);

    root = xmlnode_root_from_file(log_name);
    if (!root)
        goto out;

    node = xmlnode_child(root);
    while (node) {
        if (!xmlnode_has_prop(node, "type"))
            goto next;

        entry = g_new0(HybridLogEntry, 1);
        tmp = xmlnode_prop(node, "type");
        if (g_strcmp0(tmp, "o")) {
            entry->is_send = 1;
        } else {
            entry->is_send = 0;
        }
        g_free(tmp);

        child = xmlnode_find(node, "t");
        entry->time = xmlnode_content(child);
        child = xmlnode_find(node, "n");
        entry->name = xmlnode_content(child);
        child = xmlnode_find(node, "c");
        entry->content = xmlnode_content(child);
        list = g_slist_append(list, entry);
next:
        node = xmlnode_next(node);
    }

out:
    free(log_path);
    free(log_name);
    xmlnode_free(root);
    return list;
}

void
hybrid_logs_destroy(HybridLogs *log)
{
    if (log) {
        g_free(log->log_path);
        g_free(log->id);
        xmlnode_free(log->root);
        g_free(log);
    }
}
