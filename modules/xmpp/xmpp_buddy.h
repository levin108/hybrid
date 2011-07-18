#ifndef HYBRID_XMPP_BUDDY_H
#define HYBRID_XMPP_BUDDY_H

#include "xmpp_stream.h"

typedef struct _XmppBuddy XmppBuddy;

struct _XmppBuddy {
	gchar *jid;
	gchar *name;
	gchar *status;
	gchar *group;

	HybridBuddy *buddy;
};

#ifdef __cplusplus
extern "C" {
#endif

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
 * @param jid Jabber id of the buddy.
 *
 * @return The buddy created.
 */
XmppBuddy* xmpp_buddy_create(HybridBuddy *buddy);

/**
 * Set name for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param name  The name of the buddy.
 */
void xmpp_buddy_set_name(XmppBuddy *buddy, const gchar *name);

/**
 * Find a buddy with the given bare jabber id.
 *
 * @param jid The bare jid.
 *
 * @return NULL if not found.
 */

XmppBuddy *xmpp_buddy_find(const gchar *jid);

/**
 * Set status for the buddy.
 *
 * @param buddy  The xmpp buddy.
 * @param status The status of the buddy.
 */
void xmpp_buddy_set_status(XmppBuddy *buddy, const gchar *status);

/**
 * Set online status for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param show  The online status.
 */
void xmpp_buddy_set_show(XmppBuddy *buddy, const gchar *show);

/**
 * Set group name for the buddy.
 *
 * @param buddy The xmpp buddy.
 * @param group The group name of the buddy.
 */
void xmpp_buddy_set_group(XmppBuddy *buddy, const gchar *group);

/**
 * Get vcard information of the given buddy.
 *
 * @param stream The xmpp stream.
 * @param buddy  The xmpp buddy.
 */
void xmpp_buddy_get_info(XmppStream *stream, XmppBuddy *buddy);

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
