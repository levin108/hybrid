#include "buddyadd.h"
#include "gtkutils.h"
#include "account.h"

enum {
	BUDDYADD_ICON_COLUMN,
	BUDDYADD_NAME_COLUMN,
	BUDDYADD_ACCOUNT_COLUMN,
	BUDDYADD_COLUMNS
};

enum {
	BUDDYADD_GROUP_NAME_COLUMN,
	BUDDYADD_GROUP_GROUP_COLUMN,
	BUDDYADD_GROUP_COLUMNS
};

extern GSList *account_list;

/**
 * Create the tree model for the account combo box.
 */
static GtkTreeModel*
create_account_model(void)
{
	GtkListStore *store;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	HybridAccount *account;
	HybridModule *proto;
	gchar *name;
	gchar *nickname;
	GSList *pos;

	store = gtk_list_store_new(BUDDYADD_COLUMNS, 
							GDK_TYPE_PIXBUF,
							G_TYPE_STRING,
							G_TYPE_POINTER);

	for (pos = account_list; pos; pos = pos->next) {

		account = (HybridAccount*)pos->data;	
		proto   = account->proto;

		if (account->connect_state != HYBRID_CONNECTION_CONNECTED) {
			continue;
		}

		if (account->nickname) {
			nickname = g_strdup_printf("(%s)", account->nickname);

		} else {
			nickname = NULL;
		}

		pixbuf  = hybrid_create_proto_icon(proto->info->name, 16);
		name    = g_strdup_printf("%s %s (%s)", account->username,
						nickname ? nickname : "" , proto->info->name);

		g_free(nickname);

		gtk_list_store_append(store, &iter);

		gtk_list_store_set(store, &iter,
							BUDDYADD_ICON_COLUMN, pixbuf,
							BUDDYADD_NAME_COLUMN, name,
							BUDDYADD_ACCOUNT_COLUMN, account,
							-1);

		g_free(name);
		g_object_unref(pixbuf);
	}

	return GTK_TREE_MODEL(store);
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
	
	store = gtk_list_store_new(BUDDYADD_GROUP_COLUMNS,
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
				BUDDYADD_GROUP_NAME_COLUMN, group->name,
				BUDDYADD_GROUP_GROUP_COLUMN, group,
				-1);
	}

	return GTK_TREE_MODEL(store);
}

/**
 * Callback function of the account combo box changed signal.
 */
static void
account_changed_cb(GtkWidget *widget, HybridBuddyAddWindow *window)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	HybridAccount *account;

	if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)) {
		return;
	}

	if (!window->group_combo) {
		return;
	}

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	
	gtk_tree_model_get(model, &iter,
					BUDDYADD_ACCOUNT_COLUMN, &account,
					-1);

	model = create_group_model(account);
	gtk_combo_box_set_model(GTK_COMBO_BOX(window->group_combo), model);
	g_object_unref(model);

	gtk_combo_box_set_active(GTK_COMBO_BOX(window->group_combo), 0);
}

/**
 * Callback function of the window destroy signal.
 */
static void
destroy_cb(GtkWidget *widget, HybridBuddyAddWindow *window)
{
	g_free(window);
}

static void
add_cb(GtkWidget *widget, HybridBuddyAddWindow *window)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkComboBox *combo;
	GtkTextBuffer *buffer;
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	HybridAccount *account;
	HybridModule *proto;
	HybridGroup *group;
	const gchar *name;
	const gchar *alias;
	const gchar *tips;

	combo = GTK_COMBO_BOX(window->account_combo);

	model = gtk_combo_box_get_model(combo);

	if (!gtk_combo_box_get_active_iter(combo, &iter)) {
		return;
	}

	gtk_tree_model_get(model, &iter, BUDDYADD_ACCOUNT_COLUMN, &account, -1);

	name = gtk_entry_get_text(GTK_ENTRY(window->username_entry));

	if (!name || *name == '\0') {
		return;
	}

	alias = gtk_entry_get_text(GTK_ENTRY(window->localname_entry));

	combo = GTK_COMBO_BOX(window->group_combo);

	model = gtk_combo_box_get_model(combo);

	if (!gtk_combo_box_get_active_iter(combo, &iter)) {
		return;
	}

	gtk_tree_model_get(model, &iter, BUDDYADD_GROUP_GROUP_COLUMN, &group, -1);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(window->tips_textview));
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);

	tips = gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, TRUE);

	proto = account->proto;

	if (proto->info->buddy_add) {
		proto->info->buddy_add(account, group, name, alias, tips);
	}

	gtk_widget_destroy(window->window);
}

