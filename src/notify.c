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
#include "notify.h"
#include "gtkutils.h"
#include "pref.h"

#ifdef USE_LIBNOTIFY
#include <libnotify/notify.h>

static NotifyNotification *notification = NULL;
#endif

static void
close_cb(GtkWidget *widget, gpointer user_data)
{
    HybridNotify *notify = (HybridNotify*)user_data;

    gtk_widget_destroy(notify->window);
    
    g_free(notify);
}

HybridNotify*
hybrid_notify_create(HybridAccount *account, const gchar *title)
{
    HybridNotify *notify;
    GtkWidget *vbox;
    GtkWidget *button;
    GtkWidget *scroll;
    GtkWidget *frame;
    GtkWidget *cellview;
    GdkPixbuf *pixbuf;
    GtkListStore *store;
    GtkTreePath *path;
    GtkCellRenderer *renderer;
    gchar *name;

    notify = g_new0(HybridNotify, 1);
    notify->account = account;

    notify->window = hybrid_create_window(title ? title : _("Notification"),
                        NULL, GTK_WIN_POS_CENTER, FALSE);
    gtk_widget_set_size_request(notify->window, 400, 250);

    vbox = gtk_vbox_new(FALSE, 0);

    /* account info */
    cellview = gtk_cell_view_new();
    notify->cellview = cellview;

    store = gtk_list_store_new(NOTIFY_COLUMNS,
            GDK_TYPE_PIXBUF,
            G_TYPE_STRING);
    gtk_cell_view_set_model(GTK_CELL_VIEW(cellview), GTK_TREE_MODEL(store));

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, FALSE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
            "pixbuf", NOTIFY_ICON_COLUMN, NULL);
    g_object_set(renderer, "yalign", 0.5, "xpad", 3, "ypad", 5, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
            "markup", NOTIFY_NAME_COLUMN, NULL);

    g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 5, NULL);
    g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

    gtk_list_store_append(store, &notify->iter);
    path = gtk_tree_path_new_from_string("0");
    gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(cellview), path);
    gtk_tree_path_free(path);

    g_object_unref(store);

    pixbuf = hybrid_create_default_icon(32);
    name = g_strdup_printf("(%s):", account->username);
    gtk_list_store_set(store, &notify->iter,
            NOTIFY_ICON_COLUMN, pixbuf,
            NOTIFY_NAME_COLUMN, name, -1);
    g_object_unref(pixbuf);
    g_free(name);

    frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(frame), cellview);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 3);

    /* default account info. */
    gchar *account_text;

    pixbuf = hybrid_create_round_pixbuf(notify->account->icon_data,
            notify->account->icon_data_len, 48);

    if (notify->account->nickname) {
        account_text = g_strdup_printf("<b>%s</b> (%s):",
                notify->account->nickname, notify->account->username);

    } else {
        account_text = g_strdup_printf("%s:", notify->account->username);
    }

    gtk_list_store_set(store, &notify->iter,
            NOTIFY_NAME_COLUMN, account_text,
            NOTIFY_ICON_COLUMN, pixbuf, -1);

    g_free(account_text);
    g_object_unref(pixbuf);

    /* textview */
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
            GTK_SHADOW_ETCHED_IN);

    notify->textview = gtk_text_view_new();
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(notify->textview), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(notify->textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(notify->textview) , GTK_WRAP_CHAR);

    gtk_container_add(GTK_CONTAINER(scroll), notify->textview);

    notify->action_area = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notify->action_area, FALSE, FALSE, 5);

    button = gtk_button_new_with_label(_("Close"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(notify->action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked", G_CALLBACK(close_cb), notify);

    /* focus the send textview */
    GTK_WIDGET_SET_FLAGS(button, GTK_CAN_FOCUS);
    gtk_widget_grab_focus(button);

    gtk_container_add(GTK_CONTAINER(notify->window), vbox);

    gtk_widget_show_all(notify->window);
    
    return notify;
}

void
hybrid_notify_set_text(HybridNotify *notify, const gchar *text)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(notify->textview));

    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, text, strlen(text));
}

void
hybrid_notify_set_name(HybridNotify *notify, const gchar *name)
{
    gchar *account_text;
    GtkListStore *store;

    store = GTK_LIST_STORE(gtk_cell_view_get_model(GTK_CELL_VIEW(notify->cellview)));

    account_text = g_strdup_printf("<b>%s</b> (%s):",
            name, notify->account->username);
    gtk_list_store_set(store, &notify->iter,
            NOTIFY_NAME_COLUMN, account_text, -1);
    g_free(account_text);
}

void
hybrid_notify_popup(GdkPixbuf *pixbuf, const gchar *title,
        const gchar *summary)
{
#ifdef USE_LIBNOTIFY
    if (hybrid_pref_get_boolean("close_notify")) {
        return;
    }
    
    if (!notification) {
    #ifdef LIBNOTIFY_OLD
        notification = notify_notification_new(title, summary, NULL, NULL);
    #else
        notification = notify_notification_new(title, summary, NULL);
    #endif
        notify_notification_set_timeout(notification, 2500);

    } else {

        notify_notification_close(notification, NULL);

        notify_notification_update(notification, title, summary, NULL);
    }

    if (pixbuf) {
        notify_notification_set_icon_from_pixbuf(notification, pixbuf);
    }

    notify_notification_show(notification, NULL);

#endif
}
