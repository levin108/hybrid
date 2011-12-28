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

#include "pref.h"
#include "util.h"

#include "conv.h"
#include "preference.h"
#include "gtkutils.h"

static HybridPrefWin *main_pref_window = NULL;

void bool_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry);
void bool_pref_save(HybridPrefEntry *entry);

static PrefAddFuncs bool_add_funcs = {
    .add_entry = bool_pref_add_entry,
    .save = bool_pref_save
};

void string_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry);
void string_pref_save(HybridPrefEntry *entry);

static PrefAddFuncs string_add_funcs = {
    .add_entry = string_pref_add_entry,
    .save = string_pref_save
};

void int_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry);
void int_pref_save(HybridPrefEntry *entry);

static PrefAddFuncs int_add_funcs = {
    .add_entry = int_pref_add_entry,
    .save = int_pref_save
};

void select_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry);
void select_pref_save(HybridPrefEntry *entry);

static PrefAddFuncs select_add_funcs = {
    .add_entry = select_pref_add_entry,
    .save = select_pref_save
};

static PrefAddFuncs *pref_types[] = {
    [PREF_KEY_NONE] = NULL,
    [PREF_KEY_BOOL] = &bool_add_funcs,
    [PREF_KEY_STRING] = &string_add_funcs,
    [PREF_KEY_INT] = &int_add_funcs,
    [PREF_KEY_SELECT] = &select_add_funcs
};

void bool_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry)
{

}

void bool_pref_save(HybridPrefEntry *entry)
{

}

void string_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry)
{

}

void string_pref_save(HybridPrefEntry *entry)
{

}

void int_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry)
{

}

void int_pref_save(HybridPrefEntry *entry)
{

}

void select_pref_add_entry(GtkWidget *section, HybridPrefEntry *entry)
{

}

void select_pref_save(HybridPrefEntry *entry)
{

}



enum {
    TAB_POS_NAME_COL,
    TAB_POS_VALUE_COL,
    TAB_POS_COLS
};

static GtkTreeModel*
create_tab_pos_model(void)
{
    /* GtkTreeStore *store; */
    /* GtkTreeIter iter; */

    /* store = gtk_tree_store_new(TAB_POS_COLS, */
    /*                            G_TYPE_STRING, */
    /*                            G_TYPE_INT); */

    /* gtk_tree_store_append(store, &iter, NULL); */
    /* gtk_tree_store_set(store, &iter, TAB_POS_NAME_COL, _("Top"), */
    /*                    TAB_POS_VALUE_COL, GTK_POS_TOP, -1); */

    /* gtk_tree_store_append(store, &iter, NULL); */
    /* gtk_tree_store_set(store, &iter, TAB_POS_NAME_COL, _("Right"), */
    /*                    TAB_POS_VALUE_COL, GTK_POS_RIGHT, -1); */

    /* gtk_tree_store_append(store, &iter, NULL); */
    /* gtk_tree_store_set(store, &iter, TAB_POS_NAME_COL, _("Bottom"), */
    /*                    TAB_POS_VALUE_COL, GTK_POS_BOTTOM, -1); */

    /* gtk_tree_store_append(store, &iter, NULL); */
    /* gtk_tree_store_set(store, &iter, TAB_POS_NAME_COL, _("Left"), */
    /*                    TAB_POS_VALUE_COL, GTK_POS_LEFT, -1); */

    /* return GTK_TREE_MODEL(store); */
}

/**
 * Create the combo box for choosing the tab positons.
 */
