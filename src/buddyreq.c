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
#include "buddyreq.h"

enum {
	BUDDY_REQ_ICON_COLUMN,
	BUDDY_REQ_NAME_COLUMN,
	BUDDY_REQ_COLUMNS
};

enum {
	BUDDYREQ_GROUP_NAME_COLUMN,
	BUDDYREQ_GROUP_GROUP_COLUMN,
	BUDDYREQ_GROUP_COLUMNS
};

static void
destroy_cb(GtkWidget *widget, HybridBuddyReqWindow *req)
{
	g_free(req->buddy_id);
	g_free(req->buddy_name);

	if (req->notify && req->user_data) {
		req->notify(req->user_data);
	}
	g_free(req);
}

static void
add_cb(GtkWidget *widget, HybridBuddyReqWindow *req)
{
	HybridAccount		*account;
	HybridModule		*module;
	HybridIMOps			*ops;
	HybridGroup			*group;
	GtkTreeModel		*model;
	GtkTreeIter			 iter;

	account = req->account;
	module	= account->proto;
	ops		= module->info->im_ops;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(req->group_combo));

	if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(req->group_combo), &iter)) {
		return;
	}

	gtk_tree_model_get(model, &iter, BUDDYREQ_GROUP_GROUP_COLUMN, &group, -1);

	if (ops->buddy_req) {
		ops->buddy_req(account, group,	req->buddy_id, 
				req->buddy_name, req->accept, req->user_data);
	}

	gtk_widget_destroy(req->window);
}

static void
cancel_cb(GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_destroy(window);
}

static void
accept_toggled_cb(GtkToggleButton *button, HybridBuddyReqWindow *req)
{
	req->accept = TRUE;

	gtk_widget_set_sensitive(req->alias_entry, TRUE);
	gtk_widget_set_sensitive(req->group_combo, TRUE);
}

static void
decline_toggled_cb(GtkToggleButton *button, HybridBuddyReqWindow *req)
{
	req->accept = FALSE;

	gtk_widget_set_sensitive(req->alias_entry, FALSE);
	gtk_widget_set_sensitive(req->group_combo, FALSE);
}

/**
 * Create the account model for the group combo box.
 */
static GtkTreeModel*
create_group_model(HybridAccount *account)
{
	GtkListStore *store;
	HybridGroup *group;
	GtkTreeIter iter;

	GHashTableIter hash_iter;
	gpointer key;
	
	store = gtk_list_store_new(BUDDYREQ_GROUP_COLUMNS,
							G_TYPE_STRING,
							G_TYPE_POINTER);

	if (!account) {
		return GTK_TREE_MODEL(store);
	}

	/* Bind the groups of the current account to the group combo box. */
	g_hash_table_iter_init(&hash_iter, account->group_list);
	while (g_hash_table_iter_next(&hash_iter, &key, (gpointer*)&group)) {

		gtk_list_store_append(store, &iter);

		gtk_list_store_set(store, &iter,
				BUDDYREQ_GROUP_NAME_COLUMN, group->name,
				BUDDYREQ_GROUP_GROUP_COLUMN, group,
				-1);
	}

	return GTK_TREE_MODEL(store);
}

