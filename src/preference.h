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
#include "pref.h"

typedef struct {
    HybridPref *pref;
    GtkWidget *window;
    GtkWidget *notebook;
} HybridPrefWin;

typedef enum {
    PREF_KEY_NONE,
    PREF_KEY_BOOL,
    PREF_KEY_STRING,
    PREF_KEY_INT,
    PREF_KEY_SELECT,
    PREF_KEY_MAX
} PrefKeyType;

typedef struct _HybridPrefEntry HybridPrefEntry;

typedef struct {
    guint (*add_entry)(GtkWidget *section, guint pos, HybridPrefEntry *entry);
    void (*save)(HybridPrefEntry *entry);
    void (*destroy)(HybridPrefEntry *entry);
} PrefAddFuncs;

struct _HybridPrefEntry {
    gchar *name;
    gchar *key;
    gchar *tooltip;
    gpointer data;
    PrefAddFuncs *type;
    HybridPrefWin *win;
};

/* Pass an NULL-terminated array of SelectOption to add_entry */
/* in HybridPrefEntry.data when type is PREF_KEY_SELECT */
typedef struct {
    gchar *name;
    gint value;
} SelectOption;

#ifdef __cplusplus
extern "C" {
#endif

    HybridPrefWin *hybrid_pref_win_new(HybridPref *pref, const gchar *title);

    /* VBox */
    GtkWidget *hybrid_pref_win_add_tab(HybridPrefWin *pref_win,
                                       const gchar *name);
    /* Frame */
    GtkWidget *hybrid_pref_tab_add_section(GtkWidget *tab,
                                           const gchar *name);
    void hybrid_pref_section_add_entry(HybridPrefWin *pref_win,
                                       GtkWidget *section,
                                       PrefKeyType type,
                                       HybridPrefEntry *entry);
    void hybrid_pref_win_finish(HybridPrefWin *pref_win);

/**
 * Create the preference window, if exists, just present the window.
 */
    void hybrid_pref_create(void);

#ifdef __cplusplus
}
#endif

#endif