static GtkWidget*
chat_theme_combo_create()
{
    /* GtkWidget *combo; */
    /* GtkTreeStore *store; */
    /* GtkTreeIter iter; */
    /* GtkCellRenderer *renderer; */
    /* HybridChatTheme *themes; */
    /* gchar *chat_theme_name = NULL; */
    /* gchar *name = NULL; */
    /* gint i; */

    /* store = gtk_tree_store_new(TAB_POS_COLS, */
    /*             G_TYPE_STRING, */
    /*             G_TYPE_INT); */

    /* themes = hybrid_chat_window_get_themes(); */

    /* for (i = 0; themes[i].name; i++) { */
    /*     gtk_tree_store_append(store, &iter, NULL); */
    /*     gtk_tree_store_set(store, &iter, TAB_POS_NAME_COL, themes[i].name, -1); */
    /* } */

    /* combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store)); */

    /* renderer = gtk_cell_renderer_text_new(); */
    /* gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE); */
    /* gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, */
    /*                                "text", TAB_POS_NAME_COL, NULL); */
    /* gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0); */

    /* /\* */
    /*  * Set the active item with the local data. */
    /*  *\/ */
    /* if ((chat_theme_name = hybrid_pref_get_string(NULL, "chat_theme"))) { */
    /*     if(gtk_tree_model_get_iter_root(GTK_TREE_MODEL(store), &iter)) { */
    /*         do { */
    /*             gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, */
    /*                     TAB_POS_NAME_COL, &name, -1); */

    /*             if (g_strcmp0(name, chat_theme_name) == 0) { */

    /*                 gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter); */

    /*                 break; */
    /*             } */

    /*         } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)); */
    /*     } */
    /* } */

    /* g_free(chat_theme_name); */
    /* g_object_unref(store); */

    /* return combo; */
}

static GtkWidget*
tab_pos_combo_create()
{
    /* GtkWidget *combo; */
    /* GtkTreeModel *model; */
    /* GtkTreeIter iter; */
    /* GtkCellRenderer *renderer; */
    /* gint value; */
    /* gint tab_pos; */

    /* model = create_tab_pos_model(); */
    /* combo = gtk_combo_box_new_with_model(model); */

    /* renderer = gtk_cell_renderer_text_new(); */
    /* gtk_cell_layout_pack_start( */
    /*         GTK_CELL_LAYOUT(combo), renderer, FALSE); */
    /* gtk_cell_layout_set_attributes( */
    /*         GTK_CELL_LAYOUT(combo), renderer, */
    /*         "text", TAB_POS_NAME_COL, NULL); */
    /* gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0); */

    /* /\* */
    /*  * Set the active item with the local data. */
    /*  *\/ */
    /* if ((tab_pos = hybrid_pref_get_int(NULL, "tab_pos")) != -1) { */

    /*     if(gtk_tree_model_get_iter_root(model, &iter)) { */

    /*         do { */
    /*             gtk_tree_model_get(model, &iter, TAB_POS_VALUE_COL, &value, -1); */

    /*             if (value == tab_pos) { */

    /*                 gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter); */

    /*                 break; */
    /*             } */

    /*         } while (gtk_tree_model_iter_next(model, &iter)); */
    /*     } */
    /* } */

    /* g_object_unref(model); */

    /* return combo; */
}

/**
 * Initialize the basic settings page.
 */
