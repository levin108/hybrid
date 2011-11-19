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
#include "module.h"
#include "gtkaccount.h"
#include "gtkutils.h"
#include "conv.h"
#include "action.h"

/* Protocol combo box columns. */
enum {
	PROTOCOL_ICON_COLUMN,
	PROTOCOL_NAME_COLUMN,
	PROTOCOL_COLUMNS
};

extern GtkUIManager *menu_ui_manager;

static HybridAccountEditPanel *create_account_edit_panel(
		HybridAccountPanel *parent, gboolean is_add);
static void add_cb(GtkWidget *widget, gpointer user_data);
static void delete_cb(GtkWidget *widget, gpointer user_data);
static void disable_cb(GtkWidget *widget, HybridAccount *account);
static void enable_cb(GtkWidget *widget, HybridAccount *account);
static void paint_edit_area(HybridAccountEditPanel *panel);
static HybridAccountPanel *account_panel = NULL;

extern GSList *account_list;

static void
enable_account(HybridAccount *account)
{
	hybrid_account_set_enabled(account, TRUE);

	hybrid_account_enable(account);
}

static void
enable_toggled_cb(GtkCellRendererToggle *cell, gchar *path_str,
		gpointer user_data)
{
	HybridAccountPanel	*panel = (HybridAccountPanel*)user_data;
	GtkTreeModel		*model;
	GtkTreeIter			 iter;
	GtkTreePath			*path;
	gboolean			 fixed;
	gchar				*username;
	HybridAccount		*account;
	GSList				*pos   = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(panel->account_tree));
	path = gtk_tree_path_new_from_string(path_str);

	/* get toggled iter */
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter,
			HYBRID_ENABLE_COLUMN, &fixed,
			HYBRID_NAME_COLUMN, &username, -1);
	gtk_tree_path_free(path);

	fixed ^= 1;

	/* set new value */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			HYBRID_ENABLE_COLUMN, fixed, -1);

	/* find the account by username */
	for (pos = account_list; pos; pos = pos->next) {

		account = (HybridAccount*)pos->data;

		if (g_strcmp0(account->username, username) == 0) {

			break;
		}
	}

	if (!account) { /**< not found... */

		hybrid_debug_error("account", "FATAL account not found");
		g_free(username);

		return;
	}

	if (fixed) { /**< enable the account */
		enable_account(account);

	} else { /**< disable the account */
		hybrid_account_close(account);
	}

	g_free(username);

	/* clean up */
}

static void
render_column(HybridAccountPanel *panel)
{
	GtkTreeViewColumn	*column;
	GtkCellRenderer		*renderer;
	GtkWidget			*treeview;

	treeview = panel->account_tree;

	/* expander columns */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_column_set_visible(column, FALSE);
	gtk_tree_view_set_expander_column(GTK_TREE_VIEW(treeview), column);

	/* enable column */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(column), _("Enabled"));

	renderer = gtk_cell_renderer_toggle_new();
	g_object_set(renderer, "xalign", 0.0, "xpad", 20, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "active", HYBRID_ENABLE_COLUMN, NULL);
	g_signal_connect(renderer, "toggled",
			G_CALLBACK(enable_toggled_cb), panel);

	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));
	gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(column), _("Username"));

	/* name column */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "markup", HYBRID_NAME_COLUMN, NULL);

	/* proto column */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));
	gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(column), _("Protocol"));

	/* proto renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"pixbuf", HYBRID_PROTO_ICON_COLUMN, NULL);
	g_object_set(renderer, "xalign", 1.0, "xpad", 3, "ypad", 0, NULL);

	/* proto name */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"markup", HYBRID_PROTO_NAME_COLUMN, NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 3, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

}

