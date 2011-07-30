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

#ifndef HYBRID_CHAT_TEXTVIEW_H
#define HYBRID_CHAT_TEXTVIEW_H

#include <gtk/gtk.h>
#include "account.h"
#include "blist.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a chat box with GtkTextView.
 *
 * @return The chat box created.
 */
GtkWidget *hybrid_chat_textview_create();

/**
 * Append a message to the textview.
 *
 * @param textview The textview.
 * @param name     The sender's name to display.
 * @param message  The message.
 * @param sendout  TRUE if message is sent to buddy, FALSE if message is sent
 *                 from buddy.
 * @param msg_time The time displayed together with the message.
 */
void hybrid_chat_textview_append(GtkWidget *textview, const gchar *name,
								const gchar *message, time_t msg_time, 
								gboolean sendout);
/**
 * Append a notify message to the textview.
 *
 * @param textview The textview.
 * @param text     Content of the notification.
 * @param type     Type of the notification.
 */
void hybrid_chat_textview_notify(GtkWidget *textview, const gchar *text, gint type);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_CHAT_TEXTVIEW_H */
