#include "groupadd.h"
#include "gtkutils.h"
#include "account.h"

extern GSList *account_list;

enum {
	GROUPADD_ICON_COLUMN,
	GROUPADD_NAME_COLUMN,
	GROUPADD_ACCOUNT_COLUMN,
	GROUPADD_COLUMNS
};

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

	store = gtk_list_store_new(GROUPADD_COLUMNS, 
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
							GROUPADD_ICON_COLUMN, pixbuf,
							GROUPADD_NAME_COLUMN, name,
							GROUPADD_ACCOUNT_COLUMN, account,
							-1);

		g_free(name);
		g_object_unref(pixbuf);
	}

	return GTK_TREE_MODEL(store);
}

/**
 * Callback function of the window destroy signal.
 */
static void
destroy_cb(GtkWidget *widget, HybridGroupAddWindow *window)
{
	g_free(window);
}

/**
 * Callback function of the save button click signal.
 */
static void
save_cb(GtkWidget *widget, HybridGroupAddWindow *window)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	HybridAccount *account;
	HybridModule *proto;
	const gchar *name;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(window->account_combo));

	if (!gtk_combo_box_get_active_iter(
				GTK_COMBO_BOX(window->account_combo), &iter)) {

		hybrid_debug_error("groupadd", "no account was choosed.");

		return;
	}

	gtk_tree_model_get(model, &iter,
					GROUPADD_ACCOUNT_COLUMN, &account,
					-1);

	name = gtk_entry_get_text(GTK_ENTRY(window->name_entry));

	if (!name || *name == '\0') {

		hybrid_debug_error("groupadd", "no group name was specified.");

		return;
	}

	proto = account->proto;

	/* call the protocol hook function. */
	if (proto->info->group_add) {
		proto->info->group_add(account, name);
	}

	/* destroy the groupadd window. */
	gtk_widget_destroy(window->window);
}

/**
 * Callback function of the cancel click signal.
 */
static void
cancel_cb(GtkWidget *widget, HybridGroupAddWindow *window)
{
	gtk_widget_destroy(window->window);
}

static void
groupadd_window_init(HybridGroupAddWindow *window)
{
	GtkWidget *vbox;
	GtkWidget *fixed;
	GtkWidget *action_area;
	GtkWidget *label;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkWidget *button;
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window->window), vbox);

	fixed = gtk_fixed_new();
	gtk_box_pack_start(GTK_BOX(vbox), fixed, TRUE, TRUE, 0);

	pixbuf = hybrid_create_default_icon(64);
	image = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	gtk_fixed_put(GTK_FIXED(fixed), image, 20, 40);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Please choose an account:</b>"));
	gtk_fixed_put(GTK_FIXED(fixed), label, 110, 20);

	model = create_account_model();
	window->account_combo = gtk_combo_box_new_with_model(model);
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
	g_object_unref(model);
	gtk_fixed_put(GTK_FIXED(fixed), window->account_combo, 110, 45);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), _("<b>Please input the group name:</b>"));
	gtk_fixed_put(GTK_FIXED(fixed), label, 110, 80);

	window->name_entry = gtk_entry_new();
	gtk_widget_set_usize(window->name_entry, 270, 30);
	gtk_fixed_put(GTK_FIXED(fixed), window->name_entry, 110, 105);

	action_area = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 0);

	button = gtk_button_new_with_label(_("Save"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(save_cb), window);

	button = gtk_button_new_with_label(_("Cancel"));
	gtk_widget_set_usize(button, 100, 30);
	gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
	g_signal_connect(button, "clicked", G_CALLBACK(cancel_cb), window);
}

HybridGroupAddWindow*
hybrid_groupadd_window_create()
{
	HybridGroupAddWindow *group;

	group = g_new0(HybridGroupAddWindow, 1);

	group->window = hybrid_create_window(_("Add Group"), NULL, 
						GTK_WIN_POS_CENTER, FALSE);
	gtk_widget_set_usize(group->window, 420, 200);
	gtk_container_set_border_width(GTK_CONTAINER(group->window), 5);
	g_signal_connect(group->window, "destroy", G_CALLBACK(destroy_cb), group);

	/* init the window. */
	groupadd_window_init(group);

	gtk_widget_show_all(group->window);

	return group;
}
