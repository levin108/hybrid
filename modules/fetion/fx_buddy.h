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
	gint   carrier_status;
	gint   gender;
	gchar *portrait_crc;
	gchar *groups;              /**< it's the group IDs,in form of 3,5,7 */
	gchar *country;
	gchar *province;
	gchar *city;
	gint   status;
	gint   state;
};

typedef struct _portrait_trans portrait_trans;
typedef struct _portrait_data portrait_data;

struct _portrait_trans {
	gchar          *data;       /**< portrait image data. */
	gint            data_size;  /**< length of the portrait buffer data. */
	gint            data_len;   /**< length of the portrait image data. */
	gint            portrait_type;
	fetion_buddy   *buddy;
	fetion_account *ac;
};

/**
 * Structure to deliver to the portrait callback function.
 */
struct _portrait_data {
	fetion_buddy   *buddy;
	gint            portrait_type;
	fetion_account *ac;
};

enum {
	PORTRAIT_TYPE_BUDDY,        /**< flag to fetch buddy's portrait. */
	PORTRAIT_TYPE_ACCOUNT       /**< flag to fetch account's portrait. */
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
 * F: 999999999
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
 * Move the buddy to a new group. The message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 999999999
 * I: 4
 * Q: 2 S
 * N: SetContactInfoV4
 * L: 82
 *
 * <args><contacts><contact user-id="777777777" buddy-lists="0"/>
 * </contacts></args>
 *
 * @param ac      The fetion account context.
 * @param userid  The userid of the buddy.
 * @param groupid The group id of the new group.
 *
 * @return HYBRID_OK if success, HYBRID_ERROR if there was an error.
 */
gint fetion_buddy_move_to(fetion_account *ac, const gchar *userid,
		const gchar *groupid);

/**
 * Get the detail information of a buddy. The message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 999999999
 * I: 4
 * Q: 2 S
 * N: GetContactInfoV4
 * L: 45
 *
 * <args><contact user-id="777777777"/></args>
 *
 * @param ac       The fetion account context.
 * @param userid   The userid of the buddy.
 * @param callback The callback function to handle the get-info response.
 * @param data     The user-specified data for the callback function.
 */
gint fetion_buddy_get_info(fetion_account *ac, const gchar *userid,
		TransCallback callback, gpointer data);

/**
 * Remove a buddy. The message if:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 999999999
 * I: 7
 * Q: 2 S
 * N: DeleteBuddyV4
 * L: 83
 *
 * <args><contacts><buddies><buddy user-id="777777777"/>
 * </buddies></contacts></args>
 *
 * @param ac     The account context.
 * @param userid The userid of the buddy.
 *
 * @return HYBRID_OK if success, HYBRID_ERROR if there was an error.
 */
gint fetion_buddy_remove(fetion_account *ac, const gchar *userid);

/**
 * Rename a buddy. The message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 547264589
 * I: 4
 * Q: 2 S
 * N: GetContactInfoV4
 * L: 45
 *
 * <args><contact user-id="547123726"/></args>
 *
 * @param ac The account context.
 * @param userid The userid of the buddy.
 * @param newname The new name of the buddy.
 *
 * @return HYBRID_OK if success, HYBRID_ERROR if there was an error.
 */
gint fetion_buddy_rename(fetion_account *ac, const gchar *userid,
		const gchar *newname);

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
 * Find the fetion buddy with the specified sid in the buddy list.
 *
 * @param ac  The fetion account.
 * @param sid The buddy's fetion no.
 *
 * @return The fetion buddy if found. NULL if not found.
 */
fetion_buddy *fetion_buddy_find_by_sid(fetion_account *ac, const gchar *userid);

/**
 * Destroy a fetion buddy.
 *
 * @param buddy The buddy to destroy.
 */
void fetion_buddy_destroy(fetion_buddy *buddy);

/**
 * Add a fetion buddy, the message is:
 *
 * S fetion.com.cn SIP-C/4.0
 * F: 999999999
 * I: 4
 * Q: 2 S
 * N: AddBuddyV4
 * L: 211
 *
 * <args><contacts><buddies><buddy uri="sip:777777777" local-name="test"
 * buddy-lists="4" desc="naruto" expose-mobile-no="1" expose-name="1"
 * addbuddy-phrase-id="0"/></buddies></contacts></args>
 *
 * @param account The fetion account.
 * @param groupid The id of the group to which the new account will be added.
 * @parma no      The fetion number or the mobile number of the new buddy.
 * @param alias   The local name of the new buddy.
 *
 * @return HYBRID_OK or HYBRID_ERROR in case of an error.
 */
 gint fetion_buddy_add(fetion_account *account, const gchar *groupid,
					const gchar *no, const gchar *alias);

/* UI ops */

/**
 * Init the buddies list in the UI TreeView
 *
 * @param ac The fetion account context.
 */
void fetion_buddies_init(fetion_account *ac);

/**
 * Handle the add-buddy request.
 *
 * @param ac    The fetion account.
 */
void fetion_buddy_handle_request(fetion_account *ac, const gchar *sipuri,
		const gchar *userid, const gchar *alias, const gchar *groupid,
		gboolean accept);

/**
 * Callback function to handle the portriat connection event. This callback
 * function may be used by the fx_acccount.c, so we define it as global.
 * It start a http GET request, the full string is like:
 *
 * GET /HDS_S00/getportrait.aspx?Uri=sip%3A642781687%40fetion.com.cn
 * %3Bp%3D6543&Size=120&c=CBIOAACvnz7yxTih9INomOTTPvbcRQgbv%2BWSdvSlGM5711%2BZL9
 * GUQL4ohBdnDsU1uq0IU9piw2w8wRHIztMX6JmkQzB%2B0nEcndn0kYSKtSDg3JDaj3s%2BZePi9SU
 * HO9BIHaZ5vOsAAA%3D%3D HTTP/1.1
 * User-Agent: IIC2.0/PC 4.0.2510
 * Accept: image/pjpeg;image/jpeg;image/bmp;image/x-windows-bmp;image/png;image/gif
 * Host: hdss1fta.fetion.com.cn
 * Connection: Keep-Alive
 */
gboolean portrait_conn_cb(gint sk, gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* HYBRID_FX_BUDDY_H */
