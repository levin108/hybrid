#include "util.h"
#include "gtkaccount.h"
#include "gtkutils.h"

static HybirdAccountEditPanel *create_account_edit_panel(gboolean is_add);
static void add_cb(GtkWidget *widget, gpointer user_data);

static void
enable_toggled_cb(GtkCellRendererToggle *cell, gchar *path_str,
		gpointer user_data)
{
	HybirdAccountPanel *panel = (HybirdAccountPanel*)user_data;
	GtkTreeModel *model;
	GtkTreeIter  iter;
	GtkTreePath *path;
	gboolean fixed;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(panel->account_tree));
	path = gtk_tree_path_new_from_string(path_str);

	/* get toggled iter */
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, HYBIRD_ENABLE_COLUMN, &fixed, -1);

	/* do something with the value */
	fixed ^= 1;

	/* set new value */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			HYBIRD_ENABLE_COLUMN, fixed, -1);

	/* clean up */
	gtk_tree_path_free(path);
}

static void
render_column(HybirdAccountPanel *panel)
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
					    "active", HYBIRD_ENABLE_COLUMN, NULL);
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
					    "markup", HYBIRD_NAME_COLUMN, NULL);

	/* proto column */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));
	gtk_tree_view_column_set_title(GTK_TREE_VIEW_COLUMN(column), _("Protocol"));

	/* proto renderer */
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"pixbuf", HYBIRD_PROTO_ICON_COLUMN, NULL);
	g_object_set(renderer, "xalign", 1.0, "xpad", 3, "ypad", 0, NULL);

	/* proto name */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer,
						"markup", HYBIRD_PROTO_NAME_COLUMN, NULL);
	g_object_set(renderer, "xalign", 0.0, "xpad", 3, "ypad", 0, NULL);
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

}

static void
hybird_account_panel_init(HybirdAccountPanel *panel)
{
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;

	g_return_if_fail(panel != NULL);


	pixbuf = hybird_create_default_icon(16);
	gtk_list_store_append(GTK_LIST_STORE(panel->account_store), &iter);
	gtk_list_store_set(GTK_LIST_STORE(panel->account_store), &iter,
			HYBIRD_ENABLE_COLUMN, TRUE,
			HYBIRD_NAME_COLUMN,"15210634361",
			HYBIRD_PROTO_ICON_COLUMN, pixbuf,
			HYBIRD_PROTO_NAME_COLUMN, "fetion", -1);
	gtk_list_store_append(GTK_LIST_STORE(panel->account_store), &iter);
	gtk_list_store_set(GTK_LIST_STORE(panel->account_store), &iter,
			HYBIRD_ENABLE_COLUMN, TRUE,
			HYBIRD_NAME_COLUMN,"18768140681",
			HYBIRD_PROTO_ICON_COLUMN, pixbuf,
			HYBIRD_PROTO_NAME_COLUMN, "fetion", -1);

	g_object_unref(pixbuf);
}

static void
close_cb(GtkWidget *widget, gpointer user_data)
{
	HybirdAccountPanel *panel = (HybirdAccountPanel*)user_data;

	gtk_widget_destroy(panel->window);

	g_free(panel);
}

HybirdAccountPanel*
hybird_account_panel_create()
{
	HybirdAccountPanel *panel;
	GtkWidget *scroll;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *halign;
	GtkWidget *action_area;

	panel = g_new0(HybirdAccountPanel, 1);

	panel->window = hybird_create_window(_("Manage Account"), NULL,
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

	panel->account_store = gtk_list_store_new(HYBIRD_ACCOUNT_COLUMNS,
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

	button = gtk_button_new_with_label(_("Modify"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);

	button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), button, FALSE, FALSE, 4);
	g_signal_connect(button, "clicked", G_CALLBACK(close_cb), panel);

	gtk_container_add(GTK_CONTAINER(panel->window), vbox);

	hybird_account_panel_init(panel);

	gtk_widget_show_all(panel->window);

	return panel;
}

static HybirdAccountEditPanel*
create_account_edit_panel(gboolean is_add)
{
	GtkWidget *window;
	GtkWidget *fixed;
	HybirdAccountEditPanel *panel;

	panel = g_new0(HybirdAccountEditPanel, 1);

	panel->window = hybird_create_window(
				is_add ? _("Add a new account") : _("Edit the account"),
				NULL, GTK_WIN_POS_CENTER, FALSE);
	gtk_widget_set_usize(panel->window, 300, 400);

	gtk_widget_show_all(panel->window);

	return panel;
}

static void
add_cb(GtkWidget *widget, gpointer user_data)
{

}
