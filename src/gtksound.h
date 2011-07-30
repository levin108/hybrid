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

#ifndef HYBRID_GTKSOUND_H
#define HYBRID_GTKSOUND_H
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialize the sound context.
 */
void hybrid_sound_init(gint argc, gchar **argv);

/**
 * Play an wav file.
 *
 * @param filename The absolute path of the wav file.
 */
void hybrid_sound_play_file(const gchar *filename);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_GTKSOUND_H */
