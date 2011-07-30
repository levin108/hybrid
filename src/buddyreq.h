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

#ifndef HYBRID_BUDDYREQ_H
#define HYBRID_BUDDYREQ_H
#include <glib.h>

typedef struct _HybridBuddyReqWindow HybridBuddyReqWindow;
typedef void (*DestroyNotify)(gpointer user_data);

#include "account.h"

struct _HybridBuddyReqWindow {
	gchar *buddy_id;
	gchar *buddy_name;
	gboolean accept;

	gchar *user_data;
	DestroyNotify notify;

	HybridAccount *account;

	GtkWidget *window;
	GtkWidget *textview;
	GtkWidget *alias_entry;
	GtkWidget *group_combo;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an add-buddy request window.
 *
 * @param account The account context.
 * @param id      ID of the buddy who sent the request.
 * @param name    Name of the buddy who sent the request. [allow-none]
 *
 * @return The request window created.
 */
HybridBuddyReqWindow *hybrid_buddy_request_window_create(HybridAccount *account, 
		const gchar *id, const gchar *name);

/**
 * Set user-specified data.
 *
 * @param req       The request window.
 * @param user_data User-specified data.
 * @param notify    Notification function to destroy the user-specified data.
 *                  if not NULl, the user-specified data will be destroyed by it
 *                  when the buddy request window was destroyed.
 */
void hybrid_buddy_request_set_user_data(HybridBuddyReqWindow *req, gpointer user_data,
		DestroyNotify notify);

/**
 * Get user-specified data.
 *
 * @param req The request window.
 *
 * @return User-specified data.
 */
gpointer hybrid_buddy_request_get_user_data(HybridBuddyReqWindow *req);

#ifdef __cplusplus
}
#endif

#endif