static void
req_window_init(HybridBuddyReqWindow *req)
{
	GtkWidget *vbox;
	GtkWidget *action_area;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *cellview;
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	GtkWidget *frame;
	GtkListStore *store;
	GtkTreePath *path;
	GtkCellRenderer *renderer;
	HybridAccount *account;
	gchar *name;
	gchar *str;

	account = req->account;

	label = gtk_label_new(NULL);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(req->window), vbox);

	/* account info */
	cellview = gtk_cell_view_new();
	cellview = cellview;

	store = gtk_list_store_new(BUDDY_REQ_COLUMNS,
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING);
	gtk_cell_view_set_model(GTK_CELL_VIEW(cellview), GTK_TREE_MODEL(store));

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
			"pixbuf", BUDDY_REQ_ICON_COLUMN, NULL);
	g_object_set(renderer, "yalign", 0.5, "xpad", 3, "ypad", 5, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cellview), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cellview), renderer,
			"markup", BUDDY_REQ_NAME_COLUMN, NULL);

	g_object_set(renderer, "xalign", 0.0, "xpad", 6, "ypad", 5, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_list_store_append(store, &iter);
	path = gtk_tree_path_new_from_string("0");
	gtk_cell_view_set_displayed_row(GTK_CELL_VIEW(cellview), path);
	gtk_tree_path_free(path);

	g_object_unref(store);

	pixbuf = hybrid_create_default_icon(32);
	name = g_strdup_printf("(%s):", account->username);
	gtk_list_store_set(store, &iter,
			BUDDY_REQ_ICON_COLUMN, pixbuf,
			BUDDY_REQ_NAME_COLUMN, name, -1);
	g_object_unref(pixbuf);
	g_free(name);

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(frame), cellview);

	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 3);

	/* default account info. */
	gchar *account_text;

	pixbuf = hybrid_create_round_pixbuf(account->icon_data,
			account->icon_data_len, 32);

	if (account->nickname) {
		account_text = g_strdup_printf("<b>%s</b> (%s):",
				account->nickname, account->username);

	} else {
		account_text = g_strdup_printf("%s:", account->username);
	}

	gtk_list_store_set(store, &iter,
			BUDDY_REQ_NAME_COLUMN, account_text,
			BUDDY_REQ_ICON_COLUMN, pixbuf, -1);

	g_free(account_text);
	g_object_unref(pixbuf);

	/* textview */
	GtkWidget *scroll;
	GtkTextBuffer *buffer;
	GtkTextIter text_iter;

	str = g_strdup_printf(_("%s(%s) wants to add you as a friend."),
				req->buddy_name ? req->buddy_name : "", req->buddy_id);
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);

	req->textview = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(req->textview), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(req->textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(req->textview) , GTK_WRAP_CHAR);

	gtk_container_add(GTK_CONTAINER(scroll), req->textview);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(req->textview));

	gtk_text_buffer_get_end_iter(buffer, &text_iter);
	gtk_text_buffer_insert(buffer, &text_iter, str, strlen(str));

	/* radio button. */
	GSList *group = NULL;
	GtkWidget *radio;
	GtkWidget *hbox;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

	radio = gtk_radio_button_new_with_label(group, _("Accept"));
	g_signal_connect(radio, "toggled", G_CALLBACK(accept_toggled_cb), req);
	gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 2);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio));
	radio = gtk_radio_button_new_with_label(group, _("Decline"));
	g_signal_connect(radio, "toggled", G_CALLBACK(decline_toggled_cb), req);
	gtk_box_pack_start(GTK_BOX(hbox), radio, FALSE, FALSE, 2);

	/* alias name */
	GtkWidget *table;
	table = gtk_table_new(2, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),
			_("<span size='small'><b>Alias(Optional):</b></span>"));
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);

	req->alias_entry = gtk_entry_new();
	if (req->buddy_name) {
		gtk_entry_set_text(GTK_ENTRY(req->alias_entry), req->buddy_name);
	}
	gtk_widget_set_size_request(req->alias_entry, 270, 30);
	gtk_table_attach_defaults(GTK_TABLE(table), req->alias_entry, 1, 2, 0, 1);

	/* add-to-group combo box. */
	GtkTreeModel *model;

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), 
			_("<span size='small'><b>Add To Group:</b></span>"));
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);

	model = create_group_model(account);
	req->group_combo = gtk_combo_box_new_with_model(model);
	g_object_unref(model);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(req->group_combo),
								renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(req->group_combo),
								renderer, "text", 
								BUDDYREQ_GROUP_NAME_COLUMN, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(req->group_combo), 0);
	gtk_table_attach_defaults(GTK_TABLE(table), req->group_combo, 1, 2, 1, 2);

	/* action area. */
	action_area = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(_("OK"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(add_cb), req);

	button = gtk_button_new_with_label(_("Cancel"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked", G_CALLBACK(cancel_cb), req->window);


}

HybridBuddyReqWindow*
hybrid_buddy_request_window_create(HybridAccount *account, const gchar *id,
		const gchar *name)
{
	HybridBuddyReqWindow *req;

	g_return_val_if_fail(account != NULL, NULL);
	g_return_val_if_fail(id != NULL, NULL);
	
	req = g_new0(HybridBuddyReqWindow, 1);
	req->buddy_id = g_strdup(id);
	req->buddy_name = g_strdup(name);
	req->account = account;
	req->accept = TRUE;

	req->window = hybrid_create_window(_("Buddy-Add Request"), NULL,
					GTK_WIN_POS_CENTER, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(req->window), 5);
	gtk_widget_set_size_request(req->window, 400, 250);	
	g_signal_connect(req->window, "destroy", G_CALLBACK(destroy_cb), req);

	req_window_init(req);

	gtk_widget_show_all(req->window);

	return req;
}

void
hybrid_buddy_request_set_user_data(HybridBuddyReqWindow *req, gpointer user_data,
		DestroyNotify notify)
{
	g_return_if_fail(req != NULL);
	
	req->user_data = user_data;
	req->notify = notify;
}

gpointer
hybrid_buddy_request_get_user_data(HybridBuddyReqWindow *req)
{
	g_return_val_if_fail(req != NULL, NULL);

	return req->user_data;
}
