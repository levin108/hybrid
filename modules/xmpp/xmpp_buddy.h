#ifndef HYBRID_XMPP_BUDDY_H
#define HYBRID_XMPP_BUDDY_H

#include "xmpp_stream.h"
#include "xmpp_iq.h"

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
 * @param jid The bare jid.
 *
 * @return NULL if not found.
 */

XmppBuddy *xmpp_buddy_find(const gchar *jid);

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
 * Unsubscribe a buddy's presence message.
 *
 * @param buddy The xmpp buddy to unsubscribe.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_unsubscribe(XmppBuddy *buddy);

/**
 * Subscribe a buddy's presence message.
 *
 * @param buddy The xmpp buddy to subscribe.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
gint xmpp_buddy_subscribe(XmppBuddy *buddy);

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
