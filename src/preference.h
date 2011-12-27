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

#ifndef HYBRID_PREFERENCE_H
#define HYBRID_PREFERENCE_H
#include <gtk/gtk.h>

typedef struct _HybridPrefWin HybridPrefWin;

struct _HybridPrefWin {
    GtkWidget *window;
    GtkWidget *notebook;
};

#ifdef __cplusplus
extern "C" {
#endif

    HybridPrefWin *hybrid_pref_win_new(const gchar *title);

/**
 * Create the preference window, if exists, just present the window.
 */
    void hybrid_pref_create(void);

#ifdef __cplusplus
}
#endif

#endif
