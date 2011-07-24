#include "util.h"
#include "gtkutils.h"
#include "buddyreq.h"

enum {
	BUDDY_REQ_ICON_COLUMN,
	BUDDY_REQ_NAME_COLUMN,
	BUDDY_REQ_COLUMNS
};

static void
destroy_cb(GtkWidget *widget, BuddyReqWindow *req)
{
	g_free(req->buddy_id);
	g_free(req->buddy_name);
	g_free(req);
}

static void
add_cb(GtkWidget *widget, BuddyReqWindow *req)
{

	gtk_widget_destroy(req->window);
}

static void
cancel_cb(GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_destroy(window);
}

static void
req_window_init(BuddyReqWindow *req)
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

	str = g_strdup_printf(_("<b>%s(%s)</b> wants to add you as a friend."),
				req->buddy_name ? req->buddy_name : "", req->buddy_id);
	g_free(str);

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
			account->icon_data_len, 48);

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

	action_area = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 0);


	button = gtk_button_new_with_label(_("Add"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(add_cb), req);

	button = gtk_button_new_with_label(_("Cancel"));
	gtk_widget_set_size_request(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(cancel_cb), req->window);


}

BuddyReqWindow*
hybrid_buddy_request_window_create(HybridAccount *account, const gchar *id,
		const gchar *name)
{
	BuddyReqWindow *req;

	g_return_val_if_fail(id != NULL, NULL);
	
	req = g_new0(BuddyReqWindow, 1);
	req->buddy_id = g_strdup(id);
	req->buddy_name = g_strdup(name);
	req->account = account;

	req->window = hybrid_create_window(_("Buddy-Add Request"), NULL,
					GTK_WIN_POS_CENTER, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(req->window), 5);
	gtk_widget_set_size_request(req->window, 400, 300);	
	g_signal_connect(req->window, "destroy", G_CALLBACK(destroy_cb), req);

	req_window_init(req);

	gtk_widget_show_all(req->window);

	return req;
}
