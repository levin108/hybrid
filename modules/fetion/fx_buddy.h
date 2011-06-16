#ifndef HYBIRD_FX_BUDDY_H
#define HYBIRD_FX_BUDDY_H
#include <glib.h>
#include "fx_account.h"

typedef struct _fetion_buddy fetion_buddy;

struct _fetion_buddy {
	gchar *userid;
	gchar *sid;
	gchar *sipuri;
	gchar *mobileno;
	gchar *nickname;
	gchar *mood_phrase;
	gchar *carrier;
	gchar carrier_status;
	gchar *localname;
	gchar *portrait_crc;
	gchar *groups; /**< it's the group IDs,in form of 3,5,7 */
	gint state;
};

typedef struct _portrait_trans portrait_trans;
typedef struct _portrait_data portrait_data;

struct _portrait_trans {
	gchar *data; /**< portrait image data. */
	gint data_size; /**< length of the portrait buffer data. */
	gint data_len; /**< length of the portrait image data. */
	fetion_buddy *buddy;
	fetion_account *ac;
};

/**
 * Structure to deliver to the portrait callback function.
 */
struct _portrait_data {
	fetion_buddy *buddy;
	fetion_account *ac;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a fetion buddy. It just allocate memory.
 *
 * @return The fetion buddy created.
 */
fetion_buddy *fetion_buddy_create(void);

/**
 * Scribe the msg pushed from the server. The message is:
 *
 * SUB fetion.com.cn SIP-C/4.0
 * F: 916098834
 * I: 3
 * Q: 2 SUB
 * N: PresenceV4
 * L: 88
 *
 * <args><subscription self="v4default;mail-count"
 * buddy="v4default" version="0"/></args>
 *
 * @param sk The socket file descriptor to write sip msg to.
 * @param ac The fetion account context.
 *
 * @return HYBIRD_OK if success, HYBIRD_ERROR if there was an error.
 */
gint fetion_buddy_scribe(gint sk, fetion_account *ac);

/**
 * Update the portrait of the specified buddy.
 *
 * @param ac The fetion account context.
 * @param buddy The buddy whose portrait is to be updated.
 */
void fetion_update_portrait(fetion_account *ac, fetion_buddy *buddy);

/**
 * Find the fetion buddy with the specified userid in the buddy list.
 *
 * @param ac The fetion account.
 * @param userid The user ID.
 *
 * @return The fetion buddy if found. NULL if not found.
 */
fetion_buddy *fetion_buddy_find_by_userid(fetion_account *ac,
		const gchar *userid);
/**
 * Destroy a fetion buddy.
 *
 * @param buddy The buddy to destroy.
 */
void fetion_buddy_destroy(fetion_buddy *buddy);


/* UI ops */

/**
 * Init the buddies list in the UI TreeView
 *
 * @param ac The fetion account context.
 */
void fetion_buddies_init(fetion_account *ac);

#ifdef __cplusplus
}
#endif

#endif /* HYBIRD_FX_BUDDY_H */