static void
hybrid_account_panel_init(HybridAccountPanel *panel)
{
	extern GSList		*account_list;
	GtkTreeIter			 iter;
	GdkPixbuf			*pixbuf;
	HybridAccount		*account;
	GSList				*pos;

	g_return_if_fail(panel != NULL);

	for (pos = account_list; pos; pos = pos->next) {
		
		account = (HybridAccount*)pos->data;
		pixbuf	= hybrid_create_proto_icon(account->proto->info->name, 16);
		
		gtk_list_store_append(GTK_LIST_STORE(panel->account_store), &iter);
		gtk_list_store_set(GTK_LIST_STORE(panel->account_store), &iter,
						   HYBRID_ENABLE_COLUMN, TRUE,
						   HYBRID_NAME_COLUMN, account->username,
						   HYBRID_PROTO_ICON_COLUMN, pixbuf,
						   HYBRID_PROTO_NAME_COLUMN, account->proto->info->name,
						   HYBRID_ENABLE_COLUMN, account->enabled, -1);

		g_object_unref(pixbuf);
	}

}

static void
close_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountPanel *panel = (HybridAccountPanel*)user_data;

	gtk_widget_destroy(panel->window);

	g_free(panel);

	account_panel = NULL;
}

HybridAccountPanel*
hybrid_account_panel_create()
{
	HybridAccountPanel	*panel;
	GtkWidget			*scroll;
	GtkWidget			*vbox;
	GtkWidget			*button;
	GtkWidget			*halign;
	GtkWidget			*action_area;

	if (account_panel) {
		gtk_window_present(GTK_WINDOW(account_panel->window));
		return account_panel;
	}

	panel = g_new0(HybridAccountPanel, 1);

	account_panel = panel;

	panel->window = hybrid_create_window(_("Manage Account"), NULL,
					GTK_WIN_POS_CENTER, FALSE);
	gtk_widget_set_size_request(panel->window, 500, 300);
	g_object_set(panel->window, "border-width", 8, NULL);

	vbox = gtk_vbox_new(FALSE, 0);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_size_request(scroll, 200, 0);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
								GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
								 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	panel->account_store = gtk_list_store_new(HYBRID_ACCOUNT_COLUMNS,
											  G_TYPE_BOOLEAN,
											  G_TYPE_STRING,
											  GDK_TYPE_PIXBUF,
											  G_TYPE_STRING,
											  G_TYPE_POINTER);
	panel->account_tree = gtk_tree_view_new_with_model(
							GTK_TREE_MODEL(panel->account_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(panel->account_tree), TRUE);
	gtk_container_add(GTK_CONTAINER(scroll), panel->account_tree);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

	render_column(panel);

	halign		= gtk_alignment_new(1, 0, 0, 0);
	action_area = gtk_hbox_new(FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(halign), action_area);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(_("Add"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);
	g_signal_connect(button, "clicked", G_CALLBACK(add_cb), panel);

	button = gtk_button_new_with_label(_("Delete"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);
	g_signal_connect(button, "clicked", G_CALLBACK(delete_cb), panel);

	button = gtk_button_new_with_label(_("Modify"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);

	button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);
	g_signal_connect(button, "clicked", G_CALLBACK(close_cb), panel);

	gtk_container_add(GTK_CONTAINER(panel->window), vbox);

	hybrid_account_panel_init(panel);

	gtk_widget_show_all(panel->window);

	return panel;
}

static GtkTreeModel*
create_protocol_model()
{
	GtkTreeStore *store;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	HybridModule *module;
	GSList *pos;
	extern GSList *modules;

	store = gtk_tree_store_new(PROTOCOL_COLUMNS,
				GDK_TYPE_PIXBUF,
				G_TYPE_STRING);

	
	for (pos = modules; pos; pos = pos->next) {
		module = (HybridModule*)pos->data;
		pixbuf = hybrid_create_proto_icon(module->info->name, 16);
		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter,
						   PROTOCOL_ICON_COLUMN, pixbuf,
						   PROTOCOL_NAME_COLUMN, module->info->name,
						   -1);
		g_object_unref(pixbuf);
	}

	return GTK_TREE_MODEL(store);
}

static void
edit_account_cancel_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountEditPanel *panel;

	panel = (HybridAccountEditPanel*)user_data;

	gtk_widget_destroy(panel->window);

	g_free(panel);
}

static void
edit_account_save_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountEditPanel		*panel;
	HybridAccount				*account;
	GtkTreeModel				*model;
	GtkTreeIter					 iter;
	GdkPixbuf					*pixbuf;
	HybridAccountVariable		*var;
	GSList						*pos;
	const gchar					*username;
	const gchar					*password;
	gchar						*protoname;

	panel = (HybridAccountEditPanel*)user_data;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(panel->proto_combo));

	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(panel->proto_combo), &iter);

	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			PROTOCOL_NAME_COLUMN, &protoname, -1);

	username = gtk_entry_get_text(GTK_ENTRY(panel->username_entry));
	password = gtk_entry_get_text(GTK_ENTRY(panel->password_entry));

	if (!username || *username == '\0' ||
		!password || *password == '\0' ||
		!protoname || *protoname == '\0') {

		g_free(protoname);

		return;
	}

	if (!(account = hybrid_account_get(protoname, username))) {
		g_free(protoname);
		return;
	}

	hybrid_account_set_password(account, password);
	hybrid_account_set_enabled(account, TRUE);

	hybrid_account_update(account);

	if (account_panel) {
		/* Add an account to the account list panel. */
		pixbuf = hybrid_create_proto_icon(protoname, 16);
		model = gtk_tree_view_get_model(
				GTK_TREE_VIEW(account_panel->account_tree));
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
						   HYBRID_ENABLE_COLUMN, TRUE,
						   HYBRID_NAME_COLUMN, username,
						   HYBRID_PROTO_ICON_COLUMN, pixbuf,
						   HYBRID_PROTO_NAME_COLUMN, protoname, -1);

		g_object_unref(pixbuf);
	}

	/* create account's menus */
	hybrid_account_create_menu(account);

	/* set user-defined variables. */
	for (pos = account->proto->option_list; pos; pos = pos->next) {

		const gchar		*str_value;
		gint			 int_value;
		gboolean		 bool_value;

		var	 = (HybridAccountVariable*)pos->data;

		switch (var->type) {
		case VARIABLE_TYPE_STRING:
			str_value  = gtk_entry_get_text(GTK_ENTRY(var->widget));
			hybrid_account_set_string_variable(account, var->name, str_value);
			break;
		case VARIABLE_TYPE_BOOLEAN:
			bool_value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(var->widget));
			hybrid_account_set_bool_variable(account, var->name, bool_value);
			break;
		case VARIABLE_TYPE_INTEGER:
			str_value  = gtk_entry_get_text(GTK_ENTRY(var->widget));
			int_value  = atoi(str_value);
			hybrid_account_set_int_variable(account, var->name, int_value);
			break;
		default:
			break;
		}
		
		var->widget = NULL;
	}

	/* enable the account */
	hybrid_account_enable(account);

	g_free(protoname);

	/* Destroy Edit Panel. */
	gtk_widget_destroy(panel->window);
	g_free(panel);
}

