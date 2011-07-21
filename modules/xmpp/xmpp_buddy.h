#ifndef HYBRID_XMPP_BUDDY_H
#define HYBRID_XMPP_BUDDY_H

#include "xmpp_stream.h"
#include "xmpp_iq.h"

typedef struct _XmppBuddy XmppBuddy;

struct _XmppBuddy {
	gchar *jid;
	gchar *name;
	gchar *status;
	gchar *group;
	gchar *photo;

	gchar *subscription;
	gchar *resource;

	XmppStream *stream;
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
 * Set resource for the buddy.
 *
 * @param buddy    The xmpp buddy.
 * @param resource The resource string.
 */
void xmpp_buddy_set_resource(XmppBuddy *buddy, const gchar *resource);

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
 * Unsubscribe a buddy's presence message.
 *
 * @param buddy The xmpp buddy to unscribe.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_unsubscribe(XmppBuddy *buddy);

/**
 * Remove a buddy from the roster.
 *
 * @param buddy The buddy to remove.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_delete(XmppBuddy *buddy);

/**
 * Add a xmpp buddy.
 *
 * @param jid   Bare Jabber ID of the buddy to add.
 * @param name  Alias name set to this buddy.
 * @param group Name of group to add this buddy to.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_add(XmppStream *stream, const gchar *jid, const gchar *name,
		const gchar *group);

/**
 * Clear the buddies from the local buddy list.
 */
void xmpp_buddy_clear(void);
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
