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

#ifndef HYBRID_HEAD_H
#define HYBRID_HEAD_H

#include <gtk/gtk.h>
#include "account.h"

typedef struct _HybridHead HybridHead;

struct _HybridHead {
	GtkWidget *cellview;
	GtkWidget *eventbox;
	GtkWidget *editbox;
	GtkWidget *edit_label;
	GtkWidget *edit_entry;
	GtkWidget *vbox;
	GtkTreeIter iter;

	/* which accout is being edited. */
	HybridAccount *edit_account;
	gint edit_state;
};

enum {
	HYBRID_HEAD_EDIT_NAME,
	HYBRID_HEAD_EDIT_STATUS
};

enum {
	HYBRID_HEAD_PIXBUF_COLUMN,
	HYBRID_HEAD_NAME_COLUMN,
	HYBRID_HEAD_STATUS_ICON_COLUMN,
	HYBRID_HEAD_COLUMNS
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the head panel.
 */
void hybrid_head_init();

/**
 * Bind an account's information to the head panel.
 *
 * @param account The account.
 */
void hybrid_head_bind_to_account(HybridAccount *account);

#ifdef __cplusplus
}
#endif

#endif
