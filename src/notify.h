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

#ifndef HYBRID_NOTIFY_H
#define HYBRID_NOTIFY_H

#include <gtk/gtk.h>
#include "account.h"

typedef struct _HybridNotify HybridNotify;

struct _HybridNotify {
	GtkWidget *window;
	GtkWidget *cellview;
	GtkTreeIter iter;
	GtkWidget *textview;
	GtkWidget *action_area;
	HybridAccount *account;
};

enum {
	NOTIFY_ICON_COLUMN,
	NOTIFY_NAME_COLUMN,
	NOTIFY_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a notification box.
 *
 * @param title   The title of the box, if NULL, then set it
 *                to default _("Notification")
 * @param account The account which give out a notification.
 *
 * @return The notification box created.
 */
HybridNotify *hybrid_notify_create(HybridAccount *account, const gchar *title);

/**
 * Set the text to displayed in the text area.
 *
 * @param notify The notification box.
 * @param text   The notification text to set.
 */
void hybrid_notify_set_text(HybridNotify *notify, const gchar *text);

/**
 * Set the name of the account releted to the notification.
 *
 * @param notify The notification box.
 * @param name   The account name.
 */
void hybrid_notify_set_name(HybridNotify *notify, const gchar *name);

/**
 * Popup a notify message.
 *
 * @param pixbuf   The notify icon.
 * @param title    The title.
 * @param summary  The summary.
 */
void hybrid_notify_popup(GdkPixbuf *pixbuf, const gchar *title,
		const gchar *summary);

#ifdef __cplusplus
}
#endif

#endif
