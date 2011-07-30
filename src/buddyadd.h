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

#ifndef HYBRID_BUDDYADD_H
#define HYBRID_BUDDYADD_H

#include <gtk/gtk.h>

typedef struct _HybridBuddyAddWindow HybridBuddyAddWindow;

struct _HybridBuddyAddWindow {
	GtkWidget *window;
	GtkWidget *account_combo;
	GtkWidget *group_combo;
	GtkWidget *username_entry;
	GtkWidget *localname_entry;
	GtkWidget *tips_textview;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a buddy-add window.
 *
 * @return The buddy-add window created.
 */
HybridBuddyAddWindow *hybrid_buddyadd_window_create();

#ifdef __cplusplus
}
#endif

#endif
