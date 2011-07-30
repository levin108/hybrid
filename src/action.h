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

#ifndef HYBRID_ACTION_H
#define HYBRID_ACTION_H
#include <gtk/gtk.h>

#include "account.h"

typedef struct _HybridAction HybridAction;
typedef void (*ActionCallback)(HybridAction *);

struct _HybridAction {
	ActionCallback callback;
	gchar *text;
	HybridAccount *account;
};

/**
 * Create a new action menu.
 *
 * @param account  The account context.
 * @param text     The text of the action menu.
 * @param callback The callback function of the action menu.
 *
 * @return The action menu created.
 */
HybridAction *hybrid_action_create(HybridAccount *account,
				const gchar *text, ActionCallback callback);

/**
 * Destroy an action menu.
 *
 * @param action The action menu to destroy.
 */
void hybrid_action_destroy(HybridAction *action);

/**
 * Get the account context through the action context.
 *
 * @param action The action context.
 *
 * @return The account context.
 */
HybridAccount *hybrid_action_get_account(HybridAction *action);

#endif /* HYBRID_ACTION_H */
