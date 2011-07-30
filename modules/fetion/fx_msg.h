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

#ifndef HYBRID_FX_MSG_H
#define HYBRID_FX_MSG_H

#include <gtk/gtk.h>
#include "fx_account.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse the system message. The output is the content 
 * and a url to the website the message refered to.
 *
 * @param sipmsg  The sip message of the system message.
 * @param content The content of the message to be set.
 * @param url     The url of the url to be set.
 */
gint fetion_message_parse_sysmsg(const gchar *sipmsg,
		gchar **content, gchar **url);

/**
 * Send a text message to a buddy who is invisible or offline.
 *
 * @param account The fetion account.
 * @param userid  The receiver's userid.
 * @param text    The content of the message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_message_send(fetion_account *account, const gchar *userid,
						const gchar *text);

/**
 * Send a text message to ourselves.
 *
 * @param account The fetion account.
 * @param text    The content of the message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_message_send_to_me(fetion_account *account, const gchar *text);

/**
 * Process the received message, send back a response message, and show
 * the message in the chat panel.
 *
 * @param account The fetion account.
 * @param sipmsg  The sip message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_process_message(fetion_account *account, const gchar *sipmsg);

/**
 * Process the received invitation messsage, send back a response message,
 * and start a new channel with the endpoint specified by the server, the
 * message we received is:
 *
 * I 547264589 SIP-C/4.0
 * F: sip:547306298@fetion.com.cn;p=16165
 * I: -25
 * K: text/plain
 * K: text/html-fragment
 * K: multiparty
 * K: nudge
 * XI: 2b2b8c5041424159b61cab9992a58ce3
 * L: 111
 * Q: 200002 I
 * AL: buddy
 * A: CS address="221.176.31.12:8080;221.176.31.12:443",credential="1627694049.781705893"
 *
 * then we should send back a response message:
 *
 */
gint fetion_process_invite(fetion_account *account, const gchar *sipmsg);

/**
 * Start a new chat channel, and send the text through the new channel.
 * The message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 547264589
 * I: 4
 * Q: 2 S
 * N: StartChat
 *
 * @param account The fetion account.
 * @param userid  The receiver buddy's userid.
 * @param text    The content of the message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint fetion_message_new_chat(fetion_account *account, const gchar *userid,
								const gchar *text);


#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_MSG_H */
