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
    gchar *config_path;
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

HybridLogs*
hybrid_logs_create(HybridAccount *account, const gchar *id)
{
    HybridLogs *log;
    gchar *config_path;
    gchar *log_path;
    gchar *account_path;
    gchar *final_path;
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

    /* create log directory for a specified account. */
    account_path = g_strdup_printf("%s/%s", log_path, account->username);
    g_free(log_path);

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

    log->log_path = g_strdup_printf("%s/%d_%d_%d.html",
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
    xmlnode *head_node;
    xmlnode *body_node;
    xmlnode *time_node;
    xmlnode *font_node;
    xmlnode *name_node;
    xmlnode *cont_node;
    xmlnode *node;
    time_t now;
    const gchar *body;
    gchar *title;
    gchar *content;
    struct tm *local_time;
    gchar time_str[128];

    g_return_val_if_fail(log != NULL, HYBRID_ERROR);
    g_return_val_if_fail(name != NULL, HYBRID_ERROR);

    now = time(NULL);

    local_time = localtime(&now);


    /* log file doesn't exist, we create one. */
    if (!log->root) {
        body = "<html></html>";

        title = g_strdup_printf(_("Conversation with %s (%s) at %d-%d-%d"),
                name, log->id, local_time->tm_year + 1900,
                local_time->tm_mon, local_time->tm_mday);

        log->root = xmlnode_root(body, strlen(body));

        head_node = xmlnode_new_child(log->root, "head");

        node = xmlnode_new_child(head_node, "meta");
        xmlnode_new_prop(node, "http-equiv", "content-type");
        xmlnode_new_prop(node, "content", "text/html; charset=UTF-8");

        node = xmlnode_new_child(head_node, "title");

        xmlnode_set_content(node, title);

        node = xmlnode_new_child(log->root, "body");

        node = xmlnode_new_child(node, "h3");

        xmlnode_set_content(node, title);

        g_free(title);
    }

    if (!(body_node = xmlnode_find(log->root, "body"))) {

        hybrid_debug_error("logs", "invalid log file.");

        g_free(title);

        return HYBRID_ERROR;
    }

    node = xmlnode_new_child(body_node, "span");

    font_node = xmlnode_new_child(node, "font");
    if (sendout) {
        xmlnode_new_prop(font_node, "color", "#16569E");

    } else {
        xmlnode_new_prop(font_node, "color", "#A82F2F");
    }

    time_node = xmlnode_new_child(font_node, "font");
    xmlnode_new_prop(time_node, "size", "2");

    memset(time_str, 0, sizeof(time_str));
    strftime(time_str, sizeof(time_str) - 1, "(%H:%M:%S) ", local_time);

    xmlnode_set_content(time_node, time_str);

    content = g_strdup_printf("%s: ", name);
    name_node = xmlnode_new_child(font_node, "b");
    xmlnode_set_content(name_node, content);
    g_free(content);

    cont_node = xmlnode_new_child(node, "font");
    xmlnode_set_content(cont_node, msg);

    xmlnode_new_child(node, "br");

    xmlnode_save_file(log->root, log->log_path);

    return HYBRID_OK;
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
