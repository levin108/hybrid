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
					    NULL);
	g_object_set(renderer, "wrap-mode", PANGO_WRAP_CHAR, NULL);
	g_object_set(renderer, "wrap-width",250, NULL);
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
		g_free(item);
	}
}

static void
close_click_cb(GtkWidget *widget, gpointer user_data)
{
	HybridInfo *info = (HybridInfo*)user_data;

	gtk_widget_destroy(info->window);
}

static void
window_destroy_cb(GtkWidget *widget, gpointer user_data)
{
	HybridInfo *info = (HybridInfo*)user_data;
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

	info->store = gtk_list_store_new(HYBRID_INFO_COLUMNS,
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
	gtk_widget_set_size_request(close_button, 100, 30);
	gtk_box_pack_start(GTK_BOX(action_area), close_button, FALSE, TRUE, 0);
	g_signal_connect(close_button, "clicked", G_CALLBACK(close_click_cb), info);

	render_column(info);

	gtk_widget_show_all(info->window);

	return info;
}

void
hybrid_info_add_pair(HybridInfo *info, const gchar *name, const gchar *value)
{
	HybridInfoItem *item;
	GtkTreeIter iter;
	gchar *name_markup;
	gchar *name_escaped;
	gchar *value_escaped;

	g_return_if_fail(info != NULL);
	g_return_if_fail(name != NULL);

	item = hybrid_info_item_create(name, value);
	info->item_list = g_slist_append(info->item_list, item);

	name_escaped = g_markup_escape_text(name, -1);

	if (value) {
		value_escaped = g_markup_escape_text(value, -1);
	} else {
		value_escaped = NULL;
	}

	name_markup = g_strdup_printf("<b>%s:</b>", name_escaped);	

	gtk_list_store_append(info->store, &iter);
	gtk_list_store_set(info->store, &iter,
			HYBRID_INFO_NAME_COLUMN, name_markup,
			HYBRID_INFO_VALUE_COLUMN, value_escaped, -1);

	g_free(name_markup);
	g_free(name_escaped);
	g_free(value_escaped);
}