static void
proto_combo_changed(GtkComboBox *widget, HybridAccountEditPanel *panel)
{
	gtk_widget_destroy(panel->user_table);

	panel->user_table = gtk_table_new(2, 8, FALSE);
	gtk_box_pack_start(GTK_BOX(panel->basic_vbox),
					   panel->user_table, TRUE, TRUE, 0);

   	paint_edit_area(panel);
	gtk_widget_show_all(panel->user_table);
}

static void
paint_edit_area(HybridAccountEditPanel *panel)
{
	GtkWidget					*table;
	GtkWidget					*label;
	GtkWidget					*entry;
	GtkWidget					*align;
	GtkTreeIter					 iter;
	GtkTreeModel				*model;
	HybridModule				*module;
	gchar						*name;
	GSList						*pos;
	HybridAccountVariable		*var;
	int							 i;

	table				  = panel->user_table;
	align				  = gtk_alignment_new(1.0, 0.5, 0.0, 0.0);
	label				  = gtk_label_new(_("Username:"));
	panel->username_entry = gtk_entry_new();

	gtk_container_add(GTK_CONTAINER(align), label);
	gtk_widget_set_size_request(panel->username_entry, 150, 25);
	gtk_table_attach(GTK_TABLE(table), align, 0, 1, 1, 2,
					 GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), panel->username_entry,
					 1, 2, 1, 2, GTK_FILL, GTK_FILL, 5, 5);

	label				  = gtk_label_new(_("Password:"));
	align				  = gtk_alignment_new(1.0, 0.5, 0.0, 0.0);
	panel->password_entry = gtk_entry_new();

	gtk_container_add(GTK_CONTAINER(align), label);
	gtk_entry_set_visibility(GTK_ENTRY(panel->password_entry) , FALSE);
	gtk_widget_set_size_request(panel->password_entry, 150, 25);
	gtk_table_attach(GTK_TABLE(table), align, 0, 1, 2, 3,
					 GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), panel->password_entry,
					 1, 2, 2, 3, GTK_FILL, GTK_FILL, 5, 5);

	/* load user-defined options. */
	if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(panel->proto_combo), &iter)) {
		goto no_module;
	}
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(panel->proto_combo));
	gtk_tree_model_get(model, &iter, PROTOCOL_NAME_COLUMN, &name, -1);

	if (!(module = hybrid_module_find(name))) {
		g_free(name);
		goto no_module;
	}

	for (pos = module->option_list, i = 4; pos; pos = pos->next, i ++) {
		var = (HybridAccountVariable*)pos->data;
		if (VARIABLE_TYPE_STRING == var->type) {
			
			label = gtk_label_new(var->title);
			align = gtk_alignment_new(1.0, 0.5, 0.0, 0.0);
			gtk_container_add(GTK_CONTAINER(align), label);
			gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

			entry = gtk_entry_new();
			gtk_widget_set_size_request(entry, 150, 25);
			gtk_table_attach(GTK_TABLE(table), align, 0, 1, i, i + 1,
							 GTK_FILL, GTK_FILL, 5, 5);
			gtk_table_attach(GTK_TABLE(table), entry,
							 1, 2, i, i + 1, GTK_FILL, GTK_FILL, 5, 5);
			
			var->widget = entry;
			
		} else if (VARIABLE_TYPE_BOOLEAN == var->type) {
			
			align = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
			label = gtk_check_button_new_with_label(var->title);
			gtk_container_add(GTK_CONTAINER(align), label);
			gtk_table_attach(GTK_TABLE(table), align,
							 1, 2, i, i + 1, GTK_FILL, GTK_FILL, 5, 5);

			var->widget = label;
		}
	}
	
	g_free(name);

 no_module:
	
	/* Just to fill the space. */
	label = gtk_label_new(NULL);
	gtk_table_attach(GTK_TABLE(table), label, 0, 2, 7, 8, GTK_EXPAND, GTK_EXPAND, 0, 0);

}

