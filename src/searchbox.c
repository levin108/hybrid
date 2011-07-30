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

#include "gtkutils.h"
#include "searchbox.h"
#include "conv.h"
#include "blist.h"

#include <gdk/gdkkeysyms.h>

extern GtkWidget *hybrid_window;
extern HybridBlist *blist;

static GtkTreeModel*
create_model(const gchar *text)
{
	GtkTreeModel *model;
	GtkTreeStore *store;
	GtkTreeIter iter;
	GtkTreeIter citer;
	GtkTreeIter new_iter;
	gchar *id, *name;
	GdkPixbuf *status_icon, *proto_icon, *buddy_icon;
	gint buddy_state;
	gpointer data;

	store = gtk_tree_store_new(HYBRID_BLIST_COLUMNS,
			G_TYPE_STRING,
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING,
			GDK_TYPE_PIXBUF,
			GDK_TYPE_PIXBUF,
			G_TYPE_INT,
			G_TYPE_POINTER,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN);

	/* search for match */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(blist->treeview));

	if (!gtk_tree_model_get_iter_first(model, &iter)) {
		return GTK_TREE_MODEL(store);
	}

	do {
		if (gtk_tree_model_iter_children(model, &citer, &iter)) {

			do {

				gtk_tree_model_get(model, &citer,
						HYBRID_BLIST_BUDDY_NAME, &name,
						HYBRID_BLIST_BUDDY_ID, &id,
						-1);

				if (strstr(name, text) || strstr(id, text)) {

					gtk_tree_model_get(model, &citer,
						HYBRID_BLIST_STATUS_ICON, &status_icon,
						HYBRID_BLIST_BUDDY_ICON, &buddy_icon,
						HYBRID_BLIST_PROTO_ICON, &proto_icon,
						HYBRID_BLIST_BUDDY_STATE, &buddy_state,
						HYBRID_BLIST_OBJECT_COLUMN, &data,
						-1);

					gtk_tree_store_append(store, &new_iter, NULL);

					gtk_tree_store_set(store, &new_iter,
						HYBRID_BLIST_BUDDY_ID, id,
						HYBRID_BLIST_BUDDY_NAME, name,
						HYBRID_BLIST_STATUS_ICON, status_icon,
						HYBRID_BLIST_BUDDY_ICON, buddy_icon,
						HYBRID_BLIST_PROTO_ICON, proto_icon,
						HYBRID_BLIST_BUDDY_STATE, buddy_state,
						HYBRID_BLIST_OBJECT_COLUMN, data,
						HYBRID_BLIST_GROUP_EXPANDER_COLUMN_VISIBLE, FALSE,
						HYBRID_BLIST_CONTACT_EXPANDER_COLUMN_VISIBLE, FALSE,
						HYBRID_BLIST_STATUS_ICON_COLUMN_VISIBLE, TRUE,
						HYBRID_BLIST_PROTO_ICON_COLUMN_VISIBLE, TRUE,
						HYBRID_BLIST_BUDDY_ICON_COLUMN_VISIBLE, TRUE,
						-1);

					if (status_icon) {
						g_object_unref(status_icon);
					}

					if (buddy_icon) {
						g_object_unref(buddy_icon);
					}

					if (proto_icon) {
						g_object_unref(proto_icon);
					}
				}

				g_free(name);
				g_free(id);

			} while (gtk_tree_model_iter_next(model, &citer));
		}
	} while (gtk_tree_model_iter_next(model, &iter));

	return GTK_TREE_MODEL(store);
}

static void
render_column(GtkWidget *treeview)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	/* expander columns */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_column_set_visible(column, FALSE);
	gtk_tree_view_set_expander_column(GTK_TREE_VIEW(treeview), column);

	/* main column */
	column = gtk_tree_view_column_new ();
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));

	/* portrait */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"pixbuf", HYBRID_BLIST_BUDDY_ICON,
						"visible", HYBRID_BLIST_BUDDY_ICON_COLUMN_VISIBLE,
						NULL);
	g_object_set(renderer, "xalign", 1.0, "xpad", 3, "ypad", 0, NULL);

	/* name */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"markup", HYBRID_BLIST_BUDDY_NAME,
						NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 3, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	/* protocol icon */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"pixbuf", HYBRID_BLIST_PROTO_ICON,
						"visible", HYBRID_BLIST_PROTO_ICON_COLUMN_VISIBLE,
						NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);

	/* status icon */
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"pixbuf", HYBRID_BLIST_STATUS_ICON,
						"visible", HYBRID_BLIST_STATUS_ICON_COLUMN_VISIBLE,
						NULL);

}

