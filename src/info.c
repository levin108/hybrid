#include "util.h"
#include "gtkutils.h"
#include "info.h"

static void
render_column(HybirdInfo *info)
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
					    "markup", HYBIRD_INFO_NAME_COLUMN,
					    NULL);
	/* value */
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,
					    "markup", HYBIRD_INFO_VALUE_COLUMN,
					    NULL);
	g_object_set(renderer, "wrap-mode", PANGO_WRAP_CHAR, NULL);
	g_object_set(renderer, "wrap-width",250, NULL);
}

HybirdInfo*
hybird_info_create(HybirdBuddy *buddy)
{
	HybirdInfo *info;
	GdkPixbuf *pixbuf;
	GtkWidget *vbox;
	GtkWidget *scroll;
	GtkWidget *label;
	GtkWidget *halign;
	GtkWidget *action_area;
	GtkWidget *close_button;
	gchar *title;

	g_return_val_if_fail(buddy != NULL, NULL);

	info = g_new0(HybirdInfo, 1);
	info->buddy = buddy;

	info->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_object_set(info->window, "border-width", 10, NULL);
	pixbuf = hybird_create_default_icon(0);
	title = g_strdup_printf(_("%s's information"),
			buddy->name && *(buddy->name) != '\0' ? buddy->name : buddy->id);

	gtk_window_set_icon(GTK_WINDOW(info->window), pixbuf);
	gtk_window_set_position(GTK_WINDOW(info->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(info->window), title);
	gtk_widget_set_usize(info->window, 400, 350);
	gtk_window_set_resizable(GTK_WINDOW(info->window), FALSE);
	g_free(title);

	g_object_unref(pixbuf);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(info->window), vbox);

	/* label */
	label = gtk_label_new(NULL);
	halign = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign), label);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);

	title = g_strdup_printf("<b>Information of %s</b>",
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

	info->store = gtk_list_store_new(HYBIRD_INFO_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING);

	info->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(info->store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(info->treeview), FALSE);
	gtk_container_add(GTK_CONTAINER(scroll), info->treeview);

	/* close button */
	action_area = gtk_hbox_new(FALSE, 0);
	halign = gtk_alignment_new(1, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign), action_area);
	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);

	close_button = gtk_button_new_with_label(_("Close"));
	gtk_widget_set_usize(close_button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), close_button, FALSE, TRUE, 0);
	//g_signal_connect(close_button , "clicked" , G_CALLBACK(fx_chat_on_close_clicked) , fxchat);

	render_column(info);

	GtkTreeIter iter;
	gtk_list_store_append(info->store, &iter);
	gtk_list_store_set(info->store, &iter,
			HYBIRD_INFO_NAME_COLUMN, "<b>Name:</b>",
			HYBIRD_INFO_VALUE_COLUMN, "Hybird", -1);
	gtk_list_store_append(info->store, &iter);
	gtk_list_store_set(info->store, &iter,
			HYBIRD_INFO_NAME_COLUMN, "<b>Mood Phrase:</b>",
			HYBIRD_INFO_VALUE_COLUMN, buddy->mood, -1);
	gtk_list_store_append(info->store, &iter);
	gtk_list_store_set(info->store, &iter,
			HYBIRD_INFO_NAME_COLUMN, "<b>Gender:</b>",
			HYBIRD_INFO_VALUE_COLUMN, "Male", -1);
	gtk_list_store_append(info->store, &iter);
	gtk_list_store_set(info->store, &iter,
			HYBIRD_INFO_NAME_COLUMN, "<b>id:</b>",
			HYBIRD_INFO_VALUE_COLUMN, buddy->id, -1);


	gtk_widget_show_all(info->window);

	return info;
}