static HybridAccountEditPanel*
create_account_edit_panel(HybridAccountPanel *parent, gboolean is_add)
{
	GtkWidget			*label;
	GtkWidget			*notebook;
	GtkWidget			*vbox;
	GtkWidget			*hbox;
	GtkWidget			*halign;
	GtkWidget			*action_area;
	GtkWidget			*button;
	GtkWidget			*align;
	GtkCellRenderer		*renderer;
	GtkTreeModel		*proto_model;
	
	HybridAccountEditPanel		*panel;

	panel = g_new0(HybridAccountEditPanel, 1);

	panel->parent = parent;
	panel->window = hybrid_create_window(
				is_add ? _("Add a new account") : _("Edit the account"),
				NULL, GTK_WIN_POS_CENTER, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 8);
   	gtk_widget_set_size_request(panel->window, 300, 400);

	/* Global VBox */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(panel->window), vbox);

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 5);

	/* Basic Page. */
	label			  = gtk_label_new("Basic");
	panel->basic_vbox = gtk_vbox_new(FALSE, 0);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), panel->basic_vbox, label);

	align			   = gtk_alignment_new(1.0, 0.5, 0.0, 0.0);
	label			   = gtk_label_new(_("Protocol:"));
	proto_model		   = create_protocol_model();
	panel->proto_combo = gtk_combo_box_new_with_model(proto_model);

	gtk_combo_box_set_active(GTK_COMBO_BOX(panel->proto_combo), 0);
	g_object_unref(proto_model);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(panel->proto_combo),
							   renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(panel->proto_combo),
								   renderer, "pixbuf",
								   PROTOCOL_ICON_COLUMN, NULL);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(panel->proto_combo),
							   renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(panel->proto_combo),
								   renderer, "text",
								   PROTOCOL_NAME_COLUMN, NULL);
	gtk_widget_set_size_request(panel->proto_combo, 170, 30);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(align), label);
	gtk_box_pack_start(GTK_BOX(hbox), align, TRUE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), panel->proto_combo, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(panel->basic_vbox), hbox, FALSE, FALSE, 10);

	panel->user_table = gtk_table_new(2, 8, FALSE);
	gtk_box_pack_start(GTK_BOX(panel->basic_vbox), panel->user_table, TRUE, TRUE, 0);
	
	paint_edit_area(panel);

	/* Action Area */
	halign = gtk_alignment_new(1, 0, 0, 0);
	action_area = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(halign), action_area);

	button = gtk_button_new_with_label(_("Cancel"));
	gtk_widget_set_size_request(button, 80, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked",
					 G_CALLBACK(edit_account_cancel_cb), panel);

	button = gtk_button_new_with_label(_("Save"));
	gtk_widget_set_size_request(button, 80, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked",
					 G_CALLBACK(edit_account_save_cb), panel);


	gtk_widget_show_all(panel->window);
		
	g_signal_connect(panel->proto_combo, "changed",
					 G_CALLBACK(proto_combo_changed), panel);

	return panel;
}

