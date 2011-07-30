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
#include "gtkutils.h"
#include "info.h"

static GSList *info_list = NULL;

static void
render_column(HybridInfo *info)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	/* expander columns */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(info->treeview), column);
	gtk_tree_view_column_set_visible(column, FALSE);
	gtk_tree_view_set_expander_column(GTK_TREE_VIEW(info->treeview), column);

	/* main column */
	column = gtk_tree_view_column_new ();
	gtk_tree_view_append_column(GTK_TREE_VIEW(info->treeview), column);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(info->treeview));

	/* name */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "markup", HYBRID_INFO_NAME_COLUMN,
					    NULL);
	g_object_set(renderer, "wrap-mode", PANGO_WRAP_CHAR, NULL);
	g_object_set(renderer, "wrap-width",120, NULL);
	/* value */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "markup", HYBRID_INFO_VALUE_COLUMN,
						"visible", HYBRID_INFO_VALUE_COLUMN_VISIBLE,
					    NULL);

	g_object_set(renderer, "wrap-mode", PANGO_WRAP_CHAR, NULL);
	g_object_set(renderer, "wrap-width",250, NULL);

	/* pixbuf */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"pixbuf", HYBRID_INFO_PIXBUF_COLUMN,
						"visible", HYBRID_INFO_PIXBUF_COLUMN_VISIBLE,
						NULL);
}

/**
 * Create a new information item with the name-value pair;
 */
static HybridInfoItem*
hybrid_info_item_create(const gchar *name, const gchar *value)
{
	HybridInfoItem *item;

	g_return_val_if_fail(name != NULL, NULL);

	item = g_new0(HybridInfoItem, 1);
	item->name = g_strdup(name);
	item->value = g_strdup(value);

	return item;
}

/**
 * Destroy the information item.
 */
static void
hybrid_info_item_destroy(HybridInfoItem *item)
{
	if (item) {
		g_free(item->name);
		g_free(item->value);
		if (item->pixbuf) {
			g_object_unref(item->pixbuf);
		}
		g_free(item);
	}
}

static void
close_click_cb(GtkWidget *widget, HybridInfo *info)
{
	gtk_widget_destroy(info->window);
}

static void
window_destroy_cb(GtkWidget *widget, HybridInfo *info)
{
	/* remove the info panel from the global list. */
	info_list = g_slist_remove(info_list, info);

	g_free(info);
}

HybridInfo*
hybrid_info_create(HybridBuddy *buddy)
{
	HybridInfo *info;
	GdkPixbuf *pixbuf;
	GtkWidget *vbox;
	GtkWidget *scroll;
	GtkWidget *label;
	GtkWidget *halign;
	GtkWidget *action_area;
	GtkWidget *close_button;
	GtkListStore *store;
	gchar *title;

	GSList *pos;

	g_return_val_if_fail(buddy != NULL, NULL);

	for (pos = info_list; pos; pos = pos->next) {
		info = (HybridInfo*)pos->data;

		if (info->buddy == buddy) {
			return (HybridInfo*)pos->data;
		}
	}

	info = g_new0(HybridInfo, 1);
	info->buddy = buddy;

	info_list = g_slist_append(info_list, info);

	info->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_object_set(info->window, "border-width", 10, NULL);
	pixbuf = hybrid_create_default_icon(0);
	title = g_strdup_printf(_("%s's information"),
			buddy->name && *(buddy->name) != '\0' ? buddy->name : buddy->id);

	gtk_window_set_icon(GTK_WINDOW(info->window), pixbuf);
	gtk_window_set_position(GTK_WINDOW(info->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(info->window), title);
	gtk_widget_set_size_request(info->window, 400, 350);
	gtk_window_set_resizable(GTK_WINDOW(info->window), FALSE);
	g_signal_connect(info->window, "destroy", G_CALLBACK(window_destroy_cb),
							info);

	g_free(title);
	g_object_unref(pixbuf);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(info->window), vbox);

	/* label */
	label = gtk_label_new(NULL);
	halign = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign), label);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);

	title = g_strdup_printf(_("<b>Information of %s</b>"),
			buddy->name && *(buddy->name) != '\0' ? buddy->name : buddy->id);
	gtk_label_set_markup(GTK_LABEL(label), title);
	g_free(title);

	/* scroll -> treeview */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);

	store = gtk_list_store_new(HYBRID_INFO_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					GDK_TYPE_PIXBUF,
					G_TYPE_BOOLEAN,
					G_TYPE_BOOLEAN);

	info->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	//gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(info->treeview), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(info->treeview), FALSE);
	g_object_unref(store);
	gtk_container_add(GTK_CONTAINER(scroll), info->treeview);

	/* close button */
	action_area = gtk_hbox_new(FALSE, 0);
	halign = gtk_alignment_new(1, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign), action_area);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);

	close_button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_size_request(close_button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), close_button, FALSE, TRUE, 0);
	g_signal_connect(close_button, "clicked", G_CALLBACK(close_click_cb), info);

	render_column(info);

	gtk_widget_show_all(info->window);

	return info;
}

