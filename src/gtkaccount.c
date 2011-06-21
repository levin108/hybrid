#include "util.h"
#include "module.h"
#include "gtkaccount.h"
#include "gtkutils.h"

/* Protocol combo box columns. */
enum {
	PROTOCOL_ICON_COLUMN,
	PROTOCOL_NAME_COLUMN,
	PROTOCOL_COLUMNS
};

static HybridAccountEditPanel *create_account_edit_panel(
		HybridAccountPanel *parent, gboolean is_add);
static void add_cb(GtkWidget *widget, gpointer user_data);
static void delete_cb(GtkWidget *widget, gpointer user_data);

static HybridAccountPanel *account_panel = NULL;

static void
enable_toggled_cb(GtkCellRendererToggle *cell, gchar *path_str,
		gpointer user_data)
{
	HybridAccountPanel *panel = (HybridAccountPanel*)user_data;
	GtkTreeModel *model;
	GtkTreeIter  iter;
	GtkTreePath *path;
	gboolean fixed;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(panel->account_tree));
	path = gtk_tree_path_new_from_string(path_str);

	/* get toggled iter */
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, HYBRID_ENABLE_COLUMN, &fixed, -1);

	/* do something with the value */
	fixed ^= 1;

	/* set new value */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			HYBRID_ENABLE_COLUMN, fixed, -1);

	/* clean up */
	gtk_tree_path_free(path);
}

