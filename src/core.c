/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include <glib.h>
#include <gtk/gtk.h>

#include "config.h"

#ifdef USE_LIBNOTIFY
 #include <libnotify/notify.h>
#endif

#include "pref.h"
#include "module.h"
#include "head.h"
#include "blist.h"
#include "logs.h"
#include "util.h"
#include "preference.h"
#include "statusicon.h"
#include "gtkaccount.h"
#include "gtkutils.h"
#include "gtkconn.h"
#include "gtksound.h"
#include "groupadd.h"
#include "buddyadd.h"
#include "searchbox.h"
#include "chat-webkit.h"

extern HybridBlist *blist;
extern HybridHead *hybrid_head;
extern GSList *account_list;

GtkWidget *hybrid_window;

/* Vbox for logining panels. */
GtkWidget *hybrid_vbox;

GtkUIManager *menu_ui_manager;

static void
window_destroy(GtkWidget *widget, gpointer user_data)
{
    /*
     * Now free the memory.
     */
#ifdef USE_WEBKIT
    hybrid_webkit_destroy();
#endif

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
preference_cb(GtkWidget *widget, gpointer user_data)
{
    hybrid_pref_create();
}

static void
add_group_cb(GtkWidget *widget, gpointer user_data)
{
    hybrid_groupadd_window_create();
}

static void
add_buddy_cb(GtkWidget *widget, gpointer user_data)
{
    hybrid_buddyadd_window_create();
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
            "Preference",
            GTK_STOCK_PREFERENCES,
            "Preference",
            "<control>P",
            "Preference",
            G_CALLBACK(preference_cb)
        },
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
    gtk_action_group_add_actions(actionGroup, entries, 9, NULL);

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
    GtkWidget    *window;
    GtkWidget    *scroll;
    GtkWidget    *vbox;
    GtkWidget    *searchbox;

    /* initialize the status icon. */
    hybrid_status_icon_init();
    
    window = hybrid_create_window(_("Hybrid"), NULL, GTK_WIN_POS_CENTER, TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 500);

    hybrid_window = window;

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    g_signal_connect(window, "destroy", G_CALLBACK(window_destroy), NULL);

    /* head area */
    hybrid_head_init();
    gtk_box_pack_start(GTK_BOX(vbox), hybrid_head->vbox, FALSE, FALSE, 10);

    /* search box */
    searchbox = hybrid_search_box_create();
    gtk_box_pack_start(GTK_BOX(vbox), searchbox, FALSE, FALSE, 0);

    /* scroll area (TreeView) */
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
                                        GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);

    hybrid_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hybrid_vbox, FALSE, FALSE, 5);

    hybrid_blist_init();

    gtk_container_add(GTK_CONTAINER(scroll), blist->treeview);

    /* menu bar */
    create_basic_menus(GTK_BOX(vbox));

    gtk_widget_show_all(window);
    gtk_widget_hide(hybrid_head->editbox);
}

gint 
main(gint argc, gchar **argv)
{

    if(!g_thread_supported())
        g_thread_init(NULL);
    gdk_threads_init();

    gtk_init(&argc, &argv);

#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);
#endif	

#ifdef USE_LIBNOTIFY
    notify_init("Hybrid");
#endif

#ifdef USE_WEBKIT
    hybrid_webkit_init();
#endif

    hybrid_config_init();

    hybrid_pref_init();

    hybrid_logs_init();

    hybrid_module_init();

    hybrid_account_init();

    hybrid_conn_init();

    hybrid_sound_init(argc, argv);

    ui_init();

    hybrid_account_enable_all();

    gtk_main();

    return 0;
}