static gboolean
window_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	gtk_widget_destroy(widget);

	return TRUE;
}


static gboolean
key_press_func(GtkWidget *tree, GdkEventKey *event, GtkWidget *window)
{
	if (event->keyval == GDK_q) {

		gtk_widget_destroy(window);

		return TRUE;
	}

	return FALSE;
}


static void 
row_activate_func(GtkTreeView *view, GtkTreePath *path,
		GtkTreeViewColumn *column, GtkWidget *window)
{
	GtkTreeModel* model;
	GtkTreeIter iter;
	HybridBuddy *buddy;

	if (!path) {
		gtk_widget_destroy(window);
		return;
	}

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	if(!gtk_tree_model_get_iter(model, &iter, path)) {
		return;
	}

	gtk_tree_model_get(model, &iter,
			HYBRID_BLIST_OBJECT_COLUMN, &buddy, -1);

	hybrid_chat_window_create(buddy->account, buddy->id,
			HYBRID_CHAT_PANEL_SYSTEM);
}


static gboolean
button_press_func(GtkWidget *tree, GdkEventButton *event, gpointer user_data)
{
	GtkTreePath *path = NULL;

	if (event->type == GDK_BUTTON_PRESS && event->button == 1) {

		gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tree),
				(gint)event->x, (gint)event->y, &path, NULL, NULL, NULL);
		row_activate_func(GTK_TREE_VIEW(tree), path, NULL, user_data);

		return TRUE;
	}

	return FALSE;
}

static void
search_box_show(const gchar *text, gint x, gint y)
{
	GtkWidget *frame;
	GtkWidget *view;
	GtkWidget *window;
	GtkTreeModel *model;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_widget_set_name(window, "mainwindow");
	gtk_window_set_default_size(GTK_WINDOW(window), 280, 60);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
	gtk_window_move(GTK_WINDOW(window), x, y);
	g_signal_connect(window, "focus-out-event",
			GTK_SIGNAL_FUNC(window_focus_out), NULL);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

	model = create_model(text);
	view = gtk_tree_view_new_with_model(model);
	g_object_unref(model);

	render_column(view);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	gtk_tree_view_set_hover_selection(GTK_TREE_VIEW(view), TRUE);
	gtk_container_add(GTK_CONTAINER(window), frame);
	gtk_container_add(GTK_CONTAINER(frame), view);

	g_signal_connect(view, "key-press-event",
			G_CALLBACK(key_press_func), window);

	g_signal_connect(view, "row-activated",
			G_CALLBACK(row_activate_func), window);

	g_signal_connect(view, "button-press-event",
			G_CALLBACK(button_press_func), NULL);

	gtk_widget_show_all(window);
}

static void
show_search(GtkEntry *entry, gpointer user_data)
{
	gint x, y, ex, ey, root_x, root_y;
	const gchar *text;

	text = gtk_entry_get_text(entry);

	if(!text || *text == '\0') {
		return;
	}

	gtk_widget_translate_coordinates(GTK_WIDGET(entry), hybrid_window,
						0, 0, &ex, &ey);
	gtk_window_get_position(GTK_WINDOW(hybrid_window), &root_x, &root_y);
	x = root_x + ex + 3;
	y = root_y + ey + 46;

	search_box_show(text, x, y);
}

static void 
search_btn_click_cb(GtkEntry *entry, GtkEntryIconPosition *pos,
		GdkEvent *event, gpointer user_data)
{
	show_search(entry, user_data);	
}

GtkWidget*
hybrid_search_box_create(void)
{
	GtkWidget *entry;

	entry = gtk_entry_new();

	gtk_entry_set_icon_from_stock(GTK_ENTRY(entry),
			GTK_ENTRY_ICON_SECONDARY,
			GTK_STOCK_FIND);
	g_signal_connect(entry, "icon-press", 
			G_CALLBACK(search_btn_click_cb), NULL);
	g_signal_connect(entry, "activate",
			G_CALLBACK(show_search), NULL);

	gtk_widget_show_all(entry);

	return entry;
}
