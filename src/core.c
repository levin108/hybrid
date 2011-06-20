#include <glib.h>
#include <gtk/gtk.h>
#include "config.h"
#include "module.h"
#include "blist.h"
#include "util.h"
#include "gtkaccount.h"
#include "gtkutils.h"

extern HybridBlist *blist;

void
hybrid_start_login()
{
	HybridModule *module;

	module = hybrid_module_find("fetion");
	HybridAccount *ac = hybrid_account_create(module);

	module->info->login(ac);

}

static void
window_destroy(GtkWidget *widget, gpointer user_data)
{
	/*
	 * Now free the memory.
	 */
	GSList *pos;
	HybridGroup *group;
	HybridBuddy *buddy;
	
	extern GSList *group_list;
	extern GSList *buddy_list;

	extern HybridBlist *blist;

	g_free(blist);

	while (group_list) {
		pos = group_list;
		group = (HybridGroup*)pos->data;
		group_list = g_slist_remove(group_list, group);
		hybrid_blist_group_destroy(group);
	}

	while (buddy_list) {
		pos = buddy_list;
		buddy = (HybridBuddy*)pos->data;
		buddy_list = g_slist_remove(buddy_list, buddy);
		hybrid_blist_buddy_destroy(buddy);
	}

	gtk_main_quit();
}

static void
manage_account_cb(GtkWidget *widget, gpointer user_data)
{
	HybridAccountPanel *panel;

	panel = hybrid_account_panel_create();
	printf("abc\n");
}

static void
quit_cb(GtkWidget *widget, gpointer user_data)
{
	printf("quit\n");
	//gtk_main_quit();
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

	gtk_box_pack_start(box, gtk_ui_manager_get_widget(ui, "/MenuBar"),
			FALSE, FALSE, 0);
}

static void
ui_init(void)
{
	GtkWidget *window;
	GtkWidget *scroll;
	GtkWidget *vbox;
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 250, 500);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	GTK_WINDOW(window)->allow_shrink = TRUE;

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
	gtk_init(&argc, &argv);
	
	ui_init();

	hybrid_config_init();

	hybrid_module_init();

	hybrid_account_init();

//	hybrid_start_login();

	gtk_main();

	return 0;
}
