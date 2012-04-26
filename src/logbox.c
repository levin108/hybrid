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
#include "logbox.h"
#include "gtkutils.h"

HybridLogbox*
hybrid_logbox_create(HybridAccount *account, HybridBuddy *buddy)
{
	HybridLogbox *logbox = g_new0(HybridLogbox, 1);

	logbox->account = account;
	logbox->buddy = buddy;

	return logbox;
}

static void
destroy_cb(GtkWidget *widget, HybridLogbox *logbox)
{
    g_free(logbox);
}

static void
close_cb(GtkWidget *widget, HybridLogbox *logbox)
{
    gtk_widget_destroy(logbox->window);
}

static GtkTreeModel*
create_log_list_model(HybridLogbox *logbox)
{
    GtkTreeStore *store;
    GtkTreeIter iter;
    const gchar *path;
    const gchar *filename;
    char name[128];
    GDir *dir = NULL;
    gint i;

    store = gtk_tree_store_new(HYBRID_LOGBOX_COLUMNS,
                               G_TYPE_STRING,
                               G_TYPE_STRING);

    path = hybrid_logs_get_path(logbox->account, logbox->buddy->id);
    dir = g_dir_open(path, 0, NULL);
    if (!dir) {
        hybrid_debug_error("log", "log path %s doesn't exist.\n", path);
        goto out;
    }

    while ((filename = g_dir_read_name(dir))) {
        for (i = 0; i < strlen(filename); i ++) {
            if (filename[i] == '_') {
                name[i] = '-';
            } else if (filename[i] == '.') {
                name[i] = '\0';
                break;
            } else {
                name[i] = filename[i];
            }
        }
        gtk_tree_store_append(store, &iter, NULL);
        gtk_tree_store_set(store, &iter, HYBRID_LOGBOX_NAME, name, -1);
        gtk_tree_store_set(store, &iter, HYBRID_LOGBOX_FILE, filename, -1);
    }

out:
    g_dir_close(dir);
    return GTK_TREE_MODEL(store);
}

void
select_the_latest_log(HybridLogbox *logbox)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(logbox->loglist));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(logbox->loglist));

    if (!gtk_tree_model_get_iter_first(model, &iter)) {
        return;
    }

    gtk_tree_selection_select_iter(selection, &iter);
}

static void
render_column(HybridLogbox *logbox)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer   *renderer;

    column = gtk_tree_view_column_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(logbox->loglist), column);
    gtk_tree_view_column_set_visible(column, TRUE);
    gtk_tree_view_set_expander_column(GTK_TREE_VIEW(logbox->loglist), column);

    /* name */
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_set_attributes(column, renderer,
                        "markup", HYBRID_LOGBOX_NAME,
                        NULL);
    g_object_set(renderer, "xalign", 0.0, "xpad", 3, "ypad", 0, NULL);
}

static void
logbox_init(HybridLogbox *logbox)
{
    GtkWidget *vbox, *hbox;
    GtkWidget *scroll;
    GtkWidget *button;
    GtkWidget *action_area;
    GtkTreeModel *model;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(logbox->window), vbox);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 5);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(hbox), scroll, FALSE, FALSE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
            GTK_SHADOW_ETCHED_IN);

    model = create_log_list_model(logbox);
    logbox->loglist = gtk_tree_view_new_with_model(model);
    g_object_unref(model);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(logbox->loglist), FALSE);
    render_column(logbox);
    select_the_latest_log(logbox);
    gtk_container_add(GTK_CONTAINER(scroll), logbox->loglist);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(hbox), scroll, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
            GTK_SHADOW_ETCHED_IN);

    logbox->textview = gtk_text_view_new();
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(logbox->textview), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(logbox->textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(logbox->textview) , GTK_WRAP_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), logbox->textview);

    /* buttons */
    action_area = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5);

    button = gtk_button_new_with_label(_("Close"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked", G_CALLBACK(close_cb), logbox);
}

void
hybrid_logbox_show(HybridLogbox *logbox)
{
    logbox->window = hybrid_create_window(_("Chat Log"), NULL,
                                         GTK_WIN_POS_CENTER, FALSE);
    gtk_widget_set_size_request(logbox->window, 600, 500);
    gtk_container_set_border_width(GTK_CONTAINER(logbox->window), 8);
    g_signal_connect(logbox->window, "destroy", G_CALLBACK(destroy_cb), logbox);

	logbox_init(logbox);

    gtk_widget_show_all(logbox->window);
}
