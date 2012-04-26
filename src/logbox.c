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

static void bind_log_to_textview(HybridLogbox *logbox, const gchar *logname);

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

static gboolean
button_press_cb(GtkWidget *widget, GdkEventButton *event, HybridLogbox *logbox)
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeSelection *selection;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));

    /* left button clicked, change selection signaled. */
    if (event->button == 1) {
        GtkTreeIter iter;
        gchar *filename;

        gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget),
                (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL);

        if (!path) {
            return FALSE;
        }

        if (!gtk_tree_model_get_iter(model, &iter, path)) {
            gtk_tree_path_free(path);
            return FALSE;
        }

        gtk_tree_model_get(model, &iter, HYBRID_LOGBOX_FILE, &filename, -1);
        bind_log_to_textview(logbox, filename);

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
        gtk_tree_selection_select_path(selection, path);

        g_free(filename);
        gtk_tree_path_free(path);

        return TRUE;
    }

    return FALSE;
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

static void
bind_log_to_textview(HybridLogbox *logbox, const gchar *logname)
{
    GSList *list, *pos;
    HybridLogEntry *entry;
    GtkTextBuffer *buffer;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    GtkTextMark *mark;
    const gchar *color;
    gchar *title;

    list = hybrid_logs_read(logbox->account, logbox->buddy->id, logname);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logbox->textview));
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_delete(buffer, &start_iter, &end_iter);

    for (pos = list; pos; pos = pos->next) {
        entry = pos->data;
        if (entry->is_send) {
            color = "blue";
        } else {
            color = "green";
        }
        title = g_strdup_printf("%s ", entry->name);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &end_iter,
                        title, strlen(title), color, "wrap",
                        "bold", "small", "lm5", NULL);
        g_free(title);

        title = g_strdup_printf(_("(%s) said:"), entry->time);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &end_iter,
                        title, strlen(title), "wrap",
                        "grey", "small", NULL);

        gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);
        gtk_text_buffer_insert_with_tags_by_name(buffer, &end_iter,
                        entry->content, strlen(entry->content),
                        "lm10", "wrap", "small", NULL);

        gtk_text_buffer_insert(buffer, &end_iter, "\n\n", -1);
        gtk_text_iter_set_line_offset(&end_iter, 0);

        mark = gtk_text_buffer_get_mark(buffer, "scroll");
        gtk_text_buffer_move_mark(buffer, mark, &end_iter);
        gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(logbox->textview), mark);

        g_free(title);

        g_free(entry->name);
        g_free(entry->time);
        g_free(entry->content);
        g_free(entry);
        pos->data = NULL;
    }

    g_slist_free(list);
}

static void
select_the_latest_log(HybridLogbox *logbox)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *filename;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(logbox->loglist));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(logbox->loglist));

    if (!gtk_tree_model_get_iter_first(model, &iter)) {
        return;
    }

    gtk_tree_selection_select_iter(selection, &iter);
    gtk_tree_model_get(model, &iter, HYBRID_LOGBOX_FILE, &filename, -1);
    bind_log_to_textview(logbox, filename);
    g_free(filename);
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
    g_object_set(renderer, "xalign", 0.0, "xpad", 0, "ypad", 0, NULL);
}