static void
add_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountPanel			*panel;
	HybridAccountEditPanel		*edit_panel;

	panel = (HybridAccountPanel*)user_data;

	edit_panel = create_account_edit_panel(panel, TRUE);
}

static void
delete_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountPanel	*panel;
	HybridAccount		*account;
	GtkTreeView			*tree;
	GtkTreeModel		*model;
	GtkTreeSelection	*selection;
	GtkTreeIter			 iter;
	gchar				*username;
	gchar				*protoname;

	panel = (HybridAccountPanel*)user_data;

	tree = GTK_TREE_VIEW(panel->account_tree);

	selection = gtk_tree_view_get_selection(tree);


	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {

		gtk_tree_model_get(model, &iter, 
					HYBRID_NAME_COLUMN, &username,
					HYBRID_PROTO_NAME_COLUMN, &protoname,
					-1);

		if (!(account = hybrid_account_get(protoname, username))) {
			
			hybrid_debug_error("account", "FATAL, account doesn't exist.");
			
			g_free(username);
			g_free(protoname);

			return;
		}

		/* close the account. */
		hybrid_account_close(account);

		/* remove the menus. */
		hybrid_account_remove_menu(account);

		/* remove the item in the management panel. */
		hybrid_account_remove(protoname, username);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		g_free(username);
		g_free(protoname);
	}
}

static void
change_state_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccount				*account;
	HybridModule				*module;
	HybridIMOps					*im_ops;
	HybridAccountMenuData		*data;

	data = (HybridAccountMenuData*)user_data;
	account = data->account;

	module = account->proto;
	im_ops = module->info->im_ops;

	if (im_ops->change_state) {
		account->state = data->presence_state;

		if (im_ops->change_state(account, data->presence_state)) {
			hybrid_account_set_state(account, data->presence_state);
		}
	}
}

static void
action_cb(GtkWidget *widget, HybridAction *action)
{
	if (action->callback) {
		action->callback(action);
	}
}

static void
enable_cb(GtkWidget *widget, HybridAccount *account)
{
	GtkWidget *sub_menu;

	enable_account(account);

	gtk_widget_destroy(account->enable_menu);

	sub_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(account->account_menu));

	account->enable_menu = hybrid_create_menu(sub_menu, _("Disable Account"),
									NULL, TRUE, G_CALLBACK(disable_cb), account);

	gtk_widget_show(account->enable_menu);
}

static void
disable_cb(GtkWidget *widget, HybridAccount *account)
{
	hybrid_account_close(account);

	hybrid_account_disable_menu(account);
}