/**
 * Callback function of the cancel button clicked signal.
 */
static void
cancel_cb(GtkWidget *widget, HybridBuddyAddWindow *window)
{
	gtk_widget_destroy(window->window);
}

/**
 * Initialize the buddy-add window.
 */
static void
hybrid_buddyadd_window_init(HybridBuddyAddWindow *window)
{
	GtkWidget *vbox;
	GtkWidget *action_area;
	GtkWidget *label;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkWidget *button;
	GtkWidget *table;
	GtkWidget *scroll;

	HybridAccount *account = NULL;
	GtkTreeIter iter;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window->window), vbox);

	table = gtk_table_new(5, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);
	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Account:</b>"));
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);

	model = create_account_model();
	window->account_combo = gtk_combo_box_new_with_model(model);
	g_signal_connect(window->account_combo, "changed",
					G_CALLBACK(account_changed_cb), window);
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(window->account_combo),
								renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(window->account_combo),
								renderer, "pixbuf", 0, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(window->account_combo),
								renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(window->account_combo),
								renderer, "text", 1, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(window->account_combo), 0);

	gtk_widget_set_usize(window->account_combo, 270, 30);
	gtk_table_attach_defaults(GTK_TABLE(table), window->account_combo, 1, 2, 0, 1);

	/* username */
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Username:</b>"));
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);

	window->username_entry = gtk_entry_new();
	gtk_widget_set_usize(window->username_entry, 270, 30);
	gtk_table_attach_defaults(GTK_TABLE(table), window->username_entry, 1, 2, 1, 2);

	/* alias name */
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Alias(Optional):</b>"));
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);

	window->localname_entry = gtk_entry_new();
	gtk_widget_set_usize(window->localname_entry, 270, 30);
	gtk_table_attach_defaults(GTK_TABLE(table), window->localname_entry, 1, 2, 2, 3);

	/* add-to-group combo box. */
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Add To Group:</b>"));
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 3, 4);

	/* get the active account. */
	if (gtk_combo_box_get_active_iter(
				GTK_COMBO_BOX(window->account_combo), &iter)) {
		gtk_tree_model_get(model, &iter,
				BUDDYADD_ACCOUNT_COLUMN, &account,
				-1);
	}

	/* unref the account model. */
	g_object_unref(model);

	model = create_group_model(account);
	window->group_combo = gtk_combo_box_new_with_model(model);
	g_object_unref(model);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(window->group_combo),
								renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(window->group_combo),
								renderer, "text", 
								BUDDYADD_GROUP_NAME_COLUMN, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(window->group_combo), 0);
	gtk_table_attach_defaults(GTK_TABLE(table), window->group_combo, 1, 2, 3, 4);

	/* add-buddy request tips. */
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Tips(Optional):</b>"));
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 4, 5);

	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);
	window->tips_textview = gtk_text_view_new();
	gtk_widget_set_usize(window->tips_textview, 270, 0);
	gtk_container_add(GTK_CONTAINER(scroll), window->tips_textview);
	gtk_table_attach_defaults(GTK_TABLE(table), scroll, 1, 2, 4, 5);

	/* buttons */
	action_area = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(_("Add"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(add_cb), window);

	button = gtk_button_new_with_label(_("Cancel"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(cancel_cb), window);
}

HybridBuddyAddWindow*
hybrid_buddyadd_window_create()
{
	HybridBuddyAddWindow *buddy;
	
	buddy = g_new0(HybridBuddyAddWindow, 1);

	buddy->window = hybrid_create_window(_("Add Buddy"), NULL,
				GTK_WIN_POS_CENTER, FALSE);
	gtk_widget_set_usize(buddy->window, 420, 300);
	gtk_container_set_border_width(GTK_CONTAINER(buddy->window), 8);
	g_signal_connect(buddy->window, "destroy", G_CALLBACK(destroy_cb), buddy);

	hybrid_buddyadd_window_init(buddy);

	gtk_widget_show_all(buddy->window);

	return buddy;
}
