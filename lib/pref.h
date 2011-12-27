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

#ifndef HYBRID_PREF_H
#define HYBRID_PREF_H
#include <gtk/gtk.h>
#include "util.h"
#include "xmlnode.h"

typedef struct _HybridPref HybridPref;

struct _HybridPref {
    gchar *filename;
    xmlnode *root;

    gboolean mute;
    gboolean hide_chat_buttons;
    gboolean disable_chat_tabs;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the preference context.
 *
 * HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint hybrid_pref_init(void);

/**
 * Save the context of the preference to the local disk.
 */
void hybrid_pref_save(void);

/**
 * Set the string value for a property.
 *
 * @param name  The name of the property.
 * @param value The string value of the property;
 */
void hybrid_pref_set_string(const gchar *name, const gchar *value);

/**
 * Get the string value of a given property.
 *
 * @param name The name of the property.
 *
 * @return The value of the property, NULL if not found,
 *         need to be freed with g_free() when no longer needed.
 */
gchar *hybrid_pref_get_string(const gchar *name);

/**
 * Set the gboolean value for a property.
 *
 * @param name  The name of the property.
 * @param value The string value of the property;
 */
void hybrid_pref_set_boolean(const gchar *name, const gboolean value);

/**
 * Get the bool value of a given property.
 *
 * @param name The name of the property.
 *
 * @return The value of the property.
 */
gboolean hybrid_pref_get_boolean(const gchar *name);

/**
 * Set the integer value for a property.
 *
 * @param name  The name of the property.
 * @param value The integer value of the property.
 */
void hybrid_pref_set_int(const gchar *name, gint value);

/**
 * Get the integer value of a given property.
 *
 * @return The value of the property.
 */
gint hybrid_pref_get_int(const gchar *name);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_PREF_H */