static GtkWidget*
create_logbox_header(HybridLogbox *logbox)
{
    GtkWidget *cellview;
    GtkTreeIter iter;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GdkPixbuf *pixbuf, *ot_pixbuf;
    GtkTreePath *path;
    gchar *name;

    cellview = gtk_cell_view_new();

    store = gtk_list_store_new(HYBRID_LOGBOX_HEAD_COLUMNS,
		                       GDK_TYPE_PIXBUF,
			                   G_TYPE_STRING,
                               GDK_TYPE_PIXBUF,
                               G_TYPE_STRING);
    gtk_cell_view_set_model(GTK_CELL_VIEW(cellview), GTK_TREE_MODEL(store));

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, FALSE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
                "pixbuf", HYBRID_LOGBOX_HEAD_MY_ICON, NULL);
    g_object_set(renderer, "yalign", 0.5, "xpad", 3, "ypad", 5, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
                "markup", HYBRID_LOGBOX_HEAD_MY_NAME, NULL);
    g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 5, NULL);

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, FALSE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
                "pixbuf", HYBRID_LOGBOX_HEAD_OT_ICON, NULL);
    g_object_set(renderer, "yalign", 0.5, "xpad", 3, "ypad", 5, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
                "markup", HYBRID_LOGBOX_HEAD_OT_NAME, NULL);
    g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 5, NULL);

    gtk_list_store_append(store, &iter);
    path = gtk_tree_path_new_from_string("0");
    gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(cellview), path);
    gtk_tree_path_free(path);

    pixbuf = hybrid_create_round_pixbuf(logbox->account->icon_data,
                        logbox->account->icon_data_len, 24);

    if (!pixbuf) {
        pixbuf = hybrid_create_default_icon(24);
    }

    ot_pixbuf = hybrid_create_round_pixbuf(logbox->buddy->icon_data,
                        logbox->buddy->icon_data_length, 24);
    if (!ot_pixbuf) {
        ot_pixbuf = hybrid_create_default_icon(24);
    }

    name = g_strdup_printf("%s(%s)", logbox->account->nickname,
                                     logbox->account->username);

    gtk_list_store_set(store, &iter,
                HYBRID_LOGBOX_HEAD_MY_ICON, pixbuf,
                HYBRID_LOGBOX_HEAD_MY_NAME, name,
                HYBRID_LOGBOX_HEAD_OT_ICON, ot_pixbuf,
                HYBRID_LOGBOX_HEAD_OT_NAME, logbox->buddy->name, -1);

    g_free(name);
    g_object_unref(pixbuf);
    g_object_unref(ot_pixbuf);

    return cellview;
}

static void
logbox_init(HybridLogbox *logbox)
{
    GtkWidget *vbox, *hbox;
    GtkWidget *scroll;
    GtkWidget *button;
    GtkWidget *action_area;
    GtkTextIter end_iter;
    GtkTextBuffer *buffer;
    GtkTreeModel *model;
    GtkWidget *cellview;
    GtkWidget *frame;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(logbox->window), vbox);

    cellview = create_logbox_header(logbox);
    frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(frame), cellview);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 3);

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
    g_signal_connect(logbox->loglist, "button-press-event",
            G_CALLBACK(button_press_cb), logbox);
    gtk_container_add(GTK_CONTAINER(scroll), logbox->loglist);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(hbox), scroll, TRUE, TRUE, 5);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
            GTK_SHADOW_ETCHED_IN);

    logbox->textview = gtk_text_view_new();
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(logbox->textview), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(logbox->textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(logbox->textview) , GTK_WRAP_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), logbox->textview);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logbox->textview));
    gtk_text_buffer_create_tag(buffer, "blue", "foreground", "#639900", NULL);
    gtk_text_buffer_create_tag(buffer, "grey", "foreground", "#808080", NULL);
    gtk_text_buffer_create_tag(buffer, "green", "foreground", "#0088bf", NULL);
    gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(buffer, "lm5", "left_margin", 5, NULL);
    gtk_text_buffer_create_tag(buffer, "lm10", "left_margin", 12, NULL);
    gtk_text_buffer_create_tag(buffer, "wrap", "wrap-mode", GTK_WRAP_WORD_CHAR, NULL);
    gtk_text_buffer_create_tag(buffer, "small", "size-points", 9.0, NULL);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_create_mark(buffer, "scroll", &end_iter, FALSE);

    select_the_latest_log(logbox);

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
    gchar *title;
    title = g_strdup_printf(_("Chat Log With %s"), logbox->buddy->name);
    logbox->window = hybrid_create_window(title, NULL,
                                         GTK_WIN_POS_CENTER, TRUE);
    gtk_widget_set_size_request(logbox->window, 600, 500);
    gtk_container_set_border_width(GTK_CONTAINER(logbox->window), 8);
    g_signal_connect(logbox->window, "destroy", G_CALLBACK(destroy_cb), logbox);

	logbox_init(logbox);

    gtk_widget_show_all(logbox->window);
}
