#include <glib.h>
#include <gtk/gtk.h>
#include "config.h"
#include "module.h"
#include "blist.h"
#include "util.h"
#include "gtkaccount.h"
#include "gtkutils.h"

extern HybridBlist *blist;
extern GSList *account_list;

typedef struct state_menus HybridAccountMenuData;

struct state_menus {
	HybridAccount *account;
	gint presence_state;
};

void
hybrid_start_login()
{
	GSList *pos;
	HybridAccount *account;

	for (pos = account_list; pos; pos = pos->next) {
		account = (HybridAccount*)pos->data;
		account->proto->info->login(account);
	}
}

static void
window_destroy(GtkWidget *widget, gpointer user_data)
{
	/*
	 * Now free the memory.
	 */

	gtk_main_quit();
}

static void
manage_account_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountPanel *panel;

	panel = hybrid_account_panel_create();
}

static void
quit_cb(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}

static void
change_state_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccount *account;
	HybridModule *module;
	HybridAccountMenuData *data;

	data = (HybridAccountMenuData*)user_data;
	account = data->account;

	module = account->proto;

	if (module->info->change_state) {
		account->state = data->presence_state;

		if (module->info->change_state(account,
					data->presence_state)) {
			hybrid_account_set_state(account, data->presence_state);
		}
	}
}

static void
create_account_child_menus(HybridAccount *account)
{
	GtkWidget *account_menu;
	GtkWidget *menu_shell;
	GtkWidget *menu_item;
	GtkWidget *child_menu;
	GtkWidget *child_menu_item;
	GdkPixbuf *presence_pixbuf;
	GtkWidget *presence_image;
	HybridAccountMenuData *data;
	gint state;

	account_menu = account->account_menu;

	menu_shell = gtk_menu_new();

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(account_menu), menu_shell);

	menu_item = hybrid_create_menu(menu_shell, _("Change State"), NULL,
						TRUE, NULL, NULL);

	/* ==== change state child menus start ==== */

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

	/* ==== change state child menus end   ==== */

	hybrid_create_menu_seperator(menu_shell);

	menu_item = hybrid_create_menu(menu_shell, _("Disable Account"), "close",
						TRUE, NULL, NULL);
}

static void
create_account_menus(GtkUIManager *ui)
{
	GtkWidget *account_shell;
	GtkWidget *seperator;
	GdkPixbuf *presence_pixbuf;
	GtkWidget *presence_image;
	HybridAccount *account;
	gchar *menu_name;
	GSList *pos;

	if (!(account_shell = gtk_ui_manager_get_widget(ui, "/MenuBar/Account"))) {
		hybrid_debug_error("core", "account menu init err");
		return;
	}

	account_shell = gtk_menu_item_get_submenu(GTK_MENU_ITEM(account_shell));

	if (account_list) {
		seperator = gtk_separator_menu_item_new();
		gtk_menu_shell_insert(GTK_MENU_SHELL(account_shell), seperator, 2);
	}

	for (pos = account_list; pos; pos = pos->next) {

		/* set the name of the account menu. */
		account = (HybridAccount*)pos->data;
		menu_name = g_strdup_printf("%s (%s)", account->username,
				hybrid_get_presence_name(account->state));
		account->account_menu = hybrid_create_menu(NULL, menu_name, NULL, TRUE,
								NULL, NULL);
		g_free(menu_name);

		/* set the icon of the account menu */
		presence_pixbuf = hybrid_create_presence_pixbuf(account->state, 16);
		presence_image = gtk_image_new_from_pixbuf(presence_pixbuf);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(account->account_menu),
							presence_image);
		g_object_unref(presence_pixbuf);

		create_account_child_menus(account);

		gtk_menu_shell_insert(GTK_MENU_SHELL(account_shell),
						account->account_menu, 3);
	}
}

static void
create_basic_menus(GtkBox *box)
{
	GtkUIManager *ui;
	GtkActionGroup *actionGroup;

	GtkActionEntry entries[] = {
		{ "Account", NULL, "_Account" },
		{ "Manage Accounts", NULL, "Manage Accounts",
		 "<control>A", "Manage Account", G_CALLBACK(manage_account_cb)},
		{ "Quit", GTK_STOCK_QUIT, "Quit",
		 "<control>Q", "Quit", G_CALLBACK(quit_cb)},
		{ "Help", NULL, "_Help" },
		{ "About", GTK_STOCK_ABOUT, "About" }
	};

	actionGroup = gtk_action_group_new("Actions");
    gtk_action_group_add_actions(actionGroup, entries, 5, NULL);

	ui = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(ui, actionGroup, 0);
	gtk_ui_manager_add_ui_from_file(ui, UI_DIR"menu.xml", NULL);

	g_object_unref(actionGroup);

	create_account_menus(ui);

	gtk_box_pack_start(box, gtk_ui_manager_get_widget(ui, "/MenuBar"),
			FALSE, FALSE, 0);
}

static void
ui_init(void)
{
	GtkWidget *window;
	GtkWidget *scroll;
	GtkWidget *vbox;
	
	window = hybrid_create_window(_("Hybrid"), NULL, GTK_WIN_POS_CENTER, TRUE);
	gtk_window_set_default_size(GTK_WINDOW(window), 250, 500);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	g_signal_connect(window, "destroy", G_CALLBACK(window_destroy), NULL);

	/* menu bar */
	create_basic_menus(GTK_BOX(vbox));

	/* scroll area (TreeView) */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
								 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

	hybrid_blist_init();

	gtk_container_add(GTK_CONTAINER(scroll), blist->treeview);

	gtk_widget_show_all(window);
}

gint 
main(gint argc, gchar **argv)
{

	if(!g_thread_supported())
		g_thread_init(NULL);
	gdk_threads_init();

	gtk_init(&argc, &argv);

	hybrid_config_init();

	hybrid_module_init();

	hybrid_account_init();
	
	ui_init();

	hybrid_start_login();

	gtk_main();

	return 0;
}
