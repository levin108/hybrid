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

#ifndef HYBRID_STATUSICON_H
#define HYBRID_STATUSICON_H
#include <gtk/gtk.h>

#include "blist.h"

typedef struct _HybridStatusIcon HybridStatusIcon;

struct _HybridStatusIcon {
	GtkStatusIcon *icon;
	gint conn_id;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the status icon.
 */
void hybrid_status_icon_init(void);

/**
 * Set the status icon blinking with the icon of the given buddy.
 *
 * @param The buddy whose icon will be blinking in the status icon.
 */
void hybrid_status_icon_blinking(HybridBuddy *buddy);

#ifdef __cplusplus
}
#endif

#endif