void
pref_basic_init(GtkFixed *fixed)
{
    /* GtkWidget *label; */

    /* label = gtk_label_new(NULL); */
    /* gtk_label_set_markup(GTK_LABEL(label), _("<b>Chat Window:</b>")); */

    /* gtk_fixed_put(fixed, label, 20, 10); */

    /* pref_window->hcb_check = */
    /*     gtk_check_button_new_with_label(_("Hide Action Buttons")); */
    /* gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_window->hcb_check), */
    /*                              hybrid_pref_get_boolean(NULL, */
    /*                                                      "hide_chat_buttons")); */
    /* gtk_fixed_put(fixed, pref_window->hcb_check, 20, 35); */

    /* label = gtk_label_new(_("Chat Theme:")); */
    /* gtk_fixed_put(fixed, label, 20, 65); */

    /* pref_window->chat_theme_combo = chat_theme_combo_create(); */
    /* gtk_fixed_put(fixed, pref_window->chat_theme_combo, 120, 60); */

    /* label = gtk_label_new(NULL); */
    /* gtk_label_set_markup(GTK_LABEL(label), */
    /*         _("<b>Tabs:</b>")); */
    /* gtk_fixed_put(fixed, label, 20, 95); */

    /* pref_window->single_cw_check = */
    /*     gtk_check_button_new_with_label(_("Show Messages In A Single Window With Tabs")); */

    /* gtk_toggle_button_set_active( */
    /*         GTK_TOGGLE_BUTTON(pref_window->single_cw_check), */
    /*         hybrid_pref_get_boolean(NULL, "single_chat_window")); */
    /* gtk_fixed_put(fixed, pref_window->single_cw_check, 20, 125); */

    /* label = gtk_label_new(_("Tab Position:")); */
    /* gtk_fixed_put(fixed, label, 20, 165); */

    /* pref_window->tab_pos_combo = tab_pos_combo_create(); */
    /* gtk_fixed_put(fixed, pref_window->tab_pos_combo, 120, 160); */
}

/**
 * Initialize the sound settings panel.
 */
static void
pref_sound_init(GtkFixed *fixed)
{
    /* pref_window->mute_check = gtk_check_button_new_with_label(_("Mute")); */
    /* gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_window->mute_check), */
    /*                              hybrid_pref_get_boolean(NULL, "mute")); */
    /* gtk_fixed_put(fixed, pref_window->mute_check, 20, 15); */
}

/**
 * Callback function for the cancel button.
 */
static void
cancel_cb(GtkWidget *widget, gpointer user_data)
{
    /* g_return_if_fail(pref_window != NULL); */

    /* gtk_widget_destroy(pref_window->window); */
}

/**
 * Callback function for the save button.
 */
static void
save_cb(GtkWidget *widget, gpointer user_data)
{
    /* GtkTreeModel *model; */
    /* GtkTreeIter iter; */
    /* gint tab_pos; */
    /* gchar *chat_theme; */

    /* g_return_if_fail(pref_window != NULL); */

    /* if (gtk_toggle_button_get_active( */
    /*             GTK_TOGGLE_BUTTON(pref_window->mute_check))) { */

    /*     hybrid_pref_set_boolean(NULL, "mute", TRUE); */

    /* } else { */
    /*     hybrid_pref_set_boolean(NULL, "mute", FALSE); */
    /* } */

    /* if (gtk_toggle_button_get_active( */
    /*             GTK_TOGGLE_BUTTON(pref_window->hcb_check))) { */

    /*     hybrid_pref_set_boolean(NULL, "hide_chat_buttons", TRUE); */

    /* } else { */
    /*     hybrid_pref_set_boolean(NULL, "hide_chat_buttons", FALSE); */
    /* } */

    /* if (gtk_toggle_button_get_active( */
    /*             GTK_TOGGLE_BUTTON(pref_window->single_cw_check))) { */
    /*     hybrid_pref_set_boolean(NULL, "single_chat_window", TRUE); */
    /* } else { */
    /*     hybrid_pref_set_boolean(NULL, "single_chat_window", FALSE); */
    /* } */

    /* /\* */
    /*  * Tab position settings. */
    /*  *\/ */
    /* model = gtk_combo_box_get_model(GTK_COMBO_BOX(pref_window->tab_pos_combo)); */

    /* gtk_combo_box_get_active_iter( */
    /*         GTK_COMBO_BOX(pref_window->tab_pos_combo), &iter); */

    /* gtk_tree_model_get(model, &iter, TAB_POS_VALUE_COL, &tab_pos, -1); */

    /* hybrid_pref_set_int(NULL, "tab_pos", tab_pos); */

    /* /\* */
    /*  * Chat theme settings. */
    /*  *\/ */
    /* model = gtk_combo_box_get_model(GTK_COMBO_BOX(pref_window->chat_theme_combo)); */

    /* gtk_combo_box_get_active_iter( */
    /*         GTK_COMBO_BOX(pref_window->chat_theme_combo), &iter); */

    /* gtk_tree_model_get(model, &iter, TAB_POS_NAME_COL, &chat_theme, -1); */

    /* hybrid_pref_set_string(NULL, "chat_theme", chat_theme); */

    /* hybrid_pref_save(NULL); */

    /* gtk_widget_destroy(pref_window->window); */
}

