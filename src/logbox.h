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

#ifndef HYBRID_LOGBOX_H
#define HYBRID_LOGBOX_H

#include <gtk/gtk.h>
#include "account.h"

typedef struct _HybridLogbox HybridLogbox;

struct _HybridLogbox {
	GtkWidget *window;
	GtkWidget *textview;
	GtkWidget *loglist;

	HybridAccount *account;
	HybridBuddy *buddy;
};

enum {
    HYBRID_LOGBOX_NAME,
    HYBRID_LOGBOX_FILE,
    HYBRID_LOGBOX_COLUMNS,
};

enum {
    HYBRID_LOGBOX_HEAD_MY_ICON,
    HYBRID_LOGBOX_HEAD_MY_NAME,
    HYBRID_LOGBOX_HEAD_OT_ICON,
    HYBRID_LOGBOX_HEAD_OT_NAME,
    HYBRID_LOGBOX_HEAD_COLUMNS,
};
/**
 * Create an log box object.
 *
 * @param account The hybrid account whose log to show.
 * @param buddy The hybrid buddy to whom of the log
 *
 * @return The object created.
 */
HybridLogbox *hybrid_logbox_create(HybridAccount *account, HybridBuddy *buddy);

/**
 * Show the log box
 *
 * @return logbox The log box to show.
 */
void hybrid_logbox_show(HybridLogbox *logbox);

#endif
