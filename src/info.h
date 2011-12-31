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

#ifndef IM_INFO_H
#define IM_INFO_H

#include <gtk/gtk.h>

typedef struct _HybridInfo HybridInfo;
typedef struct _HybridInfoItem HybridInfoItem;
typedef struct _HybridNotifyInfo HybridNotifyInfo;

#include "blist.h"

struct _HybridInfo {
    GtkWidget *window;
    GtkWidget *treeview;
    HybridBuddy *buddy;
};

struct _HybridNotifyInfo {
    GSList *item_list;
};

struct _HybridInfoItem {
    gchar *name;
    gchar *value;
    GdkPixbuf *pixbuf;
    gint type; /* text item or pixbuf item. */
};

enum {
    HYBRID_INFO_ITEM_TYPE_TEXT,
    HYBRID_INFO_ITEM_TYPE_PIXBUF,
};

enum {
    HYBRID_INFO_NAME_COLUMN,
    HYBRID_INFO_VALUE_COLUMN,
    HYBRID_INFO_PIXBUF_COLUMN,
    HYBRID_INFO_VALUE_COLUMN_VISIBLE,
    HYBRID_INFO_PIXBUF_COLUMN_VISIBLE,
    HYBRID_INFO_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a profile info panel, and push it to the list.
 * If the panel associated with the given buddy ID exists in
 * the panel list, just return it instead of creating a new one.
 *
 * @param buddy The buddy whose profile panel to create.
 *
 * @return The profile info panel created.
 */
HybridInfo *hybrid_info_create(HybridBuddy *buddy);

/**
 * Create a notify information context.
 *
 * @return The notify information context created.
 */
HybridNotifyInfo *hybrid_notify_info_create();

/**
 * Destroy a notify information context;
 *
 * @param info The notify information context to destroy.
 */
void hybrid_notify_info_destroy(HybridNotifyInfo *info);

/**
 * Bind the information context to a information panel.
 *
 * @param account  The account context.
 * @param info     The notify information context.
 * @param buddy_id The id of the buddy whose information to be bind.
 */
void hybrid_info_notify(HybridAccount *account, HybridNotifyInfo *info,
                        const gchar *buddy_id);

/**
 * Add a name-value pair to the notify info context.
 *
 * @param info The info panel context.
 * @param name The name of the pair.
 * @param value The value of the pair.
 */
void hybrid_info_add_pair(HybridNotifyInfo *info, const gchar *name,
                          const gchar *value);

/**
 * Add a name-pixbuf pair to the notify info context.
 *
 * @param info   The info panel context.
 * @param name   The name of the pair.
 * @param pixbuf The pixbuf of the pair.
 */
void hybrid_info_add_pixbuf_pair(HybridNotifyInfo *info, const gchar *name,
                                 const GdkPixbuf *pixbuf);

#ifdef __cplusplus
}
#endif

#endif /* IM_INFO_H */
