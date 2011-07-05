#include <glib.h>
#include <gtk/gtk.h>
#include "config.h"
#include "module.h"
#include "head.h"
#include "blist.h"
#include "util.h"
#include "gtkaccount.h"
#include "gtkutils.h"
#include "groupadd.h"
#include "buddyadd.h"

extern HybridBlist *blist;
extern HybridHead *hybrid_head;
extern GSList *account_list;

GtkUIManager *menu_ui_manager;

void
hybrid_start_login()
{
	GSList *pos;
	HybridAccount *account;

	for (pos = account_list; pos; pos = pos->next) {
		account = (HybridAccount*)pos->data;

		if (account->enabled) {
			account->proto->info->login(account);
		}
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
add_group_cb(GtkWidget *widget, gpointer user_data)
{
	HybridGroupAddWindow *window;

	window = hybrid_groupadd_window_create();
}

static void
add_buddy_cb(GtkWidget *widget, gpointer user_data)
{
	HybridBuddyAddWindow *window;

	window = hybrid_buddyadd_window_create();
}

static void
create_account_menus(GtkUIManager *ui)
{
	GtkWidget *account_shell;
	GtkWidget *seperator;
	HybridAccount *account;
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

		hybrid_account_create_menu(account);
	}
}

static void
create_basic_menus(GtkBox *box)
{
	GtkActionGroup *actionGroup;

	GtkActionEntry entries[] = {
		/* account menu. */
		{ "Account", NULL, "_Account" },
		{	
			"Manage Accounts",
			NULL,
			"Manage Accounts",
			"<control>A",
			"Manage Account", 
			G_CALLBACK(manage_account_cb)
		},
		{	
			"Quit",
			GTK_STOCK_QUIT,
			"Quit",
			"<control>Q", 
			"Quit",
			G_CALLBACK(quit_cb)
		},
		/* tools menu. */
		{ "Tools", NULL, "_Tools" },
		{
			"Add Buddy",
			GTK_STOCK_ADD,
			"Add Buddy",
			"<control>B",
			"Add Buddy",
			G_CALLBACK(add_buddy_cb)
		},
		{ 
			"Add Group",
			GTK_STOCK_ADD, 
			"Add Group",
			"<control>G",
			"Add Group",
			G_CALLBACK(add_group_cb)
		},
		/* help menu. */
		{ "Help", NULL, "_Help" },
		{ "About", GTK_STOCK_ABOUT, "About" },
	};

	actionGroup = gtk_action_group_new("Actions");
    gtk_action_group_add_actions(actionGroup, entries, 8, NULL);

	menu_ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(menu_ui_manager, actionGroup, 0);
	gtk_ui_manager_add_ui_from_file(menu_ui_manager, UI_DIR"menu.xml", NULL);

	g_object_unref(actionGroup);

	create_account_menus(menu_ui_manager);

	gtk_box_pack_start(box, gtk_ui_manager_get_widget(menu_ui_manager, "/MenuBar"),
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

	/* head area */
	hybrid_head_init();
	gtk_box_pack_start(GTK_BOX(vbox), hybrid_head->cellview, FALSE, FALSE, 10);

	/* scroll area (TreeView) */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
								 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
			GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

	hybrid_blist_init();

	gtk_container_add(GTK_CONTAINER(scroll), blist->treeview);

	/* menu bar */
	create_basic_menus(GTK_BOX(vbox));

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
