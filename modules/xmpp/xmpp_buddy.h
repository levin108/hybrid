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

#ifndef HYBRID_XMPP_BUDDY_H
#define HYBRID_XMPP_BUDDY_H

#include "xmpp_stream.h"
#include "xmpp_iq.h"
#include "xmpp_account.h"

typedef struct _XmppBuddy XmppBuddy;
typedef struct _XmppPresence XmppPresence;


struct _XmppPresence {
	gchar *status;
	gint show;
	gchar *full_jid;

	XmppBuddy *buddy;
};

struct _XmppBuddy {
	gchar *jid;
	gchar *name;
	gchar *group;
	gchar *photo;

	gchar *subscription;

	GSList *presence_list; /* presence list. */

	XmppStream *stream;
	HybridBuddy *buddy;
};

enum {
	XMPP_PRESENCE_SUBSCRIBE,
	XMPP_PRESENCE_SUBSCRIBED,
	XMPP_PRESENCE_UNSUBSCRIBE,
	XMPP_PRESENCE_UNSUBSCRIBED
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a presence context.
 *
 * @param jid    The full jabber id.
 * @param status Status string.
 * @param show   The HYBRID presence state.
 *
 * @return Presence created.
 */
XmppPresence *xmpp_presence_create(const gchar *jid, const gchar *status, gint show);

/**
 * Destroy a presence context.
 *
 * @param presence The presence to destroy.
 */
void xmpp_presence_destroy(XmppPresence *presence);

/**
 * Find a buddy's presence with specified full jabber id.
 *
 * @param buddy    The xmpp buddy.
 * @param full_jid The full jabber id.
 *
 * @return NULL if not found.
 */
XmppPresence *xmpp_buddy_find_presence(XmppBuddy *buddy, const gchar *full_jid);

/**
 * Process the roster returned from the server.
 *
 * @param stream The xmpp stream.
 * @param root   The root node of the roster xml context.
 */
void xmpp_buddy_process_roster(XmppStream *stream, xmlnode *root);

/**
 * Create a new xmpp buddy, if buddy with the specified jid
 * exists, just return it.
 *
 * @param stream The xmpp stream.
 * @param jid    Jabber id of the buddy.
 *
 * @return The buddy created.
 */
XmppBuddy* xmpp_buddy_create(XmppStream *stream, HybridBuddy *buddy);

/**
 * Set name for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param name  The name of the buddy.
 */
void xmpp_buddy_set_name(XmppBuddy *buddy, const gchar *name);

/**
 * Set subscription for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param sub   The subscription string.
 */
void xmpp_buddy_set_subscription(XmppBuddy *buddy, const gchar *sub);

/**
 * Set photo checksum for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param name  The photo checksum of the buddy.
 */
void xmpp_buddy_set_photo(XmppBuddy *buddy, const gchar *photo);

/**
 * Find a buddy with the given bare jabber id.
 *
 * @param account The xmpp account.
 * @param jid     The bare jabber of the buddy to find.
 *
 * @return NULL if not found.
 */

XmppBuddy *xmpp_buddy_find(XmppAccount *account, const gchar *jid);

/**
 * Set status for the buddy.
 *
 * @param buddy    The xmpp buddy.
 * @param full_jid The full jabber id of this presence.
 * @param status   The status of the presence.
 */
void xmpp_buddy_set_status(XmppBuddy *buddy, const gchar *full_jid,
		const gchar *status);

/**
 * Set online status for the buddy.
 *
 * @param buddy    The xmpp buddy.
 * @param full_jid The full jabber id of this presence.
 * @param show     The online status.
 */
void xmpp_buddy_set_show(XmppBuddy *buddy, const gchar *full_jid,
		const gchar *show);

/**
 * Set group name for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param group The group name of the buddy.
 */
void xmpp_buddy_set_group_name(XmppBuddy *buddy, const gchar *group);

/**
 * Set group for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param group The group name.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_set_group(XmppBuddy *buddy, const gchar *group);

/**
 * Add a presence context to a xmpp buddy.
 *
 * @param buddy    The xmpp buddy.
 * @param presence The presence context.
 */
void xmpp_buddy_add_presence(XmppBuddy *buddy, XmppPresence *presence);

/**
 * Set the alias name for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param alias The alias name.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_alias(XmppBuddy *buddy, const gchar *alias);

/**
 * Get vcard information of the given buddy/account.
 *
 * @param stream    The xmpp stream.
 * @param jid       The jid of the buddy/account.
 * @param callback  The callback function.
 * @param user_data User-specified data for callback function.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_get_info(XmppStream *stream, const gchar *jid,
		trans_callback callback, gpointer user_data);

/**
 * Send a presence message to a jabber buddy, the type of the message
 * can be "unsubscribe", "subscribe", "unsubscribed", "subscribed"
 *
 * @param stream The xmpp Stream.
 * @param jid    Full jabber ID of the buddy to which the message is to be sent.
 * @param type   Type of the presence message.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_send_presence(XmppStream *stream, const gchar *jid, gint type);

/**
 * Remove a buddy from the roster.
 *
 * @param buddy The buddy to remove.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_delete(XmppBuddy *buddy);

/**
 * Add a xmpp buddy item to the roster.
 *
 * @param jid   Bare Jabber ID of the buddy to add.
 * @param name  Alias name set to this buddy.
 * @param group Name of group to add this buddy to.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_roster_add_item(XmppStream *stream, const gchar *jid, const gchar *name,
		const gchar *group);

/**
 * Clear the buddies from the local buddy list.
 *
 * @param stream The xmpp stream.
 */
void xmpp_buddy_clear(XmppStream *stream);
/**
 * Destroy a buddy.
 *
 * @param buddy The buddy to destroy.
 */
void xmpp_buddy_destroy(XmppBuddy *buddy);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_XMPP_BUDDY_H */
