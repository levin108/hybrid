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

#ifndef HYBRID_XMPP_STREAM_H
#define HYBRID_XMPP_STREAM_H
#include <glib.h>
#include "xmlnode.h"
#include "connect.h"
#include "account.h"
#include "xmlnode.h"

#define NS_XMPP_TLS      "urn:ietf:params:xml:ns:xmpp-tls"
#define NS_XMPP_SASL     "urn:ietf:params:xml:ns:xmpp-sasl"
#define NS_XMPP_BIND     "urn:ietf:params:xml:ns:xmpp-bind"
#define NS_XMPP_PING     "urn:xmpp:ping"
#define NS_IQ_ROSTER     "jabber:iq:roster"
#define NS_XMPP_SESSION  "urn:ietf:params:xml:ns:xmpp-session"
#define NS_GOOGLE_ROSTER "google:roster"
#define NS_CHANGESTATES  "http://jabber.org/protocol/chatstates"

typedef struct _XmppStream XmppStream;

#include "xmpp_account.h"

struct _XmppStream {
	gint  sk;                   /**< the socket descriptor. */
	guint source;               /**< event source. */
	guint keepalive_source;     /**< event source of the keep alive timeout event. */
	gint  current_iq_id;

	gchar *stream_id;
	gchar *jid;
	gint   major_version;       /**< default 1 */
	gint   miner_version;       /**< default 0 */
  

	xmlnode	*node;              /**< current xml node for parsing the xml stream. */
	gint	 state;             /**< stream state. */
	GSList	*pending_trans;     /**< The pending iq transactions. */

	XmppAccount			*account;
	HybridConnection	*conn;
	HybridSslConnection *ssl;
	xmlParserCtxt		*xml_ctxt;
};

enum {
	XMPP_STATE_TLS_AUTHENTICATING = 0,
	XMPP_STATE_TLS_STREAM_STARTING,
	XMPP_STATE_SASL_AUTHENTICATING,
	XMPP_STATE_SASL_STREAM_STARTING,
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the jabber stream.Note that this is
 * a callback function.
 *
 * @param sk     The socket destriptor.
 * @param stream The xmpp stream.
 */
gboolean xmpp_stream_init(gint sk, XmppStream *stream);

/**
 * Send a ping message on the stream to keep the 
 * connection alive.
 *
 * @param stream The xmpp stream.
 * 
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_stream_ping(XmppStream *stream);

/**
 * Create a xmpp stream.
 *
 * @param account The xmpp account context.
 *
 * @return The xmpp stream created.
 */
XmppStream *xmpp_stream_create(XmppAccount *account);

/**
 * Set id for a stream.
 *
 * @param stream The xmpp stream.
 * @param id     The stream id.
 */
void xmpp_stream_set_id(XmppStream *stream, const gchar *id);

/**
 * Set jabber id for a stream.
 *
 * @param stream The xmpp stream.
 * @param jid    The jabber id of the stream.
 */
void xmpp_stream_set_jid(XmppStream *stream, const gchar *jid);

/**
 * Get the current iq id in the form of a string.
 *
 * @param stream The xmpp stream.
 *
 * @return The iq id, should be freed with g_free() when no longer needed.
 */
gchar* xmpp_stream_get_iqid(XmppStream *stream);

/**
 * Increase the iq id of a xmpp stream.
 *
 * @param stream The xmpp stream.
 */
void xmpp_stream_iqid_increase(XmppStream *stream);

/**
 * Set state for a stream.
 *
 * @param stream The xmpp stream.
 * @param state  The new state of the stream.
 */
void xmpp_stream_set_state(XmppStream *stream, gint state);

/**
 * Destroy a xmpp stream.
 *
 * @param steam The stream to destroy.
 */
void xmpp_stream_destroy(XmppStream *stream);

/**
 * Process a xml packet.
 *
 * @param stream The xmpp stream context.
 * @param node   The root node of the xml packet.
 */
void xmpp_stream_process(XmppStream *stream, xmlnode *node);

/**
 * Get account's information.
 *
 * @param stream The stream for the account.
 */
void xmpp_stream_get_account_info(XmppStream *stream);

#endif /* HYBRID_XMPP_STREAM_H */
