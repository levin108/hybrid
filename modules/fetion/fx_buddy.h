#ifndef HYBRID_FX_BUDDY_H
#define HYBRID_FX_BUDDY_H
#include <glib.h>
#include "fx_account.h"
#include "fx_trans.h"

typedef struct _fetion_buddy fetion_buddy;

struct _fetion_buddy {
	gchar *userid;
	gchar *sid;
	gchar *sipuri;
	gchar *mobileno;
	gchar *localname;
	gchar *nickname;
	gchar *mood_phrase;
	gchar *carrier;
	gint carrier_status;
	gint gender;
	gchar *portrait_crc;
	gchar *groups; /**< it's the group IDs,in form of 3,5,7 */
	gchar *country;
	gchar *province;
	gchar *city;
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
 * @param ac The fetion account context.
 *
 * @return HYBRID_OK if success, HYBRID_ERROR if there was an error.
 */
gint fetion_buddy_scribe(fetion_account *ac);

/**
 * Get the detail information of a buddy.
 *
 * @param ac       The fetion account context.
 * @param userid   The userid of the buddy.
 * @param callback The callback function to handle the get-info response.
 * @param data     The user-specified data for the callback function.
 */
gint fetion_buddy_get_info(fetion_account *ac, const gchar *userid,
		TransCallback callback, gpointer data);

/**
 * Parse the detailed information by the get-info sip response.
 *
 * @param ac     The fetion account context.
 * @param userid User ID of the buddy.
 * @param sipmsg The get-info sip response string.
 *
 * @return The fetion buddy context with detailed information.
 */
fetion_buddy *fetion_buddy_parse_info(fetion_account *ac, 
		const gchar *userid, const gchar *sipmsg);

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

#endif /* HYBRID_FX_BUDDY_H */
