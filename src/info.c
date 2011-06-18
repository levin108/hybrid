#include "util.h"
#include "gtkutils.h"
#include "info.h"

HybirdInfo*
hybird_info_create(HybirdBuddy *buddy)
{
	HybirdInfo *info;
	GdkPixbuf *pixbuf;
	GtkWidget *vbox;
	GtkWidget *scroll;
	gchar *title;

	g_return_val_if_fail(buddy != NULL, NULL);

	info = g_new0(HybirdInfo, 1);
	info->buddy = buddy;

	info->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	pixbuf = hybird_create_default_icon(0);
	title = g_strdup_printf(_("%s's information"),
			buddy->name && *(buddy->name) != '\0' ? buddy->name : buddy->id);

	gtk_window_set_icon(GTK_WINDOW(info->window), pixbuf);
	gtk_window_set_position(GTK_WINDOW(info->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(info->window), title);
	gtk_window_set_default_size(GTK_WINDOW(info->window), 300, 400);

	g_object_unref(pixbuf);
	g_free(title);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(info->window), vbox);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);

	info->store = gtk_tree_store_new(HYBIRD_INFO_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING);

	info->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(info->store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(info->treeview), FALSE);
	gtk_container_add(GTK_CONTAINER(scroll), info->treeview);

	gtk_widget_show_all(info->window);

	return info;
}
