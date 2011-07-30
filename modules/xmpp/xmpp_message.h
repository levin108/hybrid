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

#ifndef HYBRID_XMPP_MESSAGE_H
#define HYBRID_XMPP_MESSAGE_H
#include <glib.h>
#include "xmpp_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set a message with type 'chat' to a specified buddy.
 *
 * @param stream The xmpp stream.
 * @param text   The contenxt of the message.
 * @param to     Bare jid of the receiver.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_message_send(XmppStream *stream, const gchar *text, const gchar *to);

/**
 * Set typing message to a specified buddy.
 *
 * @param stream The xmpp stream.
 * @param to     Bare jid of the receiver.
 * @param state  State of the typeing action.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_message_send_typing(XmppStream *stream, const gchar *to,
		HybridInputState state);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_MESSAGE_H */
