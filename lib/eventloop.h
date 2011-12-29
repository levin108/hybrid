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

#ifndef HYBRID_EVENTLOOP_H
#define HYBRID_EVENTLOOP_H

#include <glib.h>

#include "connect.h"

#define HYBRID_EVENT_READ  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define HYBRID_EVENT_WRITE (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

typedef gboolean (*input_func)(gint sk, gpointer user_data);

#define EVENT_CALLBACK(cb) ((input_func)cb)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add an event handler.
 *
 * @param sk The input socket descriptor.
 * @param func The callback function.
 * @param user_data User-specified data to pass to func.
 *
 * @return The event source id.
 */
guint hybrid_event_add(gint sk, gint event_type, input_func func,
                       gpointer user_data);

/**
 * Remove an event handler.
 *
 * @param source The event source descriptor.
 *
 * @return TRUE if event was removed.
 */
gboolean hybrid_event_remove(guint source);

/**
 * Add an ssl read event handler.
 *
 * @param isc The ssl context.
 * @param func The callback function.
 * @param user_data User-specified data to pass to func.
 *
 * @return The event source id.
 */
guint hybrid_ssl_event_add(HybridSslConnection *isc, ssl_callback func,
                           gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_EVENTLOOP_H */