static void
create_account_child_menus(HybridAccount *account)
{
	GtkWidget					*account_menu;
	GtkWidget					*menu_shell;
	GtkWidget					*menu_item;
	GtkWidget					*child_menu;
	GtkWidget					*child_menu_item;
	GdkPixbuf					*presence_pixbuf;
	GtkWidget					*presence_image;
	HybridAccountMenuData		*data;
	HybridAction				*action;
	HybridModule				*proto;
	GSList						*pos;
	gint						 state;

	account_menu = account->account_menu;

	menu_shell = gtk_menu_new();

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(account_menu), menu_shell);

	menu_item = hybrid_create_menu(menu_shell,
								   _("Change State"), NULL,
								   HYBRID_IS_CONNECTED(account) ? TRUE: FALSE,
								   NULL, NULL);

	/* change state child menus. */

	child_menu = gtk_menu_new();

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), child_menu);

	for (state = HYBRID_STATE_INVISIBLE; state <= HYBRID_STATE_ONLINE;
			state ++) {

		presence_pixbuf = hybrid_create_presence_pixbuf(state, 16);
		child_menu_item = hybrid_create_menu(child_menu, 
								hybrid_get_presence_name(state), 
								NULL, TRUE, NULL, NULL);
		presence_image = gtk_image_new_from_pixbuf(presence_pixbuf);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(child_menu_item),
				presence_image);
		g_object_unref(presence_pixbuf);

		data = g_new0(HybridAccountMenuData, 1);
		data->account = account;
		data->presence_state = state;

		g_signal_connect(child_menu_item, "activate",
				G_CALLBACK(change_state_cb), data);
	}

	/* create protocol-specified menus. */
	proto = account->proto;

	if (account->action_list) {

		hybrid_create_menu_seperator(menu_shell);

		for (pos = account->action_list; pos; pos = pos->next) {

			action = pos->data;

			hybrid_create_menu(menu_shell, action->text, NULL,
							   HYBRID_IS_CONNECTED(account) ? TRUE: FALSE,
							   G_CALLBACK(action_cb), action);
		}
	}

	hybrid_create_menu_seperator(menu_shell);

	if (account->enabled) {
		account->enable_menu = 
			hybrid_create_menu(menu_shell, _("Disable Account"), NULL,
							TRUE, G_CALLBACK(disable_cb), account);
	} else {
		account->enable_menu = 
			hybrid_create_menu(menu_shell, _("Enable Account"), NULL,
							TRUE, G_CALLBACK(enable_cb), account);
	}
}


void
hybrid_account_create_menu(HybridAccount *account)
{
	gchar		*menu_name;
	GdkPixbuf	*presence_pixbuf;
	GtkWidget	*presence_image;
	GtkWidget	*account_shell;

	g_return_if_fail(account != NULL);

	menu_name = g_strdup_printf("%s (%s)", account->username,
			hybrid_get_presence_name(account->state));
	account->account_menu = 
		hybrid_create_menu(NULL, menu_name, NULL, TRUE, NULL, NULL);
	g_free(menu_name);

	/* set the icon of the account menu */
	presence_pixbuf = hybrid_create_presence_pixbuf(account->state, 16);
	presence_image = gtk_image_new_from_pixbuf(presence_pixbuf);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(account->account_menu),
						presence_image);
	g_object_unref(presence_pixbuf);

	/* load the action list. */
	if (!account->action_list && account->proto->info->actions) {
		account->action_list = account->proto->info->actions(account);
	}

	create_account_child_menus(account);

	if (!(account_shell = 
			gtk_ui_manager_get_widget(menu_ui_manager, "/MenuBar/Account"))) {

		hybrid_debug_error("core", "account menu init err");

		return;
	}

	account_shell = gtk_menu_item_get_submenu(GTK_MENU_ITEM(account_shell));

	gtk_menu_shell_insert(
			GTK_MENU_SHELL(account_shell), account->account_menu, 3);

	gtk_widget_show_all(account_shell);
}

void
hybrid_account_remove_menu(HybridAccount *account)
{
	g_return_if_fail(account != NULL);

	gtk_widget_destroy(account->account_menu);
	account->account_menu = NULL;
}

void
hybrid_account_disable_menu(HybridAccount *account)
{
	GtkWidget *sub_menu;

	g_return_if_fail(account != NULL);

	gtk_widget_destroy(account->enable_menu);

	sub_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(account->account_menu));

	account->enable_menu = hybrid_create_menu(sub_menu, _("Enable Account"),
									NULL, TRUE, G_CALLBACK(enable_cb), account);

	gtk_widget_show(account->enable_menu);

}
