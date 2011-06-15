#include <glib.h>
#include <gtk/gtk.h>
#include "module.h"
#include "blist.h"
#include "util.h"

extern IMBlist *blist;

void
im_start_login()
{
	IMModule *module;

	module = im_module_find("fetion");
	IMAccount *ac = im_account_create(module);

	module->info->login(ac);

}

static void
window_destroy(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}

static void
create_basic_menus(GtkBox *box)
{
	GtkUIManager *ui;
	GtkWidget *menubar;
	GtkWidget *account_menu;

	GtkActionEntry entries[] = {
		{ "Accounts", NULL, "_Accounts" },

		{ "Open", GTK_STOCK_OPEN, "Open",
		 "<control>O", "Open File", NULL},

		{ "Save", GTK_STOCK_SAVE, "Save",
		 "<control>S", "Save File", NULL},

		{ "Quit", GTK_STOCK_QUIT, "Quit",
		 "<control>Q", "Quit", NULL},

		{ "Help", NULL, "_Help" },

		{ "About", GTK_STOCK_ABOUT, "About" }
	};
	GtkActionGroup *actionGroup;
	actionGroup = gtk_action_group_new("Actions");
    gtk_action_group_add_actions(actionGroup, entries, 6, NULL);

	ui = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(ui, actionGroup, 0);
	gtk_ui_manager_add_ui_from_file(ui, UI_DIR"menu.xml", NULL);

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

	im_blist_init();

	gtk_container_add(GTK_CONTAINER(scroll), blist->treeview);

	gtk_widget_show_all(window);
}

gint 
main(gint argc, gchar **argv)
{
	gtk_init(&argc, &argv);
	
	ui_init();

	im_module_init();

	im_start_login();

	gtk_main();

	return 0;
}
