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

#ifndef IM_GTKACCOUNT_H
#define IM_GTKACCOUNT_H

#include <gtk/gtk.h>

typedef struct _HybridAccountPanel HybridAccountPanel;
typedef struct _HybridAccountEditPanel HybridAccountEditPanel;
typedef struct _HybridAccountMenuData  HybridAccountMenuData;

struct _HybridAccountPanel {
    GtkWidget              *window;
    GtkListStore           *account_store;
    GtkWidget              *account_tree;
    HybridAccountEditPanel *edit_panel;
};

struct _HybridAccountEditPanel {
    HybridAccountPanel *parent;
    GtkWidget          *window;
    GtkWidget          *username_entry;
    GtkWidget          *password_entry;
    GtkWidget          *proto_combo;
    GtkWidget          *user_table;
    GtkWidget          *basic_vbox;
    gboolean            is_add;
};


struct _HybridAccountMenuData {
    HybridAccount *account;
    gint          presence_state;
};


enum {
    HYBRID_ENABLE_COLUMN,
    HYBRID_NAME_COLUMN,
    HYBRID_PROTO_ICON_COLUMN,
    HYBRID_PROTO_NAME_COLUMN,
    HYBRID_ACCOUNT_COLUMN,
    HYBRID_ACCOUNT_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an account management panel.
 *
 * @return The account management panel created.
 */
HybridAccountPanel *hybrid_account_panel_create();

/**
 * Create the account's menu.
 *
 * @param account The account context.
 */
void hybrid_account_create_menu(HybridAccount *account);

/**
 * Remove the account's menu.
 *
 * @param account The account context.
 */
void hybrid_account_remove_menu(HybridAccount *account);

/**
 * Hide the disable menu,show the enable menu
 *
 * @param account The account context
 */
void hybrid_account_disable_menu(HybridAccount *account);

#ifdef __cplusplus
}
#endif

#endif /* IM_GTKACCOUNT_H */