/**
 * Initialize the preference window.
 */
void
pref_window_init(void)
{
    /* GtkWidget *fixed; */
    /* GtkWidget *label; */
    /* GtkWidget *vbox; */
    /* GtkWidget *action_area; */
    /* GtkWidget *button; */

    /* g_return_if_fail(pref_window != NULL); */

    /* label = gtk_label_new(_("Basic Settings")); */

    /* pref_basic_init(GTK_FIXED(fixed)); */

    /* fixed = gtk_fixed_new(); */
    /* label = gtk_label_new(_("Sound")); */
    /* gtk_notebook_append_page(GTK_NOTEBOOK(pref_window->notebook), */
    /*                         fixed, label); */

    /* pref_sound_init(GTK_FIXED(fixed)); */

    /* action_area = gtk_hbox_new(FALSE, 0); */
    /* gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5); */
}

/**
 * Callback function for destroying the preference window.
 */
static void
destroy_cb(GtkWidget *widget, gpointer pref_win)
{
    g_free(pref_win);
}

static void
main_destroy_cb(GtkWidget *widget, gpointer p)
{
    main_pref_window = NULL;
}

HybridPrefWin*
hybrid_pref_win_new(HybridPref *pref, const gchar *title)
{
    HybridPrefWin *pref_win;
    GtkWidget *content_area;
    GdkPixbuf *icon;
    title = title ? title : _("Preference");
    pref_win = g_new0(HybridPrefWin, 1);

    pref_win->pref = pref;
    /* Use dialog window in order to be tiling wm/filter friendly. */
    /* TODO put this into hybrid_create_window */
    pref_win->window = gtk_dialog_new_with_buttons(title, NULL, 0,
                                                   GTK_STOCK_OK,
                                                   GTK_RESPONSE_ACCEPT,
                                                   GTK_STOCK_CANCEL,
                                                   GTK_RESPONSE_REJECT,
                                                   NULL);
    icon = hybrid_create_default_icon(0);
    gtk_window_set_icon(GTK_WINDOW(pref_win->window), icon);
    gtk_window_set_resizable(GTK_WINDOW(pref_win->window), FALSE);
    gtk_window_set_position(GTK_WINDOW(pref_win->window), GTK_WIN_POS_CENTER);

    /* doesn't set the pointer to NULL, need to be handled elsewhere */
    /* by connect_after to the same signal? */
    g_signal_connect(pref_win->window, "destroy",
                     G_CALLBACK(destroy_cb), pref_win);
    g_signal_connect_swapped(pref_win->window, "response",
                             G_CALLBACK(gtk_widget_destroy), pref_win->window);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(pref_win->window));

    /* let's see how it looks */
    //gtk_container_set_border_width(GTK_CONTAINER(content_area), 8);

    pref_win->notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(content_area), pref_win->notebook);

    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(pref_win->notebook), GTK_POS_TOP);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(pref_win->notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(pref_win->notebook), TRUE);

    return pref_win;
}

void
hybrid_pref_create(void)
{
    if (main_pref_window) {
        gtk_window_present(GTK_WINDOW(main_pref_window->window));
        return;
    }

    main_pref_window = hybrid_pref_win_new(NULL, NULL);

    g_signal_connect_after(main_pref_window->window, "destroy",
                           G_CALLBACK(main_destroy_cb), NULL);

    /* gtk_widget_set_size_request(main_pref_window->window, 450, 300); */

    gtk_widget_show_all(main_pref_window->window);
}
