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

#ifndef HYBRID_CHAT_WEBKIT_H
#define HYBRID_CHAT_WEBKIT_H
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set webkit ops for chat panel.
 */
void hybrid_chat_set_webkit_ops(void);

/**
 * Initialize the webkit context.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint hybrid_webkit_init(void);

/**
 * Destroy the webkit context.
 */
void hybrid_webkit_destroy(void);

/**
 * Create a chat box with WebKit.
 *
 * @return The chat box created.
 */
GtkWidget *hybrid_chat_webkit_create(void);

/**
 * Append a message to the webkit.
 *
 * @param textview The webkit textview.
 * @param account  The account context.
 * @param buddy    The buddy who sent the message.
 * @param message  The message.
 * @param msg_time The time displayed together with the message.
 */
void hybrid_chat_webkit_append(GtkWidget *textview, HybridAccount *account,
                               HybridBuddy *buddy, const gchar *message,
                               time_t msg_time);
/**
 * Append a notify message to the webkit.
 *
 * @param textview The webkit textview.
 * @param text     Content of the notification.
 * @param type     Type of the notification.
 */
void hybrid_chat_webkit_notify(GtkWidget *textview, const gchar *text,
                               gint type);

#ifdef __cplusplus
}
#endif

#endif