HybridNotifyInfo*
hybrid_notify_info_create()
{
	HybridNotifyInfo *info;

	info = g_new0(HybridNotifyInfo, 1);

	return info;
}

void
hybrid_notify_info_destroy(HybridNotifyInfo *info)
{
	HybridInfoItem *item;
	GSList *pos;

	while (info->item_list) {
		pos = info->item_list;
		item = (HybridInfoItem*)pos->data;

		info->item_list = g_slist_remove(info->item_list, item);
		hybrid_info_item_destroy(item);
	}

	info_list = g_slist_remove(info_list, info);

	g_free(info);
}

void
hybrid_info_add_pair(HybridNotifyInfo *info, const gchar *name, const gchar *value)
{
	HybridInfoItem *item;

	g_return_if_fail(info != NULL);
	g_return_if_fail(name != NULL);

	item = hybrid_info_item_create(name, value);
	item->type = HYBRID_INFO_ITEM_TYPE_TEXT;
	info->item_list = g_slist_append(info->item_list, item);
}

void hybrid_info_add_pixbuf_pair(HybridNotifyInfo *info, const gchar *name,
		const GdkPixbuf *pixbuf)
{
	HybridInfoItem *item;

	g_return_if_fail(info != NULL);
	g_return_if_fail(name != NULL);

	item = hybrid_info_item_create(name, NULL);
	item->type = HYBRID_INFO_ITEM_TYPE_PIXBUF;
	item->pixbuf = gdk_pixbuf_copy(pixbuf);
	info->item_list = g_slist_append(info->item_list, item);
}

void
hybrid_info_notify(HybridAccount *account, HybridNotifyInfo *info,
		const gchar *buddy_id)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	HybridBuddy *buddy;
	HybridInfo *info_panel;
	HybridInfoItem *item;
	GSList *info_pos;
	GSList *pos;
	gchar *name_markup;
	gchar *name_escaped;
	gchar *value_escaped;

	g_return_if_fail(info != NULL);

	for (info_pos = info_list; info_pos; info_pos = info_pos->next) {

		info_panel = (HybridInfo*)info_pos->data;

		if (g_strcmp0(info_panel->buddy->id, buddy_id) == 0) {
			goto buddy_ok;
		}

	}

	if (!(buddy = hybrid_blist_find_buddy(account, buddy_id))) {

		hybrid_debug_error("info", "can not find buddy with id :\'%s\'",
				buddy_id);
		
		return;
	}

	info_panel = hybrid_info_create(buddy);

buddy_ok:

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(info_panel->treeview));

	for (pos = info->item_list; pos; pos = pos->next) {

		item = (HybridInfoItem*)pos->data;

		name_escaped = g_markup_escape_text(item->name, -1);

		if (item->value) {
			value_escaped = g_markup_escape_text(item->value, -1);

		} else {
			value_escaped = NULL;
		}

		name_markup = g_strdup_printf("<b>%s:</b>", name_escaped);	

		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				HYBRID_INFO_NAME_COLUMN, name_markup,
				HYBRID_INFO_VALUE_COLUMN, value_escaped,
				HYBRID_INFO_PIXBUF_COLUMN, item->pixbuf,
				HYBRID_INFO_VALUE_COLUMN_VISIBLE, 
				item->type == HYBRID_INFO_ITEM_TYPE_TEXT ? TRUE : FALSE,
				HYBRID_INFO_PIXBUF_COLUMN_VISIBLE, 
				item->type == HYBRID_INFO_ITEM_TYPE_PIXBUF ? TRUE : FALSE,
				-1);

		g_free(name_markup);
		g_free(name_escaped);
		g_free(value_escaped);

	}
}
