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
#include "config.h"
#include "conv.h"
#include "chat-webkit.h"

#ifdef USE_WEBKIT
 #include <webkit/webkit.h>
#endif

static gchar    *content_html       = NULL;
static gchar    *content_css       = NULL;
static gchar    *content_full_html = NULL;
static gchar    *content_send       = NULL;
static gchar    *content_recv       = NULL;

static HybridChatTextOps webkit_ops = {
    hybrid_chat_webkit_create,
    hybrid_chat_webkit_append,
    hybrid_chat_webkit_notify
};

struct timeout_data {
    GtkWidget *webkit;
    gchar *script;
};

void
hybrid_chat_set_webkit_ops(void)
{
    hybrid_conv_set_chat_text_ops(&webkit_ops);
}

#ifdef USE_WEBKIT
static gchar*
escape_string(const gchar *str)
{
    GString *res;

    res = g_string_sized_new(strlen(str));

    while (str && *str) {
        switch (*str) {
            case 13:
                res = g_string_append(res, "<br/>");
                break;
            case '\"':
                res = g_string_append(res, "\\\"");
                break;
            case '\t':
                break;
            default:
                res = g_string_append_c(res, *str);
                break;
        }
        str++;
    }

    return g_string_free(res, FALSE);
}

static gchar*
escape_html(const gchar *str)
{
    GString *res;

    res = g_string_sized_new(strlen(str));

    while (str && *str != '\0') {
        switch (*str) {
            case '\n':
            case '\t':
                break;
            default:
                res = g_string_append_c(res, *str);
                break;
        }
        str ++;
    }

    return g_string_free(res, FALSE);
}
#endif

/**
 * Initialize the webkit context.
 */
gint
hybrid_webkit_init(void)
{
    if (!g_file_get_contents(UI_DIR"template.html", &content_html,
                             NULL, NULL)) {
        hybrid_debug_error("webkit", "read template.html failed.");
        return HYBRID_ERROR;
    }

    if (!g_file_get_contents(UI_DIR"default.css", &content_css, NULL, NULL)) {
        hybrid_debug_error("webkit", "read default.css failed.");
        return HYBRID_ERROR;
    }

    if (!g_file_get_contents(UI_DIR"send.html", &content_send, NULL, NULL)) {
        hybrid_debug_error("webkit", "read send.html failed.");
        return HYBRID_ERROR;
    }

    if (!g_file_get_contents(UI_DIR"recv.html", &content_recv, NULL, NULL)) {
        hybrid_debug_error("webkit", "read recv.html failed.");
        return HYBRID_ERROR;
    }

    content_full_html = g_strdup_printf(content_html, content_css);

    return HYBRID_OK;
}

/**
 * Destroy the webkit context.
 */
void
hybrid_webkit_destroy(void)
{
    g_free(content_html);
    g_free(content_css);
    g_free(content_send);
    g_free(content_recv);
    g_free(content_full_html);
}

GtkWidget*
hybrid_chat_webkit_create(void)
{
#ifdef USE_WEBKIT
    GtkWidget *webkit = webkit_web_view_new();

    webkit_web_view_load_string(WEBKIT_WEB_VIEW(webkit), content_full_html,
            NULL, NULL, "file://"UI_DIR);

    return webkit;
#else
    return NULL;
#endif
}

gboolean
timeout_cb(struct timeout_data *data)
{
#ifdef USE_WEBKIT
    WebKitLoadStatus status;

    g_object_get(data->webkit, "load-status", &status, NULL);

    if (status != WEBKIT_LOAD_FINISHED) {
        return TRUE;
    }

    webkit_web_view_execute_script(WEBKIT_WEB_VIEW(data->webkit), data->script);
    g_free(data->script);
    g_free(data);
#endif
    return FALSE;
}

void
hybrid_chat_webkit_append(GtkWidget *textview, HybridAccount *account,
                            HybridBuddy *buddy,    const gchar *message, time_t msg_time)
{
#ifdef USE_WEBKIT
    gchar                *html, *escaped_html, *escaped_message;
    gchar                *script;
    WebKitLoadStatus     status;
    struct timeout_data  *data;
    const gchar          *icon_path;
    gchar                *icon_name;
    gchar                 time[128];
    struct tm            *tm_time;

    g_return_if_fail(textview != NULL);

    tm_time = localtime(&msg_time);
    strftime(time, sizeof(time) - 1, _("%H:%M:%S"), tm_time);

    escaped_message = escape_string(message);

    icon_path = hybrid_config_get_path();

    if (buddy) {
        if (buddy->icon_name) {
            icon_name = g_strdup_printf("file://%s/icons/%s",
                                        icon_path, buddy->icon_name);
        } else {
            icon_name = g_strdup_printf("file://%sicons/icon.png",
                                        PIXMAPS_DIR);
        }

        html = g_strdup_printf(content_recv, icon_name,
                               buddy->name && *buddy->name ?
                               buddy->name : buddy->id,
                               time, escaped_message);
    } else {
        if (account->icon_name) {
            icon_name = g_strdup_printf("file://%s/icons/%s",
                                        icon_path, account->icon_name);
        } else {
            icon_name = g_strdup_printf("file://%sicons/icon.png",
                                        PIXMAPS_DIR);
        }

        html = g_strdup_printf(content_send,
                account->nickname && *account->nickname ?
                account->nickname : account->username,
                time,
                escaped_message,
                icon_name);
    }

    g_free(icon_name);

    escaped_html = escape_html(html);

    g_object_get(textview, "load-status", &status, NULL);

    script = g_strdup_printf("appendMessage(\"%s\")", escaped_html);

    if (WEBKIT_LOAD_FINISHED != status &&
        WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT != status) {
        data         = g_new0(struct timeout_data, 1);
        data->webkit = textview;
        data->script = script;

        g_timeout_add_seconds(0, (GSourceFunc)timeout_cb, data);

    } else {
        webkit_web_view_execute_script(WEBKIT_WEB_VIEW(textview), script);
        g_free(script);
    }

    g_free(escaped_html);
    g_free(escaped_message);
    g_free(html);
#endif
}

void
hybrid_chat_webkit_notify(GtkWidget *textview, const gchar *text, gint type)
{
#ifdef USE_WEBKIT
    WebKitLoadStatus     status;
    gchar                *script;
    struct timeout_data *data;

    g_object_get(textview, "load-status", &status, NULL);

    script = g_strdup_printf("notify(\"%s\")", text);

    if (WEBKIT_LOAD_FINISHED                        != status    &&
        WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT != status) {
        data         = g_new0(struct timeout_data, 1);
        data->webkit = textview;
        data->script = script;

        g_timeout_add_seconds(0, (GSourceFunc)timeout_cb, data);

    } else {
        webkit_web_view_execute_script(WEBKIT_WEB_VIEW(textview), script);
        g_free(script);
    }
#endif
}