static void
render_column(HybridAccountPanel *panel)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *treeview;

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
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	HybridAccount *account;
	extern GSList *account_list;
	GSList *pos;

	g_return_if_fail(panel != NULL);

	for (pos = account_list; pos; pos = pos->next) {
		
		account = (HybridAccount*)pos->data;
		pixbuf = hybrid_create_proto_icon(account->proto->info->name, 16);
		gtk_list_store_append(GTK_LIST_STORE(panel->account_store), &iter);
		gtk_list_store_set(GTK_LIST_STORE(panel->account_store), &iter,
				HYBRID_ENABLE_COLUMN, TRUE,
				HYBRID_NAME_COLUMN, account->username,
				HYBRID_PROTO_ICON_COLUMN, pixbuf,
				HYBRID_PROTO_NAME_COLUMN, account->proto->info->name, -1);

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
	HybridAccountPanel *panel;
	GtkWidget *scroll;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *halign;
	GtkWidget *action_area;

	if (account_panel) {
		gtk_window_present(GTK_WINDOW(account_panel->window));
		return account_panel;
	}

	panel = g_new0(HybridAccountPanel, 1);

	account_panel = panel;

	panel->window = hybrid_create_window(_("Manage Account"), NULL,
					GTK_WIN_POS_CENTER, FALSE);
	gtk_widget_set_usize(panel->window, 500, 300);
	g_object_set(panel->window, "border-width", 8, NULL);

	vbox = gtk_vbox_new(FALSE, 0);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_usize(scroll, 200, 0);
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

	halign = gtk_alignment_new(1, 0, 0, 0);
	action_area = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(halign), action_area);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(_("Add"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);
	g_signal_connect(button, "clicked", G_CALLBACK(add_cb), panel);

	button = gtk_button_new_with_label(_("Delete"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);
	g_signal_connect(button, "clicked", G_CALLBACK(delete_cb), panel);

	button = gtk_button_new_with_label(_("Modify"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);

	button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_usize(button, 100, 30);
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
		gtk_tree_store_set(store, &iter, PROTOCOL_ICON_COLUMN, pixbuf,
						PROTOCOL_NAME_COLUMN, module->info->name, -1);
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
	HybridAccountEditPanel *panel;
	HybridAccount *account;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	const gchar *username;
	const gchar *password;
	gchar *protoname;

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
	hybrid_account_update(account);

	if (account_panel) {
		/* Add an account to the account list panel. */
		pixbuf = hybrid_create_proto_icon(protoname, 16);
		model = gtk_tree_view_get_model(
				GTK_TREE_VIEW(account_panel));
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				HYBRID_ENABLE_COLUMN, TRUE,
				HYBRID_NAME_COLUMN, username,
				HYBRID_PROTO_ICON_COLUMN, pixbuf,
				HYBRID_PROTO_NAME_COLUMN, protoname, -1);

		g_object_unref(pixbuf);
	}

	g_free(protoname);

	/* Destroy Edit Panel. */
	gtk_widget_destroy(panel->window);
	g_free(panel);
}

static HybridAccountEditPanel*
create_account_edit_panel(HybridAccountPanel *parent, gboolean is_add)
{
	GtkWidget *fixed;
	GtkWidget *label;
	GtkWidget *notebook;
	GtkWidget *vbox;
	HybridAccountEditPanel *panel;
	GtkWidget *halign;
	GtkWidget *action_area;
	GtkWidget *button;
	GtkCellRenderer *renderer;
	GtkTreeModel *proto_model;

	panel = g_new0(HybridAccountEditPanel, 1);

	panel->parent = parent;
	panel->window = hybrid_create_window(
				is_add ? _("Add a new account") : _("Edit the account"),
				NULL, GTK_WIN_POS_CENTER, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 8);
	gtk_widget_set_usize(panel->window, 300, 400);

	/* Global VBox */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(panel->window), vbox);

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 5);

	/* Basic Page. */
	label = gtk_label_new("Basic");
	fixed = gtk_fixed_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), fixed, label);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Login Options</b>"));
	gtk_fixed_put(GTK_FIXED(fixed), label, 20, 20);


	label = gtk_label_new(_("Protocol:"));
	gtk_fixed_put(GTK_FIXED(fixed), label, 20, 60);

	proto_model = create_protocol_model();
	panel->proto_combo = gtk_combo_box_new_with_model(proto_model);
	gtk_combo_box_set_active(GTK_COMBO_BOX(panel->proto_combo), 0);
	g_object_unref(proto_model);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(panel->proto_combo),
			renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(panel->proto_combo),
			renderer, "pixbuf", PROTOCOL_ICON_COLUMN, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(panel->proto_combo),
			renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(panel->proto_combo),
			renderer, "text", PROTOCOL_NAME_COLUMN, NULL);

	gtk_widget_set_usize(panel->proto_combo, 150, 30);
	gtk_fixed_put(GTK_FIXED(fixed), panel->proto_combo, 100, 55);

	label = gtk_label_new(_("Username:"));
	gtk_fixed_put(GTK_FIXED(fixed), label, 20, 95);

	panel->username_entry = gtk_entry_new();
	gtk_widget_set_usize(panel->username_entry, 150, 25);
	gtk_fixed_put(GTK_FIXED(fixed), panel->username_entry, 100, 95);

	label = gtk_label_new(_("Password:"));
	gtk_fixed_put(GTK_FIXED(fixed), label, 20, 130);

	panel->password_entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(panel->password_entry) , FALSE);
	gtk_widget_set_usize(panel->password_entry, 150, 25);
	gtk_fixed_put(GTK_FIXED(fixed), panel->password_entry, 100, 130);

	/* Action Area */
	halign = gtk_alignment_new(1, 0, 0, 0);
	action_area = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(halign), action_area);

	button = gtk_button_new_with_label(_("Cancel"));
	gtk_widget_set_usize(button, 80, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked",
			G_CALLBACK(edit_account_cancel_cb), panel);

	button = gtk_button_new_with_label(_("Save"));
	gtk_widget_set_usize(button, 80, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 0);
	g_signal_connect(button, "clicked",
			G_CALLBACK(edit_account_save_cb), panel);


	gtk_widget_show_all(panel->window);

	return panel;
}

static void
add_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountPanel *panel;
	HybridAccountEditPanel *edit_panel;

	panel = (HybridAccountPanel*)user_data;

	edit_panel = create_account_edit_panel(panel, TRUE);
}

static void
delete_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountPanel *panel;
	GtkTreeView *tree;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gchar *username;
	gchar *protoname;

	panel = (HybridAccountPanel*)user_data;

	tree = GTK_TREE_VIEW(panel->account_tree);

	selection = gtk_tree_view_get_selection(tree);

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, HYBRID_NAME_COLUMN, &username,
					HYBRID_PROTO_NAME_COLUMN, &protoname, -1);

		hybrid_account_remove(protoname, username);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		g_free(username);
		g_free(protoname);
	}
}
